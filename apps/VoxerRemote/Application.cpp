#include "Application.hpp"
#include "Server.hpp"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/IntValidator.h>
#include <Poco/NumberParser.h>
#include <iostream>

namespace voxer::remote {

void HTTPServerApplication::initialize(Application &self) {}

void HTTPServerApplication::defineOptions(Poco::Util::OptionSet &options) {
  ServerApplication::defineOptions(options);

  using Option = Poco::Util::Option;
  using OptionCallback = Poco::Util::OptionCallback<HTTPServerApplication>;
  options.addOption(Option("help", "h", "display argument help information")
                        .required(false)
                        .repeatable(false)
                        .callback(OptionCallback(
                            this, &HTTPServerApplication::hanldle_option)));

  options.addOption(Option("port", "p", "port listening")
                        .required(false)
                        .repeatable(false)
                        .validator(new Poco::Util::IntValidator(0, 65536))
                        .callback(OptionCallback(
                            this, &HTTPServerApplication::hanldle_option)));
}

void HTTPServerApplication::hanldle_option(const std::string &name,
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

int HTTPServerApplication::main(const std::vector<std::string> &args) {
  if (m_show_help) {
    return Application::EXIT_OK;
  }

  using ServerSocket = Poco::Net::ServerSocket;
  using HTTPServer = Poco::Net::HTTPServer;
  using HTTPServerParams = Poco::Net::HTTPServerParams;

  ServerSocket svs(m_port);
  HTTPServer srv(Poco::makeShared<MyHTTPRequestHandlerFactory>(), svs,
                 Poco::makeAuto<HTTPServerParams>());
  srv.start();
  waitForTerminationRequest();
  srv.stop();
  return Application::EXIT_OK;
}

} // namespace voxer::remote