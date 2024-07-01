#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Structure to store a memory request
struct Request {
    char type;         // 'R' for read, 'W' for write
    unsigned address;  // Address to read/write
    int value;         // Value to write (ignored for read)
};

// Structure to store the result of the simulation
struct Result {
    int cycles;
    int tlbHits;
    int tlbMisses;
    int memoryAccesses;
};

// Function to parse the command line arguments
int parse_arguments(int argc, char *argv[], unsigned *tlbSize, unsigned *tlbLatency, unsigned *blockSize, unsigned *v2bBlockOffset, unsigned *memoryLatency, char **inputFile, char **traceFile);

// Function to read requests from the input file
struct Request* read_requests(const char *filename, size_t *numRequests);

// Function to print the usage information
void print_usage(const char *programName);

// Function to run the TLB simulation
struct Result run_simulation(int cycles, unsigned tlbSize, unsigned tlbLatency, unsigned blockSize, unsigned v2bBlockOffset, unsigned memoryLatency, size_t numRequests, struct Request requests[], const char *traceFile);

#endif // UTILS_H
