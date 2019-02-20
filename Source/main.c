#include <stdlib.h>
#include <stdio.h>
#include "../Include/entities.h"
#include "../Include/hash_table_lp.h"
#include "../Include/utils.h"

#define RECORDS_N 1000

uint32_t ids[RECORDS_N];

int main() {
    HT_LP hash_table;
    HT_LP_Create(&hash_table, RECORDS_N, &hash_function);
    for (size_t i = 0U; i != RECORDS_N; ++i) {
        char song_name[32] = {0};
        char artist_name[32] = {0};
        snprintf(song_name, sizeof(song_name), "Song_Name_%zu", i);
        snprintf(artist_name, sizeof(artist_name), "Artist_Name_%zu", i);
        ids[i] = generate_id();
        Record record = create_record(ids[i], song_name, artist_name, 90);
        HT_LP_Insert(&hash_table, ids[i], &record, sizeof(Record));
    }
    printf("======================================= AFTER INSERT =======================================\n");
    for (size_t i = 0U; i != RECORDS_N; ++i) {
        Record *record;
        int res = HT_LP_Try_Get_Value(&hash_table, ids[i], (void **) &record);
        if (res) {
            fprintf(stderr, "Couldn't find the record\n");
        } else {
            print_record(record);
        }
    }
    for (size_t i = 0U; i != RECORDS_N - 10U; ++i) {
        HT_LP_Delete(&hash_table, ids[i]);
    }
    printf("======================================= AFTER DELETE =======================================\n");
    for (size_t i = 0U; i != RECORDS_N; ++i) {
        Record *record;
        int res = HT_LP_Try_Get_Value(&hash_table, ids[i], (void **) &record);
        if (res) {
            fprintf(stderr, "Couldn't find the record\n");
        } else {
            print_record(record);
        }
    }
    HT_LP_Free(&hash_table);
    return EXIT_SUCCESS;
}