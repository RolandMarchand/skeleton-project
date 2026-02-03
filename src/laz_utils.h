#pragma once

/* laz_utils.h - 0BSD utility functions and macros - Roland Marchand 2026
 * Single-header C library for C99 development.
 * Use `#define LAZ_UTILS_IMPLEMENTATION` before using. */

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

/* WARNING: these macros evaluate their input twice, so if you call
 * MAX(f1(),f2()), one of the two functions will be called twice. */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(n, min, max) ((n) < (min) ? (min) : (n) > (max) ? (max) : (n))

#if defined(__GNUC__) || defined(__clang__)
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

#ifdef __cplusplus
#define LAZ_RESTRICT
#define LAZ_INIT { }
#else
#define LAZ_RESTRICT restrict
#define LAZ_INIT { 0 }
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

u64 get_nanoseconds(void);
int errorf(const char *LAZ_RESTRICT format, ...);
/* Can only read files <2GiB. Reading files >=2GiB is undefined behavior. When
 * `out` is null, return the size of the buffer to allocate, including null
 * term. Otherwise, write to `out` and return the amount of bytes written,
 * including the null term. */
long int load_file(const char *path, char *out);
u32 fnv1a_32_buf(const void *buf, size_t len);
u32 fnv1a_32_str(const char *str);
u64 fnv1a_64_buf(const void *buf, size_t len);
u64 fnv1a_64_str(const char *str);

#ifdef LAZ_UTILS_IMPLEMENTATION

u64 get_nanoseconds(void) {
	struct timespec ts = LAZ_INIT;
	timespec_get(&ts, TIME_UTC);
	return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
}

int errorf(const char *LAZ_RESTRICT format, ...)
{
	va_list args;
	va_start(args, format);

	int ret = vfprintf(stderr, format, args);

	va_end(args);
	return ret;
}

long int load_file(const char *path, char *out)
{
	FILE *file = fopen(path, "rb");
	long int file_size = 0;
	size_t bytes_read = 0;

	if (file == NULL) {
		/* No file provided */
		(void)errorf("Error: unable to read file %s\n", path);
		return -1;
	}

	/* Get file size */
	if (fseek(file, 0, SEEK_END) != 0) {
		/* Reading error */
		(void)errorf("Error: unable to read file %s\n", path);
		(void)fclose(file);
		return -1;
	}

	file_size = ftell(file);

	if (file_size == 0) {
		/* Empty file */
		(void)fclose(file);
		if (out != NULL) {
			out[0] = '\0';
		}
		return 1; /* Size of null term */
	}

	if (file_size < 0) {
		/* Error reading */
		(void)errorf("Error: unable to read file %s\n", path);
		(void)fclose(file);
		return -1;
	}

	/* Return size of file to allocate, no writing */
	if (out == NULL) {
		(void)fclose(file);
		return file_size + 1;
	}

	/* Setup to read file contents */
	if (fseek(file, 0, SEEK_SET) != 0) {
		/* Reading error */
		(void)errorf("Error: unable to read file %s\n", path);
		(void)fclose(file);
		return -1;
	}

	/* Reading file */
	bytes_read = fread(out, 1, file_size, file);

	if (bytes_read != (size_t)file_size) {
		/* Failed to read all of the file */
		(void)errorf("Error: unable to read file %s\n", path);
		(void)fclose(file);
		return -1;
	}

	out[bytes_read] = '\0';

	if (fclose(file) != 0) {
		/* Unable to close file, not fatal since bytes are read */
		(void)errorf("Warning: unable to close file %s\n",
			      path);
	}

	return (long int)(bytes_read + 1);
}

#define FNV1A_32_PRIME 0x01000193U
#define FNV1A_32_INITIAL_VAL 0x811c9dc5U
#define FNV1A_64_PRIME 0x100000001b3ULL
#define FNV1A_64_INITIAL_VAL 0xcbf29ce484222325ULL

u32 fnv1a_32_buf(const void *buf, size_t len)
{
	const unsigned char *bp = (const unsigned char *)buf;
	const unsigned char *be = bp + len;
	u32 hval = FNV1A_32_INITIAL_VAL;

	for (; bp < be; bp++) {
		hval ^= (u32)bp[0];
		hval *= FNV1A_32_PRIME;
		/* Force 32-bit behavior for if you plan to replace uint32_t
		 * with unsigned long for c89 compatibility */
		/* hval &= 0xFFFFFFFFUL; */
	}

	return hval;
}

u32 fnv1a_32_str(const char *str)
{
	const unsigned char *s = (const unsigned char *)str;
	u32 hval = FNV1A_32_INITIAL_VAL;

	for (; s[0] != '\0'; s++) {
		hval ^= (u32)s[0];
		hval *= FNV1A_32_PRIME;
		/* Force 32-bit behavior for if you plan to replace uint32_t
		 * with unsigned long for c89 compatibility */
		/* hval &= 0xFFFFFFFFUL; */
	}

	return hval;
}

u64 fnv1a_64_buf(const void *buf, size_t len)
{
	const unsigned char *bp = (const unsigned char *)buf;
	const unsigned char *be = bp + len;
	u64 hval = FNV1A_64_INITIAL_VAL;

	for (; bp < be; bp++) {
		hval ^= (u64)bp[0];
		hval *= FNV1A_64_PRIME;
	}

	return hval;
}

u64 fnv1a_64_str(const char *str)
{
	const unsigned char *s = (const unsigned char *)str;
	u64 hval = FNV1A_64_INITIAL_VAL;

	for (; s[0] != '\0'; s++) {
		hval ^= (u64)s[0];
		hval *= FNV1A_64_PRIME;
	}

	return hval;
}

#undef LAZ_RESTRICT
#undef LAZ_INIT
#endif /* LAZ_UTILS_IMPLEMENTATION */
