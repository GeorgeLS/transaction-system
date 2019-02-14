#include <stdlib.h>
#include <stdio.h>
#include "entities.h"
#include "hash_table_lp.h"
#include "utils.h"

#define RECORDS_N 1000

uint64_t _id;

int main() {
    HT_LP hash_table;
    size_t buckets = calculate_hash_table_size(RECORDS_N);
    int res = HT_LP_Create(&hash_table, buckets, &hash_function);
    for (size_t i = 0U; i != RECORDS_N; ++i) {
        char song_name[32];
        char artist_name[32];
        snprintf(song_name, sizeof(song_name), "Song_Name_%zu", i);
        snprintf(artist_name, sizeof(artist_name), "Artist_Name_%zu", i);
        Record record = create_record(_id++, song_name, artist_name, 90);
        HT_LP_Insert(&hash_table, &record);
    }
    printf("======================================= AFTER INSERT =======================================\n");
    for (size_t i = 0U; i != RECORDS_N; ++i) {
        Record *record;
        int res = HT_LP_Try_Get_Value(&hash_table, i, (void **) &record);
        if (res) {
            fprintf(stderr, "Couldn't find the record\n");
        } else {
            print_record(record);
        }
    }
    HT_LP_Delete(&hash_table, 9);
    printf("======================================= AFTER DELETE =======================================\n");
    for (size_t i = 0U; i != RECORDS_N; ++i) {
        Record *record;
        int res = HT_LP_Try_Get_Value(&hash_table, i, (void **) &record);
        if (res) {
            fprintf(stderr, "Couldn't find the record\n");
        } else {
            print_record(record);
        }
    }
    HT_LP_Free(&hash_table);
    return EXIT_SUCCESS;
}