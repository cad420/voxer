#include "ManagerAPI/ManagerAPIClient.hpp"
#include "Service/VolumeRenderingService.hpp"
#include <fmt/core.h>
#include <mpack/mpack.h>
#include <thread>

namespace voxer::remote {

Scene parse_scene(uint8_t *message, uint32_t size, std::string &id) {
  mpack_tree_t tree;
  mpack_tree_init_data(&tree, reinterpret_cast<const char *>(message), size);
  mpack_tree_parse(&tree);
  mpack_node_t root = mpack_tree_root(&tree);

  Scene scene{};

  auto id_node = mpack_node_map_cstr(root, "id");
  auto id_length = mpack_node_strlen(id_node);
  id = std::string(mpack_node_str(id_node), id_length);
  auto params = mpack_node_map_cstr(root, "params");
  auto renderer_node = mpack_node_map_cstr(params, "renderer");
  auto renderer_len = mpack_node_strlen(renderer_node);
  scene.renderer = std::string(mpack_node_str(renderer_node), renderer_len);

  auto tfcns = mpack_node_map_cstr(params, "tfcns");
  auto total = mpack_node_array_length(tfcns);
  for (int i = 0; i < total; i++) {
    TransferFunction tfcn{};
    auto tfcn_node = mpack_node_array_at(tfcns, i);
    auto point_len = mpack_node_array_length(tfcn_node);
    for (int j = 0; j < point_len; j++) {
      ControlPoint point{};
      auto points_node = mpack_node_array_at(tfcn_node, j);
      point.x = mpack_node_float(mpack_node_map_cstr(points_node, "x"));
      point.y = mpack_node_float(mpack_node_map_cstr(points_node, "y"));
      auto color_node = mpack_node_map_cstr(points_node, "color");
      auto color_length = mpack_node_strlen(color_node);
      point.color = std::string(mpack_node_str(color_node), color_length);
      tfcn.emplace_back(std::move(point));
    }
    scene.tfcns.emplace_back(std::move(tfcn));
  }

  auto volumes = mpack_node_map_cstr(params, "volumes");
  total = mpack_node_array_length(volumes);
  for (int i = 0; i < total; i++) {
    Volume volume{};
    auto volume_node = mpack_node_array_at(volumes, i);
    auto dataset_node = mpack_node_map_cstr(volume_node, "dataset");
    auto dataset_length = mpack_node_strlen(dataset_node);
    volume.dataset = std::string(mpack_node_str(dataset_node), dataset_length);
    volume.tfcn_idx = mpack_node_int(mpack_node_map_cstr(volume_node, "tfcn"));
    volume.render = mpack_node_bool(mpack_node_map_cstr(volume_node, "render"));
    scene.volumes.emplace_back(std::move(volume));
  }

  auto isosurfaces = mpack_node_map_cstr(params, "isosurfaces");
  total = mpack_node_array_length(isosurfaces);
  for (int i = 0; i < total; i++) {
    Isosurface isosurface{};
    auto isosurface_node = mpack_node_array_at(isosurfaces, i);
    auto dataset_node = mpack_node_map_cstr(isosurface_node, "dataset");
    auto dataset_length = mpack_node_strlen(dataset_node);
    isosurface.dataset =
        std::string(mpack_node_str(dataset_node), dataset_length);
    isosurface.value =
        mpack_node_float(mpack_node_map_cstr(isosurface_node, "value"));
    auto color_node = mpack_node_map_cstr(isosurface_node, "color");
    auto color_length = mpack_node_strlen(color_node);
    isosurface.color = std::string(mpack_node_str(color_node), color_length);
    isosurface.render =
        mpack_node_bool(mpack_node_map_cstr(isosurface_node, "render"));
    scene.isosurfaces.emplace_back(std::move(isosurface));
  }

  auto camera_node = mpack_node_map_cstr(params, "camera");
  auto camera_pos_node = mpack_node_map_cstr(camera_node, "pos");
  scene.camera.pos[0] =
      mpack_node_float(mpack_node_array_at(camera_pos_node, 0));
  scene.camera.pos[1] =
      mpack_node_float(mpack_node_array_at(camera_pos_node, 1));
  scene.camera.pos[2] =
      mpack_node_float(mpack_node_array_at(camera_pos_node, 2));

  auto camera_target_node = mpack_node_map_cstr(camera_node, "target");
  scene.camera.target[0] =
      mpack_node_float(mpack_node_array_at(camera_target_node, 0));
  scene.camera.target[1] =
      mpack_node_float(mpack_node_array_at(camera_target_node, 1));
  scene.camera.target[2] =
      mpack_node_float(mpack_node_array_at(camera_target_node, 2));

  auto camera_up_node = mpack_node_map_cstr(camera_node, "up");
  scene.camera.up[0] = mpack_node_float(mpack_node_array_at(camera_up_node, 0));
  scene.camera.up[1] = mpack_node_float(mpack_node_array_at(camera_up_node, 1));
  scene.camera.up[2] = mpack_node_float(mpack_node_array_at(camera_up_node, 2));

  scene.camera.zoom =
      mpack_node_float(mpack_node_map_cstr(camera_node, "zoom"));
  scene.camera.width =
      mpack_node_int(mpack_node_map_cstr(camera_node, "width"));
  scene.camera.height =
      mpack_node_int(mpack_node_map_cstr(camera_node, "height"));

  auto bg_node = mpack_node_map_cstr(params, "background");
  scene.background[0] = mpack_node_float(mpack_node_array_at(bg_node, 0));
  scene.background[1] = mpack_node_float(mpack_node_array_at(bg_node, 1));
  scene.background[2] = mpack_node_float(mpack_node_array_at(bg_node, 2));

  return scene;
}

ManagerAPIClient::ManagerAPIClient(std::string address)
    : m_address(std::move(address)), m_uri("http://" + m_address) {

  m_service = std::make_unique<VolumeRenderingService>();
  m_ws = std::make_unique<WebSocketClient>();
  m_ws->on_message([ws = m_ws.get(), service = m_service.get()](
                       uint8_t *message, uint32_t size, bool is_binary) {
    std::string id{};
    auto scene = parse_scene(message, size, id);
    service->render(
        scene, [ws, id](const uint8_t *msg, uint32_t size, bool is_binary) {
          if (!is_binary)
            return;

          char *data;
          size_t total;
          mpack_writer_t writer;
          mpack_writer_init_growable(&writer, &data, &total);

          mpack_start_map(&writer, 3);
          mpack_write_cstr(&writer, "id");
          mpack_write_cstr(&writer, id.c_str());
          mpack_write_cstr(&writer, "method");
          mpack_write_cstr(&writer, "render");
          mpack_write_cstr(&writer, "result");
          mpack_write_bin(&writer, reinterpret_cast<const char *>(msg), size);
          mpack_finish_map(&writer);

          // finish writing
          if (mpack_writer_destroy(&writer) != mpack_ok) {
            fprintf(stderr, "An error occurred encoding the data!\n");
            return;
          }

          ws->send(reinterpret_cast<const uint8_t *>(data), total, is_binary);
          free(data);
        });
  });

  m_thread = std::thread([ws = m_ws.get(), uri = m_uri]() {
    ws->connect(uri.getHost().c_str(), uri.getPort(), "/worker");
  });
}

DatasetLoadInfo ManagerAPIClient::get_dataset_info(const std::string &id) {
  return request<DatasetLoadInfo>("/datasets/" + id);
}

ManagerAPIClient::ManagerAPIClient() : ManagerAPIClient("127.0.0.1:3001") {}

ManagerAPIClient::~ManagerAPIClient() {
  m_ws->close();

  m_thread.join();
}

ManagerAPIClient::ManagerAPIClient(ManagerAPIClient &&another) noexcept {
  m_uri = another.m_uri;
  m_address = another.m_address;
  m_ws = std::move(another.m_ws);
  m_thread = std::move(another.m_thread);
}

ManagerAPIClient &
ManagerAPIClient::operator=(ManagerAPIClient &&another) noexcept {
  m_uri = another.m_uri;
  m_address = another.m_address;
  m_ws = std::move(another.m_ws);
  m_thread = std::move(another.m_thread);
  return *this;
}

} // namespace voxer::remote