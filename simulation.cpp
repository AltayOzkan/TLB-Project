#include <systemc.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>
#include "simulation.h"

using namespace sc_core;
using namespace sc_dt;

// Define TLB module
SC_MODULE(TLB) {
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<uint32_t> virtualAddr;
    sc_out<uint32_t> physicalAddr;
    sc_out<bool> hit;

    struct TLBEntry {
        uint32_t virtualAddr;
        uint32_t physicalAddr;
        bool valid;
    };

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
        offset_bits = static_cast<int>(log2(64)); // Assume block size is 64 bytes
    }

    ~TLB() {
        delete[] tlb;
    }
};

// Define Simulation module
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
        if (reset.read() == true) {
            currentRequest = 0;
            result = {0};
            return;
        }

        if (currentRequest < numRequests) {
            Request req = requests[currentRequest];
            virtualAddr.write(req.addr);

            wait(tlbLatency, SC_NS);

            if (hit.read() == true) {
                result.hits++;
                if (trace_fp.is_open()) {
                    trace_fp << "Hit: Virtual Address " << req.addr << ", Physical Address " << physicalAddr.read() << "\n";
                }
            } else {
                result.misses++;
                wait(memoryLatency, SC_NS);
                uint32_t physAddr = req.addr + v2bBlockOffset * blocksize;
                virtualAddr.write(physAddr);
                if (trace_fp.is_open()) {
                    trace_fp << "Miss: Virtual Address " << req.addr << ", Translated Physical Address " << physAddr << "\n";
                }
                // Update the TLB
                tlb->tlb_update(req.addr, physAddr);
            }

            result.cycles++;
            currentRequest++;
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

std::vector<Request> read_requests_from_file(const char* filename) {
    std::vector<Request> requests;
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error opening input file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    std::getline(infile, line); // Skip header line

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string addr_str;
        if (iss >> addr_str) {
            Request req;
            req.addr = std::stoul(addr_str, nullptr, 16); // Convert hex string to uint32_t
            requests.push_back(req);
        }
    }

    return requests;
}

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
) {
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
    sim.physicalAddr(physicalAddr);
    sim.hit(hit);
    sim.requests = requests;
    sim.numRequests = numRequests;
    sim.tlbLatency = tlbLatency;
    sim.memoryLatency = memoryLatency;
    sim.blocksize = blocksize;
    sim.v2bBlockOffset = v2bBlockOffset;
    sim.set_tlb(&tlb);

    if (tracefile) {
        sim.trace_fp.open(tracefile);
    }

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
    }

    return sim.result;
}

int sc_main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    const char* input_file = argv[1];
    std::vector<Request> requests = read_requests_from_file(input_file);
    
    int cycles = 1000;
    unsigned tlbSize = 128;
    unsigned tlbLatency = 3;
    unsigned blocksize = 64;
    unsigned v2bBlockOffset = 4;
    unsigned memoryLatency = 5;
    const char* tracefile = "trace.txt";

    Result result = run_simulation(cycles, tlbSize, tlbLatency, blocksize, v2bBlockOffset, memoryLatency, requests.size(), requests.data(), tracefile);

    std::cout << "Cycles: " << result.cycles << std::endl;
    std::cout << "Misses: " << result.misses << std::endl;
    std::cout << "Hits: " << result.hits << std::endl;
    std::cout << "Primitive Gate Count: " << result.primitiveGateCount << std::endl;

    return 0;
}
