/**
 * @file memory_man.c
 * @brief Memory utilities
 * @author Mikhael Kaa (Михаил Каа)
 * @date 21.06.2025
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

static void print_usage(void);
static void mem_dump(uint8_t *buf, uint32_t len);
static uint32_t mem_test(uint8_t *buf, uint32_t len);

#ifdef BAREMETAL
int ucmd_mem(int argc, char *argv[]) {
#define ENDL "\r\n"
#else
int main(int argc, char *argv[]) {
#define ENDL "\n"
#endif // BAREMETAL

    uint32_t addr, data, len;
    uint8_t *ptr;

    switch (argc) {
    case 1:
    case 2:
        print_usage();
        return -EINVAL;
    
    case 3:
        if (strcmp(argv[1], "read") == 0) {
            if (sscanf(argv[2], "%lx", &addr) != 1) {
                printf("Invalid address format" ENDL);
                return -EINVAL;
            }
            ptr = (uint8_t *)addr;
            printf("0x%02x" ENDL, *ptr);
            return 0;
        }
        break;
    
    case 4:
        if (strcmp(argv[1], "test") == 0) {
            if (sscanf(argv[2], "%lx", &addr) != 1 ||
                sscanf(argv[3], "%lx", &len) != 1) 
            {
                printf("Invalid arguments format" ENDL);
                return -EINVAL;
            }
            return mem_test((uint8_t *)addr, len);
        }
        
        if (strcmp(argv[1], "dump") == 0) {
            if (sscanf(argv[2], "%lx", &addr) != 1 ||
                sscanf(argv[3], "%lx", &len) != 1) 
            {
                printf("Invalid arguments format" ENDL);
                return -EINVAL;
            }
            mem_dump((uint8_t *)addr, len);
            return 0;
        }
        
        if (strcmp(argv[1], "write") == 0) {
            if (sscanf(argv[2], "%lx", &addr) != 1 ||
                sscanf(argv[3], "%lx", &data) != 1) 
            {
                printf("Invalid arguments format" ENDL);
                return -EINVAL;
            }
            ptr = (uint8_t *)addr;
            *ptr = (uint8_t)data;
            return 0;
        }
        break;
    
    case 5:
        if (strcmp(argv[1], "cpy") == 0) {
            uint32_t src;
            if (sscanf(argv[2], "%lx", &addr) != 1 ||
                sscanf(argv[3], "%lx", &src) != 1 ||
                sscanf(argv[4], "%lx", &len) != 1) 
            {
                printf("Invalid arguments format" ENDL);
                return -EINVAL;
            }
            memcpy((void *)addr, (void *)src, len);
            return 0;
        }
        break;

    default:
        print_usage();
        return -EINVAL;
    }
    
    print_usage();
    return -EINVAL;
}

static void print_usage(void) {
    printf("Usage: mem <command> [arguments]" ENDL);
    printf("Commands:" ENDL);
    printf("  dump <adr> <len>    - Hexdump of memory region" ENDL);
    printf("  read <adr>          - Read byte from address" ENDL);
    printf("  write <adr> <data>  - Write byte to address" ENDL);
    printf("  test <adr> <len>    - Test memory region" ENDL);
    printf("  cpy <dst> <src> <len> - Copy memory block" ENDL);
}

static void mem_dump(uint8_t *buf, uint32_t len) {
    uint32_t i = 0;
    uint32_t current_addr = (uint32_t)buf;
    const uint32_t bytes_per_line = 16;
    
    while (i < len) {
        /* Print address at start of each line */
        printf("0x%08lx: ", (unsigned long)current_addr);
        
        /* Print hex values */
        uint32_t bytes_printed = 0;
        for (uint32_t j = 0; j < bytes_per_line; j++) {
            if (i + j < len) {
                printf("%02x ", buf[i + j]);
                bytes_printed++;
            } else {
                printf("   "); /* Padding for incomplete lines */
            }
        }
        
        /* Print ASCII representation */
        printf("| ");
        for (uint32_t j = 0; j < bytes_printed; j++) {
            uint8_t c = buf[i + j];
            putchar((c >= 32 && c <= 126) ? c : '.');
        }
        printf("" ENDL);
        
        i += bytes_printed;
        current_addr += bytes_printed;
    }
}

static uint32_t mem_test(uint8_t *buf, uint32_t len) {
    uint32_t error_count = 0;
    volatile uint8_t *b = buf;
    const uint8_t patterns[] = {0x00, 0x55, 0xAA, 0xFF};
    const int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    const uint32_t max_errors_to_print = 10;

    if (len == 0) {
        printf("Zero-length test skipped" ENDL);
        return 0;
    }

    for (uint32_t i = 0; i < len; i++) {
        uint8_t original = b[i];

        /* Test patterns */
        for (int p = 0; p < num_patterns; p++) {
            uint8_t test_val = patterns[p];
            b[i] = test_val;
            
            if (b[i] != test_val) {
                if (error_count < max_errors_to_print) {
                    printf("ERROR @ 0x%08lx: Wrote 0x%02X, Read 0x%02X" ENDL, 
                          (unsigned long)(b + i), test_val, b[i]);
                }
                error_count++;
            }
        }

        /* Restore original value */
        b[i] = original;
        if (b[i] != original) {
            if (error_count < max_errors_to_print) {
                printf("RESTORE ERROR @ 0x%08lx! Original: 0x%02X, Current: 0x%02X" ENDL,
                      (unsigned long)(b + i), original, b[i]);
            }
            error_count++;
        }

        /* Progress indicator */
        if ((i % 8192) == 0) {
            printf("Tested %lu/%lu bytes (errors: %lu)" ENDL, i, len, error_count);
        }
    }

    /* Final report */
    if (error_count == 0) {
        printf("Memory test PASSED: %lu bytes" ENDL, len);
    } else {
        printf("Memory test FAILED! Errors: %lu/%lu bytes" ENDL, error_count, len);
    }
    
    return error_count;
}

#undef ENDL
