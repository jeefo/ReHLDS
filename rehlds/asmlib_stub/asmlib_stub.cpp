#include "asmlib.h"

#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

extern "C" {

void *A_memcpy(void *dest, const void *src, size_t count) { return memcpy(dest, src, count); }
void *A_memmove(void *dest, const void *src, size_t count) { return memmove(dest, src, count); }
void *A_memset(void *dest, int c, size_t count) { return memset(dest, c, count); }
int A_memcmp(const void *buf1, const void *buf2, size_t num) { return memcmp(buf1, buf2, num); }

char *A_strcat(char *dest, const char *src) { return strcat(dest, src); }
char *A_strcpy(char *dest, const char *src) { return strcpy(dest, src); }
size_t A_strlen(const char *str) { return strlen(str); }
int A_strcmp(const char *a, const char *b) { return strcmp(a, b); }
char *A_strstr(char *haystack, const char *needle) { return strstr(haystack, needle); }

int A_stricmp(const char *string1, const char *string2)
{
	for (;;)
	{
		unsigned char c1 = (unsigned char)*string1++;
		unsigned char c2 = (unsigned char)*string2++;

		if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
		if (c2 >= 'A' && c2 <= 'Z') c2 += 32;

		if (c1 != c2)
			return (int)c1 - (int)c2;
		if (c1 == 0)
			return 0;
	}
}

void A_strtolower(char *string)
{
	for (; *string; ++string)
	{
		if (*string >= 'A' && *string <= 'Z')
			*string += 32;
	}
}

void cpuid_ex(int abcd[4], int eax, int ecx)
{
#ifdef _MSC_VER
	__cpuidex(abcd, eax, ecx);
#else
	__asm__ __volatile__("cpuid"
		: "=a"(abcd[0]), "=b"(abcd[1]), "=c"(abcd[2]), "=d"(abcd[3])
		: "a"(eax), "c"(ecx));
#endif
}

}
