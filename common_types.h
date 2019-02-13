//
// Created by gliontos on 2/13/19.
//

#ifndef DBMS_COMMON_TYPES_H
#define DBMS_COMMON_TYPES_H

#include <stdint.h>

// A typedef of function pointer that represents a generic hash function
typedef uint64_t (*Hash_Function)(void *data, size_t bytes);

#endif //DBMS_COMMON_TYPES_H
