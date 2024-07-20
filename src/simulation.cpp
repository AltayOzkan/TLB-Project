#include <systemc.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <climits> // For SIZE_MAX
#include <string>
#include <vector> // For std::vector
#include "simulation.h"

using namespace sc_core;
using namespace sc_dt;

struct TLBEntry {
    uint32_t tag;
    uint32_t physicalAddr;
    bool valid;
};

class TLB : public sc_module {
public:
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<uint32_t> virtualAddr;
    sc_out<uint32_t> physicalAddr;
    sc_out<bool> hit;

    std::vector<TLBEntry> tlb; // Vector to store TLB entries
    size_t tlbSize;
    int offset_bits;
    int index_bits;
    int tag_bits;

    SC_HAS_PROCESS(TLB);

    TLB(sc_module_name name, size_t size, int offset) : sc_module(name), tlbSize(size), offset_bits(offset), tlb(size) {
        SC_METHOD(tlb_lookup);
        sensitive << clk.pos();
        initialize_tlb();
        index_bits = static_cast<int>(log2(tlbSize));
        tag_bits = 32 - index_bits - offset_bits;
    }

    void initialize_tlb() {
        for (size_t i = 0; i < tlbSize; ++i) {
            tlb[i].valid = false;
        }
    }

    void tlb_lookup() {
        if (reset.read() == true) {
            physicalAddr.write(0);
            hit.write(false);
            return;
        }

        uint32_t virtualAddr_read = virtualAddr.read();
        uint32_t index = (virtualAddr_read >> offset_bits) & ((1 << index_bits) - 1); // Extract the index
        uint32_t tag = virtualAddr_read >> (offset_bits + index_bits); // Extract the tag

        if (tlb[index].valid && tlb[index].tag == tag) {
            physicalAddr.write(tlb[index].physicalAddr + (virtualAddr_read & ((1 << offset_bits) - 1))); // Combine physical page and offset
            hit.write(true);
        } else {
            hit.write(false);
        }
    }

    void tlb_update(uint32_t virtAddr, uint32_t physAddr) {
        uint32_t index = (virtAddr >> offset_bits) & ((1 << index_bits) - 1); // Extract the index
        uint32_t tag = virtAddr >> (offset_bits + index_bits); // Extract the tag
        tlb[index].tag = tag;
        tlb[index].physicalAddr = physAddr & ~((1 << offset_bits) - 1); // Store the physical page number
        tlb[index].valid = true;
    }
};

SC_MODULE(Simulation) {
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_out<uint32_t> virtualAddr;
    sc_in<uint32_t> physicalAddr;
    sc_in<bool> hit;

    TLB* tlb;
    Request* requests;
    size_t numRequests;
    size_t currentRequest;
    int tlbLatency;
    int memoryLatency;
    unsigned blocksize;
    unsigned v2bBlockOffset;
    std::ofstream trace_fp;
    Result result;
    int maxCycles;

    void run() {
        std::cout << "Simulation started." << std::endl;
        wait();

        while (true) {
            if (reset.read() == true) {
                currentRequest = 0;
                result = {0, 0, 0, 0}; // Initialize result with zeros
                std::cout << "Simulation reset" << std::endl;
            }

            if (currentRequest < numRequests) {
                Request req = requests[currentRequest];
                virtualAddr.write(req.addr);

                wait(clk.posedge_event());
                if (reset.read() == true) {
                    continue;
                }

                wait(tlbLatency, SC_NS); // Wait for TLB lookup latency

                if (hit.read() == true) {
                    result.hits++;
                    result.cycles += tlbLatency; // Add TLB latency for every access
                    if (trace_fp.is_open()) {
                        trace_fp << "Hit: Virtual Address " << std::hex << req.addr << ", Physical Address " << physicalAddr.read() << "\n";
                        trace_fp.flush(); // Ensure data is written to the file
                    }
                } else {
                    result.misses++;
                    result.cycles += tlbLatency + memoryLatency; // Add TLB and memory latency for misses
                    wait(memoryLatency, SC_NS); // Wait for memory access latency
                    uint32_t physAddr = req.addr + v2bBlockOffset * blocksize;
                    if (trace_fp.is_open()) {
                        trace_fp << "Miss: Virtual Address " << std::hex << req.addr << ", Translated Physical Address " << physAddr << "\n";
                        trace_fp.flush(); // Ensure data is written to the file
                    }
                    // Update the TLB
                    tlb->tlb_update(req.addr, physAddr);
                }

                currentRequest++;
                std::cout << "Cycle: " << result.cycles << ", Current Request: " << currentRequest << std::endl;

                // Additional check: Prevent excessive cycle count
                
            } else {
                std::cout << "Simulation complete" << std::endl;
                sc_stop(); // Stop the simulation
            }

            wait();
        }
    }

    SC_CTOR(Simulation) : maxCycles(0) {
        SC_THREAD(run);
        sensitive << clk.pos();
        currentRequest = 0;
    }

    ~Simulation() {
        if (trace_fp.is_open()) {
            trace_fp.close();
        }
    }

    void set_tlb(TLB* tlb_module) {
        tlb = tlb_module;
    }

    void set_max_cycles(int cycles) {
        maxCycles = cycles;
    }

    void check_completion() {
        if (currentRequest < numRequests) {
            result.cycles = SIZE_MAX;
        }
    }
};

unsigned calculate_primitive_gates(unsigned tlb_size, unsigned block_size, unsigned v2b_block_offset, unsigned memory_latency, unsigned tlb_latency) {
    unsigned base_gates = 1000; // Gates required for basic circuitry

    // Calculate gates for storing TLB entries
    unsigned bits_per_entry = 32 * 2 + 1; // tag (32 bits), physicalAddr (32 bits), valid (1 bit)
    unsigned storage_gates_per_entry = bits_per_entry * 4; // 4 gates per bit for storage
    unsigned total_storage_gates = tlb_size * storage_gates_per_entry;

    // Adding gates for data path logic
    // Assuming each addition of two 32-bit numbers requires approximately 150 gates
    unsigned datapath_gates = tlb_size * 150; // Assuming that in every TLB entry we will require one arithmetic operation

    // Combining all gates
    unsigned total_gates = base_gates + total_storage_gates + datapath_gates;

    return total_gates;
}

bool is_power_of_two(unsigned x) {
    return (x != 0) && ((x & (x - 1)) == 0);
}

extern "C" Result run_simulation(
    int cycles,
    unsigned tlbSize,
    unsigned tlbLatency,
    unsigned blocksize,
    unsigned v2bBlockOffset,
    unsigned memoryLatency,
    size_t numRequests,
    Request* requests,
    const char* tracefile
) {
    if (!is_power_of_two(blocksize)) {
        std::cerr << "Error: Block size must be powers of two." << std::endl;
        exit(1);
    }

    std::cout << "run_simulation called with parameters:" << std::endl;
    std::cout << "Cycles: " << cycles << ", TLB Size: " << tlbSize << ", TLB Latency: " << tlbLatency << std::endl;
    std::cout << "Block Size: " << blocksize << ", V2B Block Offset: " << v2bBlockOffset << ", Memory Latency: " << memoryLatency << std::endl;
    std::cout << "Number of Requests: " << numRequests << std::endl;

    sc_signal<bool> clk;
    sc_signal<bool> reset;
    sc_signal<uint32_t> virtualAddr;
    sc_signal<uint32_t> physicalAddr;
    sc_signal<bool> hit;

    TLB tlb("TLB", tlbSize, static_cast<int>(log2(blocksize)));
    tlb.clk(clk);
    tlb.reset(reset);
    tlb.virtualAddr(virtualAddr);
    tlb.physicalAddr(physicalAddr);
    tlb.hit(hit);

    Simulation sim("Simulation");
    sim.clk(clk);
    sim.reset(reset);
    sim.virtualAddr(virtualAddr);
    sim.physicalAddr(physicalAddr);
    sim.hit(hit);
    sim.requests = requests;
    sim.numRequests = numRequests;
    sim.tlbLatency = tlbLatency;
    sim.memoryLatency = memoryLatency;
    sim.blocksize = blocksize;
    sim.v2bBlockOffset = v2bBlockOffset;
    sim.set_tlb(&tlb);
    sim.set_max_cycles(cycles);

    if (tracefile) {
        std::cout << "Attempting to open trace file: " << tracefile << std::endl;
        sim.trace_fp.open(tracefile);
        if (!sim.trace_fp.is_open()) {
            std::cerr << "Error opening trace file: " << tracefile << std::endl;
            exit(1);
        } else {
            std::cout << "Trace file opened successfully." << std::endl;
        }
    } else {
        std::cerr << "Trace file path is null." << std::endl;
    }

    std::cout << "Starting simulation..." << std::endl;
    sc_start(0, SC_NS);
    reset = true;
    clk = false;
    sc_start(1, SC_NS);
    clk = true;
    sc_start(1, SC_NS);
    reset = false;

    for (int i = 0; i < cycles; ++i) {
        clk = false;
        sc_start(1, SC_NS);
        clk = true;
        sc_start(1, SC_NS);
        if (!sc_is_running()) {
            break;
        }
    }

    // Check if all requests were processed within the given cycles
    sim.check_completion();

    Result result = sim.result;
    result.primitiveGateCount = calculate_primitive_gates(tlbSize, blocksize, v2bBlockOffset, memoryLatency, tlbLatency);

    std::cout << "Simulation finished." << std::endl;

        sim.check_completion();

    return result;
}

int sc_main(int argc, char* argv[]) {
    // Example values
    return 0;
}
