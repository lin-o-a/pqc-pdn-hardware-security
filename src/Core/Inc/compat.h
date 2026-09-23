#ifndef COMPAT_H
#define COMPAT_H

#include <stdint.h>

// PQClean uses these for constant-time comparisons across C standards
static inline int crypto_verify_32(const uint8_t *x, const uint8_t *y) {
    uint32_t differentbits = 0;
    for (int i = 0; i < 32; i++) {
        differentbits |= x[i] ^ y[i];
    }
    return (int)(1 & ((differentbits - 1) >> 8));
}

#endif

#ifndef PQCLEAN_PREVENT_BRANCH_HACK
#define PQCLEAN_PREVENT_BRANCH_HACK(val) __asm__("" : "+r"(val))
#endif
