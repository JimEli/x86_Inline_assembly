/*************************************************************************
* Title: inline assembly string functions
* File: test.c
* Author: James Eli
* Date: 12/13/2017
*
* Demonstrates C & MASM Project Setup and a little bit of nonsense.
*
* Notes:
*  (1) Little to no error checking/input validation.
*  (2) Compiled with Eclipse Oxygen GNU GCC 5.3, using C language options.
*  (3) Compiled/tested with MS Visual Studio 2017 Community (v141), and
*      Windows SDK version 10.0.17134.0, and Eclipse and MinGW gcc 6.3.0.
*
*************************************************************************
* Change Log:
*   12/13/2017: Initial release. JME
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>

// C/C++ Preprocessor Definitions: _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996) 

char string[] = "abcdefghijklm"; // A string.
char stringCopy[14] = { 0 };     // An empty string.

//size_t strlen(const char *str) {
//  const char *s;
//  for (s=str; *s; ++s);
//  return (s - str);
//}
__inline unsigned int strLen(const char *s) {
	__asm {
		mov   edi, s			; Load string address.
		or    ecx, 0ffffffffh	; Init for counting down.
		xor   eax, eax			; Zero.
		cmp   edi, 0			; Check for null string.
		je    len1
		repne scasb				; Repeat until zero.

		not   ecx				; Invert
		dec   ecx				; Subtract 1.
		mov   eax, ecx			; Prepare return value.
	len1:
	}
}

//char* strcpy(char *dst, const char *src) {
//  char *cp = dst;
//  while (*cp++ = *src++);
//  return dst;
//}
__inline char *strCpy(char *d, const char *s)
{
	char *d_ = d;

	__asm {
		mov esi, s    ; put address into the source index
		mov edi, d    ; put address into the destination index
	copy:
		mov al, [esi] ; copy byte at address in esi to al
		inc esi       ; increment address in esi
		mov [edi], al ; copy byte in al to address in edi
		inc edi       ; increment address in edi
		cmp al, 0     ; see if its an ascii zero
		jne copy      ; jump back and read next byte if not
	}

	return d_;
}

//char *strcat(char *dst, const char *src) {
//  char *cp = dst;
//  while (*cp)
//    cp++;
//  while (*cp++ = *src++);
//  return dst;
//}
__inline char *strCat(char * dest, const char * src)
{
	char *cp = dest;

	__asm {
		; while (*cp)
		mov   ecx, src
		mov   eax, cp
	cat1:
		movsx ebx, byte ptr [eax]
		test  ebx, ebx
		je    cat2
		;  cp++;
		inc   eax
		jmp   cat1
		; while (*cp++ = *src++);
	cat2:
		mov   dl, byte ptr[ecx]
		mov   byte ptr[eax], dl
		inc   eax
		inc   ecx
		cmp   dl, 0
		je    cat3
		jmp   cat2
		; return dest;
	cat3:
		mov   eax, dest
	}
}

//  char *strchr(const char *s, int ch) {
//    while (*s && *s != (char)ch)
//      s++;
//    if (*s == (char)ch)
//      return (char *)s;
//    return NULL;
//  }
__inline char *strChr(const char *s, int c) 
{
	__asm {
		mov  edi, s				; edi == source ptr.
		mov  ebx, c				; cl == target.
		mov  al, 0				; Set to null byte.
		xor  ecx, ecx			; Zero.
	schr:
		mov  cl, byte ptr[edi]	; Get a character.
		cmp  bl, cl				; Check if character is target.
		jz   schr_exit			; Jump to exit if match.
		scasb					; Check if null byte.
		jnz  schr				; Loop if no match.
		mov  edi, 0				; Return null.
	schr_exit:
		mov eax, edi			; Return results.
	}
}

//   int strcmp(const char *s1, const char *s2) {
//     for ( ; *s1 == *s2; s1++, s2++)
//	     if (*s1 == '\0')
//	       return 0;
//     return ( (*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1 );
//   }
__inline int strCmp(const char *s1, const char *s2) 
{
	int r;

	__asm {
		mov esi, s1
		mov edi, s2
		push edi		; determine length of string
		call strLen
		add  esp, 4		; clean up stack
		mov  ebx, eax
		push esi		; determine length of string
		call strLen
		add  esp, 4		; clean up stack
		cmp  eax, ebx	; compare lengths
		ja   greater	; first string is longer
		jb   less		; second string is longer
		mov  ecx, eax	; length of strings
		repe cmpsb		; compare strings
		jg   greater	; first string is greater
		jl   less		; second string is greater
		mov  r, 0		; strings are equal
		jmp  strcmp_exit
		greater:
		mov  r, 1
		jmp  strcmp_exit
		less:
		mov  r, -1
		jmp  strcmp_exit
		strcmp_exit:
	}

	return r;
}

//  char *strstr(const char *str1, const char *str2) {
//    char *cp = (char *)str1, *s1, *s2;
//    if (!*str2)
//      return (char *)str1;
//    while (*cp) {
//      s1 = cp;
//      s2 = (char *)str2;
//      while (*s1 && *s2 && !(*s1 - *s2))
//        s1++, s2++;
//      if (!*s2)
//        return cp;
//      cp++;
//    }
//    return NULL;
//  }
__inline char *strStr(const char *s, const char *target) 
{
	__asm {
		mov  esi, s
		mov  edi, target
		push esi
		push edi		; pass target string to strlen
		call strLen
		add  esp, 4		; clean up stack
		push eax

		push esi		; pass s string to strlen
		call strLen
		add  esp, 4		; clean up stack

		pop  ecx		; length of s string
		cmp  ecx, eax	; compare target to s
		ja   str2		; jump if target longer than s

		cld				; direction up
		sub  eax, ecx	; subtract target length from s length
		mov  ebx, eax	; loop counter
		mov  eax, ecx	; eax == target length

	str1:
		inc eax			; index counter
		inc esi			; advance to next character in s
		push esi
		push edi
		push ecx
		rep  cmpsb		; search for target in s at this index
		pop  ecx
		pop edi
		pop edi

		je   str3		; found target
		dec  ebx		; decrement loop counter
		jnz  str1		; repeat search
	
	str2:
		xor  eax, eax	; not found, zeroize eax

	str3:
		pop ecx			; pop start of s string
		add  ecx, eax	; add index of target if found
		mov  eax, ecx	; save return pointer
	}
}

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
 char *memCpy(char *d, const char *s, const int n)
{
	__asm {
		mov esi, s
		mov edi, d
		mov ecx, n
		cld			; direction = up
		rep movsb	; Copy string byte from esi to edi until ecx == 0
	}

	return (char *)d;
}

//void *memset(void *p, int c, size_t n) {
//  char *pb = (char *)p, *pbend = pb + n;
//  while (pb != pbend)
//    *pb++ = c;
//  return p;
//}
__inline void *memSet(void *d, int c, size_t n) 
{
	__asm {
		mov edi, d
		mov ecx, n
		mov eax, c
		push edi	; save return pointer
		rep stosb	; set data
		pop eax		; retrieve return pointer
	}
}

 // Reverse string using the stack.
 char *reverse(const char *src) 
 {
	 char *dest = (char *)src;

	__asm {
		 xor ecx, ecx
		 mov esi, src
	 L1:
		movzx edx, byte ptr[esi + ecx]
		cmp dl, 0
		je L2
		inc ecx
		push edx
		jmp L1
	L2:
		cmp ecx, 0
		je L4
	L3:
		pop edx
		mov byte ptr[esi], dl
		inc esi
		loop L3
	L4:
	}

	return dest;
}

void printStrings(const char *pc, const char *p) {
	printf(" original: %s\n", p);
	printf(" copy:     %s\n", pc);
}

int main(void) {
	// strlen
	printf("\"%s\" StrLen = %d\n", string, strLen(string));

	// strcpy
	puts("strcpy:"); strCpy(stringCopy, string); printStrings(stringCopy, string);

	// reverse
	printf("\"%s\" reversed = ", stringCopy); printf("\"%s\"\n", reverse(stringCopy));

	// memcpy
	puts("memcpy:"); memCpy(string, stringCopy, strLen(stringCopy)); printStrings(string, stringCopy);

	// strcat
	char* s1 = (char *)malloc(7); // empty strings.
	char* s2 = (char *)malloc(4);
	for (int i = 0; i < 3; i++)
		*(s2 + i) = 'a' + i;
	*(s2 + 3) = 0;
	puts("strcpy:"); strCpy(s1, s2); printStrings(s1, s2); puts("strcat:"); s2 = strCat(s2, s1); printStrings(s1, s2);

	// strchr
	printf("\"%s\" strchr 'i' = %s\n", string, strChr(string, 'i'));

	// memset
	printf("memset = %s\n", memSet(string, 'x', strLen(string)));
	
	// strcmp
	printf("strcmp = %d\n", strCmp(stringCopy, string));
	printStrings(stringCopy, string);

	// strstr
	const char haystack[20] = "ThisAintNoFreeLunch";
	const char needle[10] = "Free";
	printf("substring from strstr = %s\n", strstr_(haystack, needle));
}

/*
// The 'strncpy' function copies not more than `n' characters (characters
// that follow a null character are not copied) from the array pointed to
// by 'src' to the array pointed to by 'dest'.  If copying takes place between
// objects that overlap, the behavior is undefined.
// If the array pointed to by 'src' is a string that is shorter than 'n'
// characters, null characters are appended to the copy in the array
// pointed to by 'dest', until 'n' characters in all have been written.
// The 'strncpy' function returns the value of 'dest'.
char* strncpy_(char *dest, const char *src, size_t n)
{
	char *s = dest;

	while (n > 0 && *src != '\0')
	{
		*s++ = *src++;
		--n;
	}
	while (n > 0)
	{
		*s++ = '\0';
		--n;
	}

	return dest;
}

strncpy_:
	push    %ebp
	mov     %esp, %ebp
	sub     $0x10, %esp
	//char *s = dest;
	mov     0x8(%ebp), %eax
	mov     %eax, -0x4(%ebp)
	//while (n > 0 && *src != '\0')
	jmp     strncpy_ + 41
	// *s++ = *src++;
	mov     -0x4(%ebp), %eax
	lea     0x1(%eax), %edx
	mov     %edx, -0x4(%ebp)
	mov     0xc(%ebp), %edx
	lea     0x1(%edx), %ecx
	mov     %ecx, 0xc(%ebp)
	movzbl  (%edx), %edx
	mov     %dl, (%eax)
	// --n;
	subl    $0x1, 0x10(%ebp)

	cmpl    $0x0, 0x10(%ebp)
	je      strncpy_ + 75
	mov     0xc(%ebp), %eax
	movzbl  (%eax), %eax
	test    %al, %al
	jne     strncpy_ + 14
	//while (n > 0)
	jmp     strncpy_ + 75
	// *s++ = '\0';
	mov     -0x4(%ebp), %eax
	lea     0x1(%eax), %edx
	mov     %edx, -0x4(%ebp)
	movb    $0x0, (%eax)
	// --n;
	subl    $0x1, 0x10(%ebp)

	cmpl    $0x0, 0x10(%ebp)
	jne     strncpy_ + 59
	//return dest;
	mov     0x8(%ebp), %eax
	leave


//void strins(char* d, char* s, int n) {
//  char *temp;
//  temp = (char*)malloc(strlen_(d) + strlen_(s) + 1);
	//strncpy(temp, d, n);
//  strcpy_(temp, d);
//  temp[n] = '\0';
//  strcat_(temp, s);
//  strcat_(temp, d + n);
//  strcpy_(d, temp);
//  free(temp);
//}
void strins_(char *d, const char *s, const size_t n) {
	size_t i = 0;
	char *temp = NULL;

	asm volatile (
		//temp = (char*)malloc(strlen(d) + strlen(s) + 1);
		"push  %3            \n" // preserve n
		"push  %0            \n" // d
		"call  _strlen_      \n"
		"add   $4, %%esp     \n"
		"mov   %%eax, %4     \n" // i = strlen(d)

		"push  %1            \n" // s
		"call  _strlen_      \n"
		"add   $4, %%esp     \n"
		"add   %%eax, %4     \n" // i = strlen(d) + strlen(s)
		"inc   %4            \n" // +1 for terminating null

		"push  %4            \n" // reserve memory
		"call  _malloc       \n"
		"add   $4, %%esp     \n"
		"cmp   $0, %%eax     \n" // malloc fail?
		"je    strins_exit   \n"
		"mov   %%eax, %2     \n" // temp

		//strcpy_(temp, d);
		"push  %0            \n" // d
		"push  %2            \n" // temp
		"call  _strcpy_      \n"
		"add   $8, %%esp     \n"

		//temp[n] = '\0';
		"pop %3              \n" // preserve n
		"mov   %2, %%eax     \n" // eax = temp
		"add   %3, %%eax     \n" // eax = temp + n
		"movb  $0x0, (%%eax) \n" // null terminate string

		//strcat_(temp, s);
		"push  %1            \n" // s
		"push  %2            \n" // temp
		"call  _strcat_      \n"
		"add   $8, %%esp     \n"

		//strcat_(temp, d + n);
		"mov   %0, %%eax     \n" // eax = d
		"add   %3, %%eax     \n" // eax = d + n
		"push  %%eax         \n"
		"push  %2            \n" // temp
		"call  _strcat_      \n"
		"add   $8, %%esp     \n"

		//strcpy_(d, temp);
		"push  %2            \n" // temp
		"push  %0            \n" // d
		"call  _strcpy_      \n"
		"add   $8, %%esp     \n"

		//free(temp);
		"push  %2            \n" // temp
		"call   _free        \n"
		"add   $4, %%esp     \n"
		"strins_exit:        \n"

		: : "D" (d), "S" (s), "b" (temp), "d" (n), "c" (i) : "eax", "memory"
		);
}
*/

