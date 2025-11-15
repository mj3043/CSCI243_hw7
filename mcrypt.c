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
 * - Only alphabetic characters (isalpha) are translated using the
 *   keystream. Keystream advances only for translated bytes.
 * - Newline passes through unchanged.
 * - Printable characters print as themselves in stdout mode.
 * - Non-printable characters (except newline) print as lowercase hex.
 * - When writing to a file, bytes are written as raw binary.
 */

#include "KStream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

static void usage(void)
{
    (void)fprintf(stderr,
        "usage: mcrypt key-file in-file [ out-file | - ]\n");
}

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

static void print_byte_stdout(unsigned char b)
{
    if (b == (unsigned char)'\n') {
        (void)putchar('\n');
    } else if (isprint(b)) {
        (void)putchar((int)b);
    } else {
        (void)printf("%02x", b);
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

    int ch;
    unsigned char inb[1];
    unsigned char outb[1];

    while ((ch = fgetc(inf)) != EOF) {
        unsigned char c = (unsigned char)ch;

        /* newline: pass through unchanged */
        if (c == (unsigned char)'\n') {
            if (to_stdout) {
                print_byte_stdout(c);
            } else {
                (void)fputc((int)c, outf);
            }
            continue;
        }

        /* Only treat bytes <128 as ASCII; keep others untouched */
        if (c >= 128) {
            if (to_stdout) {
                print_byte_stdout(c);
            } else {
                (void)fputc((int)c, outf);
            }
            continue;
        }

        /* non-alphabetic ASCII → unchanged */
        if (!isalpha(c)) {
            if (to_stdout) {
                print_byte_stdout(c);
            } else {
                (void)fputc((int)c, outf);
            }
            continue;
        }

        /* alphabetic → translate (keystream advances once) */
        inb[0] = c;
        ks_translate(ks, inb, outb, 1u);
        unsigned char result = outb[0];

        if (to_stdout) {
            print_byte_stdout(result);
        } else {
            (void)fputc((int)result, outf);
        }
    }

    ks_destroy(ks);
    (void)fclose(inf);
    if (outf != NULL) {
        (void)fclose(outf);
    }

    return EXIT_SUCCESS;
}
