#include "VoxerRemoteApplication.hpp"
#include "Server.hpp"
#include "Service/DatasetService.hpp"
#include "Service/SliceService.hpp"
#include "Service/VolumeRenderingService.hpp"
#include "Store/DatasetStore.hpp"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/NumberParser.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/IntValidator.h>
#include <iostream>

namespace voxer::remote {

void VoxerRemoteApplication::initialize(Application &self) {}

void VoxerRemoteApplication::defineOptions(Poco::Util::OptionSet &options) {
  ServerApplication::defineOptions(options);

  using Option = Poco::Util::Option;
  using OptionCallback = Poco::Util::OptionCallback<VoxerRemoteApplication>;
  options.addOption(
      Option("help", "h", "display argument help information")
          .required(false)
          .repeatable(false)
          .callback(OptionCallback(this, &VoxerRemoteApplication::hanldle_option)));

  options.addOption(
      Option("port", "p", "port listening")
          .required(false)
          .argument("port")
          .repeatable(false)
          .validator(new Poco::Util::IntValidator(0, 65536))
          .callback(OptionCallback(this, &VoxerRemoteApplication::hanldle_option)));
}

void VoxerRemoteApplication::hanldle_option(const std::string &name,
                                 const std::string &value) {
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

int VoxerRemoteApplication::main(const std::vector<std::string> &args) {
  if (m_show_help) {
    return Application::EXIT_OK;
  }

  using ServerSocket = Poco::Net::ServerSocket;
  using HTTPServer = Poco::Net::HTTPServer;
  using HTTPServerParams = Poco::Net::HTTPServerParams;

  auto routes = Poco::makeShared<MyHTTPRequestHandlerFactory>();
  DatasetStore datasets;

  auto dataset_service = std::make_unique<DatasetService>();
  dataset_service->m_datasets = &datasets;
  auto volume_rendering_service = std::make_unique<VolumeRenderingService>();
  volume_rendering_service->m_datasets = &datasets;
  auto slice_service = std::make_unique<SliceService>();
  slice_service->m_datasets = &datasets;
  //  AnnotationService annotation_service{};
  //  annotation_service.m_datasets = &datasets;

  routes->add_service(std::move(dataset_service));
  routes->add_service(std::move(volume_rendering_service));
  routes->add_service(std::move(slice_service));

  ServerSocket svs(m_port);
  HTTPServer srv(routes, svs, Poco::makeAuto<HTTPServerParams>());
  srv.start();
  this->logger().information("server starts at port: " +
                             std::to_string(m_port));
  waitForTerminationRequest();
  srv.stop();
  return Application::EXIT_OK;
}

} // namespace voxer::remote