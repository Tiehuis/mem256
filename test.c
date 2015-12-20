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
    return rop->limb[3] == a &&
        rop->limb[2] == b &&
        rop->limb[1] == c &&
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

    if (!count)
        printf("All tests passed\n");
    else
        printf("%d tests failed\n", count);
}
