#ifndef SIMULATION_H
#define SIMULATION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    unsigned addr;
    unsigned data;
    int we;
} Request;

typedef struct {
    size_t cycles;
    size_t hits;
    size_t misses;
    size_t primitiveGateCount;
} Result;

Result run_simulation(
    int cycles,
    unsigned tlbSize,
    unsigned tlbLatency,
    unsigned blocksize,
    unsigned v2bBlockOffset,
    unsigned memoryLatency,
    size_t numRequests,
    Request* requests,
    const char* tracefile
);

#ifdef __cplusplus
}
#endif

#endif
