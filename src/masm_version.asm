; Simplistic string functions.
.386
.model flat, C

.code

; int strlen_(const char *s)
strlen_ PROC USES ebp edi ecx
	mov ebp, esp			; construct stack frame
	
	mov edi, [ebp + 16]		; fetch string address
	
	or  ecx, 0ffffffffh		; setup for counting down
	xor eax, eax
	
	cmp edi, 0				; check for null string.
	je Exit

	repne scasb				; count bytes

	not ecx					; flip count down value
	dec ecx

	mov eax, ecx			; prepare return value
Exit :
	ret
strlen_ ENDP

memcpy_ PROC
	push ebp
	mov ebp, esp
	push esi
	push edi
	
	mov edi, [ebp + 8]		; dest
	mov esi, [ebp + 12]		; src
	mov ecx, [ebp + 16]		; count
	cld						; direction = up
	
	rep movsb				; copy string byte from esi to edi until ecx == 0
	
	mov eax, [ebp + 12]		; Return value = dest
	
	pop edi
	pop esi
	pop ebp
	ret
memcpy_ ENDP

; void *memset(void *, int, size_t)
memset_ PROC
	push edi
	
	mov edi, [esp + 8]
	mov al, [esp + 12]
	mov ecx, [esp + 16]
	
	rep stosb
	
	mov eax, [esp + 8]
	
	pop edi
	ret
memset_ ENDP

; char* strcpy(char* dest, const char* src);
;strcpy_ PROC
;	mov  eax, [esp + 8]			; src
;	
;	push eax
;	call strlen_				; length of src
;	pop  ecx					; ecx = src, assume unchanged by strlen_
;	
;	inc  eax					; include terminating zero in length
;	mov  edx, [esp + 4]			; dest
;	
;	push eax					; length + 1
;	push ecx					; src
;	push edx					; dest
;	call memcpy_				; copy
;	add  esp, 12				; clean up stack
;	mov  eax, [esp+4]			; Return value = dest
;	ret
;strcpy_ ENDP

; char *strcat_(char * dest, const char * src);
strcat_ PROC
	mov  eax, [esp + 8]			; src
	
	push eax
	call strlen_				; length of src
	
	inc  eax					; include terminating zero in length
	
	push eax					; strlen(src) + 1
	mov  edx, [esp + 4 + 8]		; dest
	push edx
	call strlen_				; length of dest
	pop  edx					; dest, assume unchanged by strlen_
	
	add  edx, eax				; dest + strlen(dest)
	mov  ecx, [esp + 8 + 8]		; src
								; strlen(src) + 1 is on stack
	push ecx					; src
	push edx					; dest + strlen(dest)
	call memcpy_				; copy
	add  esp, 16				; clean up stack
	
	mov  eax, [esp + 4]			; return dest
	ret
strcat_ ENDP

; char *strchr(const char *str, int c)
strchr_ PROC
; push/pop edi
;	push ebp
;	mov  ebp, esp
;	mov  edi, [ebp + 8]			; get first parameter
;	mov  bl, [ebp + 12]			; set bl to second parameter
	mov  edi, [esp + 4]			; get first parameter
	mov  bl, [esp + 8]			; set bl to second parameter
	mov  al, 0					; set al to null byte
L1 :
	mov  cl, [edi]				; store current character
	cmp  cl, bl					; check if character is what we search
	jz  L2						; jump to return if match

	scasb						; check if null byte
	jnz  L1						; loop if no match

	mov  edi, 0					; set edi to zero, so function will return null
L2 :
	mov  eax, edi				; return pointer to first occurence
;	mov  esp, ebp
;	pop  ebp
	ret
strchr_ ENDP

;int strcmp(const char *, const char *)
strcmp_ PROC
	push ebp
	mov ebp, esp

	push ebx
	push edi
	push esi
	mov edi, [ebp + 12]			; get second string 
	push edi					; determine length of string
	call strlen_				;  
	add esp, 4					; clean up stack                  
	mov ebx, eax				; 
	mov esi, [ebp + 8]			; get first string

	push esi					; determine length of string
	call strlen_				;  
	add esp, 4					; clean up stack          

	cmp eax, ebx				; compare lengths
	ja Greater					; first string is longer 
	jb Less						; second string is longer 
	jmp Equal					; strings have same length

Greater:
	mov eax, 1
	jmp Exit

Less:
	mov eax, -1
	jmp Exit

Equal:
	mov edi, [ebp + 12]			; get second string (restore)
	mov esi, [ebp + 8]			; get first string (restore)
	mov ecx, eax				; length of strings
	repe cmpsb					; compare strings
	jg Greater					; first string is greater
	jl Less						; second string is greater
	mov eax, 0					; strings are equal
	jmp Exit

Exit:
	pop esi
	pop edi
	pop ebx
	mov esp, ebp
	pop ebp
	ret
strcmp_ ENDP

strstr_ PROC
	push	ebp
	mov	ebp, esp
	sub	esp, 4

	push ebx
	push ecx
	push edx
	push esi
	push edi

	mov	esi, [ebp + 8]
	push dword ptr [ebp + 12]
	call strlen_
	add	esp, 4
	cmp	ax, 0
	je	L3

	mov	dword ptr[ebp - 4], eax
	push dword ptr [ebp + 8]
	call strlen_
	add	esp, 4
	cmp	ax, 0
	je L4

	mov	ebx, eax
	dec	esi
	add	ebx, esi				; ebx -> ptr sur fin de chaine
L1:
	xor	eax, eax
	cmp	esi, ebx
	je L5

	inc	esi
	mov	edx, esi
	mov	edi, [ebp + 12]
	mov	ecx, [ebp - 4]
L2:
	mov	al, byte ptr [edx]
	cmp	al, [edi]
	jne	L1

	inc	edx
	inc	edi
	dec	ecx
	jnz	L2

	mov	eax, esi
	jmp	L5
L3:
	mov	eax, [ebp + 8]
	jmp	L5
L4:
	xor	eax, eax
L5:
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret
strstr_	endp

; reverse_ PROC string : PTR BYTE
reverse_ PROC
	push esi
	xor ecx, ecx
	mov esi, [esp + 8]
L1:
	movzx edx, BYTE PTR[esi + ecx]
	cmp dl, 0
	je L2
	inc ecx
	push edx
	jmp L1
L2:
	cmp ecx, 0
	je Exit
	xor eax, eax
L3:
	pop edx
	mov BYTE PTR [esi + eax], dl
	inc eax
	loop L3
Exit:
	pop esi
	mov eax, [esp + 4]		; Return value = dest
	ret
reverse_ ENDP

END 
