; Raul Emlio Romero
;
;
        .clink
        .global start

        .asg "5000000",  DELAY
        .asg  0x44e07000, GPIO0       ; GPIO Bank 0, See the AM335x TRM
        .asg  0x481ac000, GPIO2       ; GPIO Bank 2,

        .asg  0x190, GPIO_CLRDATAOUT  ; for clearing the GPIO registers
        .asg  0x194, GPIO_SETDATAOUT  ; for setting the GPIO registers
        .asg  0x138, GPIO_DATAOUT     ; for reading the GPIO registers
	.asg  0x138, GPIO_DATAIN      ; for reading the GPIO registers

	.asg  0x130, GPIO_CTRL        ; ENABLE GPIO PORT
	.asg  0x00,  GPIO_CTRL_ENABLE ;
	.asg  0x134, GPIO_OE          ; SET GPIO INPUT-OUTPUT
        .asg  0xFC3C003F, GPIO_OE_OUTPUT   ; Pines usados como salida bit6 a bit17 y bit22 al bit25 en 0
        .asg  0xF0,  GPIO_OE_INPUT    ; Para todos lo pines usados como entrada bit8 al bit10 en 1

        ;Gpio read
        .asg 1<<10, GPIO0_10_in0     ; P8_31 gpio0[10] input_0 config-pin P8_31 gpio_pd
        .asg 1<<11, GPIO0_11_in1     ; P8_32 gpio0[11] input_1
        .asg 1<<9,  GPIO0_9_in2      ; P8_33 gpio0[9] input_2
        .asg 1<<8,  GPIO0_8_in3      ; P8_35 gpio0[8] input_3

        ; Gpio write
	.asg 1<<22, GPIO2_22_out0    ; P8_27 gpio2[22] output_0 congif-pin P8_27 gpio
	.asg 1<<24, GPIO2_24_out1    ; P8_28 gpio2[24] output_1
	.asg 1<<23, GPIO2_23_out2    ; P8_29 gpio2[23] output_2
	.asg 1<<25, GPIO2_25_out3    ; P8_30 gpio2[25] output_3

        ; Motor
        .asg 1<<17, GPIO2_17_MA_E      ; P8_34 gpio2[17] output_MA_E config-pin P8_34 gpio
	.asg 1<<16, GPIO2_16_MA_D      ; P8_36 gpio2[16] output_MA_D
	.asg 1<<14, GPIO2_14_MA_S      ; P8_37 gpio2[14] output_MA_S
	.asg 1<<15, GPIO2_15_MB_E      ; P8_38 gpio2[15] output_MB_E
	.asg 1<<12, GPIO2_12_MB_D      ; P8_39 gpio2[12] output_MB_D
	.asg 1<<13, GPIO2_13_MB_S      ; P8_40 gpio2[13] output_MB_S
	.asg 1<<10, GPIO2_10_MC_E      ; P8_41 gpio2[10] output_MC_E
	.asg 1<<11, GPIO2_11_MC_D      ; P8_42 gpio2[11] output_MC_D
	.asg 1<<8,  GPIO2_8_MC_S       ; P8_43 gpio2[8] output_MC_S
	.asg 1<<9,  GPIO2_9_MD_E       ; P8_44 gpio2[9] output_MD_E
	.asg 1<<6,  GPIO2_6_MD_D       ; P8_45 gpio2[6] output_MD_D
	.asg 1<<7,  GPIO2_7_MD_S       ; P8_46 gpio2[7] output_MD_S

        .asg  32, PRU0_R31_VEC_VALID  ; allows notification of program completion
        .asg  3,  PRU_EVTOUT_0        ; the event number that is sent back

start:

; GPIO0 configuration
        LDI32   r0, (GPIO0|GPIO_CTRL)  ; Load GPIO0 control register address
        LDI32   r1, GPIO_CTRL_ENABLE   ; Load control enable value
        SBBO    &r1, r0, 0, 4          ; Write enable value to GPIO0 control register

        LDI32   r0, (GPIO0|GPIO_OE)    ; Load GPIO0 output enable register address
        LDI32   r1, GPIO_OE_INPUT      ; Set GPIO0 as input
        SBBO    &r1, r0, 0, 4          ; Write input configuration to GPIO0

; GPIO2 configuration
        LDI32   r0, (GPIO2|GPIO_CTRL)  ; Load GPIO2 control register address
        LDI32   r1, GPIO_CTRL_ENABLE   ; Load control enable value
        SBBO    &r1, r0, 0, 4          ; Write enable value to GPIO2 control register

        LDI32   r0, (GPIO2|GPIO_OE)    ; Load GPIO2 output enable register address
        LDI32   r1, GPIO_OE_OUTPUT     ; Set GPIO2 as output
        SBBO    &r1, r0, 0, 4          ; Write output configuration to GPIO2

MAN:
        LDI32   r1, (GPIO2|GPIO_SETDATAOUT)  ; load addr for GPIO Set data r1
        LDI32   r2, GPIO2_10_MC_E            ; write GPIO0_30 to r2
        SBBO    &r2, r1, 0, 4                ; write r2 to the r1 address value

        LDI32   r0, DELAY
DELAYON:
        SUB     r0, r0, 1
        QBNE    DELAYON, r0, 0   ; Loop to DELAYON, unless REG0=0
LEDOFF:
	;Importante, aunque se sobre escriba todo el registro no se afectan las
        ;Salidas que no queremos afectar. Ver TRM pag. 5015
        LDI32   r1, (GPIO2|GPIO_CLRDATAOUT) ; load addr for GPIO Clear data.
        LDI32   r2, GPIO2_10_MC_E     ; write GPIO_30 to r2
        SBBO    &r2, r1, 0, 4    ; write r2 to the r1 address - LED OFF
        LDI32   r0, DELAY        ; Reset REG0 to the length of the delay

DELAYOFF:
        SUB     r0, r0, 1              ; decrement REG0 by 1
        QBNE    DELAYOFF, r0, 0        ; Loop to DELAYOFF, unless REG0=0

	LDI32   r5, (GPIO0|GPIO_DATAIN); load read addr for DATAIN
        LBBO    &r6, r5, 0, 4          ; Load the value at r5 into r6. Aca vemos todos los pines GPIO0
	QBBC    MAN, r31, 3            ; is the button pressed? If not, loop
END:                                   ; notify the calling app that finished

        LDI32   R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)
        HALT                     ; halt the pru program
