TITLE ASM_Function
.386
.model flat, c

.data
endl EQU <0dh,0ah>            ; end of line sequence
message LABEL BYTE
    BYTE "This program is a simple demonstration of"
    BYTE "console mode output, using the GetStdHandle"
    BYTE "and WriteConsole functions.",endl
messageSize DWORD ($ - message)

consoleHandle HANDLE 0        ; handle to standard output device
bytesWritten  DWORD ?         ; number of bytes written
.code
mania PROC
  ; Get the console output handle:
    INVOKE GetStdHandle, STD_OUTPUT_HANDLE
    mov consoleHandle,eax
  ; Write a string to the console:
    INVOKE WriteConsole,
      consoleHandle,          ; console output handle
      ADDR message,           ; string pointer
      messageSize,            ; string length
      ADDR bytesWritten,      ; returns num bytes written
      0                       ; not used
mania ENDP

;------------------------------------------------------
negate PROC inp:DWORD
;  This function take the input, negates it and returns
;  the resulting value in eax
;------------------------------------------------------
	mov eax, inp
	neg eax
	
	ret
negate ENDP

;------------------------------------------------------
mask_bits PROC inp:DWORD
;  This function take the input, applies the mask
;  0000FFFFh to the number and returns the result
;------------------------------------------------------
	mov eax, inp
	and eax, 0000FFFFh
	
	ret
mask_bits ENDP

;----------------------------------------------------
; function gcd(a, b)
;     while b != 0
;         c := b
;         b := a mod b
;         a := c
;     return a
;
;-----------------------------------------------------
gcd PROC a:DWORD, b:DWORD
;-----------------------------------------------------
    mov edx, 0         ; store a in edx:eax
    mov eax, a
    mov ebx, b         ; store b in ebx
    LDO:
       cmp ebx, 0      ; While b != 0
       jz LDONE
       mov ecx, ebx    ; c = b
       div ebx         ; b = a % b
       mov ebx, edx    ; store result of % in b
       mov edx, 0      ; store c in edx:eax
       mov eax, ecx 
       jmp LDO
    LDONE:
       ret             ; GCD is in eax the standard return value register
    
gcd ENDP

;drawGridPoint PROC x:DWORD, y:DWORD, color:DWORD 
;
	;; set the x coordinate to the current column
	;mov dh,BYTE PTR x
	;; set the y coordinate to the current row
	;mov dl,BYTE PTR y
;
	;; passes color to eax
	;mov eax, DWORD PTR color
;
	;call GotoXY				;move cursor to target grid point
	;call SetTextColor		;set square to target color
	;mov edx, OFFSET Square	;output a square
	;call WriteString
	;ret
;drawGridPoint ENDP

END
