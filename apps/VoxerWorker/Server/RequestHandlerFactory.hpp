#pragma once
#include "Service/AbstractService.hpp"
#include "Store/DatasetStore.hpp"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <unordered_map>

namespace voxer::remote {

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
  Poco::Net::HTTPRequestHandler *
  createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

  template <class Service>
  void register_service(const char *path, DatasetStore *datasets) noexcept {
    services.emplace(path, [datasets]() {
      auto service = new Service();
      service->m_datasets = datasets;
      return service;
    });
  }

private:
  std::unordered_map<std::string, std::function<AbstractService *()>> services;
};

} // namespace voxer::remote