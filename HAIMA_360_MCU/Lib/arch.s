
;	AREA    |.ARM.__AT_0x20000000|, NOINIT, READWRITE, ALIGN=3
;	EXPORT __ipc_base
;__ipc_base SPACE 1024

	PRESERVE8
	
;	AREA    |.ARM.__AT_0x8003200|, CODE, READONLY
;	IMPORT __main
;	DCD	__main
;	DCD ipc_base
;	DCD		0x47414c46 ; 'G' 'A' 'L' 'F'

	AREA    ARCH, CODE, READONLY
	EXPORT  asm_delay
	EXPORT  asm_process_switch

; Function void asm_delay(u32 tm, u32 div)

asm_delay PROC

	; r1 = 12000 for 72MHz config, 6000 for 36MHz => ms
	; r1 = 12 for 72MHz config, 6 for 36MHz => us
	mul r0, r1, r0
next
	cmp r0, #0 ; 1 cycle
	beq back ;  3 if taken 1 if not
	subs r0, r0, #1 ; 1 cycle
	b next ; 3 cycles
	
back
	bx lr
	
	ENDP

; Function void asm_process_switch(u32 entry, u32 stack)

asm_process_switch PROC

	mov sp, r1
	bx r0

	ENDP

	END

