#include "VoxerWorkerApplication.hpp"
#include "Store/DatasetStore.hpp"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/NumberParser.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/IntValidator.h>
#include <exception>
#include <iostream>

namespace voxer::remote {

void VoxerWorkerApplication::initialize(Application &self) {}

void VoxerWorkerApplication::defineOptions(Poco::Util::OptionSet &options) {
  using Option = Poco::Util::Option;
  using OptionCallback = Poco::Util::OptionCallback<VoxerWorkerApplication>;

  ServerApplication::defineOptions(options);

  options.addOption(Option("help", "h", "display argument help information")
                        .required(false)
                        .repeatable(false)
                        .callback(OptionCallback(
                            this, &VoxerWorkerApplication::hanldle_option)));

  options.addOption(Option("port", "p", "port listening")
                        .required(false)
                        .argument("port")
                        .repeatable(false)
                        .validator(new Poco::Util::IntValidator(1, 65536))
                        .callback(OptionCallback(
                            this, &VoxerWorkerApplication::hanldle_option)));

  options.addOption(Option("manager", "m", "manager address")
                        .required(false)
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

  options.addOption(Option("debug", "d", "show debug log")
                        .required(false)
                        .repeatable(false)
                        .callback(OptionCallback(
                            this, &VoxerWorkerApplication::hanldle_option)));
}

void VoxerWorkerApplication::hanldle_option(const std::string &name,
                                            const std::string &value) {
  if (name == "port") {
    m_port = Poco::NumberParser::parse(value);
    return;
  }

  if (name == "manager") {
    m_manager_address = value;
    return;
  }

  if (name == "storage") {
    m_storage = value;
    return;
  }

  if (name == "debug") {
    spdlog::set_level(spdlog::level::debug);
    return;
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
}

int VoxerWorkerApplication::main(const std::vector<std::string> &args) {
  if (m_show_help) {
    return Application::EXIT_OK;
  }

  // create dataset store
  auto &store = DatasetStore::default_instance();
  store.set_storage(m_storage);

  // connect to manager if exist
  std::unique_ptr<ManagerAPIClient> manager = nullptr;
  if (!m_manager_address.empty()) {
    manager = std::make_unique<ManagerAPIClient>(m_manager_address);
    store.set_manager(manager.get());
    spdlog::info("RPC client created");
  }

  // start rpc server
  std::unique_ptr<Poco::Net::HTTPServer> server = nullptr;
  if (m_port > 0) {
    Poco::Net::ServerSocket svs(m_port);
    server = std::make_unique<Poco::Net::HTTPServer>(
        Poco::makeShared<RequestHandlerFactory>(), svs,
        Poco::makeAuto<Poco::Net::HTTPServerParams>());
    server->start();
    spdlog::info("RPC server starts at port: {}", m_port);
  }

  waitForTerminationRequest();

  if (server != nullptr) {
    server->stop();
  }

  if (manager != nullptr) {
    manager->shutdown();
  }

  return Application::EXIT_OK;
}

} // namespace voxer::remote