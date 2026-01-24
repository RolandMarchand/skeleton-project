#pragma once

/* laz_utils.h - 0BSD utility functions and macros - Roland Marchand 2026
 * Single-header C library for C99 development.
 * Use `#define LAZ_UTILS_IMPLEMENTATION` before using. */

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

/* WARNING: these macros evaluate their input twice, so if you call
 * MAX(f1(),f2()), one of the two functions will be called twice. */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#if defined(__GNUC__) || defined(__clang__)
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

static int errorf(const char *restrict format, ...);
/* When passed with out as NULL, return the size needed in bytes to load the
 * whole file into memory, including the null terminator. A return value of < 0
 * indicates an error. When out is not NULL, write the file's contents to out
 * and return how many bytes were written, including the null terminator. */
long int load_file(const char *path, char *out);
uint32_t fnv1a_32_buf(const void *buf, size_t len);
uint32_t fnv1a_32_str(const char *str);
uint64_t fnv1a_64_buf(const void *buf, size_t len);
uint64_t fnv1a_64_str(const char *str);

#ifdef LAZ_UTILS_IMPLEMENTATION

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
unsigned long long get_nanoseconds(void) {
	FILETIME ft = { 0 };
	GetSystemTimeAsFileTime(&ft);
	unsigned long long time100ns =
		((unsigned long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	return time100ns * 100;
}
#else
#include <time.h>
unsigned long long get_nanoseconds(void) {
	struct timespec ts = { 0 };
	clock_gettime(CLOCK_REALTIME, &ts);
	return (unsigned long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}
#endif

static int errorf(const char *restrict format, ...)
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

uint32_t fnv1a_32_buf(const void *buf, size_t len)
{
	const unsigned char *bp = (const unsigned char *)buf;
	const unsigned char *be = bp + len;
	uint32_t hval = FNV1A_32_INITIAL_VAL;

	for (; bp < be; bp++) {
		hval ^= (uint32_t)bp[0];
		hval *= FNV1A_32_PRIME;
		/* Force 32-bit behavior for if you plan to replace uint32_t
		 * with unsigned long for c89 compatibility */
		/* hval &= 0xFFFFFFFFUL; */
	}

	return hval;
}

uint32_t fnv1a_32_str(const char *str)
{
	const unsigned char *s = (const unsigned char *)str;
	uint32_t hval = FNV1A_32_INITIAL_VAL;

	for (; s[0] != '\0'; s++) {
		hval ^= (uint32_t)s[0];
		hval *= FNV1A_32_PRIME;
		/* Force 32-bit behavior for if you plan to replace uint32_t
		 * with unsigned long for c89 compatibility */
		/* hval &= 0xFFFFFFFFUL; */
	}

	return hval;
}

uint64_t fnv1a_64_buf(const void *buf, size_t len)
{
	const unsigned char *bp = (const unsigned char *)buf;
	const unsigned char *be = bp + len;
	uint64_t hval = FNV1A_64_INITIAL_VAL;

	for (; bp < be; bp++) {
		hval ^= (uint64_t)bp[0];
		hval *= FNV1A_64_PRIME;
	}

	return hval;
}

uint64_t fnv1a_64_str(const char *str)
{
	const unsigned char *s = (const unsigned char *)str;
	uint64_t hval = FNV1A_64_INITIAL_VAL;

	for (; s[0] != '\0'; s++) {
		hval ^= (uint64_t)s[0];
		hval *= FNV1A_64_PRIME;
	}

	return hval;
}

#endif /* LAZ_UTILS_IMPLEMENTATION */
