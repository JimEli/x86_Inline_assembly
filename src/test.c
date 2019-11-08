/*************************************************************************
* Title: inline assembly string functions
* File: gcc_version.c
* Author: James Eli
* Date: 12/13/2017
*
* Notes:
*  (1) Little to no error checking/input validation.
*  (2) Compiled with Eclipse Oxygen GNU GCC 5.3, using C language options.
*************************************************************************
* Change Log:
*   12/13/2017: Initial release. JME
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// function declarations.
char   *memcpy_(char *, const char *, const int);
void   *memset_(void *, int, size_t);
size_t  strlen_(const char *);
char   *strcpy_(char *, const char *);
char   *strcat_(char *, const char *);
char   *strchr_(const char *, int);
int     strcmp_(const char *, const char *);
char   *strstr_(const char *, const char *);
char   *reverse_(const char *);
void    insert_(char *d, const char *s, const size_t n);

//  void *memcpy(void *dst, const void *src, size_t n) {
//    char *s = (char *)src, *end = s + n, *d = (char *)dst;
//    if ((((unsigned int) s) | ((unsigned int) d) | n) && sizeof(unsigned int) - 1) {
//      while (s != end)
//        *d++ = *s++;
//    } else {
//      while (s != end)
//        *((unsigned int *) d)++ = *((unsigned int *) s)++;
//    }
//    return dst;
//  }
char *memcpy_(char *d, const char *s, const int n) {
	asm volatile (
		"cld       \n" // direction = up
		"rep movsb \n" // copy string byte from esi to edi until ecx == 0
		: : "S" (s), "D" (d), "c" (n) : "memory"
	);

	return (char *)d;
}

//  void *memset(void *p, int c, size_t n) {
//    char *pb = (char *)p, *pbend = pb + n;
//    while (pb != pbend)
//      *pb++ = c;
//    return p;
//  }
void *memset_(void *d, int c, size_t n) {
	void *ret;

	asm volatile (
		"push %1   \n" // save return pointer
		"rep stosb \n" // set data
		"pop %0    \n" // retrieve return pointer
		: "=r" (ret) : "D" (d), "c" (n), "a" (c) : "memory"
	);
	return ret;
}

//  size_t strlen(const char *str) {
//    const char *s;
//    for (s=str; *s; ++s);
//    return (s - str);
//  }
size_t strlen_(const char *s) {
	size_t len = 0;
	int i = 0;

	asm volatile (
		"mov %1, %%edi       \n" // load string address.
		"or  $0xffffffff, %2 \n" // initialize for counting down.
		"xor %%eax, %%eax    \n" // zeroize.
		"cmp $0, %%edi       \n" // check for null string.
		"je  strlen_exit_    \n"

		"repne scasb         \n" // repeat until zero.
		"not %2              \n" // invert.
		"dec %2              \n" // subtract 1.

	"strlen_exit_:           \n"
		"mov %2, %0          \n" // return value.
		: "+r" (len) : "S" (s), "c" (i) : "eax", "edi"
	);

	return len;
}

//  char *strcpy(char *dst, const char *src) {
//    char *cp = dst;
//    while (*cp++ = *src++);
//    return dst;
//  }
char *strcpy_(char *d, const char *s) {
	asm volatile (
		"push %0           \n" // pass src string to strlen
		"call _strlen_     \n" // get length of src
		"add  $4, %%esp    \n" // clean up stack
		"inc  %%eax        \n" // account for NULL terminator
		"mov  %%eax, %%ecx \n" // set counter
		"cld               \n" // direction = up
		"rep  movsb        \n" // copy string byte from esi to edi until ecx == 0
		: : "S" (s), "D" (d) : "%ecx", "memory"
	);

	return d;
}

//  char *strcat(char *dst, const char *src) {
//    char *cp = dst;
//    while (*cp)
//      cp++;
//    while (*cp++ = *src++);
//    return dst;
//  }
char *strcat_(char *d, const char *s) {
	asm volatile (
		"push %0           \n" // pass source string to strlen
		"call _strlen_     \n"
		"add  $4, %%esp    \n" // clean up stack
		"inc  %%eax        \n" // include terminating zero in length
		"push %%eax        \n" // save source length + 1

		"push %1           \n" // pass destination string to strlen
		"call _strlen_     \n"
		"add  $4, %%esp    \n" // clean up stack
		"add  %%eax, %%edi \n" // calculate end of destination

		"pop  %%ecx        \n" // length of source string to copy
		"cld               \n" // direction = up
		"rep  movsb        \n" // copy string byte from esi to edi until ecx == 0
		: : "S" (s), "D" (d) : "%ecx", "memory"
	);

	return d;
}

//  char *strchr(const char *s, int ch) {
//    while (*s && *s != (char)ch)
//      s++;
//    if (*s == (char)ch)
//      return (char *)s;
//    return NULL;
//  }
char *strchr_(const char *s, int c) {
	asm volatile (
			"mov  $0, %%al     \n" // set to null byte
		"1:                    \n"
			"mov  (%0), %%cl   \n" // get current character
			"cmp  %%cl, %%bl   \n" // check if character is what we search
			"jz   strchr_exit_ \n" // jump to return if match
			"scasb             \n" // check if null byte
			"jnz  1b           \n" // loop if no match
		"strchr_exit_:         \n"
			: "+D" (s) :  "b" (c) : "%eax", "%ecx"
	);

	return (char *)s; // cast to silence discarded-qualifiers warning.
}

//   int strcmp(const char *s1, const char *s2) {
//     for ( ; *s1 == *s2; s1++, s2++)
//	     if (*s1 == '\0')
//	       return 0;
//     return ( (*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1 );
//   }
int strcmp_(const char *s1, const char *s2) {
	int ret;

	asm volatile (
		"push %2           \n" // determine length of string
		"call _strlen_     \n"
		"add  $4, %%esp    \n" // clean up stack
		"mov  %%eax, %%ebx \n"
		"push %1           \n" // determine length of string
		"call _strlen_     \n"
		"add  $4, %%esp    \n" // clean up stack
		"cmp  %%ebx, %%eax \n" // compare lengths
		"ja   greater      \n" // first string is longer
		"jb   less         \n" // second string is longer
		"mov  %%eax, %%ecx \n" // length of strings
		"repe cmpsb        \n" // compare strings
		"jg   greater      \n" // first string is greater
		"jl   less         \n" // second string is greater
		"mov  $0, %0       \n" // strings are equal
		"jmp  strcmp_exit_ \n"
	"greater:              \n"
		"mov  $1, %0       \n"
		"jmp  strcmp_exit_ \n"
	"less:                 \n"
		"mov  $-1, %0      \n"
		"jmp  strcmp_exit_ \n"
	"strcmp_exit_:         \n"
		: "=r" (ret) : "S" (s1), "D" (s2) : "%ebx", "%ecx"
	);

	return ret;
}

//  char *strstr(const char *str1, const char *str2) {
//    char *cp = (char *)str1, *s1, *s2;
//    if (!*str2)
//      return (char *)str1;
//    while (*cp) {
//      s1 = cp;
//      s2 = (char *)str2;
//      while (*s1 && *s2 && !(*s1 - *s2)) s1++, s2++;
//      if (!*s2)
//        return cp;
//      cp++;
//    }
//    return NULL;
//  }
char *strstr_(const char *s, const char *target) {
	char *ret;

	asm volatile (
		"push %1           \n"
		"push %2           \n" // pass target string to strlen
		"call _strlen_     \n"
		"add  $4, %%esp    \n" // clean up stack
		"push %%eax        \n"

		"push %1           \n" // pass s string to strlen
		"call _strlen_     \n"
		"add  $4, %%esp    \n" // clean up stack

		"pop  %%ecx        \n" // length of s string
		"cmp  %%eax, %%ecx \n" // compare target to s
		"ja   2f           \n" // jump if target longer than s

		"cld               \n" // direction up
		"sub  %%ecx, %%eax \n" // subtract target length from s length
		"mov  %%eax, %%ebx \n" // loop counter
		"mov  %%ecx, %%eax \n" // eax == target length
		"xor  %%eax, %%eax \n"

	"1:                    \n"
		"inc  %%eax        \n" // index counter
		"inc  %1           \n" // advance to next character in s
		"push %1           \n"
		"push %2           \n"
		"push %%ecx        \n"
		"rep  cmpsb        \n" // search for target in s at this index
		"pop  %%ecx        \n"
		"pop  %2           \n"
		"pop  %1           \n"

		"je   3f           \n" // found target
		"dec  %%ebx        \n" // decrement loop counter
		"jnz  1b           \n" // repeat search

	"2:                    \n"
		"xor  %%eax, %%eax \n" // not found, zeroize eax

	"3:                    \n"
		"pop  %%ecx        \n" // pop start of s string
		"add  %%eax, %%ecx \n" // add index of target if found
		"mov  %%ecx, %0    \n" // save as return pointer

		: "=r" (ret) : "S" (s), "D" (target) : "%ebx", "%ecx"
	);

	return ret;
}

char *reverse_(const char *s) {
	asm volatile (
		"xor   %%ecx, %%ecx         \n" // zeroize eax
	"1:                             \n"
		"movzx (%0, %%ecx), %%edx   \n" // grab next character
		"cmp   $0, %%dl             \n" // zero?
		"je    2f                   \n"
		"inc   %%ecx                \n" // increment string counter
		"push  %%edx                \n" // push character onto stack
		"jmp   1b                   \n" // repeat
	"2:                             \n"
		"cmp   $0, %%ecx            \n" // any characters?
		"je    reverse_exit_        \n"
		"xor   %%eax, %%eax         \n" // zeroize eax
	"3:                             \n"
		"pop   %%edx                \n" // pull characters from stack in reverse order
		"mov   %%dl, (%%esi, %%eax) \n" // save character in string
		"inc   %%eax                \n" // increment string counter
		"loop  3b                   \n" // repeat until ecx==0
	"reverse_exit_:                 \n"
		: : "S" (s) : "%eax", "%ecx", "%edx", "memory"
	);

	return (char *)s; // cast to silence discarded-qualifiers warning.
}

void insert_(char *d, const char *s, const size_t n) {
	asm volatile (
		"pushf             \n"
		"push %0           \n" // d
		"push %1           \n" // s
		"push %2           \n" // n
		// string lengths
		"push %0           \n" // ebx = d length
		"call _strlen_     \n"
		"add  $4, %%esp    \n" // clean up stack
		"mov  %%eax, %%ebx \n"
		"push %1           \n" // eax = s length
		"call _strlen_     \n"
		"add  $4, %%esp    \n" // clean up stack
		// make room
		"add  %%ebx, %0    \n" // destination + string length
		"mov  %0, %1       \n" // use as source addr
		"add  %%eax, %0    \n" // + text length
		"mov  %%eax, %%ecx \n" // move text length characters +1
		"inc  %%ecx        \n"
		"std               \n" // direction = down
		"rep  movsb        \n" // copy string byte from esi to edi until ecx == 0
		// copy text
		"pop  %%ebx        \n" // offset
		"pop  %%esi        \n" // source text
		"pop  %%edi        \n" // destination addr
		"add  %%ebx, %%edi \n"
		"mov  %%eax, %%ecx \n" // number chars to copy
		"cld               \n" // direction = up
		"rep  movsb        \n" // copy string byte from esi to edi until ecx == 0

		"popf              \n"

		: : "D" (d), "S" (s), "c" (n) : "memory"
	);
}

int main(void) {
	char s1[14] = "abcdefghijklm";
	char s2[14] = "mlkjihgfedcba";
	char s3[7] = "abc";

	// Required to make eclipse console output work properly.
	setvbuf(stdout, NULL, _IONBF, 0);
	fflush(stdout);

	printf("strlen \"%s\" = %d\n", s1, strlen_(s1));
	printf("memset \"%s\"\n", (char *)memset_(s1, 'x', 13));
	printf("memcpy \"%s\"\n", memcpy_(s1, s2, 13));
	printf("strcpy \"%s\"\n", strcpy_(s1, "abcdefghijklm"));
	printf("strcat \"%s\"\n", strcat_(s3, "def"));
	printf("strchr \"%s\" 'i' = %s\n", s1, strchr_(s1, 'i'));
	printf("strcmp \"%s\" vs. \"%s\" = %d\n", s1, s1, strcmp_(s1, s1));
	printf("strcmp \"%s\" vs. \"%s\" = %d\n", s1, s2, strcmp_(s1, s2));
	printf("strcmp \"%s\" vs. \"%s\" = %d\n", s2, s1, strcmp_(s2, s1));

	const char haystack[20] = "ThisAintNoFreeLunch";
	const char needle[6] = "Free";
	printf("strstr = %s\n", strstr_(haystack, needle));
	const char noNeedle[6] = "Frex";
	printf("strstr = %s\n", strstr_(haystack, noNeedle));

	printf("reverse \"%s\" to ", s1); printf("\"%s\"\n", reverse_(s1));

	char s4[] = { '1', '2', '3', '4', 0, 0, 0, 0, 0, 0, 0 };
    printf("insert \"%s\" into \"%s\" at %d = ", s3, s4, 2);
    insert_(s4, s3, 2);
    printf("\"%s\"\n", s4);
}

