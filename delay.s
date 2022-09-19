; Area de datos, zona de RAM
	AREA	data, READWRITE
		

; Area de programa para las cadenas constantes de mensajes
	AREA	|.text|, CODE, READONLY


	PRESERVE8 {TRUE} 
; Area de programa
	AREA	|.text|, CODE, READONLY, ALIGN=2
	EXPORT delay
	ALIGN	2

delay
    subs    r0, #1
    bne     delay
    bx      lr
	
	ALIGN

	END
