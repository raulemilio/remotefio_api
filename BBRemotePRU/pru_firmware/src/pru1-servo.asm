; Raul Emlio Romero
;
;
        .clink
        .global start

        .asg "5000000",  DELAY

        ; Memory
        .asg 0x00000000, RAM0_ADDR    ; local addr ram0. In linux space addr is 0x4a300000
        .asg 0x00002000, RAM1_ADDR    ; local addr ram1. In linux space addr is 0x4a302000
        .asg 0x00010000, SHARED_ADDR  ; local addr shared. In linux space addr is 0x4a310000
        .asg 0x00, SHARED_0_OFFSET    ; shared[0]
	.asg 0x04, SHARED_1_OFFSET    ; shared[1]
	.asg 0x08, SHARED_2_OFFSET    ; shared[2]
        .asg 0x0C, SHARED_3_OFFSET    ; shared[3]

        ; Gpio bank
        .asg 0x44e07000, GPIO0       ; GPIO Bank 0, See the AM335x TRM
        .asg 0x481ac000, GPIO2       ; GPIO Bank 2,

        .asg 0x190, GPIO_CLRDATAOUT  ; for clearing the GPIO registers
        .asg 0x194, GPIO_SETDATAOUT  ; for setting the GPIO registers
        .asg 0x138, GPIO_DATAOUT     ; for reading the GPIO registers
	.asg 0x138, GPIO_DATAIN      ; for reading the GPIO registers

	.asg 0x130, GPIO_CTRL        ; ENABLE GPIO PORT
	.asg 0x00,  GPIO_CTRL_ENABLE ;
	.asg 0x134, GPIO_OE          ; SET GPIO INPUT-OUTPUT
        .asg 0xFC3C003F, GPIO_OE_OUTPUT   ; Pines usados como salida bit6 a bit17 y bit22 al bit25 en 0
        .asg 0xF0,  GPIO_OE_INPUT    ; Para todos lo pines usados como entrada bit8 al bit10 en 1

        ; Gpio_read
        ;.asg (1<<8),  GPIO0_8_in0      ; P8_35 gpio0[8] input_0 config-pin P8_35 gpio_pd
        ;.asg (1<<9),  GPIO0_9_in1      ; P8_33 gpio0[9] input_1
        ;.asg (1<<10), GPIO0_10_in2     ; P8_31 gpio0[10] input_2
        ;.asg (1<<11), GPIO0_11_in3     ; P8_32 gpio0[11] input_3

        ; Gpio_write
	.asg (1<<22), GPIO_out_Base    ;
	;.asg (1<<22), GPIO2_22_out0    ; P8_27 gpio2[22] output_0 config-pin P8_27 gpio
        ;.asg (1<<23), GPIO2_23_out1    ; P8_29 gpio2[23] output_1
	;.asg (1<<24), GPIO2_24_out2    ; P8_28 gpio2[24] output_2
	;.asg (1<<25), GPIO2_25_out3    ; P8_30 gpio2[25] output_3

        ; Motor
	;.asg (1<<6),  GPIO2_6_MD_E       ; P8_45 gpio2[6] output_MA_E config-pin P8_45 gpio
	;.asg (1<<7),  GPIO2_7_MD_D       ; P8_46 gpio2[7] output_MA_D
        ;.asg (1<<8),  GPIO2_8_MC_S       ; P8_43 gpio2[8] output_MA_S

        ;.asg (1<<9),  GPIO2_9_MD_E       ; P8_44 gpio2[9] output_MB_E
        ;.asg (1<<10), GPIO2_10_MC_D      ; P8_41 gpio2[10] output_MB_D
        ;.asg (1<<11), GPIO2_11_MC_S      ; P8_42 gpio2[11] output_MB_S

        ;.asg (1<<12), GPIO2_12_MB_E      ; P8_39 gpio2[12] output_MC_E
        ;.asg (1<<13), GPIO2_13_MB_D      ; P8_40 gpio2[13] output_MC_D
        ;.asg (1<<14), GPIO2_14_MA_S      ; P8_37 gpio2[14] output_MC_S

        ;.asg (1<<15), GPIO2_15_MB_E      ; P8_38 gpio2[15] output_MD_E
        ;.asg (1<<16), GPIO2_16_MA_D      ; P8_36 gpio2[16] output_MD_D
        ;.asg (1<<17), GPIO2_17_MA_S      ; P8_34 gpio2[17] output_MD_S

	; Flags
	.asg 0x00, GPIO_READ_FLAG     ;
        .asg 0x01, GPIO_WRITE_FLAG    ;
	.asg 0x02, MOTOR_FLAG         ;
        .asg 0x00, gpio_w_pos_flag    ;


        .asg 32, PRU0_R31_VEC_VALID     ; allows notification of program completion
        .asg 3,  PRU_EVTOUT_0           ; the event number that is sent back

start:

SETUP:
; Memory inicializer
        ZERO    r1, 0                              ;
        LDI32   r0, SHARED_0_OFFSET                ; Load the RAM address (SHARED[0]) into r0
        SBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Store the modified value back to SHARED[0]
        LDI32   r0, SHARED_1_OFFSET                ; Load the RAM address (SHARED[1]) into r0
        SBBO    &r1, r0, SHARED_1_OFFSET, 4        ; Store the modified value back to SHARED[1]
        LDI32   r0, SHARED_2_OFFSET                ; Load the RAM address (SHARED[2]) into r0
        SBBO    &r1, r0, SHARED_2_OFFSET, 4        ; Store the modified value back to SHARED[2]
        LDI32   r0, SHARED_3_OFFSET                ; Load the RAM address (SHARED[3]) into r0
        SBBO    &r1, r0, SHARED_3_OFFSET, 4        ; Store the modified value back to SHARED[3]
        LDI32   r0, SHARED_3_OFFSET                ; Load the RAM address (SHARED[4]) into r0
        SBBO    &r1, r0, SHARED_4_OFFSET, 4        ; Store the modified value back to SHARED[4]
        LDI32   r0, SHARED_3_OFFSET                ; Load the RAM address (SHARED[5]) into r0
        SBBO    &r1, r0, SHARED_5_OFFSET, 4        ; Store the modified value back to SHARED[5]
        LDI32   r0, SHARED_3_OFFSET                ; Load the RAM address (SHARED[6]) into r0
        SBBO    &r1, r0, SHARED_6_OFFSET, 4        ; Store the modified value back to SHARED[6]
        LDI32   r0, SHARED_3_OFFSET                ; Load the RAM address (SHARED[7]) into r0
        SBBO    &r1, r0, SHARED_7_OFFSET, 4        ; Store the modified value back to SHARED[7]

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

MAIN_LOOP:
        ; Get state gpio_read function flag
        LDI32   r0, SHARED_ADDR                    ; Load the RAM address (SHARED) into r0
        LBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Load 4 bytes from SHARED[0] into r1
	QBBS    GPIO_READ, r1, GPIO_READ_FLAG      ; Jump is GPIO_READ_FLAG is SET

        ; Get state gpio_write function flag
	LDI32   r0, SHARED_ADDR                    ; Load the RAM address (SHARED) into r0
        LBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Load 4 bytes from SHARED[0] into r1
        QBBS    GPIO_WRITE, r1, GPIO_WRITE_FLAG    ; Jump is GPIO_WRITE_FLAG is SET

        ; Get state motor function flag
        LDI32   r0, SHARED_ADDR                    ; Load the RAM address (SHARED) into r0
        LBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Load 4 bytes from SHARED[0] into r1
        QBBS    MOTOR, r1, MOTOR_FLAG              ; Jump is MOTOR_FLAG is SET


GPIO_READ:
; Read GPIO IN
        LDI32   r0, (GPIO0|GPIO_DATAIN)            ; load read addr for DATAIN
        LBBO    &r1, r0, 0, 4                      ; Load the values at r0 into r1.
; Write GPIO IN DATA INTO SHARED[1]
        LDI32   r0, (SHARED_ADDR|SHARED_1_OFFSET)  ; Load the RAM address (SHARED[1]) into r0
        SBBO    &r1, r0, 0, 4                      ; Store the values of r3 at shared[1]
; Clear flag
	LDI32   r0, SHARED_0_OFFSET                ; Load the RAM address (SHARED[0]) into r0
	LBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Load 4 bytes from SHARED[0] into r1
        CLR     r1, r1, GPIO_READ_FLAG             ; clear flag gpio_read
        SBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Store the modified value back to SHARED[0]
	RET

GPIO_WRITE:
; Read Gpio_output to SET
        LDI32   r0, (SHARED_ADDR|SHARED_2_OFFSET)  ; Load the RAM address shared[2] into r0
        LBBO    &r1, r0, 0, 4                      ; Load 4 bytes from shared[2] into r1
        AND     r2, r1, 0x0F      		   ; Extrae los primeros 4 bits (0-3) en r2. Flag de mascaras
        AND     r3, r1, 0xF0      		   ; Extrae los bits 4-7 en r3. Pin state
        LSR     r3, r3, 4         		   ; Desplaza los bits 4-7 a la posicion menos significativa en r3

        MOV	r4, 0            	           ; Inicializa el contador de bits (0 a 3)
	LDI32   r7, (1<<22)			   ; Direccion base de GPIO_out0

check_bits:
        AND     r5, r3, 1  			   ; Extrae el bit menos significativo de r3 (state 0 o 1)
        QBBS    write_out_funct, r2, r4            ; qbbs myLabel r2, r4 //Branch if( r2&(1<<r4) )
        LSR     r3, r3, 1			   ; Desplaza r3 a la derecha para procesar el siguiente bit de state
        LSL     r7, r7, 1                          ; Desplaza hacia el proximo bit de GPIO_out1,...GPIO_out2....
	ADD     r4, r4, 1			   ; Increnta contador
	QBLT    r4, 4, check_bits                  ; Se realiza 4 veces ya que son cuatro salidas
; Clear flag
        LDI32   r0, SHARED_0_OFFSET                ; Load the RAM address (SHARED[0]) into r0
        LBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Load 4 bytes from SHARED[0] into r1
        CLR     r1, r1, GPIO_WRITE_FLAG            ; clear flag gpio_read
        SBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Store the modified value back to SHARED[0]
	RET

gpio_write_funct:
        QBBS    write_out_set, r5, 0		   ;
        QBBC    write_out_clr, r5, 0               ; Jamp si bit0 de r5 es clr
        RET
write_out_set:
        LDI32   r6, (GPIO2|GPIO_SETDATAOUT)        ; load addr for GPIO Set data r6
        SBBO    &r7, r6, 0, 4                      ; write r7 to the r6 address valu
	RET
write_out_clr:
        LDI32   r6, (GPIO2|GPIO_CLRDATAOUT)        ; load addr for GPIO Clear data.
        SBBO    &r7, r6, 0, 4                      ; write r7 to the r6 address - LED OFF
	RET

MOTOR:
        LDI32   r0, (SHARED_ADDR|SHARED_3_OFFSET)  ; Load the RAM address shared[3] into r0
        LBBO    &r1, r0, 0, 4                      ; Load 4 bytes from shared[3] into r1
        AND     r2, r1, 0x0F                       ; Extrae los primeros 4 bits (0-3) en r2. Flag de mascaras
        AND     r3, r1, 0xFF0                      ; Extrae los bits 4-11 en r3. Pins enable
        LSR     r3, r3, 4                          ; Desplaza los bits 4-11 a la posicion menos significativa en r3

        MOV     r4, 0                              ; Inicializa el contador de bits (0 a 3)
	LDI32   r7, (1<<6)			   ; Direccion base GPIO2_6_ME_BASE

check_bits:
        AND     r5, r3, 0x02                       ; Extrae los dos primeros  bits. Bit0 -> Enable Bit1-> Dir
        QBBS    motor_config, r2, r4               ; qbbs myLabel r2, r4 //Branch if( r2&(1<<r4) )
        LSR     r3, r3, 2                          ; Desplaza r3 a la derecha para procesar los siguientes 2 bits
        ADD     r4, r4, 1                          ; Increnta contador
	ADD     r7, r7, 2                          ; Desplazo a las siguientes direcciones
        QBLT    r4, 4, check_bits                  ; Se realiza 4 veces ya que son cuatro motores
; Clear flag
        LDI32   r0, SHARED_0_OFFSET                ; Load the RAM address (SHARED[0]) into r0
        LBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Load 4 bytes from SHARED[0] into r1
        CLR     r1, r1, GPIO_MOTOR_FLAG            ; clear flag gpio_read
        SBBO    &r1, r0, SHARED_0_OFFSET, 4        ; Store the modified value back to SHARED[0]

motor_config:
        QBBS    enable_set, r3, 0		   ;
	QBBS    dir_set, r3, 1			   ;
	QBBC    enable_clr, r4, 0		   ;
	QBBC    dir_clr, r3, 1    		   ;
enable_set:
	LDI32   r6, (GPIO2|GPIO_SETDATAOUT)        ; load addr for GPIO Set data r6
	SBBO    &r5, r6, 0, 4                      ; write r7 to the r6 address value
        RET
dir_set:
        LDI32   r6, (GPIO2|GPIO_SETDATAOUT)        ; load addr for GPIO Set data r6
        SBBO    &r5, r6, 0, 4                      ; write r7 to the r6 address value
        RET
enable_clr:
        LDI32   r6, (GPIO2|GPIO_SETDATAOUT)        ; load addr for GPIO Set data r6
        SBBO    &r5, r6, 0, 4                      ; write r7 to the r6 address value
	RET
dir_clr:
        LDI32   r6, (GPIO2|GPIO_SETDATAOUT)        ; load addr for GPIO Set data r6
        SBBO    &r5, r6, 0, 4                      ; write r7 to the r6 address value
        RET

delay_A:
	QBBS    step_set, r3, 4                    ; Jump is enable MA set

step_set:
	



        LDI32   R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)
        HALT                     ; halt the pru program
