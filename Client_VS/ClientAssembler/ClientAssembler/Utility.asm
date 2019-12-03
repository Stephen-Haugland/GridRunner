; Assembler Code for Displaying the Game

TITLE  Utility_Functions
;INCLUDE Irvine32.inc

.386
.model flat, c
.data

Square BYTE "  ",0


.code
drawGridPoint PROC x:DWORD, y:DWORD, color:DWORD 

	; set the x coordinate to the current column
	mov dh,BYTE PTR x
	; set the y coordinate to the current row
	mov dl,BYTE PTR y

	; passes color to eax
	mov eax, DWORD PTR color

	;call GotoXY				;move cursor to target grid point
	;call SetTextColor		;set square to target color
	;mov edx, OFFSET Square	;output a square
	;call WriteString
	ret
drawGridPoint ENDP


END