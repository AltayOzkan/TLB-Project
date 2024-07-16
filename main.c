#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "simulation.h"

// Help 
void print_help() {
    printf("Usage: program [options] <input_file>\n");
    printf("Options:\n");
    printf("  -c, --cycles <number>        Number of cycles to simulate\n");
    printf("  -b, --blocksize <number>     Size of memory blocks in bytes\n");
    printf("  -o, --v2b-block-offset <number>  Offset to translate virtual to physical addresses\n");
    printf("  -t, --tlb-size <number>      Size of the TLB in entries\n");
    printf("  -l, --tlb-latency <number>   TLB latency in cycles\n");
    printf("  -m, --memory-latency <number> Memory latency in cycles\n");
    printf("  -f, --tf <file>              Tracefile to output signals\n");
    printf("  -h, --help                   Print this help message\n");
}

// Main Function
int main(int argc, char *argv[]) {
    int cycles = 0;
    unsigned blocksize = 0;
    unsigned v2b_block_offset = 0;
    unsigned tlb_size = 0;
    unsigned tlb_latency = 0;
    unsigned memory_latency = 0;
    char *tracefile = NULL;
    char *input_file = NULL;

    // Assigning values to the parameters so it will be easier to put it as switch cases
    static struct option long_options[] = {
        {"cycles", required_argument, 0, 'c'},
        {"blocksize", required_argument, 0, 'b'},
        {"v2b-block-offset", required_argument, 0, 'o'},
        {"tlb-size", required_argument, 0, 't'},
        {"tlb-latency", required_argument, 0, 'l'},
        {"memory-latency", required_argument, 0, 'm'},
        {"tf", required_argument, 0, 'f'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int long_index = 0;
    //Parsing CLI
    while ((opt = getopt_long(argc, argv, "c:b:o:t:l:m:f:h", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'c':
                cycles = atoi(optarg);
                break;
            case 'h':
                print_help();
                return 0;
            case 'b':
                blocksize = atoi(optarg);
                break;
            case 'o':
                v2b_block_offset = atoi(optarg);
                break;
            case 't':
                tlb_size = atoi(optarg);
                break;
            case 'l':
                tlb_latency = atoi(optarg);
                break;
            case 'm':
                memory_latency = atoi(optarg);
                break;
            case 'f':
                tracefile = optarg;
                break;
            default:
                print_help();
                return 1;
        }
    }

    // Taking the input file as positional argument
    if (optind < argc) {
        input_file = argv[optind];
    } else {
        fprintf(stderr, "Input file is required\n");
        print_help();
        return 1;
    }

    // Checking the legitimacy of parameters
    if (cycles <= 0 || blocksize == 0 || tlb_size == 0 || tlb_latency == 0 || memory_latency == 0) {
        fprintf(stderr, "All parameters must be set and greater than zero\n");
        print_help();
        return 1;
    }

    // Reading the cvs data
    FILE *file = fopen(input_file, "r");
    if (!file) {
        perror("Failed to open file");
        fprintf(stderr, "Error opening file: %s\n", input_file); // More detailed version of a help message
        return 1;
    }

    // Reading the request struct and storing in an array
    Request *requests = NULL;
    size_t num_requests = 0;
    size_t capacity = 10;
    requests = malloc(capacity * sizeof(Request));
    if (!requests) {
        perror("Failed to allocate memory");
        fclose(file);
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (num_requests >= capacity) {
            capacity *= 2;
            Request *new_requests = realloc(requests, capacity * sizeof(Request));
            if (!new_requests) {
                perror("Failed to reallocate memory");
                free(requests);
                fclose(file);
                return 1;
            }
            requests = new_requests;
        }
        char type;
        unsigned addr;
        unsigned data = 0;
        int fields = sscanf(line, "%c %x %x", &type, &addr, &data);
        if (fields < 2) {
            fprintf(stderr, "Invalid format in input file\n");
            free(requests);
            fclose(file);
            return 1;
        }
        requests[num_requests].addr = addr;
        requests[num_requests].data = data;
        requests[num_requests].we = (type == 'W') ? 1 : 0;
        num_requests++;
    }
    fclose(file);

    // Run the simulation, used extern in the cpp file
    Result result = run_simulation(cycles, tlb_size, tlb_latency, blocksize, v2b_block_offset, memory_latency, num_requests, requests, tracefile);

    printf("Cycles: %zu\n", result.cycles);
    printf("Hits: %zu\n", result.hits);
    printf("Misses: %zu\n", result.misses);
    printf("Primitive Gate Count: %zu\n", result.primitiveGateCount);

    free(requests);
    return 0;
}
