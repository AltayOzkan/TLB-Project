#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define MAX_TLB_ENTRIES 1024

struct Request {
    uint32_t addr;
    uint32_t data;
    int we;
};

struct Result {
    size_t cycles;
    size_t misses;
    size_t hits;
    size_t primitiveGateCount;
};

struct Result run_simulation(
    int cycles,
    unsigned tlbSize,
    unsigned tlbLatency,
    unsigned blocksize,
    unsigned v2bBlockOffset,
    unsigned memoryLatency,
    size_t numRequests,
    struct Request* requests,
    const char* tracefile
);

#ifdef __cplusplus
}
#endif
