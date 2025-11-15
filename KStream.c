/*
 * KStream.c
 *
 * Author: Munkh-Orgil Jargalsaikhan
 * Date:   2025-11-14
 * Version: 1.1
 *
 * Implementation of the KStream ADT declared in KStream.h.
 *
 * Internal helpers and data are file-scoped (static).
 */

#include "KStream.h"

#include <stdlib.h>  /* malloc, free */
#include <string.h>  /* memset, memcpy */
#include <stdint.h>  /* uint8_t */
#include <assert.h>

typedef unsigned char byte; /**< 8-bit byte type alias */

/**
 * KStream - internal representation.
 *
 * S:    permutation array (256 bytes).
 * i, j: indices used by the generator.
 * key: key bytes (8 bytes) - stored directly as read from file.
 */
struct KStream {
    byte S[256];
    unsigned int i;
    unsigned int j;
    byte key[8];
};

/* Forward declarations for static helpers */
static void ks_init_state(KStream * ks);
static byte ks_next_byte(KStream * ks);

static void ks_init_state(KStream * ks)
{
    assert(ks != NULL);

    for (unsigned int v = 0u; v < 256u; ++v) {
        ks->S[v] = (byte)v;
    }

    unsigned int j = 0u;
    for (unsigned int i = 0u; i < 256u; ++i) {
        j = (j + ks->S[i] + ks->key[i % 8u]) & 0xFFu;
        byte tmp = ks->S[i];
        ks->S[i] = ks->S[j];
        ks->S[j] = tmp;
    }

    ks->i = 0u;
    ks->j = 0u;
}

static byte ks_next_byte(KStream * ks)
{
    ks->i = (ks->i + 1u) & 0xFFu;
    ks->j = (ks->j + ks->S[ks->i]) & 0xFFu;

    byte tmp = ks->S[ks->i];
    ks->S[ks->i] = ks->S[ks->j];
    ks->S[ks->j] = tmp;

    byte B = ks->S[(ks->S[ks->i] + ks->S[ks->j]) & 0xFFu];
    return B;
}

KStream * ks_create(const unsigned char key_bytes[8])
{
    KStream * ks = (KStream *)malloc(sizeof(*ks));
    if (ks == NULL) {
        return NULL;
    }

    /* Store key bytes in REVERSE order (interpret as big-endian) */
    for (int i = 0; i < 8; ++i) {
        ks->key[i] = key_bytes[7 - i];
    }
    
    ks_init_state(ks);
    
    /* Prime the keystream by discarding first 1024 bytes */
    for (int n = 0; n < 1024; ++n) {
        (void)ks_next_byte(ks);
    }
    return ks;
}

void ks_destroy(KStream * ks)
{
    if (ks == NULL) {
        return;
    }

    /* Wipe sensitive data before free */
    memset(ks->S, 0, sizeof(ks->S));
    memset(ks->key, 0, sizeof(ks->key));
    ks->i = 0u;
    ks->j = 0u;
    free(ks);
}

void ks_translate(KStream * ks,
                  const unsigned char * inbuf,
                  unsigned char * outbuf,
                  size_t len)
{
    if (ks == NULL || len == 0u) {
        return;
    }
    assert(inbuf != NULL);
    assert(outbuf != NULL);

    for (size_t idx = 0u; idx < len; ++idx) {
        byte k = ks_next_byte(ks);
        outbuf[idx] = (unsigned char)(inbuf[idx] ^ k);
    }
}