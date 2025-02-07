
; PRU setup definitions
	; .asg    C4,     CONST_SYSCFG
	.asg    C28,    CONST_PRUSHAREDRAM

	.asg	0x22000,	PRU0_CTRL
	.asg    0x24000,    PRU1_CTRL       ; page 19
	; .asg    0x28,       CTPPR0          ; page 75

	.asg	0x000,	OWN_RAM
	.asg	0x020,	OTHER_RAM
	.asg    0x100,	SHARED_RAM       ; This is so prudebug can find it.
	.asg    64,     CNT_OFFSET

	.asg	0x00000000, ram0_addr	; local addr ram0. In linux space addr is 0x4a300000
	.asg	0x00002000, ram1_addr	; local addr ram0. In linux space addr is 0x4a302000
	.asg	0x00010000, shared_addr	; local addr ram0. In linux space addr is 0x4a310000
	.asg	0x800, data0		; 2048 in dec
	.asg	0xfff, data1		; 4095 in dec
	.asg	0x0ff, data3		; 255 in dec
	.clink
	.global start
start:
; initialize by setting current state of two channels
	LDI32	r0, data0	  ;
	LDI32	r1, ram0_addr	  ;
	LDI32	r2, data1	  ;
	LDI32	r3, ram1_addr	  ;
        LDI32   r4, data3         ;
        LDI32   r5, shared_addr   ;
	SBBO	&r0, r1, 0, 4	; write to ram0
	SBBO	&r2, r3, 0, 4	; write to ram1
	SBBO	&r4, r5, 0, 4	; write to shared

	HALT	; we should never actually get here
