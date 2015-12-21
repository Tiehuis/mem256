#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "mem256.h"

void mem256_set(mem256_t *rop, uint64_t a, uint64_t b, uint64_t c, uint64_t d)
{
    rop->limb[3] = a;
    rop->limb[2] = b;
    rop->limb[1] = c;
    rop->limb[0] = d;
}

void mem256_seta(mem256_t *rop, uint64_t value)
{
    mem256_set(rop, value, value, value, value);
}

bool mem256_eq(mem256_t *rop, uint64_t a, uint64_t b, uint64_t c, uint64_t d)
{
    return rop->limb[3] == a && rop->limb[2] == b && rop->limb[1] == c &&
                rop->limb[0] == d;
}

bool mem256_eqa(mem256_t *rop, uint64_t value)
{
    return mem256_eq(rop, value, value, value, value);
}

#define asserteq(function, x, s, a, b, c, d)                                  \
    do {                                                                      \
        function(x, s);                                                       \
        if (!mem256_eq(x, a, b, c, d)) {                                      \
            count++;                                                          \
            fprintf(stderr,                                                   \
                "Assert: Expected (%llu, %llu, %llu, %llu) but found"         \
                " (%llu, %llu, %llu, %llu)\n"                                 \
                "    Function: '%s(%s, %s)'\n\n",                             \
                a, b, c, d, (x)->limb[3], (x)->limb[2],                       \
                (x)->limb[1], (x)->limb[0], #function, #x, #s);               \
        }                                                                     \
    } while (0)

#define asserteq2(function, x, e)                                             \
    do {                                                                      \
        const int g = function(x);                                            \
        if (g != (e)) {                                                       \
            count++;                                                          \
            fprintf(stderr,                                                   \
                "Assert: Expected %llu but found %llu\n"                      \
                "   Function: '%s(%s)'\n\n",                                  \
                e, g, #function, #x);                                         \
        }                                                                     \
    } while (0)

#define asserteq3(function, x, s, e, a, b, c, d)                              \
    do {                                                                      \
        function(x, s, e);                                                    \
        if (!mem256_eq(x, a, b, c, d)) {                                      \
            count++;                                                          \
            fprintf(stderr,                                                   \
                "Assert: Expected (%llu, %llu, %llu, %llu) but found"         \
                " (%llu, %llu, %llu, %llu)\n"                                 \
                "    Function: '%s(%s, %s, %s)'\n\n",                         \
                a, b, c, d, (x)->limb[3], (x)->limb[2],                       \
                (x)->limb[1], (x)->limb[0], #function, #x, #s, #e);           \
        }                                                                     \
    } while (0)

/* 1 << 63 */
#define _BN 9223372036854775808ull

int main(void)
{
    int count = 0;

    mem256_t a;

    /* Left Shifts */

    /* Check: [0, 64) bit shifts */
    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 0, 1, 1, 1, 1);

    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 1, 2, 2, 2, 2);

    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 63, _BN, _BN, _BN, _BN);

    /* Check: [64, 128) bit shifts */
    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 64, 1, 1, 1, 0);

    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 65, 2, 2, 2, 0);

    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 127, _BN, _BN, _BN, 0);

    /* Check: [128, 192) bit shifts */
    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 128, 1, 1, 0, 0);

    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 129, 2, 2, 0, 0);

    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 130, 4, 4, 0, 0);

    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 191, _BN, _BN, 0, 0);

    /* Check: [192, 256) bit shifts */
    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 192, 1, 0, 0, 0);

    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 193, 2, 0, 0, 0);

    mem256_seta(&a, 1);
    asserteq(mem256_lshift, &a, 255, _BN, 0, 0, 0);

    /* Right Shifts */
    /* Check: [0, 64) bit shifts */
    mem256_seta(&a, 1);
    asserteq(mem256_rshift, &a, 0, 1, 1, 1, 1);

    mem256_seta(&a, 2);
    asserteq(mem256_rshift, &a, 1, 1, 1, 1, 1);

    mem256_seta(&a, _BN);
    asserteq(mem256_rshift, &a, 63, 1, 1, 1, 1);

    /* Check: [64, 128) bit shifts */
    mem256_seta(&a, 1);
    asserteq(mem256_rshift, &a, 64, 0, 1, 1, 1);

    mem256_seta(&a, 2);
    asserteq(mem256_rshift, &a, 65, 0, 1, 1, 1);

    mem256_seta(&a, _BN);
    asserteq(mem256_rshift, &a, 127, 0, 1, 1, 1);

    /* Check: [128, 192) bit shifts */
    mem256_seta(&a, 1);
    asserteq(mem256_rshift, &a, 128, 0, 0, 1, 1);

    mem256_seta(&a, 2);
    asserteq(mem256_rshift, &a, 129, 0, 0, 1, 1);

    mem256_seta(&a, _BN);
    asserteq(mem256_rshift, &a, 191, 0, 0, 1, 1);

    /* Check: [192, 256) bit shifts */
    mem256_seta(&a, 1);
    asserteq(mem256_rshift, &a, 192, 0, 0, 0, 1);

    mem256_seta(&a, 2);
    asserteq(mem256_rshift, &a, 193, 0, 0, 0, 1);

    mem256_seta(&a, _BN);
    asserteq(mem256_rshift, &a, 255, 0, 0, 0, 1);

    /* Check: popcnt function */
    mem256_seta(&a, 1);
    asserteq2(mem256_popcnt, &a, 4);

    mem256_seta(&a, _BN);
    asserteq2(mem256_popcnt, &a, 4);

    mem256_seta(&a, ((uint64_t) 1 << 16) - 1);
    asserteq2(mem256_popcnt, &a, 64);

    mem256_seta(&a, ((uint64_t) 1 << 32) - 1);
    asserteq2(mem256_popcnt, &a, 128);

    /* Check: highbit function */
    mem256_seta(&a, 0);
    asserteq2(mem256_highbit, &a, 0);

    mem256_seta(&a, 0);
    a.limb[0] = 1;
    asserteq2(mem256_highbit, &a, 1);

    mem256_seta(&a, 0);
    a.limb[0] = ((uint64_t) 1 << 62);
    asserteq2(mem256_highbit, &a, 63);

    mem256_seta(&a, 0);
    a.limb[1] = ((uint64_t) 1 << 62);
    asserteq2(mem256_highbit, &a, 127);

    mem256_seta(&a, 1);
    asserteq2(mem256_highbit, &a, 193);

    a.limb[3] = ((uint64_t) 1 << 62);
    asserteq2(mem256_highbit, &a, 255);

    /* Check: fillones function */
    mem256_seta(&a, 0);
    asserteq3(mem256_fillones, &a, 0, 1, 0, 0, 0, 1);

    mem256_seta(&a, 0);
    asserteq3(mem256_fillones, &a, 64, 65, 0, 0, 1, 0);

    mem256_seta(&a, 0);
    asserteq3(mem256_fillones, &a, 128, 129, 0, 1, 0, 0);

    mem256_seta(&a, 0);
    asserteq3(mem256_fillones, &a, 192, 193, 1, 0, 0, 0);

    if (!count)
        printf("All tests passed\n");
    else
        printf("%d tests failed\n", count);
}
