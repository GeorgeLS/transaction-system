//
// Created by gliontos on 2/13/19.
//

#include <stdint.h>
#include <stddef.h>
#include "hash_table_lp.h"
#include "utils.h"
#include "error_enums.h"

typedef struct {
    uint16_t id;
    size_t offset;
} Key;

int HT_LP_Create(HT_LP *ht_out, const size_t value_size, const size_t buckets, Hash_Function hash_function) {
    void *array = __MALLOC_BYTES(value_size * buckets);
    if (!array) return ENOMEM;
    if (!hash_function) return ENOHASHFUNCTION;
    *ht_out = (HT_LP) {
            .value_size = value_size,
            .buckets = buckets,
            .hash_function = hash_function,
            .array = array
    };
    return 0;
}

void HT_LP_Free(HT_LP *hash_table) { free(hash_table->array); }

// TODO(gliontos): Implement this!
int HT_LP_Try_Get_Value(HT_LP *ht, const uint64_t id, void *value_out) {
    void *array = ht->array;
    uint8_t low_key_byte = (uint8_t) (id & 0xFF);
    uint8_t high_key_byte = (uint8_t) ((id >> 8U) & 0xFF);
    size_t outer_bucket = ht->hash_function(&low_key_byte, sizeof(uint8_t));
    size_t inner_bucket = ht->hash_function(&high_key_byte, sizeof(uint8_t));
    return -1;
}