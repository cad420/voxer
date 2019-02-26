#include "ParallelRenderer/ConfigManager.h"
#include "ParallelRenderer/DatasetManager.h"
#include "ParallelRenderer/Encoder.h"
#include "ParallelRenderer/Renderer.h"
#include "ParallelRenderer/UserManager.h"
#include "ParallelRenderer/http/RequestHandler.h"
#include "Poco/Format.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/URI.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"
#include "third_party/rapidjson/document.h"
#include <iostream>
#include <map>
#include <string>

using namespace std;
using ospcommon::vec2i;
using Poco::format;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

DatasetManager datasets;
ConfigManager configs;
UserManager users;
Renderer renderer;
Encoder encoder;

class RequestHandlerFactory : public HTTPRequestHandlerFactory {
public:
  HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) {
    Application &app = Application::instance();
    app.logger().information("Request from " +
                             request.clientAddress().toString() + ": " +
                             request.getMethod() + " " + request.getURI() +
                             " " + request.getVersion());
    if (request.find("Upgrade") != request.end() &&
        Poco::icompare(request["Upgrade"], "websocket") == 0) {
      return new WebSocketRequestHandler();
    } else {
      Poco::URI uri(request.getURI().c_str());
      vector<string> segments;
      uri.getPathSegments(segments);

      if (segments.size() == 0) {
        return new ImageRequestHandler(uri);
      } else {
        return new JSONRequestHandler(uri);
      }
    }
  }
};

class VoVis : public ServerApplication {
public:
  VoVis() : _helpRequested(false) {}
  ~VoVis() {}
  string datasetFile;
  string configureFile;

protected:
  void initialize(Application &self) {
    loadConfiguration();
    int argc = self.argv().size();
    vector<const char *> argvec;
    argvec.reserve(argc);
    for (auto &arg : self.argv()) {
      if (arg == "--mpi") {
        argvec.push_back("--osp:mpi");
      } else if (arg == "--loglevel") {
        argvec.push_back("--osp:loglevel");
      } else if (arg == "--nthreads") {
        argvec.push_back("--osp:numthreads");
      } else {
        argvec.push_back(const_cast<char *>(arg.c_str()));
      }
    }
    const char **argv = &argvec[0];
    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR) {
      logger().error("OSP Error");
      exit(1);
    }
    ServerApplication::initialize(self);
  }

  void defineOptions(OptionSet &options) {
    ServerApplication::defineOptions(options);
    options.addOption(Option("mpi", "", "OSPRay MPI Offload Rendering Mode.")
                          .required(false)
                          .repeatable(false));
    options.addOption(Option("loglevel", "", "OSPRay loglevel.")
                          .required(false)
                          .argument("<the value>", true)
                          .repeatable(false));
    options.addOption(Option("nthreads", "", "OSPRay numthreads.")
                          .required(false)
                          .argument("<the value>", true)
                          .repeatable(false));
    options.addOption(Option("datasets", "", "volume datasets configure file.")
                          .required(true)
                          .argument("<the value>", true)
                          .repeatable(false));
    options.addOption(
        Option("configures", "", "visuailization configures save file.")
            .required(true)
            .argument("<the value>", true)
            .repeatable(false));
    options.addOption(
        Option("help", "h",
               "display help information on command line arguments")
            .required(false)
            .repeatable(false));
  }

  void handleOption(const string &name, const string &value) {
    ServerApplication::handleOption(name, value);
    if (name == "datasets") {
      datasetFile = value;
    } else if (name == "configures") {
      configureFile = value;
    } else if (name == "help")
      _helpRequested = true;
  }

  void displayHelp() {
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("Server Application of Volume Visualization.");
    helpFormatter.format(cout);
  }

  int main(const vector<string> &args) {
    if (_helpRequested) {
      displayHelp();
    } else {
      try {
        datasets.load(datasetFile);
        configs.load(configureFile);
        auto& user = users.get("tester");
        for (auto &dataset : datasets.datasets) {
          user.load(dataset.first);
        }
      } catch (string &exc) {
        logger().error(exc);
        exit(1);
      }
      logger().information(to_string(datasets.size()) + " dataset loaded.");
      auto port = (unsigned short)config().getInt("WebSocketServer.port", 3000);
      ServerSocket svs(port);
      HTTPServer server(new RequestHandlerFactory, svs, new HTTPServerParams);
      server.start();
      logger().information("Server starts at port: " + to_string(port));
      waitForTerminationRequest();
      server.stop();
    }
    return Application::EXIT_OK;
  }

private:
  bool _helpRequested;
};

POCO_SERVER_MAIN(VoVis)