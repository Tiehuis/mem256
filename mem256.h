/**
 * mem256.h
 *
 * Implements a 256-bit contiguous memory block with support for fast shifting
 * across the entire memory block.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

enum { _LOW, _MID_LOW, _MID_HIGH, _HIGH };

typedef struct {
    uint64_t _limb[4]; /* Least to Most Significant */
} mem256_t;

/**
 * Return the number of 1-bits set in a 64-bit integer.
 */
#if defined(__GNUC__)
#   define mem256_64popcnt(x) __builtin_popcountll(x)
#else
static int mem256_64popcnt(uint64_t x)
{
    int count = 0;
    while (x) {
        x &= x - 1;
        count++;
    }
    return count;
}
#endif

/**
 * Shift the entire 256 memory block left by the specified shift. Any shift
 * value > 255 is first truncated before being applied.
 *
 * Return true if overflow occured during shift, else false.
 */
bool mem256_lshift(mem256_t *rop, int shift)
{
    bool overflow = false;
    const int shift_type = (shift & 255) >> 6;

    /* Normalize shift into the region of a single limb */
    shift &= 63;

    /* Upper and lower region masks are shared between different cases */
    const uint64_t hmask = ((1ULL << shift) - 1) << (64 - shift);
    const uint64_t lmask = ~hmask;

    switch (shift_type) {
    case 0:; { /* Less than one limb shift: (0 <= shift < 64) */

        overflow = (rop->_limb[_HIGH] & hmask) != 0;
        rop->_limb[_HIGH] <<= shift;

        #define SHIFT_LIMB(limb)                                            \
        do {                                                                \
            const uint64_t upper = rop->_limb[(limb)] & hmask;             \
            rop->_limb[(limb) + 1] |= (upper >> (64 - shift));             \
            rop->_limb[(limb)] <<= shift;                                  \
        } while (0)

        /* This could likely be vectorized */
        SHIFT_LIMB(_MID_HIGH);
        SHIFT_LIMB(_MID_LOW);
        SHIFT_LIMB(_LOW);
        #undef SHIFT_LIMB
        break;
    }
    case 1:; { /* One limb shift: (64 <= shift < 128)  */

        overflow = rop->_limb[_HIGH] != 0 || (rop->_limb[_MID_HIGH] & hmask);

        rop->_limb[_HIGH] = (rop->_limb[_MID_HIGH] & lmask) << shift;
        rop->_limb[_MID_HIGH] = 0;

        #define SHIFT_LIMB(limb)                                            \
        do {                                                                \
            const uint64_t upper = rop->_limb[(limb)] & hmask;             \
            rop->_limb[(limb) + 2] |= (upper >> (64 - shift));             \
            const uint64_t lower = rop->_limb[(limb)] & lmask;             \
            rop->_limb[(limb) + 1] |= (lower << shift);                    \
            rop->_limb[(limb)] = 0;                                        \
        } while (0)

        SHIFT_LIMB(_MID_LOW);
        SHIFT_LIMB(_LOW);
        #undef SHIFT_LIMB
        break;
    }
    case 2:; { /* Two limb shift: (128 <= shift < 192) */

        overflow = rop->_limb[_HIGH] != 0 || rop->_limb[_MID_HIGH] != 0
                    || (rop->_limb[_MID_LOW] & hmask);
        rop->_limb[_MID_HIGH] = 0;

        rop->_limb[_HIGH] = rop->_limb[_MID_LOW] << shift;
        rop->_limb[_HIGH] |= ((rop->_limb[_LOW] & hmask) >> (64 - shift));
        rop->_limb[_MID_HIGH] |= (rop->_limb[_LOW] << shift); // THIS
        rop->_limb[_MID_LOW] = rop->_limb[_LOW] = 0;
        break;
    }
    case 3:; { /* Three limb shift: (192 <= shift < 256) */

        overflow = rop->_limb[_HIGH] != 0 || rop->_limb[_MID_HIGH] != 0
                    || rop->_limb[_MID_LOW] != 0 || (rop->_limb[_LOW] & hmask);

        rop->_limb[_HIGH] = rop->_limb[_LOW] << shift;
        rop->_limb[_MID_HIGH] = rop->_limb[_MID_LOW] = rop->_limb[_LOW] = 0;
        break;
    }
    default: /* Never reached */
        return -1;
    }

    return overflow;
}

/**
 * Shift the entire 256 memory block right by the specified shift. Any shift
 * value > 255 is first truncated before being applied.
 *
 * Return true if underflow occured during shift, else false.
 */
bool mem256_rshift(mem256_t *rop, int shift)
{
    bool overflow = false;
    const int shift_type = (shift & 255) >> 6;

    /* Normalize shift into the region of a single limb */
    shift &= 63;

    /* Upper and lower region masks are shared between different cases */
    const uint64_t lmask = (1ULL << shift) - 1;
    const uint64_t hmask = ~lmask;

    switch (shift_type) {
    case 0:; { /* Less than one limb shift: (0 <= shift < 64) */

        overflow = (rop->_limb[_LOW] & lmask) != 0;
        rop->_limb[_LOW] >>= shift;

        #define SHIFT_LIMB(limb)                                           \
        do {                                                               \
            const uint64_t lower = rop->_limb[(limb)] & lmask;             \
            rop->_limb[(limb) - 1] |= (lower << (64 - shift));             \
            rop->_limb[(limb)] >>= shift;                                  \
        } while (0)

        /* This could likely be vectorized */
        SHIFT_LIMB(_MID_LOW);
        SHIFT_LIMB(_MID_HIGH);
        SHIFT_LIMB(_HIGH);
        #undef SHIFT_LIMB
        break;
    }
    case 1:; { /* One limb shift: (64 <= shift < 128)  */

        overflow = rop->_limb[_LOW] != 0 || (rop->_limb[_MID_LOW] & lmask);

        rop->_limb[_LOW] = (rop->_limb[_MID_LOW] & hmask) >> shift;
        rop->_limb[_MID_LOW] = 0;

        #define SHIFT_LIMB(limb)                                           \
        do {                                                               \
            const uint64_t lower = rop->_limb[(limb)] & lmask;             \
            rop->_limb[(limb) - 2] |= (lower << (64 - shift));             \
            const uint64_t upper = rop->_limb[(limb)] & hmask;             \
            rop->_limb[(limb) - 1] |= (upper >> shift);                    \
            rop->_limb[(limb)] = 0;                                        \
        } while (0)

        SHIFT_LIMB(_MID_HIGH);
        SHIFT_LIMB(_HIGH);
        #undef SHIFT_LIMB
        break;
    }
    case 2:; { /* Two limb shift: (128 <= shift < 192) */

        overflow = rop->_limb[_LOW] != 0 || rop->_limb[_MID_LOW] != 0
                    || (rop->_limb[_MID_HIGH] & hmask);

        rop->_limb[_MID_LOW] = 0;
        rop->_limb[_LOW] = (rop->_limb[_MID_HIGH] & hmask) >> shift;
        rop->_limb[_LOW] |= ((rop->_limb[_HIGH] & lmask) << (64 - shift));
        rop->_limb[_MID_LOW] |= ((rop->_limb[_HIGH] & hmask) >> shift);
        rop->_limb[_HIGH] = rop->_limb[_MID_HIGH] = 0;
        break;
    }
    case 3:; { /* Three limb shift: (192 <= shift < 256) */

        overflow = rop->_limb[_LOW] != 0 || rop->_limb[_MID_LOW] != 0
                    || rop->_limb[_MID_HIGH] != 0 || (rop->_limb[_HIGH] & lmask);

        rop->_limb[_LOW] = (rop->_limb[_HIGH] & hmask) >> shift;
        rop->_limb[_MID_LOW] = rop->_limb[_MID_HIGH] = rop->_limb[_HIGH] = 0;
        break;
    }
    default: /* Never reached */
        return -1;
    }

    return overflow;
}

/**
 * A general shift function which translates negative shifts to rshifts */
bool mem256_shift(mem256_t *rop, int index)
{
    if (index == 0)
        return false;
    else if (index > 0)
        return mem256_lshift(rop, index);
    else
        return mem256_rshift(rop, -index);
}

/**
 * Return non-zero if bit at index is non-zero, else zero.
 */
int mem256_get(mem256_t *rop, int index)
{
    return rop->_limb[index >> 6] & (1ULL << (index & 63));
}

/* Return the number of set bits over the entire memory region. */
int mem256_popcnt(mem256_t *rop)
{
    return mem256_64popcnt(rop->_limb[_HIGH])
         + mem256_64popcnt(rop->_limb[_MID_HIGH])
         + mem256_64popcnt(rop->_limb[_MID_LOW])
         + mem256_64popcnt(rop->_limb[_LOW]);
}

/* Store the bitwise-or result of op1 and op2 in rop */
void mem256_ior(mem256_t *restrict rop, mem256_t *restrict op)
{

    rop->_limb[_HIGH]     |= op->_limb[_HIGH];
    rop->_limb[_MID_HIGH] |= op->_limb[_MID_HIGH];
    rop->_limb[_MID_LOW]  |= op->_limb[_MID_LOW];
    rop->_limb[_LOW]      |= op->_limb[_LOW];
}

/* Store the bitwise-and result of op1 and op2 in rop */
void mem256_and(mem256_t *restrict rop, mem256_t *restrict op)
{
    rop->_limb[_HIGH]     &= op->_limb[_HIGH];
    rop->_limb[_MID_HIGH] &= op->_limb[_MID_HIGH];
    rop->_limb[_MID_LOW]  &= op->_limb[_MID_LOW];
    rop->_limb[_LOW]      &= op->_limb[_LOW];
}

/* Store the bitwise-xor result of op1 and op2 in rop */
void mem256_xor(mem256_t *restrict rop, mem256_t *restrict op)
{
    rop->_limb[_HIGH]     ^= op->_limb[_HIGH];
    rop->_limb[_MID_HIGH] ^= op->_limb[_MID_HIGH];
    rop->_limb[_MID_LOW]  ^= op->_limb[_MID_LOW];
    rop->_limb[_LOW]      ^= op->_limb[_LOW];
}
