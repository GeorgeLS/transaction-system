//
// Created by gliontos on 2/13/19.
//

#ifndef DBMS_COMMON_TYPES_H
#define DBMS_COMMON_TYPES_H

#include <stdint.h>

// A typedef of function pointer that represents a generic hash function
typedef size_t (*Hash_Function)(void *data, size_t bytes, size_t buckets);

// A macro that generates an anonymous struct with two fields of the specified type
#define Pair(T1, T2) \
struct { \
    T1 v1; \
    T2 v2; \
} \

#endif //DBMS_COMMON_TYPES_H
