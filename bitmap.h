//
// Created by gliontos on 2/17/19.
//

#ifndef DBMS_BITMAP_H
#define DBMS_BITMAP_H

#include "common_types.h"

typedef struct Bitmap {
    uint64_t *_bitmap;
} Bitmap_t;

int create_bitmap(Bitmap_t *bitmap_out, size_t bits);

void delete_bitmap(Bitmap_t *bitmap);

// NOTE(gliontos): positioning here starts from 1 and not from 0

uint8_t get_bit(Bitmap_t *bitmap, size_t pos);

void set_bit(Bitmap_t *bitmap, size_t pos);

void clear_bit(Bitmap_t *bitmap, size_t pos);

void toggle_bit(Bitmap_t *bitmap, size_t pos);

#endif //DBMS_BITMAP_H
