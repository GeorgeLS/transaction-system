//
// Created by gliontos on 2/13/19.
//

#include <stdint.h>
#include <stddef.h>
#include "hash_table_lp.h"
#include "utils.h"
#include "error_enums.h"

#define current_table_element table[outer_probe][inner_probe]

static inline HT_LP_Record_Entry HT_LP_Record_Entry_Create(uint16_t key, size_t value) {
    return (HT_LP_Record_Entry) {
            .key_value_pair.v1 = key,
            .key_value_pair.v2 = value,
            .is_deleted = 0U,
            .is_empty = 1U
    };
}

int HT_LP_Create(HT_LP *ht_out, const size_t buckets, Hash_Function hash_function) {
    if (!hash_function) return ENOHASHFUNCTION;
    void *records = __MALLOC(buckets * buckets, void*);
    if (!records) return ENOMEM;
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
            .table = table,
            .records = records
    };
    return 0;
}

void HT_LP_Free(HT_LP *ht) {
    for (size_t i = 0U; i != ht->buckets; ++i) {
        free(ht->table[i]);
    }
    free(ht->table);
}

int HT_LP_Insert(HT_LP *ht, Record *record) {
#define probe_stop_condition (current_table_element.is_empty || current_table_element.is_deleted)

    size_t buckets = ht->buckets;
    uint16_t outer_bucket_key = (uint16_t) (record->id & 0xFFFF);
    size_t outer_bucket = ht->hash_function(&outer_bucket_key, sizeof(uint16_t), buckets);
    uint16_t inner_bucket_key = (uint16_t) ((record->id >> sizeof(uint16_t)) & 0xFFFF);
    size_t inner_bucket = ht->hash_function(&inner_bucket_key, sizeof(uint16_t), buckets);
    size_t outer_probe = outer_bucket;
    HT_LP_Record_Entry **table = ht->table;
    uint8_t wrapped = 0U;
    // TODO(gliontos): Find a way to compute offset
    while (!wrapped) {
        HT_LP_Record_Entry *entry = &table[outer_bucket][inner_bucket];
        if (entry->is_empty || entry->is_deleted) {
            size_t offset = (outer_bucket * buckets) + inner_bucket;
            entry->key_value_pair.v1 = inner_bucket_key;
            entry->key_value_pair.v2 = offset;
            entry->is_empty = 0;
            ht->records[offset] = __MALLOC(1, Record);
            *((Record *) ht->records[offset]) = *record;
            goto __EXIT;
        }
        size_t inner_probe = (inner_bucket + 1U) % buckets;
        while (inner_probe != inner_bucket) {
            if (probe_stop_condition) {
                size_t offset = (outer_probe * buckets) + inner_probe;
                current_table_element.key_value_pair.v1 = inner_bucket_key;
                current_table_element.key_value_pair.v2 = offset;
                ht->records[offset] = record;
                goto __EXIT;
            }
            inner_probe = (inner_probe + 1U) % buckets;
        }
        outer_probe = (outer_probe + 1U) % buckets;
        wrapped = (uint8_t) (outer_probe == outer_bucket);
    }
    return EHTFULL;
    __EXIT:
    return 0;
}

int HT_LP_Try_Get_Value(HT_LP *ht, const uint64_t id, void **value_out) {
    size_t buckets = ht->buckets;
    uint16_t outer_bucket_key = (uint16_t) (id & 0xFFFF);
    size_t outer_bucket = ht->hash_function(&outer_bucket_key, sizeof(uint16_t), buckets);
    uint16_t inner_bucket_key = (uint16_t) ((id >> sizeof(uint16_t)) & 0xFFFF);
    size_t inner_bucket = ht->hash_function(&inner_bucket_key, sizeof(uint16_t), buckets);
    size_t outer_probe = outer_bucket;
    HT_LP_Record_Entry **table = ht->table;
    uint8_t wrapped = 0U;
    while (!wrapped) {
        HT_LP_Record_Entry *entry = &table[outer_bucket][inner_bucket];
        if (entry->is_empty) return ENOEXISTS;
        else if (entry->key_value_pair.v1 == inner_bucket_key && !entry->is_deleted) {
            *value_out = ht->records[entry->key_value_pair.v2];
            goto __EXIT;
        }
        size_t inner_probe = (inner_bucket + 1U) % buckets;
        while (inner_probe != inner_bucket) {
            if (current_table_element.is_empty) return ENOEXISTS;
            else if (current_table_element.key_value_pair.v1 == inner_bucket_key &&
                     !current_table_element.is_deleted) {
                *value_out = ht->records[current_table_element.key_value_pair.v2];
                goto __EXIT;
            }
            inner_probe = (inner_probe + 1U) % buckets;
        }
        ++outer_probe;
        wrapped = (uint8_t) (outer_probe == outer_bucket);
    }
    return ENOEXISTS;
    __EXIT:
    return 0;
}

int HT_LP_Delete(HT_LP *ht, uint64_t id) {
    size_t buckets = ht->buckets;
    uint16_t outer_bucket_key = (uint16_t) (id & 0xFFFF);
    size_t outer_bucket = ht->hash_function(&outer_bucket_key, sizeof(uint16_t), buckets);
    uint16_t inner_bucket_key = (uint16_t) ((id >> sizeof(uint16_t)) & 0xFFFF);
    size_t inner_bucket = ht->hash_function(&inner_bucket_key, sizeof(uint16_t), buckets);
    size_t outer_probe = outer_bucket;
    HT_LP_Record_Entry **table = ht->table;
    uint8_t wrapped = 0U;
    while (!wrapped) {
        HT_LP_Record_Entry *entry = &table[outer_bucket][inner_bucket];
        if (entry->is_empty) return ENOEXISTS;
        else if (entry->key_value_pair.v1 == inner_bucket_key && !entry->is_deleted) {
            entry->is_deleted = 1U;
            free(ht->records[entry->key_value_pair.v2]);
            ht->records[entry->key_value_pair.v2] = NULL;
            goto __EXIT;
        }
        size_t inner_probe = (inner_bucket + 1U) % buckets;
        while (inner_probe != inner_bucket) {
            if (current_table_element.is_empty) return ENOEXISTS;
            else if (current_table_element.key_value_pair.v1 == inner_bucket_key &&
                     !current_table_element.is_deleted) {
                current_table_element.is_deleted = 1U;
                free(ht->records[current_table_element.key_value_pair.v2]);
                ht->records[current_table_element.key_value_pair.v2] = NULL;
                goto __EXIT;
            }
            inner_probe = (inner_probe + 1U) % buckets;
        }
        ++outer_probe;
        wrapped = (uint8_t) (outer_probe == outer_bucket);
    }
    return ENOEXISTS;
    __EXIT:
    return 0;
}