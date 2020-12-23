#pragma once
#include "ManagerAPI/ManagerAPIClient.hpp"
#include "Server/RequestHandlerFactory.hpp"
#include "Store/DatasetStore.hpp"
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionCallback.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/ServerApplication.h>

namespace voxer::remote {

class VoxerWorkerApplication : public Poco::Util::ServerApplication {
protected:
  void initialize(Application &self) override;

  void defineOptions(Poco::Util::OptionSet &options) override;

  int main(const std::vector<std::string> &args) override;

  void hanldle_option(const std::string &name, const std::string &value);

private:
  bool m_show_help = false;
  std::unique_ptr<ManagerAPIClient> m_manager;
  std::string m_storage = ".";
  uint32_t m_port = 3040;
  std::unique_ptr<DatasetStore> m_datasets;

  auto resgiter_services() -> Poco::SharedPtr<RequestHandlerFactory>;

  void register_rpc_methods();
};

} // namespace voxer::remote