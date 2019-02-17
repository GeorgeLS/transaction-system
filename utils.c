//
// Created by gliontos on 2/14/19.
//

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "bitmap.h"
#include "utils.h"


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

size_t hash_function(const void *data, const size_t bytes, const size_t buckets) {
    size_t hash_value = 0U;
    for (size_t i = 0U; i != bytes; ++i, ++data) {
        hash_value += *(uint8_t *) data;
    }
    return hash_value % buckets;
}

uint32_t generate_id(void) {
    static Bitmap_t low_bitmap;
    static Bitmap_t high_bitmap;
    static uint8_t bitmaps_initialized = 0U;
    if (!bitmaps_initialized) {
        srandom((unsigned int) time(NULL));
        create_bitmap(&low_bitmap, BITS(uint16_t));
        create_bitmap(&high_bitmap, BITS(uint16_t));
        bitmaps_initialized = 1U;
    }
    while (1) {
        uint16_t low_bits = (uint16_t) ((random() % UINT16_MAX) + 1U);
        uint16_t high_bits = (uint16_t) ((random() % UINT16_MAX) + 1U);
        if (!get_bit(&low_bitmap, low_bits) && !get_bit(&high_bitmap, high_bits)) {
            uint32_t res = (high_bits << 16U) | low_bits;
            set_bit(&low_bitmap, low_bits);
            set_bit(&high_bitmap, high_bits);
            return res;
        }
    }
}