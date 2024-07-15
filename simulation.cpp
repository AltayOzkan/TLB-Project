#include <systemc.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include "simulation.h"

using namespace sc_core;
using namespace sc_dt;

#define MAX_TLB_ENTRIES 1024

struct TLBEntry {
    uint32_t virtualAddr;
    uint32_t physicalAddr;
    bool valid;
};

SC_MODULE(TLB) {
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<uint32_t> virtualAddr;
    sc_out<uint32_t> physicalAddr;
    sc_out<bool> hit;

    TLBEntry* tlb;
    size_t tlbSize;
    int offset_bits;

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

        uint32_t index = (virtualAddr.read() >> offset_bits) % tlbSize;
        if (tlb[index].valid && tlb[index].virtualAddr == virtualAddr.read()) {
            physicalAddr.write(tlb[index].physicalAddr);
            hit.write(true);
        } else {
            hit.write(false);
        }
    }

    void tlb_update(uint32_t virtAddr, uint32_t physAddr) {
        uint32_t index = (virtAddr >> offset_bits) % tlbSize;
        tlb[index].virtualAddr = virtAddr;
        tlb[index].physicalAddr = physAddr;
        tlb[index].valid = true;
    }

    SC_CTOR(TLB) {
        SC_METHOD(tlb_lookup);
        sensitive << clk.pos();
        tlbSize = MAX_TLB_ENTRIES;
        tlb = new TLBEntry[tlbSize];
        initialize_tlb();
    }

    ~TLB() {
        delete[] tlb;
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

    void run() {
        std::cout << "Simulation started." << std::endl;
        wait(); // First waiting period for stabilization of signal

        while (true) {
            if (reset.read() == true) {
                currentRequest = 0;
                result = {0};
                std::cout << "Simulation reset" << std::endl;
            }

            if (currentRequest < numRequests) {
                Request req = requests[currentRequest];
                virtualAddr.write(req.addr);

                wait(clk.posedge_event()); 
                if (reset.read() == true) {
                } else {
                    wait(tlbLatency, SC_NS); // Wait for TLB lookup latency

                    if (hit.read() == true) {
                        result.hits++;
                        if (trace_fp.is_open()) {
                            trace_fp << "Hit: Virtual Address " << std::hex << req.addr << ", Physical Address " << physicalAddr.read() << "\n";
                            trace_fp.flush(); // Ensure data is written to the file
                        } else {
                            std::cerr << "Trace file is not open for writing at hit check." << std::endl;
                        }
                    } else {
                        result.misses++;
                        wait(memoryLatency, SC_NS); // Wait for memory access latency
                        uint32_t physAddr = req.addr + v2bBlockOffset * blocksize;
                        if (trace_fp.is_open()) {
                            trace_fp << "Miss: Virtual Address " << std::hex << req.addr << ", Translated Physical Address " << physAddr << "\n";
                            trace_fp.flush(); // Ensure data is written to the file
                        } else {
                            std::cerr << "Trace file is not open for writing at miss check." << std::endl;
                        }
                        // Update the TLB
                        tlb->tlb_update(req.addr, physAddr);
                    }

                    result.cycles++;
                    currentRequest++;
                    std::cout << "Cycle: " << result.cycles << ", Current Request: " << currentRequest << std::endl;

                    // Additional check: Prevent excessive cycle count
                    if (result.cycles > 1000) { // Adjust as necessary
                        std::cerr << "Error: Cycle count exceeded limit" << std::endl;
                        sc_stop(); // Stop simulation
                    }
                }

            } else {
                std::cout << "Simulation complete" << std::endl;
                sc_stop(); // Stop the simulation
            }

            wait(); // Wait for the next clock cycle
        }
    }

    SC_CTOR(Simulation) {
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
};

unsigned calculate_primitive_gates(unsigned tlb_size, unsigned block_size, unsigned v2b_block_offset, unsigned memory_latency, unsigned tlb_latency) {
    unsigned base_gates = 1000; // Gates required basic circuitry

    // Calculate gates for storing TLB entries
    unsigned bits_per_entry = 32 * 2 + 1; // virtualAddr (32 bits), physicalAddr (32 bits), valid (1 bit)
    unsigned storage_gates_per_entry = bits_per_entry * 4; // 4 gates per bit for storage
    unsigned total_storage_gates = tlb_size * storage_gates_per_entry;

    // Adding gates for data path logic
    // Assuming each addition of two 32-bit numbers requires approximately 150 gates
    unsigned datapath_gates = tlb_size * 150; // Assumıng that ın every TLB entry we wıll requıre one arıthmetıc operatıon

    // Combining all gates
    unsigned total_gates = base_gates + total_storage_gates + datapath_gates;

    return total_gates;
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
    std::cout << "run_simulation called with parameters:" << std::endl;
    std::cout << "Cycles: " << cycles << ", TLB Size: " << tlbSize << ", TLB Latency: " << tlbLatency << std::endl;
    std::cout << "Block Size: " << blocksize << ", V2B Block Offset: " << v2bBlockOffset << ", Memory Latency: " << memoryLatency << std::endl;
    std::cout << "Number of Requests: " << numRequests << std::endl;

    sc_signal<bool> clk;
    sc_signal<bool> reset;
    sc_signal<uint32_t> virtualAddr;
    sc_signal<uint32_t> physicalAddr;
    sc_signal<bool> hit;

    TLB tlb("TLB");
    tlb.clk(clk);
    tlb.reset(reset);
    tlb.virtualAddr(virtualAddr);
    tlb.physicalAddr(physicalAddr);
    tlb.hit(hit);
    tlb.tlbSize = tlbSize;
    tlb.offset_bits = static_cast<int>(log2(blocksize));

    Simulation sim("Simulation");
    sim.clk(clk);
    sim.reset(reset);
    sim.virtualAddr(virtualAddr);
    sim.physicalAddr(physicalAddr); // sc_in olarak kullanılıyor
    sim.hit(hit); // sc_in olarak kullanılıyor
    sim.requests = requests;
    sim.numRequests = numRequests;
    sim.tlbLatency = tlbLatency;
    sim.memoryLatency = memoryLatency;
    sim.blocksize = blocksize;
    sim.v2bBlockOffset = v2bBlockOffset;
    sim.set_tlb(&tlb);

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

    Result result = sim.result;
    result.primitiveGateCount = calculate_primitive_gates(tlbSize, blocksize, v2bBlockOffset, memoryLatency, tlbLatency);

    std::cout << "Simulation finished." << std::endl;
    std::cout << "Cycles: " << result.cycles << ", Hits: " << result.hits << ", Misses: " << result.misses << std::endl;
    return result;
}

int sc_main(int argc, char* argv[]) {  
    // Example values
    return 0;
}
