; Area de datos, zona de RAM
	AREA	data, READWRITE

; Area de programa para las cadenas constantes de mensajes
	AREA	|.text|, CODE, READONLY

	PRESERVE8 {TRUE} 
; Area de programa
	AREA	|.text|, CODE, READONLY, ALIGN=2
	EXPORT  xorshift32
	ALIGN   2

xorshift32
    eor     r0, r0, r0, lsl #13
    eor     r0, r0, r0, lsr #17
    eor     r0, r0, r0, lsl #5
    bx      lr
	
	ALIGN

	END
