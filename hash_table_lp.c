//
// Created by gliontos on 2/13/19.
//

#include <stdint.h>
#include <stddef.h>
#include "hash_table_lp.h"
#include "utils.h"
#include "error_enums.h"

static inline HT_LP_Record_Entry HT_LP_Record_Entry_Create(uint16_t key, size_t value) {
    return (HT_LP_Record_Entry) {
            .key_value_pair.v1 = key,
            .key_value_pair.v2 = value,
            .is_deleted = 0U,
            .is_empty = 1U
    };
}

int HT_LP_Create(HT_LP *ht_out, const size_t value_size, const size_t buckets, Hash_Function hash_function) {
    if (!hash_function) return ENOHASHFUNCTION;
    HT_LP_Record_Entry **table = __MALLOC(buckets, HT_LP_Record_Entry*);
    if (!table) return ENOMEM;
    for (size_t i = 0U; i != buckets; ++i) {
        table[i] = __MALLOC(buckets, HT_LP_Record_Entry);
        if (!table[i]) return ENOMEM;
        for (size_t j = 0U; j != buckets; ++j) {
            table[i][j] = HT_LP_Record_Entry_Create(0, 0);
        }
    }
    *ht_out = (HT_LP) {
            .buckets = buckets,
            .hash_function = hash_function,
            .table = table
    };
    return 0;
}

void HT_LP_Free(HT_LP *hash_table) {
    for (size_t i = 0U; i != hash_table->buckets; ++i) {
        free(hash_table->table[i]);
    }
    free(hash_table->table);
}

int HT_LP_Insert(HT_LP *hash_table, Record *record) {
#define current_table_element table[outer_probe][inner_probe]
#define probe_stop_condition (current_table_element.is_empty || current_table_element.is_deleted)

    size_t buckets = hash_table->buckets;
    uint16_t outer_bucket_key = (uint16_t) (record->id & 0xFFFF);
    size_t outer_bucket = hash_table->hash_function(&outer_bucket_key, sizeof(uint16_t), buckets);
    uint16_t inner_bucket_key = (uint16_t) ((record->id >> sizeof(uint16_t)) & 0xFFFF);
    size_t inner_bucket = hash_table->hash_function(&inner_bucket_key, sizeof(uint16_t), buckets);
    size_t outer_probe = outer_bucket;
    HT_LP_Record_Entry **table = hash_table->table;
    uint8_t wrapped = 0U;
    // TODO(gliontos): Find a way to compute offset
    while (!wrapped) {
        HT_LP_Record_Entry *entry = &table[outer_bucket][inner_bucket];
        if (entry->is_empty || entry->is_deleted) {
            entry->key_value_pair.v1 = inner_bucket_key;
            entry->key_value_pair.v2 = 0U;
            goto __EXIT;
        }
        size_t inner_probe = (inner_bucket + 1U) % buckets;
        while (inner_probe != inner_bucket) {
            if (probe_stop_condition) {
                current_table_element.key_value_pair.v1 = inner_bucket_key;
                current_table_element.key_value_pair.v2 = 0U;
                goto __EXIT;
            }
            ++inner_probe;
        }
        outer_probe = (outer_probe + 1U) % buckets;
        wrapped = (uint8_t) (outer_probe == outer_bucket);
    }
    return EHTFULL;
    __EXIT:
    return 0;
}

// TODO(gliontos): Implement this!
//int HT_LP_Try_Get_Value(HT_LP *ht, const uint64_t id, void *value_out) {
//    void *array = ht->array;
//    uint8_t low_key_byte = (uint8_t) (id & 0xFF);
//    uint8_t high_key_byte = (uint8_t) ((id >> 8U) & 0xFF);
//    size_t outer_bucket = ht->hash_function(&low_key_byte, sizeof(uint8_t), ht->buckets);
//    size_t inner_bucket = ht->hash_function(&high_key_byte, sizeof(uint8_t), ht->buckets);
//    return -1;
//}