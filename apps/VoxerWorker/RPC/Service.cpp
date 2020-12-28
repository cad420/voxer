#include "RPC/Service.hpp"
#include "DataModel/Annotation.hpp"
#include "DataModel/Image.hpp"
#include <seria/deserialize/mpack.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <voxer/Filters/AnnotationGrabCutFilter.hpp>
#include <voxer/Filters/AnnotationLevelSetFilter.hpp>
#include <voxer/Rendering/VolumeRenderer.hpp>

using namespace std;
using namespace voxer;

namespace voxer::remote {

Service::Service(DatasetStore *datasets)
    : m_datasets(datasets), m_methods(std::make_unique<RPCMethodsStore>()) {
  const char *env = getenv("LOG_LEVEL");
  m_log_time = env != nullptr && strlen(env) > 0;

  m_methods->register_method(
      "get_dataset_info",
      RPCMethodsStore::GetHandler(&Service::get_dataset_info, *this));
  m_methods->register_method(
      "render", RPCMethodsStore::GetHandler(&Service::render, *this));
  m_methods->register_method(
      "get_dataset_slice",
      RPCMethodsStore::GetHandler(&Service::get_dataset_slice, *this));
#ifdef ENABLE_ANNOTATION_SERVICE
  m_methods->register_method(
      "apply_levelset",
      RPCMethodsStore::GetHandler(&Service::apply_levelset, *this));
  m_methods->register_method(
      "apply_grabcut",
      RPCMethodsStore::GetHandler(&Service::apply_grabcut, *this));
#endif
}

void Service::on_message(const uint8_t *message, uint32_t size,
                         const MessageCallback &callback) noexcept {
  assert(m_datasets != nullptr && callback != nullptr);
  if (m_datasets == nullptr || callback == nullptr) {
    return;
  }

  spdlog::stopwatch sw{};

  uint8_t *response;
  uint32_t total;
  mpack_writer_t response_writer;

  // extended JSON-RPC, see https://www.jsonrpc.org/specification
  try {
    mpack_tree_t tree;
    mpack_tree_init_data(&tree, reinterpret_cast<const char *>(message), size);
    mpack_tree_parse(&tree);

    if (mpack_tree_error(&tree) != mpack_ok) {
      throw JSONRPCParseError();
    }

    mpack_node_t root = mpack_tree_root(&tree);

    if (root.data->type != mpack_type_map ||
        !mpack_node_map_contains_cstr(root, "caller") ||
        !mpack_node_map_contains_cstr(root, "id") ||
        !mpack_node_map_contains_cstr(root, "method") ||
        !mpack_node_map_contains_cstr(root, "params")) {
      throw JSONRPCInvalidRequestError();
    }

    auto caller_node = mpack_node_map_cstr(root, "caller");
    if (caller_node.data->type != mpack_type_str) {
      throw JSONRPCInvalidRequestError();
    }

    auto id_node = mpack_node_map_cstr(root, "id");
    if (id_node.data->type != mpack_type_str) {
      throw JSONRPCInvalidRequestError();
    }

    auto method_node = mpack_node_map_cstr(root, "method");
    if (method_node.data->type != mpack_type_str) {
      throw JSONRPCInvalidRequestError();
    }

    auto caller = std::string(mpack_node_str(caller_node),
                              mpack_node_strlen(caller_node));
    auto id = std::string(mpack_node_str(id_node), mpack_node_strlen(id_node));
    auto method = std::string(mpack_node_str(method_node),
                              mpack_node_strlen(method_node));
    auto param_node = mpack_node_map_cstr(root, "params");

    mpack_writer_init_growable(&response_writer,
                               reinterpret_cast<char **>(&response),
                               reinterpret_cast<size_t *>(&total));
    mpack_start_map(&response_writer, 4);
    mpack_write_cstr(&response_writer, "caller");
    mpack_write_cstr(&response_writer, caller.c_str());
    mpack_write_cstr(&response_writer, "id");
    mpack_write_cstr(&response_writer, id.c_str());
    mpack_write_cstr(&response_writer, "method");
    mpack_write_cstr(&response_writer, method.c_str());

    m_methods->invoke(method, param_node, &response_writer);

    if (m_log_time) {
      spdlog::info("{} elapsed {}", method, sw);
    }
  } catch (JSONRPCError &error) {
    on_error(error, &response_writer);
  } catch (std::exception &error) {
    on_error(JSONRPCServerError(error.what()), &response_writer);
  }

  mpack_finish_map(&response_writer);
  if (mpack_writer_destroy(&response_writer) == mpack_ok) {
    callback(reinterpret_cast<const uint8_t *>(response), total);
  }

  free(response);
}

void Service::on_error(const JSONRPCError &error, mpack_writer_t *writer) {
  mpack_write_cstr(writer, "error");
  // encode error object
  mpack_start_map(writer, 2);
  mpack_write_cstr(writer, "code");
  mpack_write(writer, 500);
  mpack_write_cstr(writer, "message");
  mpack_write(writer, error.what());
  mpack_finish_map(writer);
}

Image Service::render(const Scene &scene) {
  assert(m_datasets != nullptr);

  auto renderer = scene.renderer;
  std::transform(renderer.begin(), renderer.end(), renderer.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  if (m_renderer == nullptr || m_renderer->get_backend() != renderer) {
    m_renderer = make_unique<VolumeRenderer>(renderer.c_str());
  }

  m_renderer->set_camera(scene.camera);
  m_renderer->set_background(scene.background[0], scene.background[1],
                             scene.background[2]);

  unordered_map<uint32_t, shared_ptr<voxer::TransferFunction>> tfcns_map;

  for (auto &volume_desc : scene.volumes) {
    if (!volume_desc.render) {
      continue;
    }

    auto dataset = m_datasets->get(volume_desc.dataset);

    auto volume = make_shared<voxer::Volume>();
    volume->dataset = std::move(dataset);
    volume->spacing = volume_desc.spacing;

    if (tfcns_map.find(volume_desc.tfcn_idx) == tfcns_map.end()) {
      auto tfcn = make_shared<voxer::TransferFunction>();
      auto &tfcn_desc = scene.tfcns[volume_desc.tfcn_idx];
      for (auto &item : tfcn_desc) {
        voxer::RGBColor color;
        color.from_hex(item.color.data());
        tfcn->add_point(item.x, item.y, color.data);
      }
      auto res = tfcns_map.emplace(volume_desc.tfcn_idx, move(tfcn));
      volume->tfcn = res.first->second;
    } else {
      volume->tfcn = tfcns_map[volume_desc.tfcn_idx];
    }

    m_renderer->add_volume(volume);
  }

  for (auto &isosurface_desc : scene.isosurfaces) {
    if (!isosurface_desc.render) {
      continue;
    }

    auto dataset = m_datasets->get(isosurface_desc.dataset);
    auto isosurface = make_shared<voxer::Isosurface>();
    isosurface->dataset = std::move(dataset);
    isosurface->color.from_hex(isosurface_desc.color.c_str());
    isosurface->value = isosurface_desc.value;
    m_renderer->add_isosurface(isosurface);
  }

  m_renderer->render();
  auto &image = m_renderer->get_colors();
  auto compressed =
      Image::encode(image, Image::Format::JPEG, Image::Quality::MEDIUM, true);

  m_renderer->clear_scene();

  return compressed;
}

Image Service::get_dataset_slice(const std::string &dataset_id,
                                 StructuredGrid::Axis axis, uint32_t index) {
  auto dataset = m_datasets_for_slice.has(dataset_id)
                     ? *m_datasets_for_slice.get(dataset_id)
                     : m_datasets->get(dataset_id);

  auto image = dataset->get_slice(axis, index);
  auto jpeg = Image::encode(image, Image::Format::JPEG, Image::Quality::HIGH);

  m_datasets_for_slice.emplace(dataset_id, std::move(dataset));

  return jpeg;
}

#ifdef ENABLE_ANNOTATION_SERVICE
std::vector<voxer::Annotation>
Service::apply_levelset(const vector<voxer::Annotation> &annotations,
                        const string &dataset_id, StructuredGrid::Axis axis,
                        uint32_t index) {
  auto dataset = m_datasets->get(dataset_id);
  if (!dataset) {
    throw std::runtime_error("cannot find dataset " + dataset_id);
  }
  auto slice = dataset->get_slice(axis, index + 1);

  std::vector<Annotation> result{};
  result.reserve(annotations.size());

  AnnotationLevelSetFilter filter{};
  for (auto &input : annotations) {
    result.emplace_back(filter.process(input, slice));
  }

  return result;
}

std::vector<voxer::Annotation>
Service::apply_grabcut(const vector<voxer::Annotation> &annotations,
                       const string &dataset_id, StructuredGrid::Axis axis,
                       uint32_t index) {
  auto dataset = m_datasets->get(dataset_id);
  if (!dataset) {
    throw std::runtime_error("cannot find dataset " + dataset_id);
  }
  auto slice = dataset->get_slice(axis, index + 1);
  AnnotationGrabCutFilter filter{};
  return filter.process(slice, annotations);
}

DatasetInfo Service::get_dataset_info(const string &id, const string &name,
                                      const string &path) {
  auto dataset = m_datasets->load(id, name, path);

  DatasetInfo result;
  result.id = id;
  result.histogram = dataset->get_histogram();
  result.dimensions = dataset->info.dimensions;
  result.range = dataset->original_range;

  return result;
}

#endif

} // namespace voxer::remote