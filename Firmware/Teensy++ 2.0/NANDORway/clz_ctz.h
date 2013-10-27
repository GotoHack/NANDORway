#ifndef CLZ_CTH_H
#define CLZ_CTH_H

/**
 * \internal
 * \def ERROR_FUNC(name, msg)
 * \brief Fail compilation if function call isn't eliminated
 *
 * If the compiler fails to optimize away all calls to the function \a
 * name, terminate compilation and display \a msg to the user.
 *
 * \note Not all compilers support this, so this is best-effort only.
 * Sometimes, there may be a linker error instead, and when optimization
 * is disabled, this mechanism will be completely disabled.
 */
#ifndef ERROR_FUNC
# define ERROR_FUNC(name, msg)                      \
	extern int name(void)
#endif

//! Error function for failed demultiplexing.
ERROR_FUNC(compiler_demux_bad_size, "Invalid parameter size");

/**
 * \internal
 * \brief Demultiplex function call based on size of datatype
 *
 * Evaluates to a function call to a function name with suffix 8, 16 or 32
 * depending on the size of the datatype. Any number of parameters can be
 * passed to the function.
 *
 * Usage:
 * \code
 * void foo8(uint8_t a, void *b);
 * void foo16(uint16_t a, void *b);
 * void foo32(uint32_t a, void *b);
 *
 * #define foo(x, y)    compiler_demux_size(sizeof(x), foo, x, y)
 * \endcode
 *
 * \param size Size of the datatype.
 * \param func Base function name.
 * \param ... List of parameters to pass to the function.
 */
#define compiler_demux_size(size, func, ...)        \
	(((size) == 1) ? func##8(__VA_ARGS__) :     \
	 ((size) == 2) ? func##16(__VA_ARGS__) :    \
	 ((size) == 4) ? func##32(__VA_ARGS__) :    \
	 compiler_demux_bad_size())

#define clz(x)    compiler_demux_size(sizeof(x), clz, (x))

__attribute__ ((always_inline)) static uint8_t clz8(uint8_t x)
{
    uint8_t bit = 0;

    if (x & 0xf0) {
        x >>= 4;
    } else {
        bit += 4;
    }

    if (x & 0x0c) {
        x >>= 2;
    } else {
        bit += 2;
    }

    if (!(x & 0x02)) {
        bit++;
    }

    return bit;
}

__attribute__ ((always_inline)) static uint8_t clz16(uint16_t x)
{
    uint8_t bit = 0;

    if (x & 0xff00) {
        x >>= 8;
    } else {
        bit += 8;
    }

    return bit + clz8(x);
}

__attribute__ ((always_inline)) static uint8_t clz32(uint32_t x)
{
    uint8_t bit = 0;

    if (x & 0xffff0000) {
        x >>= 16;
    } else {
        bit += 16;
    }

    return bit + clz16(x);
}

#define ctz(x)    compiler_demux_size(sizeof(x), ctz, (x))

__attribute__ ((always_inline)) static uint8_t ctz8(uint8_t x)
{
    uint8_t bit = 0;

    if (!(x & 0x0f)) {
        bit += 4;
        x >>= 4;
    }
    if (!(x & 0x03)) {
        bit += 2;
        x >>= 2;
    }
    if (!(x & 0x01))
        bit++;

    return bit;
}

__attribute__ ((always_inline)) static uint8_t ctz16(uint16_t x)
{
    uint8_t bit = 0;

    if (!(x & 0x00ff)) {
        bit += 8;
        x >>= 8;
    }

    return bit + ctz8(x);
}

__attribute__ ((always_inline)) static uint8_t ctz32(uint32_t x)
{
    uint8_t bit = 0;

    if (!(x & 0x0000ffff)) {
        bit += 16;
        x >>= 16;
    }

    return bit + ctz16(x);
}

#endif /* CLZ_CTZ_H */
