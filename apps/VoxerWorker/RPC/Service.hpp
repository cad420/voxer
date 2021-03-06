#pragma once
#include "DataModel/Annotation.hpp"
#include "DataModel/Scene.hpp"
#include "RPC/RPCMethodsStore.hpp"
#include "Store/DatasetStore.hpp"
#include <memory>
#include <seria/deserialize/mpack.hpp>
#include <seria/object.hpp>
#include <seria/serialize/mpack.hpp>
#include <unordered_map>
#include <voxer/Renderers/VolumeRenderer.hpp>
#include <voxer/Util/LRUCache.hpp>

namespace voxer::remote {

class Service {
public:
  using Callback = std::function<void(const uint8_t *, uint32_t)>;

  explicit Service(DatasetStore *datasets);

  void on_message(const uint8_t *message, uint32_t size,
                  const Callback &callback) noexcept;

  static void on_error(const JSONRPCError &error, mpack_writer_t *writer);

  [[nodiscard]] Image render(const Scene &scene);

  [[nodiscard]] Image render_with_processed_data(
      const Scene &scene,
      const std::unordered_map<std::string, std::shared_ptr<StructuredGrid>>
          &processed_data);

  [[nodiscard]] Image get_dataset_slice(const std::string &dataset_id,
                                        StructuredGrid::Axis axis,
                                        uint32_t index);

  [[nodiscard]] DatasetInfo get_dataset_info(const std::string &id,
                                             const std::string &name,
                                             const std::string &path);

  void run_pipeline(const std::vector<mpack_node_t> &params,
                    mpack_writer_t *writer);

#ifdef ENABLE_ANNOTATION_SERVICE
  [[nodiscard]] std::vector<voxer::Annotation>
  apply_levelset(const std::string &, StructuredGrid::Axis, uint32_t,
                 const std::vector<voxer::Annotation> &);

  [[nodiscard]] std::vector<voxer::Annotation>
  apply_grabcut(const std::string &, StructuredGrid::Axis, uint32_t,
                const std::vector<voxer::Annotation> &);
#endif

private:
  std::unique_ptr<voxer::VolumeRenderer> m_renderer = nullptr;
  std::unique_ptr<RPCMethodsStore> m_methods = nullptr;
  LRUCache<DatasetID, std::shared_ptr<StructuredGrid>> m_datasets_for_slice;
  DatasetStore *m_datasets = nullptr;
  bool m_log_time = false;
};

} // namespace voxer::remote
