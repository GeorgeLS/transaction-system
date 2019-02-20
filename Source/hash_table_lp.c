//
// Created by gliontos on 2/13/19.
//

#include <stdint.h>
#include <stddef.h>
#include "../Include/hash_table_lp.h"
#include "../Include/utils.h"
#include "../Include/error_enums.h"

#define current_table_element table[outer_probe][inner_probe]

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

static size_t compute_offset(HT_LP *ht, size_t outer_bucket, size_t inner_bucket) {
    size_t offset = 0U;
    size_t *inner_buckets = ht->inner_buckets;
    for (size_t i = 0U; i != outer_bucket; ++i) {
        offset += inner_buckets[i];
    }
    offset += inner_bucket;
    return offset;
}

int HT_LP_Create(HT_LP *ht_out, const size_t buckets, Hash_Function hash_function) {
    if (!hash_function) return ENOHASHFUNCTION;
    void **records = __MALLOC(buckets, void*);
    if (!records) return ENOMEM;
    for (size_t i = 0U; i != buckets; ++i) { records[i] = NULL; }
    size_t outer_buckets = ((buckets - 1U) / UINT16_MAX) + 1U;
    HT_LP_Record_Entry **table = __MALLOC(outer_buckets, HT_LP_Record_Entry*);
    if (!table) return ENOMEM;
    size_t *inner_buckets = __MALLOC(outer_buckets, size_t);
    if (!inner_buckets) return ENOMEM;
    for (size_t i = 0U; i != outer_buckets - 1U; ++i) {
        inner_buckets[i] = UINT16_MAX;
        table[i] = __MALLOC(UINT16_MAX, HT_LP_Record_Entry);
        if (!table[i]) return ENOMEM;
        __create_HT_LP_Record_Entries(table[i], UINT16_MAX);
    }
    size_t _buckets = buckets % (UINT16_MAX + 1U);
    inner_buckets[outer_buckets - 1U] = _buckets;
    table[outer_buckets - 1U] = __MALLOC(_buckets, HT_LP_Record_Entry);
    if (!table[outer_buckets - 1U]) return ENOMEM;
    __create_HT_LP_Record_Entries(table[outer_buckets - 1U], _buckets);
    *ht_out = (HT_LP) {
            .outer_buckets = outer_buckets,
            .hash_function = hash_function,
            .table = table,
            .records = records,
            .inner_buckets = inner_buckets,
            .total_buckets = buckets,
            .insertions = 0U
    };
    return 0;
}

void HT_LP_Free(HT_LP *ht) {
    for (size_t i = 0U; i != ht->outer_buckets; ++i) {
        free(ht->table[i]);
    }
    for (size_t i = 0U; i != ht->total_buckets; ++i) {
        free(ht->records[i]);
    }
    free(ht->inner_buckets);
    free(ht->records);
    free(ht->table);
}

static inline void HT_LP_Record_Entry_Set(HT_LP_Record_Entry *entry, const uint16_t key, const size_t offset,
                                          void **record_table, void *value, size_t value_size) {
    entry->is_empty = entry->is_deleted = 0;
    entry->key_value_pair.v1 = key;
    entry->key_value_pair.v2 = offset;
    record_table[offset] = __MALLOC_BYTES(value_size);
    memcpy(record_table[offset], value, value_size);
}

int HT_LP_Insert(HT_LP *ht, uint32_t id, void *value, size_t value_size) {
    if (ht->insertions == ht->total_buckets) return EHTFULL;
    size_t outer_buckets = ht->outer_buckets;
    uint16_t outer_bucket_key = (uint16_t) (id & 0xFFFF);
    size_t outer_bucket = ht->hash_function(&outer_bucket_key, sizeof(uint16_t), outer_buckets);
    uint16_t inner_bucket_key = (uint16_t) (id >> BITS(uint16_t));
    size_t inner_bucket = ht->hash_function(&inner_bucket_key, sizeof(uint16_t), ht->inner_buckets[outer_bucket]);
    size_t outer_probe = outer_bucket;
    HT_LP_Record_Entry **table = ht->table;
    uint8_t wrapped = 0U;
    while (!wrapped) {
        HT_LP_Record_Entry *entry = &table[outer_bucket][inner_bucket];
        if (entry->is_empty || entry->is_deleted) {
            size_t offset = compute_offset(ht, outer_bucket, inner_bucket);
            HT_LP_Record_Entry_Set(entry, inner_bucket_key, offset, ht->records, value, value_size);
            goto __EXIT;
        }
        size_t inner_buckets = ht->inner_buckets[outer_probe];
        size_t inner_probe = (inner_bucket + 1U) % inner_buckets;
        while (inner_probe != inner_bucket) {
            if (current_table_element.is_empty || current_table_element.is_deleted) {
                size_t offset = compute_offset(ht, outer_probe, inner_probe);
                HT_LP_Record_Entry_Set(&current_table_element, inner_bucket_key, offset, ht->records, value,
                                       value_size);
                goto __EXIT;
            }
            inner_probe = (inner_probe + 1U) % inner_buckets;
        }
        outer_probe = (outer_probe + 1U) % outer_buckets;
        wrapped = (uint8_t) (outer_probe == outer_bucket);
    }
    return EHTFULL;
    __EXIT:
    ++ht->insertions;
    return 0;
}

int HT_LP_Try_Get_Value(HT_LP *ht, const uint32_t id, void **value_out) {
    size_t outer_buckets = ht->outer_buckets;
    uint16_t outer_bucket_key = (uint16_t) (id & 0xFFFF);
    size_t outer_bucket = ht->hash_function(&outer_bucket_key, sizeof(uint16_t), outer_buckets);
    uint16_t inner_bucket_key = (uint16_t) (id >> BITS(uint16_t));
    size_t inner_bucket = ht->hash_function(&inner_bucket_key, sizeof(uint16_t), ht->inner_buckets[outer_bucket]);
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
        size_t inner_buckets = ht->inner_buckets[outer_probe];
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
    --ht->insertions;
    return 0;
}

int HT_LP_Delete(HT_LP *ht, uint32_t id) {
    if (ht->insertions == 0) return EHTEMPTY;
    size_t outer_buckets = ht->outer_buckets;
    uint16_t outer_bucket_key = (uint16_t) (id & 0xFFFF);
    size_t outer_bucket = ht->hash_function(&outer_bucket_key, sizeof(uint16_t), outer_buckets);
    uint16_t inner_bucket_key = (uint16_t) ((id >> BITS(uint16_t)) & 0xFFFF);
    size_t inner_bucket = ht->hash_function(&inner_bucket_key, sizeof(uint16_t), ht->inner_buckets[outer_bucket]);
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
        size_t inner_buckets = ht->inner_buckets[outer_probe];
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