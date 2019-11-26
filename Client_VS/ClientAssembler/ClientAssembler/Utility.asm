; Assembler
; 


TITLE  Utility.asm
INCLUDE Irvine32.inc

.386
.model flat, c

.data

Square BYTE "  ",0

.code

main PROC
    


    exit
main ENDP


drawGridPoint PROC C
	call GotoXY				;move cursor to target grid point
	call SetTextColor		;set square to target color
	mov edx, OFFSET Square	;output a square
	call WriteString



ret
drawGridPoint ENDP


END main