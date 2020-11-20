#include "VoxerWorkerApplication.hpp"
#include "Server.hpp"
#ifdef ENABLE_ANNOTATION_SERVICE
#include "Service/AnnotationService.hpp"
#endif
#include "DataModel/DatasetInfo.hpp"
#include "Service/JSONRPCService.hpp"
#include "Service/SliceService.hpp"
#include "Service/VolumeRenderingService.hpp"
#include "Store/DatasetStore.hpp"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/NumberParser.h>
#include <Poco/URI.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/IntValidator.h>
#include <exception>
#include <iostream>
#include <voxer/Filters/AnnotationGrabCutFilter.hpp>
#include <voxer/Filters/AnnotationLevelset.hpp>
#include <voxer/Filters/histogram.hpp>

namespace voxer::remote {

void VoxerWorkerApplication::initialize(Application &self) {}

void VoxerWorkerApplication::defineOptions(Poco::Util::OptionSet &options) {
  ServerApplication::defineOptions(options);

  using Option = Poco::Util::Option;
  using OptionCallback = Poco::Util::OptionCallback<VoxerWorkerApplication>;
  options.addOption(Option("help", "h", "display argument help information")
                        .required(false)
                        .repeatable(false)
                        .callback(OptionCallback(
                            this, &VoxerWorkerApplication::hanldle_option)));

  options.addOption(Option("port", "p", "port listening")
                        .required(false)
                        .argument("port")
                        .repeatable(false)
                        .validator(new Poco::Util::IntValidator(0, 65536))
                        .callback(OptionCallback(
                            this, &VoxerWorkerApplication::hanldle_option)));

  options.addOption(Option("manager", "m", "manager address")
                        .required(true)
                        .argument("manager")
                        .repeatable(false)
                        .callback(OptionCallback(
                            this, &VoxerWorkerApplication::hanldle_option)));

  options.addOption(Option("storage", "s", "storage path")
                        .required(false)
                        .argument("storage")
                        .repeatable(false)
                        .callback(OptionCallback(
                            this, &VoxerWorkerApplication::hanldle_option)));
}

void VoxerWorkerApplication::hanldle_option(const std::string &name,
                                            const std::string &value) {
  if (name == "manager") {
    try {
      Poco::URI uri{value};
      m_manager = value;
    } catch (std::exception &exp) {
      std::cerr << "invalid manager address" << std::endl;
      stopOptionsProcessing();
    }
  }

  if (name == "storage") {
    m_storage = value;
  }

  if (name == "help") {
    using HelpFormatter = Poco::Util::HelpFormatter;
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("scientific visualization cloud service");
    helpFormatter.format(std::cout);
    stopOptionsProcessing();
    m_show_help = true;
    return;
  }

  if (name == "port") {
    m_port = Poco::NumberParser::parse(value);
    return;
  }
}

int VoxerWorkerApplication::main(const std::vector<std::string> &args) {
  if (m_show_help) {
    return Application::EXIT_OK;
  }

  if (m_manager.empty()) {
    return Application::EXIT_DATAERR;
  }

  using ServerSocket = Poco::Net::ServerSocket;
  using HTTPServer = Poco::Net::HTTPServer;
  using HTTPServerParams = Poco::Net::HTTPServerParams;

  m_datasets = std::make_unique<DatasetStore>(m_manager, m_storage);
  register_rpc_methods();
  auto routes = resgiter_services();

  ServerSocket svs(m_port);
  HTTPServer srv(routes, svs, Poco::makeAuto<HTTPServerParams>());
  srv.start();
  this->logger().information("server starts at port: " +
                             std::to_string(m_port));
  waitForTerminationRequest();
  srv.stop();
  return Application::EXIT_OK;
}

void VoxerWorkerApplication::register_rpc_methods() {
  auto rpc_methods = RPCMethodsStore::get_instance();

  std::function<int(int, int)> add = [](int i, int j) -> int { return i + j; };
  rpc_methods->resgister_method("add", RPCMethodsStore::GetHandler(add));

  std::function<std::vector<voxer::Annotation>(
      const std::vector<voxer::Annotation> &, const std::string &,
      StructuredGrid::Axis, uint32_t)>
      apply_levelset = [datasets = m_datasets.get()](
                           const std::vector<voxer::Annotation> &annotations,
                           const std::string &dataset_id,
                           StructuredGrid::Axis axis,
                           uint32_t index) -> std::vector<voxer::Annotation> {
    auto dataset = datasets->get(dataset_id);
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
  };
  rpc_methods->resgister_method("apply_levelset",
                                RPCMethodsStore::GetHandler(apply_levelset),
                                {"annotations", "dataset", "axis", "index"});

  std::function<std::vector<Annotation>(const std::vector<voxer::Annotation> &,
                                        const std::string &,
                                        StructuredGrid::Axis, uint32_t)>
      apply_grabcut = [datasets = m_datasets.get()](
                          const std::vector<voxer::Annotation> &annotations,
                          const std::string &dataset_id,
                          StructuredGrid::Axis axis, uint32_t index) {
        auto dataset = datasets->get(dataset_id);
        if (!dataset) {
          throw std::runtime_error("cannot find dataset " + dataset_id);
        }
        auto slice = dataset->get_slice(axis, index + 1);
        AnnotationGrabCutFilter filter{};
        return filter.process(slice, annotations);
      };
  rpc_methods->resgister_method("apply_grabcut",
                                RPCMethodsStore::GetHandler(apply_grabcut),
                                {"annotations", "dataset", "axis", "index"});

  std::function<DatasetInfo(const std::string &)> get_dataset_info =
      [datasets = m_datasets.get()](const std::string &id) {
        auto dataset = datasets->get(id);

        DatasetInfo result;
        result.id = id;
        result.histogram = voxer::calculate_histogram(*dataset);
        result.dimensions = dataset->info.dimensions;
        result.range = dataset->original_range;

        return result;
      };
  rpc_methods->resgister_method("get_dataset_info",
                                RPCMethodsStore::GetHandler(get_dataset_info));
}

auto VoxerWorkerApplication::resgiter_services()
    -> Poco::SharedPtr<MyHTTPRequestHandlerFactory> {
  auto routes = Poco::makeShared<MyHTTPRequestHandlerFactory>();

  auto datasets = m_datasets.get();
  routes->register_service<VolumeRenderingService>("/render", datasets);
  routes->register_service<SliceService>("/slice", datasets);
  routes->register_service<JSONRPCService>("/jsonrpc", datasets);
#ifdef ENABLE_ANNOTATION_SERVICE
  routes->register_service<AnnotationService>("/annotations", datasets);
#endif

  return routes;
}

} // namespace voxer::remote