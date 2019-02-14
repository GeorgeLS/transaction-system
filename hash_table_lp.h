//
// Created by gliontos on 2/13/19.
//

/**
 * A generic hash table that resolves collisions with linear probing
 */

#ifndef DBMS_HASH_TABLE_LP_H
#define DBMS_HASH_TABLE_LP_H

#include "entities.h"
#include "common_types.h"

typedef struct {
    Pair(uint16_t, size_t) key_value_pair;
    uint8_t is_deleted : 1;
    uint8_t is_empty : 1;
} HT_LP_Record_Entry;

typedef struct {
    HT_LP_Record_Entry **table;
    void **records;
    size_t buckets;
    Hash_Function hash_function;
} HT_LP;

/**
 * HT_LP_Create - Creates a new hash table with the given specifications
 * @param ht_out The address where the new hash table will be stored (output parameter)
 * @param buckets The number of buckets the hash table will have
 * @param hash_function The hash function that the hash table will use to hash it's values
 * @return On Success returns 0
 * ENOMEM or ENOHASHFUNCTION on failure (see error_enums.h for details)
 */
int HT_LP_Create(HT_LP *ht_out, size_t buckets, Hash_Function hash_function);

/**
 * HT_LP_Free - Deallocates properly the hash table passed as parameter
 * @param ht The hash table to deallocate
 */
void HT_LP_Free(HT_LP *ht);

/**
 * HT_LP_Insert - Inserts a new record to the hash table
 * @param ht The hash table
 * @param record The record to insert
 * @return On Success returns 0,
 * EHTFULL on failure (see error_enums.h for details)
 */
int HT_LP_Insert(HT_LP *ht, Record *record);

/**
 * HT_LP_Try_Get_Value - Tries to retrieve the value with the specified ID
 * @param ht The hash table
 * @param id The id to search for
 * @param value_out The address to put the result value in
 * @return On Success returns 0,
 * ENOEXISTS on failure (see error_enums.h for details)
 */
int HT_LP_Try_Get_Value(HT_LP *ht, uint64_t id, void **value_out);

/**
 * HT_LP_Delete - Deletes the record with the id specified from the hash table
 * @param ht The hash table
 * @param id The id of the record to delete
 * @return On Success returns 0,
 * ENOEXISTS on failure (see error_enums.h for details)
 */
int HT_LP_Delete(HT_LP *ht, uint64_t id);

#endif //DBMS_HASH_TABLE_LP_H
