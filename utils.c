//
// Created by gliontos on 2/14/19.
//

#include <stddef.h>
#include <stdint.h>

uint8_t is_prime(uint64_t n) {
    if (n < 2U) return 0U;
    if (n == 2U) return 1U;
    if (!(n & 1U)) return 0U;
    if (!(n % 3U)) return (uint8_t) (n == 3U);
    uint64_t i, j;
    for (i = 5U; (j = i * i), j <= n && j > i; i += 6U) {
        if (!(n % i)) return 0U;
        if (!(n % (i + 2U))) return 0U;
    }
    return 1U;
}

size_t calculate_hash_table_size(size_t max_items) {
    max_items = (size_t) (1.3 * max_items);
    ++max_items;
    while (!is_prime(max_items)) ++max_items;
    return max_items;
}