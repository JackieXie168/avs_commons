/*
 * AVSystem Commons Library
 *
 * Copyright (C) 2014 AVSystem <http://www.avsystem.com/>
 *
 * This code is free and open source software licensed under the MIT License.
 * See the LICENSE file for details.
 */

#ifndef AVS_COMMONS_BUFFER_H
#define AVS_COMMONS_BUFFER_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @file buffer.h
 *
 * Implementation of a byte buffer with circular semantics.
 */

struct avs_buffer_struct;
typedef struct avs_buffer_struct avs_buffer_t;
/**<
 * Circular byte buffer object type.
 */

/**
 * Allocates a new buffer with a specified size (capacity).
 *
 * @param buffer Pointer to a variable which will be updated with the newly
 *               allocated buffer object.
 *
 * @param size   Desired capacity of the buffer, in bytes.
 *
 * @return 0 for success, or -1 in case of error.
 */
int avs_buffer_create(avs_buffer_t **buffer, size_t size);

/**
 * Destroys a buffer object, freeing any used resources.
 *
 * @param buffer Pointer to a variable containing a buffer to free. It will be
 *               reset to <c>NULL</c> afterwards.
 */
void avs_buffer_free(avs_buffer_t **buffer);

/**
 * Clears the buffer, making all its capacity available to data.
 *
 * @param buffer Buffer object to operate on.
 */
void avs_buffer_reset(avs_buffer_t *buffer);

/**
 * Returns the amount of data currently contained in the buffer.
 *
 * @param buffer Buffer object to operate on.
 *
 * @return Number of bytes ready to consume in the buffer.
 */
size_t avs_buffer_data_size(avs_buffer_t *buffer);

/**
 * Returns the capacity of the buffer.
 *
 * @param buffer Buffer object to operate on.
 *
 * @return Total number of all bytes usable by the buffer.
 */
size_t avs_buffer_capacity(avs_buffer_t *buffer);

/**
 * Returns the size of free space in the buffer.
 *
 * @param buffer Buffer object to operate on.
 *
 * @return Number of bytes that can currently be appended to the buffer.
 */
size_t avs_buffer_space_left(avs_buffer_t *buffer);

/**
 * Returns a raw pointer to consumable data in the buffer.
 *
 * @param buffer Buffer object to operate on.
 *
 * @return Pointer to a contiguous array of @ref avs_buffer_data_size bytes of
 *         data that has been appended but not yet consumed.
 */
char *avs_buffer_data(avs_buffer_t *buffer);

/**
 * Return a raw pointer to free space in the buffer.
 *
 * This can be used to pass the buffer to external functions such as receiving
 * from a network socket.
 *
 * After filling the buffer, @ref avs_buffer_advance_ptr shall be called with
 * the number of bytes filled.
 *
 * @param buffer Buffer object to operate on.
 *
 * @return Pointer to a contiguous array of @ref avs_buffer_space_left bytes
 *         (which may already contain some bogus data) that can be filled with
 *         new input data.
 */
char *avs_buffer_raw_insert_ptr(avs_buffer_t *buffer);

/**
 * Marks some amount of data as consumed, freeing portion of the available
 * capacity.
 *
 * @param buffer      Buffer object to operate on.
 *
 * @param bytes_count Number of bytes to mark as consumed.
 *
 * @return 0 for success, or -1 in case of error (not enough data in buffer).
 */
int avs_buffer_consume_bytes(avs_buffer_t *buffer, size_t bytes_count);

/**
 * Appends bytes to the end of the buffer, making them (eventually) available
 * for consumption.
 *
 * Provided that the arguments are valid, it is semantically equivalent to:
 *
 * @code
 * memcpy(avs_buffer_raw_insert_ptr(buffer), data, data_length);
 * avs_buffer_advance_ptr(buffer, data_length);
 * @endcode
 *
 * However, in many cases this function will execute faster than the above
 * block, so it is preferable to use it whenever possible.
 *
 * @param buffer      Buffer object to operate on.
 *
 * @param data        Pointer to data to append.
 *
 * @param data_length Number of bytes to append.
 *
 * @return 0 for success, or -1 in case of error (not enough free space in
 *         buffer).
 */
int avs_buffer_append_bytes(avs_buffer_t *buffer,
                            const void *data,
                            size_t data_length);

/**
 * Marks some amount of data as appended.
 *
 * This function is complementary to @ref avs_buffer_raw_insert_ptr, both
 * functions together can be used to replicate functionality of
 * @ref avs_buffer_append_bytes in cases where raw pointer to input data cannot
 * be obtained.
 *
 * @param buffer Buffer object to operate on.
 *
 * @param count  Number of bytes to mark as appended.
 *
 * @return 0 for success, or -1 in case of error (not enough free space in
 *         buffer).
 */
int avs_buffer_advance_ptr(avs_buffer_t *buffer, size_t count);

/**
 * Appends a number of bytes with a specified value to the buffer.
 *
 * Provided that the arguments are valid, it is semantically equivalent to:
 *
 * @code
 * memset(avs_buffer_raw_insert_ptr(buffer), value, bytes_count);
 * avs_buffer_advance_ptr(buffer, bytes_count);
 * @endcode
 *
 * However, in many cases this function will execute faster than the above
 * block, so it is preferable to use it whenever possible.
 *
 * @param buffer      Buffer object to operate on.
 *
 * @param value       <c>unsigned char</c> value of constant byte to fill the
 *                    data, cast to <c>int</c>. This usage is analogous to the
 *                    interface used by <c>memset()</c>.
 *
 * @param bytes_count Number of bytes to append.
 *
 * @return 0 for success, or -1 in case of error (not enough free space in
 *         buffer).
 */
int avs_buffer_fill_bytes(avs_buffer_t *buffer, int value, size_t bytes_count);

#ifdef	__cplusplus
}
#endif

#endif	/* AVS_COMMONS_BUFFER_H */
