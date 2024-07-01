#include "tlb.h"

// Constructor to initialize TLB with given size and latency
TLB::TLB(size_t size, unsigned latency)
    : size(size), latency(latency) {}

// Method to lookup a virtual address in the TLB
bool TLB::lookup(unsigned virtualAddress, unsigned &physicalAddress) {
    auto it = map.find(virtualAddress);
    if (it != map.end()) {
        // Move the accessed entry to the front of the list (LRU policy)
        entries.splice(entries.begin(), entries, it->second);
        physicalAddress = it->second->physicalAddress;
        return true;
    }
    return false;
}

// Method to insert a new entry into the TLB
void TLB::insert(unsigned virtualAddress, unsigned physicalAddress) {
    auto it = map.find(virtualAddress);
    if (it != map.end()) {
        // Update the existing entry
        entries.splice(entries.begin(), entries, it->second);
        it->second->physicalAddress = physicalAddress;
    } else {
        // If TLB is full, remove the least recently used entry
        if (entries.size() == size) {
            auto last = entries.back();
            map.erase(last.virtualAddress);
            entries.pop_back();
        }
        // Insert the new entry at the front of the list
        entries.push_front({virtualAddress, physicalAddress});
        map[virtualAddress] = entries.begin();
    }
}

// Method to clear all entries in the TLB
void TLB::clear() {
    entries.clear();
    map.clear();
}

// Method to get the latency of the TLB
unsigned TLB::getLatency() const {
    return latency;
}
