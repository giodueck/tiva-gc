; Area de datos, zona de RAM
	AREA	data, READWRITE
		

; Area de programa para las cadenas constantes de mensajes
	AREA	|.text|, CODE, READONLY


	PRESERVE8 {TRUE} 
; Area de programa
	AREA	|.text|, CODE, READONLY, ALIGN=2
	EXPORT _delay
	ALIGN	2

_delay
    subs    r0, #1
    bne     _delay
    bx      lr
	
	ALIGN

	END
