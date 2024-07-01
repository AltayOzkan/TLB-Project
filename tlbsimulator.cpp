#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "tlb.h"
#include "utils.h"

struct Request {
    uint32_t addr;
    uint32_t data;
    int we; // write enable: 0 for read, 1 for write
};

struct Result {
    size_t cycles;
    size_t misses;
    size_t hits;
    size_t primitiveGateCount;
};

Result run_simulation(
    int cycles,
    unsigned tlbSize,
    unsigned tlbLatency,
    unsigned blocksize,
    unsigned v2bBlockOffset,
    unsigned memoryLatency,
    size_t numRequests,
    struct Request requests[],
    const char* tracefile) 
{
    Result result = {0, 0, 0, 0};
    TLB tlb(tlbSize, tlbLatency);
    std::ofstream trace;

    if (tracefile) {
        trace.open(tracefile);
    }

    for (size_t i = 0; i < numRequests; ++i) {
        uint32_t virtualAddr = requests[i].addr;
        uint32_t physicalAddr;

        bool hit = tlb.lookup(virtualAddr, physicalAddr);

        if (hit) {
            result.hits++;
            result.cycles += tlbLatency;
        } else {
            result.misses++;
            physicalAddr = (virtualAddr / blocksize + v2bBlockOffset) * blocksize;
            result.cycles += memoryLatency;
            tlb.insert(virtualAddr, physicalAddr);
        }

        result.cycles += cycles;

        if (requests[i].we == 1) { // Write operation
            // simulate write
        } else { // Read operation
            // simulate read
        }

        if (tracefile) {
            trace << "Cycle: " << result.cycles
                  << " Address: " << virtualAddr
                  << " Physical Address: " << physicalAddr
                  << " Hit: " << hit
                  << "\n";
        }
    }

    if (tracefile) {
        trace.close();
    }

    return result;
}