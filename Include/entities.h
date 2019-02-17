//
// Created by gliontos on 2/13/19.
//

#ifndef DBMS_ENTITIES_H
#define DBMS_ENTITIES_H

#include <stdint.h>

// NOTE(gliontos): Use uint32_t for ID for now. Maybe we will create 128-bit keys in the future
typedef struct record {
    uint32_t id;
    char song_name[32];
    char artist_name[32];
    uint32_t duration;
} Record;

/**
 * create_record - Creates a new record with the specified parameters
 * @param song_name The name of the song
 * @param artist_name The artist name
 * @param duration The duration of the song
 * @return Returns a new record with values the parameters specified
 */
Record create_record(uint32_t id, const char *restrict song_name, const char *restrict artist_name, uint32_t duration);

/**
 * print_record - Prints a given record
 * @param record A pointer to a record to print
 */
void print_record(const Record *record);

#endif //DBMS_ENTITIES_H
