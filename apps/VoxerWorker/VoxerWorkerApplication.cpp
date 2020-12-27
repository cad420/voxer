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
                        .validator(new Poco::Util::IntValidator(0, 65536))
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
}

void VoxerWorkerApplication::hanldle_option(const std::string &name,
                                            const std::string &value) {
  if (name == "port") {
    m_port = Poco::NumberParser::parse(value);
    return;
  }

  if (name == "manager") {
    try {
      m_manager = std::make_unique<ManagerAPIClient>(value);
    } catch (std::exception &exp) {
      spdlog::critical("invalid manager address");
      stopOptionsProcessing();
    }
    return;
  }

  if (name == "storage") {
    m_storage = value;
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

  m_datasets = std::make_unique<DatasetStore>(m_storage);

  if (m_manager) {
    m_datasets->set_manager(m_manager.get());
    m_manager->set_datasets(m_datasets.get());
  }

  Poco::Net::ServerSocket svs(m_port);
  Poco::Net::HTTPServer srv(
      Poco::makeShared<RequestHandlerFactory>(m_datasets.get()), svs,
      Poco::makeAuto<Poco::Net::HTTPServerParams>());
  srv.start();

  spdlog::info("server starts at port: {}", m_port);

  waitForTerminationRequest();
  srv.stop();
  return Application::EXIT_OK;
}

} // namespace voxer::remote