/*
 * KStream.h
 *
 * Author: Munkh-Orgil Jargalsaikhan
 * Date:   2025-11-14
 * Version: 1.0
 *
 * Keystream ADT interface for an RC4-like stream cipher.
 *
 * Design:
 *  - ks_create accepts an unsigned long key (client reads key file).
 *  - ks_translate requires client-supplied input and output buffers.
 *
 * Build with: gcc -std=c99 -Wall -Wextra -pedantic -Werror
 */

#ifndef KSTREAM_H
#define KSTREAM_H

#include <stddef.h> /* size_t */

/**
 * Opaque keystream type.
 *
 * The internal structure is hidden from clients.  Use the public
 * functions to create, use, and destroy instances.
 */
typedef struct KStream KStream;

/**
 * ks_create - construct and initialize a KStream instance.
 *
 * @key:  unsigned long key value (interpreted as eight bytes,
 *        little-endian).
 *
 * Returns: pointer to a newly allocated KStream on success, or NULL
 *          if memory allocation fails.
 *
 * @pre: client has read the key value from a key file and supplies it.
 * @post: returned KStream is initialized and primed (first 1024 bytes
 *        discarded) and ready for translation.
 */
KStream * ks_create(unsigned long key);

/**
 * ks_destroy - free storage associated with a KStream instance.
 *
 * @ks: pointer returned by ks_create.
 *
 * @pre: ks is a valid pointer returned by ks_create or NULL.
 * @post: memory associated with ks is wiped and freed; ks must not be
 *        used after this call.
 */
void ks_destroy(KStream * ks);

/**
 * ks_translate - translate bytes using the keystream.
 *
 * For t in [0..len-1]: outbuf[t] = inbuf[t] XOR next_keystream_byte.
 *
 * @ks:     pointer to initialized KStream instance.
 * @inbuf:  pointer to input buffer (len bytes).
 * @outbuf: pointer to output buffer (len bytes); caller allocates it.
 * @len:    number of bytes to translate.
 *
 * @pre: ks is initialized.  inbuf and outbuf are valid pointers if
 *       len > 0.
 * @post: outbuf contains translated bytes.
 */
void ks_translate(KStream * ks,
                  const unsigned char * inbuf,
                  unsigned char * outbuf,
                  size_t len);

#endif /* KSTREAM_H */
