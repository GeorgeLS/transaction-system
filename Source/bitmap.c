//
// Created by gliontos on 2/17/19.
//

#include <stdlib.h>
#include "../Include/bitmap.h"
#include "../Include/utils.h"
#include "../Include/error_enums.h"

#define BYTE_POS(pos) ((pos - 1U) >> 6U)
// That's equivalent to ((pos -1U) % 64U) but since 64 is power of 2 we can do it using bitwise operations
#define BIT_OFFSET(pos) ((pos - 1U) & 63U)

int create_bitmap(Bitmap_t *bitmap_out, size_t bits) {
    size_t units = BYTE_POS(bits) + 1U;
    uint64_t *bitmap = __MALLOC(units, uint64_t);
    if (!bitmap) return ENOMEM;
    memset(bitmap, '\0', units * sizeof(uint64_t));
    *bitmap_out = (Bitmap_t) {._bitmap = bitmap};
    return 0;
}

void delete_bitmap(Bitmap_t *bitmap) {
    free(bitmap->_bitmap);
}

uint8_t get_bit(Bitmap_t *bitmap, size_t pos) {
    return (uint8_t) ((bitmap->_bitmap[BYTE_POS(pos)] >> BIT_OFFSET(pos)) & 1U);
}

void set_bit(Bitmap_t *bitmap, size_t pos) {
    bitmap->_bitmap[BYTE_POS(pos)] |= (1U << BIT_OFFSET(pos));
}

void clear_bit(Bitmap_t *bitmap, size_t pos) {
    bitmap->_bitmap[BYTE_POS(pos)] &= ~(1U << BIT_OFFSET(pos));
}

void toggle_bit(Bitmap_t *bitmap, size_t pos) {
    bitmap->_bitmap[BYTE_POS(pos)] ^= (1U << BIT_OFFSET(pos));
}