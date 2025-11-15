/*
 * mcrypt.c
 *
 * Author:  Munkh-Orgil Jargalsaikhan
 * Date:    2025-11-14
 * Version: 1.3
 *
 * Main program for keystream-based file translation.
 *
 * Usage: mcrypt key-file in-file [ out-file | - ]
 *
 * Behavior:
 * - EVERY byte is translated using the keystream (XOR).
 * - When output is '-', non-ASCII bytes are printed as two-digit lowercase hex.
 * - Printable ASCII characters (including space, punctuation) are printed as-is.
 * - Newline is printed as '\n'.
 * - When writing to a file, raw binary bytes are written.
 */

#include "KStream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

/**
 * Print usage message to stderr.
 */
static void usage(void)
{
    (void)fprintf(stderr,
        "usage: mcrypt key-file in-file [ out-file | - ]\n");
}

/**
 * Read an 8-byte unsigned long key from a binary file.
 *
 * @param path Path to key file
 * @return Key value on success, -1 on error
 */
static unsigned long read_keyfile(const char * path)
{
    FILE * f = fopen(path, "rb");
    if (f == NULL) {
        (void)fprintf(stderr,
            "error: cannot open key file '%s': %s\n",
            path, strerror(errno));
        return (unsigned long)-1;
    }

    unsigned long key = 0UL;
    size_t got = fread(&key, sizeof(key), 1u, f);

    if (got != 1u) {
        if (feof(f)) {
            (void)fprintf(stderr,
                "error: key file '%s' too short\n", path);
        } else {
            (void)fprintf(stderr,
                "error: reading key file '%s'\n", path);
        }
        (void)fclose(f);
        return (unsigned long)-1;
    }

    (void)fclose(f);
    return key;
}

/**
 * Print a single byte to stdout in human-readable form.
 * - Printable ASCII → as character
 * - Non-printable, non-ASCII → lowercase hex (e.g. 0a → "0a")
 *
 * @param b Byte to print
 */
static void print_byte_stdout(unsigned char b)
{
    if (isascii(b) && isprint(b)) {
        (void)putchar((int)b);
    } else {
        (void)printf("%02x", (unsigned int)b);
    }
}

int main(int argc, char * argv[])
{
    if (argc != 4) {
        usage();
        return EXIT_FAILURE;
    }

    const char * keypath = argv[1];
    const char * inpath  = argv[2];
    const char * outpath = argv[3];

    unsigned long key = read_keyfile(keypath);
    if (key == (unsigned long)-1) {
        return EXIT_FAILURE;
    }

    FILE * inf = fopen(inpath, "rb");
    if (inf == NULL) {
        (void)fprintf(stderr,
            "error: cannot open input file '%s': %s\n",
            inpath, strerror(errno));
        return EXIT_FAILURE;
    }

    int to_stdout = (strcmp(outpath, "-") == 0);
    FILE * outf = NULL;
    if (!to_stdout) {
        outf = fopen(outpath, "wb");
        if (outf == NULL) {
            (void)fprintf(stderr,
                "error: cannot open output file '%s': %s\n",
                outpath, strerror(errno));
            (void)fclose(inf);
            return EXIT_FAILURE;
        }
    }

    KStream * ks = ks_create(key);
    if (ks == NULL) {
        (void)fprintf(stderr, "error: failed to initialize keystream\n");
        (void)fclose(inf);
        if (outf != NULL) {
            (void)fclose(outf);
        }
        return EXIT_FAILURE;
    }

    /* Buffer for efficient translation */
    #define BUF_SIZE 4096
    unsigned char inbuf[BUF_SIZE];
    unsigned char outbuf[BUF_SIZE];
    size_t bytes_read;

    /* Process file in chunks */
    while ((bytes_read = fread(inbuf, 1, BUF_SIZE, inf)) > 0) {
        /* Translate EVERY byte */
        ks_translate(ks, inbuf, outbuf, bytes_read);

        if (to_stdout) {
            for (size_t i = 0; i < bytes_read; ++i) {
                print_byte_stdout(outbuf[i]);
            }
        } else {
            (void)fwrite(outbuf, 1, bytes_read, outf);
        }
    }

    /* Cleanup */
    ks_destroy(ks);
    (void)fclose(inf);
    if (outf != NULL) {
        (void)fclose(outf);
    }

    return EXIT_SUCCESS;
}