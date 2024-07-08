// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "simulation.h"

// Yardım mesajı
void print_help() {
    printf("Usage: program [options] <input_file>\n");
    printf("Options:\n");
    printf("  -c, --cycles <number>        Number of cycles to simulate\n");
    printf("      --blocksize <number>     Size of memory blocks in bytes\n");
    printf("      --v2b-block-offset <number>  Offset to translate virtual to physical addresses\n");
    printf("      --tlb-size <number>      Size of the TLB in entries\n");
    printf("      --tlb-latency <number>   TLB latency in cycles\n");
    printf("      --memory-latency <number> Memory latency in cycles\n");
    printf("      --tf <file>              Tracefile to output signals\n");
    printf("  -h, --help                   Print this help message\n");
}

// Ana fonksiyon
int main(int argc, char *argv[]) {
    int cycles = 0;
    unsigned blocksize = 0;
    unsigned v2b_block_offset = 0;
    unsigned tlb_size = 0;
    unsigned tlb_latency = 0;
    unsigned memory_latency = 0;
    char *tracefile = NULL;
    char *input_file = NULL;

    // Komut satırı argümanlarını işlemek için getopt_long kullan
    static struct option long_options[] = {
        {"cycles", required_argument, 0, 'c'},
        {"blocksize", required_argument, 0, 0},
        {"v2b-block-offset", required_argument, 0, 0},
        {"tlb-size", required_argument, 0, 0},
        {"tlb-latency", required_argument, 0, 0},
        {"memory-latency", required_argument, 0, 0},
        {"tf", required_argument, 0, 0},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "c:h", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'c':
                cycles = atoi(optarg);
                break;
            case 'h':
                print_help();
                return 0;
            case 0:
                if (strcmp("blocksize", long_options[long_index].name) == 0) {
                    blocksize = atoi(optarg);
                } else if (strcmp("v2b-block-offset", long_options[long_index].name) == 0) {
                    v2b_block_offset = atoi(optarg);
                } else if (strcmp("tlb-size", long_options[long_index].name) == 0) {
                    tlb_size = atoi(optarg);
                } else if (strcmp("tlb-latency", long_options[long_index].name) == 0) {
                    tlb_latency = atoi(optarg);
                } else if (strcmp("memory-latency", long_options[long_index].name) == 0) {
                    memory_latency = atoi(optarg);
                } else if (strcmp("tf", long_options[long_index].name) == 0) {
                    tracefile = optarg;
                }
                break;
            default:
                print_help();
                return 1;
        }
    }

    // Positional argüman olarak input file'ı al
    if (optind < argc) {
        input_file = argv[optind];
    } else {
        fprintf(stderr, "Input file is required\n");
        print_help();
        return 1;
    }

    // Parametrelerin geçerliliğini kontrol et
    if (cycles <= 0 || blocksize == 0 || tlb_size == 0 || tlb_latency == 0 || memory_latency == 0) {
        fprintf(stderr, "All parameters must be set and greater than zero\n");
        print_help();
        return 1;
    }

    // Dosyayı aç ve CSV verisini oku
    FILE *file = fopen(input_file, "r");
    if (!file) {
        perror("Failed to open file");
        fprintf(stderr, "Error opening file: %s\n", input_file); // Daha ayrıntılı hata mesajı
        return 1;
    }

    // Request structlarını oku ve bir arrayde sakla
    struct Request *requests = NULL;
    size_t num_requests = 0;
    size_t capacity = 10;
    requests = malloc(capacity * sizeof(struct Request));
    if (!requests) {
        perror("Failed to allocate memory");
        fclose(file);
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (num_requests >= capacity) {
            capacity *= 2;
            requests = realloc(requests, capacity * sizeof(struct Request));
            if (!requests) {
                perror("Failed to reallocate memory");
                fclose(file);
                return 1;
            }
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

    // Simülasyonu çalıştır
    struct Result result = run_simulation(cycles, tlb_size, tlb_latency, blocksize, v2b_block_offset, memory_latency, num_requests, requests, tracefile);

    // Sonuçları yazdır
    printf("Cycles: %zu\n", result.cycles);
    printf("Hits: %zu\n", result.hits);
    printf("Misses: %zu\n", result.misses);
    printf("Primitive Gate Count: %zu\n", result.primitiveGateCount);

    free(requests);
    return 0;
}
