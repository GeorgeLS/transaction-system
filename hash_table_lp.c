//
// Created by gliontos on 2/13/19.
//

#include <stdint.h>
#include <stddef.h>
#include "hash_table_lp.h"
#include "utils.h"
#include "error_enums.h"

#define current_table_element table[outer_probe][inner_probe]

#define MAX_INNER_BUCKET_SIZE (1U << 16U)

static inline HT_LP_Record_Entry HT_LP_Record_Entry_Create(void) {
    return (HT_LP_Record_Entry) {
            .key_value_pair.v1 = 0U,
            .key_value_pair.v2 = 0U,
            .is_deleted = 0U,
            .is_empty = 1U
    };
}

static inline void __create_HT_LP_Record_Entries(HT_LP_Record_Entry *mem, size_t records) {
    for (size_t i = 0U; i != records; ++i) {
        mem[i] = HT_LP_Record_Entry_Create();
    }
}

// TODO(gliontos): Implement a free list so that operation takes O(1)
static ssize_t get_available_record_index(HT_LP *ht) {
    ssize_t index = -1;
    for (ssize_t i = 0; i != ht->buckets; ++i) {
        if (ht->records[i] == NULL) {
            index = i;
            break;
        }
    }
    return index;
}

int HT_LP_Create(HT_LP *ht_out, const size_t buckets, Hash_Function hash_function) {
    if (!hash_function) return ENOHASHFUNCTION;
    void **records = __MALLOC(buckets, void*);
    if (!records) return ENOMEM;
    for (size_t i = 0U; i != buckets; ++i) { records[i] = NULL; }
    size_t outer_buckets = ((buckets - 1U) / MAX_INNER_BUCKET_SIZE) + 1U;
    size_t inner_buckets = MAX_INNER_BUCKET_SIZE;
    HT_LP_Record_Entry **table = __MALLOC(outer_buckets, HT_LP_Record_Entry*);
    if (!table) return ENOMEM;
    for (size_t i = 0U; i != outer_buckets; ++i) {
        table[i] = __MALLOC(inner_buckets, HT_LP_Record_Entry);
        if (!table[i]) return ENOMEM;
        __create_HT_LP_Record_Entries(table[i], inner_buckets);
    }
    *ht_out = (HT_LP) {
            .buckets = buckets,
            .outer_buckets = outer_buckets,
            .inner_buckets = inner_buckets,
            .hash_function = hash_function,
            .table = table,
            .records = records
    };
    return 0;
}

void HT_LP_Free(HT_LP *ht) {
    for (size_t i = 0U; i != ht->outer_buckets; ++i) {
        free(ht->table[i]);
    }
    for (size_t i = 0U; i != ht->buckets; ++i) {
        free(ht->records[i]);
    }
    free(ht->records);
    free(ht->table);
}

static inline void HT_LP_Record_Entry_Set(HT_LP_Record_Entry *entry, const uint16_t key, const size_t value,
                                          Record **record_table, Record *record) {
    entry->is_empty = entry->is_deleted = 0;
    entry->key_value_pair.v1 = key;
    entry->key_value_pair.v2 = value;
    record_table[value] = __MALLOC(1, Record);
    copy_record(record_table[value], record);
}

int HT_LP_Insert(HT_LP *ht, Record *record) {
#define probe_stop_condition (current_table_element.is_empty || current_table_element.is_deleted)

    size_t outer_buckets = ht->outer_buckets;
    size_t inner_buckets = ht->inner_buckets;
    uint16_t outer_bucket_key = (uint16_t) (record->id & 0xFFFF);
    size_t outer_bucket = ht->hash_function(&outer_bucket_key, sizeof(uint16_t), outer_buckets);
    uint16_t inner_bucket_key = (uint16_t) ((record->id >> BITS(uint16_t)) & 0xFFFF);
    size_t inner_bucket = ht->hash_function(&inner_bucket_key, sizeof(uint16_t), inner_buckets);
    size_t outer_probe = outer_bucket;
    HT_LP_Record_Entry **table = ht->table;
    uint8_t wrapped = 0U;
    while (!wrapped) {
        HT_LP_Record_Entry *entry = &table[outer_bucket][inner_bucket];
        if (entry->is_empty || entry->is_deleted) {
            ssize_t offset = get_available_record_index(ht);
            if (offset == -1) return EHTFULL;
            HT_LP_Record_Entry_Set(entry, inner_bucket_key, offset, ht->records, record);
            goto __EXIT;
        }
        size_t inner_probe = (inner_bucket + 1U) % inner_buckets;
        while (inner_probe != inner_bucket) {
            if (probe_stop_condition) {
                ssize_t offset = get_available_record_index(ht);
                if (offset == -1) return EHTFULL;
                HT_LP_Record_Entry_Set(&current_table_element, inner_bucket_key, offset, ht->records, record);
                goto __EXIT;
            }
            inner_probe = (inner_probe + 1U) % inner_buckets;
        }
        outer_probe = (outer_probe + 1U) % outer_buckets;
        wrapped = (uint8_t) (outer_probe == outer_bucket);
    }
    return EHTFULL;
    __EXIT:
    return 0;
}

int HT_LP_Try_Get_Value(HT_LP *ht, const uint32_t id, void **value_out) {
    size_t outer_buckets = ht->outer_buckets;
    size_t inner_buckets = ht->inner_buckets;
    uint16_t outer_bucket_key = (uint16_t) (id & 0xFFFF);
    size_t outer_bucket = ht->hash_function(&outer_bucket_key, sizeof(uint16_t), outer_buckets);
    uint16_t inner_bucket_key = (uint16_t) ((id >> BITS(uint16_t)) & 0xFFFF);
    size_t inner_bucket = ht->hash_function(&inner_bucket_key, sizeof(uint16_t), inner_buckets);
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
        size_t inner_probe = (inner_bucket + 1U) % inner_buckets;
        while (inner_probe != inner_bucket) {
            if (current_table_element.is_empty) return ENOEXISTS;
            else if (current_table_element.key_value_pair.v1 == inner_bucket_key &&
                     !current_table_element.is_deleted) {
                *value_out = ht->records[current_table_element.key_value_pair.v2];
                goto __EXIT;
            }
            inner_probe = (inner_probe + 1U) % inner_buckets;
        };
        outer_probe = (outer_probe + 1U) % outer_buckets;
        wrapped = (uint8_t) (outer_probe == outer_bucket);
    }
    return ENOEXISTS;
    __EXIT:
    return 0;
}

int HT_LP_Delete(HT_LP *ht, uint32_t id) {
    size_t outer_buckets = ht->outer_buckets;
    size_t inner_buckets = ht->inner_buckets;
    uint16_t outer_bucket_key = (uint16_t) (id & 0xFFFF);
    size_t outer_bucket = ht->hash_function(&outer_bucket_key, sizeof(uint16_t), outer_buckets);
    uint16_t inner_bucket_key = (uint16_t) ((id >> BITS(uint16_t)) & 0xFFFF);
    size_t inner_bucket = ht->hash_function(&inner_bucket_key, sizeof(uint16_t), inner_buckets);
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
        size_t inner_probe = (inner_bucket + 1U) % inner_buckets;
        while (inner_probe != inner_bucket) {
            if (current_table_element.is_empty) return ENOEXISTS;
            else if (current_table_element.key_value_pair.v1 == inner_bucket_key &&
                     !current_table_element.is_deleted) {
                current_table_element.is_deleted = 1U;
                free(ht->records[current_table_element.key_value_pair.v2]);
                ht->records[current_table_element.key_value_pair.v2] = NULL;
                goto __EXIT;
            }
            inner_probe = (inner_probe + 1U) % inner_buckets;
        }
        outer_probe = (outer_probe + 1U) % outer_buckets;
        wrapped = (uint8_t) (outer_probe == outer_bucket);
    }
    return ENOEXISTS;
    __EXIT:
    return 0;
}