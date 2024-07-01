#include "utils.h"

// Function to parse the command line arguments
int parse_arguments(int argc, char *argv[], unsigned *tlbSize, unsigned *tlbLatency, unsigned *blockSize, unsigned *v2bBlockOffset, unsigned *memoryLatency, char **inputFile, char **traceFile) {
    if (argc != 8) {
        print_usage(argv[0]);
        return -1;
    }

    *tlbSize = strtoul(argv[1], NULL, 10);
    *tlbLatency = strtoul(argv[2], NULL, 10);
    *blockSize = strtoul(argv[3], NULL, 10);
    *v2bBlockOffset = strtoul(argv[4], NULL, 10);
    *memoryLatency = strtoul(argv[5], NULL, 10);
    *inputFile = argv[6];
    *traceFile = argv[7];

    return 0;
}

// Function to read requests from the input file
struct Request* read_requests(const char *filename, size_t *numRequests) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open input file");
        return NULL;
    }

    size_t capacity = 10;
    struct Request *requests = malloc(capacity * sizeof(struct Request));
    if (requests == NULL) {
        perror("Failed to allocate memory for requests");
        fclose(file);
        return NULL;
    }

    *numRequests = 0;
    while (fscanf(file, " %c %x %d", &requests[*numRequests].type, &requests[*numRequests].address, &requests[*numRequests].value) == 3) {
        (*numRequests)++;
        if (*numRequests == capacity) {
            capacity *= 2;
            struct Request *temp = realloc(requests, capacity * sizeof(struct Request));
            if (temp == NULL) {
                perror("Failed to reallocate memory for requests");
                free(requests);
                fclose(file);
                return NULL;
            }
            requests = temp;
        }
    }

    fclose(file);
    return requests;
}

// Function to print the usage information
void print_usage(const char *programName) {
    printf("Usage: %s <TLB size> <TLB latency> <block size> <v2b block offset> <memory latency> <input file> <trace file>\n", programName);
}

// Function to run the TLB simulation
struct Result run_simulation(int cycles, unsigned tlbSize, unsigned tlbLatency, unsigned blockSize, unsigned v2bBlockOffset, unsigned memoryLatency, size_t numRequests, struct Request requests[], const char *traceFile) {
    struct Result result = {0, 0, 0, 0};
    
    FILE *trace = fopen(traceFile, "w");
    if (trace == NULL) {
        perror("Failed to open trace file");
        return result;
    }

    // Placeholder simulation logic
    for (size_t i = 0; i < numRequests; ++i) {
        if (requests[i].type == 'R') {
            fprintf(trace, "Read from address %x\n", requests[i].address);
        } else if (requests[i].type == 'W') {
            fprintf(trace, "Write %d to address %x\n", requests[i].value, requests[i].address);
        }

        // Simulate TLB hit or miss (simplified logic)
        if (requests[i].address % tlbSize == 0) {
            result.tlbHits++;
        } else {
            result.tlbMisses++;
        }
        result.memoryAccesses++;
    }

    result.cycles = cycles + (result.tlbHits * tlbLatency) + (result.tlbMisses * memoryLatency);

    fclose(trace);
    return result;
}
