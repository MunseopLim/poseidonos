
#ifndef AIR_PROCESS_MANAGER_H
#define AIR_PROCESS_MANAGER_H

#include <string>

#include "src/config/ConfigInterface.h"
#include "src/lib/json/Json.h"
#include "src/meta/GlobalMeta.h"
#include "src/meta/NodeMeta.h"
#include "src/process/Processor.h"
#include "src/process/TimingDistributor.h"
#include "src/profile_data/node/NodeManager.h"

namespace process
{
class ProcessManager
{
public:
    ProcessManager(meta::GlobalMetaGetter* new_global_meta_getter,
        meta::NodeMetaGetter* new_node_meta_getter,
        node::NodeManager* new_node_manager)
    : global_meta_getter(new_global_meta_getter),
      node_meta_getter(new_node_meta_getter),
      node_manager(new_node_manager)
    {
    }
    ~ProcessManager(void);
    int Init(void);
    void StreamData(void);
    void SetTimeSlot(void);

private:
    void _AddGroupInfo(air::JSONdoc& doc);
    void _AddNodeInfo(std::string& group_name, std::string& node_name,
        uint32_t nid, air::ProcessorType type);

    Processor* processor[cfg::GetArrSize(config::ConfigType::NODE)]{
        nullptr,
    };
    meta::GlobalMetaGetter* global_meta_getter{nullptr};
    meta::NodeMetaGetter* node_meta_getter{nullptr};
    node::NodeManager* node_manager{nullptr};
    TimingDistributor timing_distributor;
    const uint32_t MAX_NID_SIZE{
        config::ConfigInterface::GetArrSize(config::ConfigType::NODE)};
};

} // namespace process

#endif // AIR_PROCESS_MANAGER_H
