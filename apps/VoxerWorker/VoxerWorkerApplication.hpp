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

  void hanldle_option(const std::string &name, const std::string &value);

  int main(const std::vector<std::string> &args) override;

private:
  bool m_show_help = false;
  std::string m_manager_address;
  std::string m_storage = ".";
  uint32_t m_port = 0;
};

} // namespace voxer::remote