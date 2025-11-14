/*
 * mcrypt.c
 *
 * Author:  Munkh-Orgil Jargalsaikhan
 * Date:    2025-11-14
 * Version: 1.1
 *
 * Main program for keystream-based file translation.
 *
 * Usage: mcrypt key-file in-file [ out-file | - ]
 *
 * If out-file is '-', output is printed to stdout. Printable characters
 * are printed as-is; non-printables are printed as two lowercase hex
 * digits.
 */

#include "KStream.h"

#include <stdio.h>     /* FILE, fopen, fread, fwrite, putchar, printf */
#include <stdlib.h>    /* EXIT_SUCCESS, EXIT_FAILURE, malloc, free    */
#include <string.h>    /* strcmp, strerror                            */
#include <errno.h>     /* errno                                        */
#include <ctype.h>     /* isprint                                      */
#include <stdint.h>    /* uint8_t                                      */

/* Print correct usage message. */
static void usage(void)
{
    (void)fprintf(stderr,
        "usage: mcrypt key-file in-file [ out-file | - ]\n");
}

/**
 * read_keyfile - read an unsigned long key from a binary key file.
 *
 * @path: path to key file
 *
 * Returns: key on success, or (unsigned long)-1 on error.
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
        (void)fprintf(stderr,
            "error: failed to initialize keystream\n");
        (void)fclose(inf);
        if (outf != NULL) {
            (void)fclose(outf);
        }
        return EXIT_FAILURE;
    }

    enum { CHUNK = 4096 };
    unsigned char * inbuf  = malloc(CHUNK);
    unsigned char * outbuf = malloc(CHUNK);

    if (inbuf == NULL || outbuf == NULL) {
        (void)fprintf(stderr, "error: memory allocation failed\n");
        free(inbuf);
        free(outbuf);
        ks_destroy(ks);
        (void)fclose(inf);
        if (outf != NULL) {
            (void)fclose(outf);
        }
        return EXIT_FAILURE;
    }

    while (1) {
        size_t nread = fread(inbuf, 1u, CHUNK, inf);
        if (nread == 0u) {
            break;
        }

        ks_translate(ks, inbuf, outbuf, nread);

        if (to_stdout) {
            for (size_t i = 0u; i < nread; ++i) {
                unsigned char b = outbuf[i];

                if (isprint((unsigned char)b)) {
                    (void)putchar((int)b);
                } else {
                    (void)printf("%02x", b);
                }
            }
            (void)fflush(stdout);
        } else {
            size_t wrote = fwrite(outbuf, 1u, nread, outf);
            if (wrote != nread) {
                (void)fprintf(stderr,
                    "error: failed to write to '%s'\n", outpath);
                free(inbuf);
                free(outbuf);
                ks_destroy(ks);
                (void)fclose(inf);
                (void)fclose(outf);
                return EXIT_FAILURE;
            }
        }

        if (nread < (size_t)CHUNK) {
            break;
        }
    }

    free(inbuf);
    free(outbuf);
    ks_destroy(ks);
    (void)fclose(inf);

    if (outf != NULL) {
        (void)fclose(outf);
    }

    return EXIT_SUCCESS;
}
