// Core/Src/randombytes.c
#include <stddef.h>
#include <stdint.h>

int randombytes(uint8_t *out, size_t outlen) {
    // For deterministic testing or SCA profiling, fill with a fixed seed.
    // Or wire up ST's HAL_RNG_GenerateRandomNumber() here.
    for (size_t i = 0; i < outlen; i++) {
        out[i] = (uint8_t)(i & 0xFF);
    }
    return 0;
}
