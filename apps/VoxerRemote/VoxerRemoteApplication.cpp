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
#include <Poco/URI.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/IntValidator.h>
#include <exception>
#include <iostream>

namespace voxer::remote {

void VoxerRemoteApplication::initialize(Application &self) {}

void VoxerRemoteApplication::defineOptions(Poco::Util::OptionSet &options) {
  ServerApplication::defineOptions(options);

  using Option = Poco::Util::Option;
  using OptionCallback = Poco::Util::OptionCallback<VoxerRemoteApplication>;
  options.addOption(Option("help", "h", "display argument help information")
                        .required(false)
                        .repeatable(false)
                        .callback(OptionCallback(
                            this, &VoxerRemoteApplication::hanldle_option)));

  options.addOption(Option("port", "p", "port listening")
                        .required(false)
                        .argument("port")
                        .repeatable(false)
                        .validator(new Poco::Util::IntValidator(0, 65536))
                        .callback(OptionCallback(
                            this, &VoxerRemoteApplication::hanldle_option)));

  options.addOption(Option("manager", "m", "manager address")
                        .required(true)
                        .argument("manager")
                        .repeatable(false)
                        .callback(OptionCallback(
                            this, &VoxerRemoteApplication::hanldle_option)));
}

void VoxerRemoteApplication::hanldle_option(const std::string &name,
                                            const std::string &value) {
  if (name == "manager") {
    try {
      Poco::URI uri{value};
      m_manager_address = value;
    } catch (std::exception &exp) {
      std::cerr << "invalid manager address" << std::endl;
      stopOptionsProcessing();
    }
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

int VoxerRemoteApplication::main(const std::vector<std::string> &args) {
  if (m_show_help) {
    return Application::EXIT_OK;
  }

  if (m_manager_address.empty()) {
    return Application::EXIT_DATAERR;
  }

  using ServerSocket = Poco::Net::ServerSocket;
  using HTTPServer = Poco::Net::HTTPServer;
  using HTTPServerParams = Poco::Net::HTTPServerParams;

  auto routes = Poco::makeShared<MyHTTPRequestHandlerFactory>();
  DatasetStore datasets{m_manager_address};

  routes->register_service("/datasets", [&datasets]() {
    auto service = new DatasetService();
    service->m_datasets = &datasets;
    return service;
  });
  routes->register_service("/render", [&datasets]() {
    auto service = new VolumeRenderingService();
    service->m_datasets = &datasets;
    return service;
  });
  routes->register_service("/slice", [&datasets]() {
    auto service = new SliceService();
    service->m_datasets = &datasets;
    return service;
  });
  //  routes->register_service("/annotation", [&datasets]() {
  //    auto service = new AnnotationService();
  //    service->m_datasets = &datasets;
  //    return service;
  //  });

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