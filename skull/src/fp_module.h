#include <stdint.h>
#include <stdbool.h>

#define FIXED_SHIFT     12   // 2^12 = 4096

// ==========================================================================
// HARDWARE ACCELERATION PROTOTYPES
// ==========================================================================

// --- Original 16-bit Integer Math ---
extern int16_t  mathSignedDivision(int16_t a, int16_t b);
extern int32_t  mathSignedMultiply(int16_t a, int16_t b);
extern uint32_t mathUnsignedAddition(uint32_t a, uint32_t b);

// --- S.19.12 Fixed Point Math (32-bit container) ---
// Format: 1 Sign, 19 Integer, 12 Fractional. Scale factor: 4096.
typedef int32_t fixed19_12_t; 

extern fixed19_12_t mathFixedAdd(fixed19_12_t a, fixed19_12_t b);
extern fixed19_12_t mathFixedSub(fixed19_12_t a, fixed19_12_t b);
extern fixed19_12_t mathFixedMul(fixed19_12_t a, fixed19_12_t b); // Returns (a*b) >> 12
extern fixed19_12_t mathFixedDiv(fixed19_12_t a, fixed19_12_t b); // Returns (a<<12) / b

// --- IEEE 754 Floating Point Math ---
// Depending on compiler, standard operators (+, -, *, /) might call these 
// implicitly, or we can call them explicitly to force the specific HW instruction.
extern float mathFloatAdd(float a, float b);
extern float mathFloatSub(float a, float b);
extern float mathFloatMul(float a, float b);
extern float mathFloatDiv(float a, float b);

// ========================================================================== 
// FLOAT FPU STATUS (written by asm stubs)
// ==========================================================================
// The float hardware reports operation status in per-unit registers. The asm
// mathFloat* routines are responsible for writing the masked status (output
// valid bit removed) into this byte after each operation.
//
// bit 0 - NaN
// bit 1 - Overflow
// bit 2 - Underflow
// bit 3 - Zero
// bit 4 - Division by Zero (division op only)
extern volatile uint8_t g_fp_last_status;

#define FP_STATUS_NAN        0x01u
#define FP_STATUS_OVERFLOW   0x02u
#define FP_STATUS_UNDERFLOW  0x04u
#define FP_STATUS_ZERO       0x08u
#define FP_STATUS_DIV0       0x10u

// Exceptions that can create invalid/unsafe projection results.
#define FP_STATUS_EXC_ANY    (FP_STATUS_NAN | FP_STATUS_OVERFLOW | FP_STATUS_DIV0)

// Fast float to int16 conversion without software float library
// Uses IEEE 754 bit manipulation.
//
// IMPORTANT: This target has very slow 32-bit shifts/arithmetic in generated code.
// Keep this routine byte-oriented to avoid pulling in large helper sequences.
static inline int16_t mathFloatToInt16(float f) {
    // Assumes IEEE-754 32-bit float, little-endian.
    uint8_t b[4];
    __builtin_memcpy(b, &f, 4);

    // Layout (little-endian):
    // b[0] = mantissa low
    // b[1] = mantissa mid
    // b[2] = mantissa high (7 bits) + exponent low bit
    // b[3] = sign + exponent high 7 bits
    const uint8_t sign = (uint8_t)(b[3] >> 7);
    const uint8_t exp_raw = (uint8_t)(((uint8_t)(b[3] << 1)) | (uint8_t)(b[2] >> 7));

    // Unbias exponent.
    const int8_t exp = (int8_t)exp_raw - 127;
    if (exp < 0) {
        return 0;
    }
    if (exp >= 15) {
        return sign ? (int16_t)-32768 : (int16_t)32767;
    }

    // 24-bit mantissa with implicit leading 1.
    uint8_t m2 = (uint8_t)((b[2] & 0x7Fu) | 0x80u);
    uint8_t m1 = b[1];
    uint8_t m0 = b[0];

    // Shift right by (23 - exp) to obtain the integer part.
    uint8_t shift = (uint8_t)(23 - (uint8_t)exp);

    while (shift >= 8) {
        m0 = m1;
        m1 = m2;
        m2 = 0;
        shift = (uint8_t)(shift - 8);
    }
    while (shift != 0) {
        // Right shift 24-bit value by 1.
        const uint8_t carry1 = (uint8_t)(m2 & 1u);
        const uint8_t carry0 = (uint8_t)(m1 & 1u);
        m2 = (uint8_t)(m2 >> 1);
        m1 = (uint8_t)((m1 >> 1) | (carry1 << 7));
        m0 = (uint8_t)((m0 >> 1) | (carry0 << 7));
        --shift;
    }

    // Integer result is the low 16 bits after shifting.
    int16_t result = (int16_t)(((uint16_t)m1 << 8) | (uint16_t)m0);
    return sign ? (int16_t)(-result) : result;
}

// Helper to convert 16-bit Int to S.19.12 Fixed Point
// We simply shift left by 12. Since int16 fits in the 19-bit integer part, this is safe.
static inline fixed19_12_t IntToFixed(int16_t val) {
    return (fixed19_12_t)val << FIXED_SHIFT;
}

// Helper to convert S.19.12 back to 16-bit Int (with rounding)
static inline int16_t FixedToInt(fixed19_12_t val) {
    // Add 0.5 (2048 in fixed) for rounding before truncation
    return (int16_t)((val + 2048) >> FIXED_SHIFT);
}

