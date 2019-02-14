//
// Created by gliontos on 2/13/19.
//

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "entities.h"
#include "utils.h"

Record create_record(uint64_t id, const char *restrict const song_name,
                     const char *restrict const artist_name, uint32_t duration) {
    Record record = (Record) {.id = id, .duration = duration};
    STR_COPY(record.song_name, song_name, strlen(song_name));
    STR_COPY(record.artist_name, artist_name, strlen(artist_name));
    return record;
}

void print_record(const Record *record) {
    printf("Record:\n"
           "\tID: %" PRIu64 "\n"
           "\tSong Name: %s\n"
           "\tArtist Name: %s\n"
           "\tDuration: %" PRIu32 " sec\n",
           record->id, record->song_name,
           record->artist_name, record->duration);
}