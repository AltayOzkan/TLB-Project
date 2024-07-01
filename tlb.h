#ifndef TLB_H
#define TLB_H

#include <unordered_map>
#include <list>
#include <cstddef>

// Struct to represent a single TLB entry
struct TLBEntry {
    unsigned virtualAddress;
    unsigned physicalAddress;
};

class TLB {
public:
    // Constructor to initialize TLB with given size and latency
    TLB(size_t size, unsigned latency);

    // Method to lookup a virtual address in the TLB
    bool lookup(unsigned virtualAddress, unsigned &physicalAddress);

    // Method to insert a new entry into the TLB
    void insert(unsigned virtualAddress, unsigned physicalAddress);

    // Method to clear all entries in the TLB
    void clear();

    // Method to get the latency of the TLB
    unsigned getLatency() const;

private:
    size_t size; // Maximum size of the TLB
    unsigned latency; // Latency of the TLB
    std::unordered_map<unsigned, std::list<TLBEntry>::iterator> map; // Map for fast lookups
    std::list<TLBEntry> entries; // List to maintain LRU order
};

#endif // TLB_H
