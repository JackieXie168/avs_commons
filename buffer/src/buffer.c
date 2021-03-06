/*
 * AVSystem Commons Library
 *
 * Copyright (C) 2014 AVSystem <http://www.avsystem.com/>
 *
 * This code is free and open source software licensed under the MIT License.
 * See the LICENSE file for details.
 */

#include <config.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <avsystem/commons/buffer.h>

#define MODULE_NAME avs_buffer
#include <x_log_config.h>

#ifdef HAVE_VISIBILITY
#pragma GCC visibility push(hidden)
#endif

struct avs_buffer_struct {
    size_t capacity;
    char *begin;
    char *end;
    union {
        char data[1]; /* variable length */
        avs_max_align_t align;
    } data;
};

size_t avs_buffer_space_left(avs_buffer_t *buffer) {
    return buffer->capacity - avs_buffer_data_size(buffer);
}

static size_t space_left_without_moving(avs_buffer_t *buffer) {
    return buffer->capacity - (size_t)(buffer->end - buffer->data.data);
}

void avs_buffer_reset(avs_buffer_t *buffer) {
    buffer->begin = buffer->data.data;
    buffer->end = buffer->data.data;
}

int avs_buffer_create(avs_buffer_t **buffer_ptr, size_t capacity) {
    *buffer_ptr = (avs_buffer_t *)
            malloc(offsetof(avs_buffer_t, data) + capacity);
    if (*buffer_ptr) {
        (*buffer_ptr)->capacity = capacity;
        avs_buffer_reset(*buffer_ptr);
        return 0;
    } else {
        LOG(ERROR, "cannot allocate buffer");
        return -1;
    }
}

void avs_buffer_free(avs_buffer_t **buffer) {
    free(*buffer);
    *buffer = NULL;
}

size_t avs_buffer_data_size(avs_buffer_t *buffer) {
    return (size_t)(buffer->end - buffer->begin);
}

size_t avs_buffer_capacity(avs_buffer_t *buffer) {
    return buffer->capacity;
}

char *avs_buffer_data(avs_buffer_t *buffer) {
    return buffer->begin;
}

static void defragment_buffer(avs_buffer_t *buffer) {
    if (buffer->begin != buffer->data.data) {
        size_t used = avs_buffer_data_size(buffer);
        memmove(buffer->data.data, buffer->begin, used);
        buffer->end = buffer->data.data + used;
        buffer->begin = buffer->data.data;
    }
}

char *avs_buffer_raw_insert_ptr(avs_buffer_t *buffer) {
    defragment_buffer(buffer);
    return buffer->end;
}

int avs_buffer_consume_bytes(avs_buffer_t *buffer, size_t bytes_count) {
    if (bytes_count > avs_buffer_data_size(buffer)) {
        LOG(ERROR, "not enough data");
        return -1;
    }
    buffer->begin += bytes_count;

    return 0;
}

int avs_buffer_append_bytes(avs_buffer_t *buffer,
                            const void *data,
                            size_t data_length) {
    if (data_length > avs_buffer_space_left(buffer)) {
        LOG(ERROR, "buffer too small");
        return -1;
    } else {
        if (data_length > space_left_without_moving(buffer)) {
            defragment_buffer(buffer);
        }
        memcpy(buffer->end, data, data_length);
        buffer->end += data_length;
        return 0;
    }
}

int avs_buffer_advance_ptr(avs_buffer_t *buffer, size_t n) {
    if (n > avs_buffer_space_left(buffer)) {
        LOG(ERROR, "position out of bounds");
        return -1;
    } else {
        if (n > space_left_without_moving(buffer)) {
            defragment_buffer(buffer);
        }
        buffer->end += n;
        return 0;
    }
}

int avs_buffer_fill_bytes(avs_buffer_t *buffer, int value, size_t bytes_count) {
    if (bytes_count > avs_buffer_space_left(buffer)) {
        return -1;
    } else {
        if (bytes_count > space_left_without_moving(buffer)) {
            defragment_buffer(buffer);
        }
        memset(buffer->end, value, bytes_count);
        buffer->end += bytes_count;
        return 0;
    }
}

#ifdef AVS_UNIT_TESTING
#include "test/test_buffer.c"
#endif
