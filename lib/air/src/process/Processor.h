
#ifndef AIR_PROCESSOR_H
#define AIR_PROCESSOR_H

#include <string>

#include "src/lib/Data.h"
#include "src/profile_data/node/NodeThread.h"

namespace process
{
class Processor
{
public:
    virtual ~Processor(void)
    {
    }
    bool IsIdle(lib::Data* data);
    bool StreamData(std::string node_name, uint32_t tid, const char* tname,
        node::Thread* thread, air::ProcessorType ptype, uint32_t time,
        uint32_t max_aid_size);
    bool StreamData(std::string node_name, lib::AccLatencyData* data,
        uint32_t aid);

protected:
    uint32_t time{1};

private:
    virtual bool _ProcessData(lib::Data* air_data, lib::AccData* acc_data) = 0;
    virtual void _InitData(lib::Data* air_data, lib::AccData* acc_data) = 0;
};

class PerformanceProcessor : public Processor
{
public:
    virtual ~PerformanceProcessor(void)
    {
    }

private:
    virtual bool _ProcessData(lib::Data* air_data, lib::AccData* acc_data);
    virtual void _InitData(lib::Data* air_data, lib::AccData* acc_data);
};

class LatencyProcessor : public Processor
{
public:
    virtual ~LatencyProcessor(void)
    {
    }

private:
    virtual bool _ProcessData(lib::Data* air_data, lib::AccData* acc_data);
    virtual void _InitData(lib::Data* air_data, lib::AccData* acc_data);
    void _Calculate(lib::AccLatencySeqData* lat_data);

    static const uint64_t OVERFLOW_THRESHOLD{0x7FFFFFFFFFFFFFFF};
};

class QueueProcessor : public Processor
{
public:
    virtual ~QueueProcessor(void)
    {
    }

private:
    virtual bool _ProcessData(lib::Data* air_data, lib::AccData* acc_data);
    virtual void _InitData(lib::Data* air_data, lib::AccData* acc_data);
};

} // namespace process

#endif // AIR_PROCESSOR_H
