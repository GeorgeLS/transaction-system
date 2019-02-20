//
// Created by gliontos on 2/13/19.
//

//High availability, robust, optimized transaction system. It just hasn't even started yet.

#ifndef DBMS_UTILS_H
#define DBMS_UTILS_H

#include <stdlib.h>
#include <memory.h>

/**
 * A macro that calls memcpy to copy a string.
 * Adds the null terminator in the end of the string
 */
#define STR_COPY(dest, source, bytes_n) \
do { \
    memcpy(dest, source, bytes_n); \
    (dest)[bytes_n] = '\0'; \
} while (0) \

#define __MALLOC(size, type) ((type*) malloc((size) * sizeof(type)))

#define __MALLOC_BYTES(bytes) ((void*) malloc((bytes)))

#define BITS(type) (sizeof(type) << 3U)

uint8_t is_prime(uint64_t n);

size_t calculate_hash_table_size(size_t max_items);

size_t hash_function(const void *data, const size_t bytes, const size_t buckets);

uint32_t generate_id(void);

#endif //DBMS_UTILS_H
