; Raul Emlio Romero
;
; cd /sys/class/remoteproc/remoteproc2
; echo 'am335x-pru1-digital-fw' > firmware
; echo 'start' > state
; echo 'stop' > state
; cd /sys/kernel/debug/remoteproc/remoteproc2
; sudo cat regs
;
; PRU definitions

; pru1Control
  .asg 0x22000, PRU0_CTRL
  .asg 0x24000, PRU1_CTRL                               ; page 19
  .asg 0x28, CTPPR0                                     ; page 75

; memory
  .asg 0x00000000, RAM0_ADDR                            ; local addr ram0. In linux space addr is 0x4a300000
  .asg 0x00002000, RAM1_ADDR                            ; local addr ram1. In linux space addr is 0x4a302000
  .asg 0x00010000, SHARED_ADDR                          ; local addr shared. In linux space addr is 0x4a310000

  .asg 0x00, SHD_GPIO_INPUT_FLAGS   	                ; shared[0] gpio_input flag function
  .asg 0x04, SHD_GPIO_INPUT_DATA      	                ; shared[1] gpio_input data

  .asg 0x08, SHD_GPIO_OUTPUT_GET_FLAGS                  ; shared[2] gpio_output get flag function
  .asg 0x0C, SHD_GPIO_OUTPUT_GET_DATA                   ; shared[3] gpio_output get data

  .asg 0x10, SHD_GPIO_OUTPUT_SET_FLAGS                  ; shared[4] gpio_output set flag function
  .asg 0x14, SHD_GPIO_OUTPUT_SET_DATA                   ; shared[5] gpio_output sert data

  .asg 0x18, SHD_MOTOR_GET_FLAGS                        ; shared[6] motor get flag function
  .asg 0x1C, SHD_MOTOR_GET_DATA                         ; shared[7] motor get data
  .asg 0x20, SHD_MOTOR_SET_FLAGS                        ; shared[8] motor set flag function
  .asg 0x24, SHD_MOTOR_SET_DATA                         ; shared[9] motor set data

  .asg 0x28, SHD_MOTOR_STEP_PERIOD_A                    ; shared[10] motor_A STEP_PERIOD_A
  .asg 0x2C, SHD_MOTOR_STEP_PERIOD_B                    ; shared[11] motor_B STEP_PERIOD_B
  .asg 0x30, SHD_MOTOR_STEP_PERIOD_C                    ; shared[12] motor_C STEP_PERIOD_C
  .asg 0x34, SHD_MOTOR_STEP_PERIOD_D                    ; shared[13] motor_D STEP_PERIOD_D

  .asg 0x0F, SHARED_MEM_SIZE                            ; 15 en decimal
  .asg 0x04, OFFSET_MEM                                 ;

; gpio bank
  .asg 0x44e07000, GPIO0                                ; GPIO Bank 0, See the AM335x TRM
  .asg 0x481ac000, GPIO2                                ; GPIO Bank 2

  .asg 0x190, GPIO_CLRDATAOUT                           ; for clearing the GPIO registers
  .asg 0x194, GPIO_SETDATAOUT                           ; for setting the GPIO registers
  .asg 0x13C, GPIO_DATAOUT                              ; for read-write the GPIO registers
  .asg 0x138, GPIO_DATAIN                               ; for reading the GPIO registers

  .asg 0x130, GPIO_CTRL                                 ; enable GPIO port
  .asg 0x00,  GPIO_CTRL_ENABLE                          ;
  .asg 0x134, GPIO_OE                                   ; set GPIO input - output

  .asg 0x38,  GPIO_IRQSTATUS_SET_1			; habilita las interrupciones por set
  .asg 0x148, GPIO_RISINGDETECT				; habilita las interrupciones por flanco asc
  .asg 0x30,  GPIO_IRQSTATUS_1				; get-set irq

; gpio_read
  .asg 22, GPIO2_22_GPIO_INPUT_0                        ; P8_27
  .asg 23, GPIO2_23_GPIO_INPUT_1                        ; P8_29
  .asg 24, GPIO2_24_GPIO_INPUT_2                        ; P8_28
  .asg 25, GPIO2_25_GPIO_INPUT_3                        ; P8_30

  ; INPUT TRIGGER
  .asg 6, GPIO2_6_GPIO_INPUT_TRIGGER                    ; P8_45
  .asg 7, GPIO2_7_MOTOR_TRIGGER                         ; P8_46

; gpio_write
  .asg (1<<8),  GPIO_OUT_BASE                           ;
  .asg 8,  GPIO0_8_OUTPUT_0                             ; P8_35
  .asg 9,  GPIO0_9_OUTPUT_1                             ; P8_33
  .asg 10, GPIO0_10_OUTPUT_2                            ; P8_31
  .asg 11, GPIO0_11_OUTPUT_3                            ; P8_32

; motor
  ; Enable-Direction
  .asg (1<<2), GPIO_MOTOR_ENA_BASE                      ;
  .asg (1<<14), GPIO_MOTOR_DIR_BASE                     ;
  ; Pins enable
  .asg 2, OUTPUT_MA_E                                   ; GPIO2[2] P8_07
  .asg 3, OUTPUT_MB_E                                   ; GPIO2[2] P8_08
  .asg 4, OUTPUT_MC_E                                   ; GPIO2[2] P8_10
  .asg 5, OUTPUT_MD_E                                   ; GPIO2[2] P8_09
  ; Pins direction
  .asg 14, OUTPUT_MA_D                                  ; GPIO2[14] P8_37
  .asg 15, OUTPUT_MB_D                                  ; GPIO2[15] P8_38
  .asg 16, OUTPUT_MC_D                                  ; GPIO2[16] P8_36
  .asg 17, OUTPUT_MD_D                                  ; GPIO2[17] P8_34

  ; Step period (medio ciclo de la senial)
  .asg (1<<4), OUTPUT_MA_S                              ; PRU1_R30_4 P8_41
  .asg (1<<5), OUTPUT_MB_S                              ; PRU1_R30_5 P8_42
  .asg (1<<6), OUTPUT_MC_S                              ; PRU1_R30_6 P8_39
  .asg (1<<7), OUTPUT_MD_S                              ; PRU1_r30_7 P8_40

; functions flags
  .asg 0, GPIO_INPUT_MODE0_FLAG                         ;
  .asg 1, GPIO_INPUT_MODE1_FLAG                         ;
  .asg 2, GPIO_INPUT_MODE2_FLAG                         ;
  .asg 0, GPIO_OUTPUT_GET_MODE0_FLAG                    ;
  .asg 0, GPIO_OUTPUT_SET_MODE0_FLAG                    ;
  .asg 0, MOTOR_GET_MODE0_FLAG                          ;
  .asg 1, MOTOR_GET_MODE1_FLAG                          ;
  .asg 2, MOTOR_GET_MODE2_FLAG                          ;
  .asg 0, MOTOR_SET_MODE0_FLAG                          ;

  .asg 12, GPIO_INPUT_DATARDY_FLAG 	                ;
  .asg 12, GPIO_OUTPUT_GET_DATARDY_FLAG                 ;
  .asg 12, GPIO_OUTPUT_SET_DATARDY_FLAG                 ;
  .asg 12, MOTOR_GET_DATARDY_FLAG                       ;
  .asg 12, MOTOR_SET_DATARDY_FLAG                       ;

  .asg 32, PRU0_R31_VEC_VALID                           ; allows notification of program completion
  .asg 3,  PRU_EVTOUT_0                                 ; the event number that is sent back

  .clink
  .global start

start:
; Registro fijos
  ; r10-> SHARED  -> STEP_PERIOD_A
  ; r11-> INTERNAL-> STEP_PERIOD_A
  ; r12-> SHARED  -> STEP_PERIOD_B
  ; r13-> INTERNAL-> STEP_PERIOD_B
  ; r14-> SHARED  -> STEP_PERIOD_C
  ; r15-> INTERNAL-> STEP_PERIOD_C
  ; r16-> SHARED  -> STEP_PERIOD_D
  ; r17-> INTERNAL-> STEP_PERIOD_D
  ; r20-> SHARED BASE ADDR
  ; r21-> r31 previus state
  ; r22-> GPIO2 interrup addr
  ; r23-> load interrupt state
SETUP:
  LDI32 r20, SHARED_ADDR                                ; shared_addr_base
; borrado de momoria
  ZERO  &r0, 4                                          ; zero put register
  LDI32 r1, 0                                           ; offset_mem
  LDI32 r2, 0                                           ; Count mem
mem_init:
  SBBO  &r0, r20, r1, 4                                 ;
  ADD   r1, r1, 4                                       ; cada posicion de memoria ocupa 4 bytes
  ADD   r2, r2, 1                                       ; count++
  QBGT  mem_init, r2, SHARED_MEM_SIZE                   ; qbgt myLabel, r3, r4. Branch if r4 > r3
  LDI32 r0, 1000;
  MOV r11, r0;
  MOV r13, r0;
  MOV r15, r0;
  MOV r17, r0;

gpio_config:
;GPIO0
  LDI32 r0, (GPIO0|GPIO_CTRL)                           ; load GPIO0 control register address
  LDI32 r1, GPIO_CTRL_ENABLE                            ; load control enable value
  SBBO  &r1, r0, 0, 4                                   ; write enable value to GPIO0 control register

  LDI32 r0, (GPIO0|GPIO_OE)                             ; load GPIO0 output enable register address
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  CLR   r1, r1, GPIO0_8_OUTPUT_0                        ;
  CLR   r1, r1, GPIO0_9_OUTPUT_1                        ;
  CLR   r1, r1, GPIO0_10_OUTPUT_2                       ;
  CLR   r1, r1, GPIO0_11_OUTPUT_3                       ;
  SBBO  &r1, r0, 0, 4                                   ; write input configuration to GPIO0

;GPIO2
  LDI32 r0, (GPIO2|GPIO_CTRL)                           ; load GPIO0 control register address
  LDI32 r1, GPIO_CTRL_ENABLE                            ; load control enable value
  SBBO  &r1, r0, 0, 4                                   ; write enable value to GPIO0 control register

  LDI32 r0, (GPIO2|GPIO_OE)
  LBBO  &r1, r0, 0, 4
  SET   r1, r1, GPIO2_6_GPIO_INPUT_TRIGGER
  SET   r1, r1, GPIO2_7_MOTOR_TRIGGER
  SET   r1, r1, GPIO2_22_GPIO_INPUT_0
  SET   r1, r1, GPIO2_23_GPIO_INPUT_1
  SET   r1, r1, GPIO2_24_GPIO_INPUT_2
  SET   r1, r1, GPIO2_25_GPIO_INPUT_3
  CLR   r1, r1, OUTPUT_MA_E
  CLR   r1, r1, OUTPUT_MB_E
  CLR   r1, r1, OUTPUT_MC_E
  CLR   r1, r1, OUTPUT_MD_E
  CLR   r1, r1, OUTPUT_MA_D
  CLR   r1, r1, OUTPUT_MB_D
  CLR   r1, r1, OUTPUT_MC_D
  CLR   r1, r1, OUTPUT_MD_D
  SBBO  &r1, r0, 0, 4

  ; rising detect
  LDI32 r0, (GPIO2|GPIO_RISINGDETECT)
  LBBO  &r1, r0, 0, 4
  SET   r1, r1, GPIO2_6_GPIO_INPUT_TRIGGER
  SET   r1, r1, GPIO2_7_MOTOR_TRIGGER
  SET   r1, r1, GPIO2_22_GPIO_INPUT_0
  SET   r1, r1, GPIO2_23_GPIO_INPUT_1
  SET   r1, r1, GPIO2_24_GPIO_INPUT_2
  SET   r1, r1, GPIO2_25_GPIO_INPUT_3
  SBBO  &r1, r0, 0, 4

  ; enable irq set_1
  LDI32 r0, (GPIO2|GPIO_IRQSTATUS_SET_1)
  LBBO  &r1, r0, 0, 4
  SET   r1, r1, GPIO2_6_GPIO_INPUT_TRIGGER
  SET   r1, r1, GPIO2_7_MOTOR_TRIGGER
  SET   r1, r1, GPIO2_22_GPIO_INPUT_0
  SET   r1, r1, GPIO2_23_GPIO_INPUT_1
  SET   r1, r1, GPIO2_24_GPIO_INPUT_2
  SET   r1, r1, GPIO2_25_GPIO_INPUT_3
  SBBO  &r1, r0, 0, 4

  LDI32 r0, (GPIO2|GPIO_DATAOUT)
  LBBO  &r1, r0, 0, 4
  SET   r1, r1, OUTPUT_MA_E      		; DISABLE A4988
  SET   r1, r1, OUTPUT_MB_E
  SET   r1, r1, OUTPUT_MC_E
  SET   r1, r1, OUTPUT_MD_E
  SBBO  &r1, r0, 0 , 4

  LDI32 r22, (GPIO2|GPIO_IRQSTATUS_1)

MAIN_LOOP:
level_gpio_input:
  LBBO  &r0, r20, SHD_GPIO_INPUT_FLAGS, 4
  QBBS  GPIO_INPUT_MODE0, r0, GPIO_INPUT_MODE0_FLAG
; get irq status
  LBBO  &r23, r22, 0, 4
  QBBS  GPIO_INPUT_MODE1, r23, GPIO2_6_GPIO_INPUT_TRIGGER
level_gpio_input_mode2:
  QBBS  GPIO_INPUT_MODE2, r23, GPIO2_22_GPIO_INPUT_0
  QBBS  GPIO_INPUT_MODE2, r23, GPIO2_23_GPIO_INPUT_1
  QBBS  GPIO_INPUT_MODE2, r23, GPIO2_24_GPIO_INPUT_2
  QBBS  GPIO_INPUT_MODE2, r23, GPIO2_25_GPIO_INPUT_3
level_gpio_output_get:
  LBBO  &r0, r20, SHD_GPIO_OUTPUT_GET_FLAGS, 4
  QBBS  GPIO_OUTPUT_GET_MODE0, r0, GPIO_OUTPUT_GET_MODE0_FLAG
level_gpio_output_set:
  LBBO  &r0, r20, SHD_GPIO_OUTPUT_SET_FLAGS, 4
  QBBS  GPIO_OUTPUT_SET_MODE0, r0, GPIO_OUTPUT_SET_MODE0_FLAG
level_motor_get:
  LBBO  &r0, r20, SHD_MOTOR_GET_FLAGS, 4
  QBBS  MOTOR_GET_MODE0, r0, MOTOR_GET_MODE0_FLAG
  QBBS  MOTOR_GET_MODE1, r23, GPIO2_7_MOTOR_TRIGGER
level_motor_set:
  LBBO  &r0, r20, SHD_MOTOR_SET_FLAGS, 4
  QBBS  MOTOR_SET_MODE0, r0, MOTOR_SET_MODE0_FLAG

level_motorA_step_time:
  QBA  STEP_PERIOD_A
level_motorB_step_time:
  QBA  STEP_PERIOD_B
level_motorC_step_time:
  QBA  STEP_PERIOD_C
level_motorD_step_time:
  QBA  STEP_PERIOD_D

  QBA   MAIN_LOOP                                       ; program loop

GPIO_INPUT_MODE0:
; clr flag gpio_input
  LBBO  &r0, r20, SHD_GPIO_INPUT_FLAGS, 4    	        ;
  CLR   r0, r0, GPIO_INPUT_MODE0_FLAG                   ;
  SBBO  &r0, r20, SHD_GPIO_INPUT_FLAGS, 4               ;
; write GPIO IN DATA INTO SHARED
  LDI32 r0, (GPIO2|GPIO_DATAIN)				;
  LBBO  &r1, r0, 0, 4					;
  LDI32 r0, 0x03C00000                                  ; mascara desde bit22 a bit25
  AND   r1, r1, r0                                      ;
  LSR   r1, r1, 22                                      ; se desplazan 22 posiciones para que los datos queden a partir del bit 0
; set flag data ready
  SET   r1, r1, GPIO_INPUT_DATARDY_FLAG                 ; bit12-> flag data ready
  SBBO  &r1, r20, SHD_GPIO_INPUT_DATA, 4                ; Cargamos valores de las entradas gpio_read bit0-bit3 y el flag data ready
  QBA   level_gpio_output_get                           ;

GPIO_INPUT_MODE1:
  LBBO  &r0, r20, SHD_GPIO_INPUT_FLAGS, 4               ;
  QBBC  level_gpio_input_mode2, r0, GPIO_INPUT_MODE1_FLAG;
; clear IRQ_GPIO_IRQSTATUS_1
  LBBO  &r0, r22, 0, 4                                  ;
  SET   r0, r0, GPIO2_6_GPIO_INPUT_TRIGGER              ;
  SBBO  &r0, r22, 0, 4                                  ;
; read GPIO IN
  LDI32 r0, (GPIO2|GPIO_DATAIN)                         ;
  LBBO  &r1, r0, 0, 4                                   ;
  LDI32 r0, 0x03C00000                                  ;
  AND   r1, r1, r0                                      ;
  LSR   r1, r1, 22                                      ;
; set flag rising detect
  SET   r1, r1, GPIO_INPUT_DATARDY_FLAG                 ;
  SBBO  &r1, r20, SHD_GPIO_INPUT_DATA, 4                ;
  QBA   level_gpio_output_get

GPIO_INPUT_MODE2:
  LBBO  &r0, r20, SHD_GPIO_INPUT_FLAGS, 4               ;
  QBBC  level_gpio_output_get, r0, GPIO_INPUT_MODE2_FLAG;
; clear IRQ_GPIO_IRQSTATUS_1
  LBBO  &r0, r22, 0, 4                                  ;
  SET   r0, r0, GPIO2_22_GPIO_INPUT_0                   ;
  SET   r0, r0, GPIO2_23_GPIO_INPUT_1                   ;
  SET   r0, r0, GPIO2_24_GPIO_INPUT_2                   ;
  SET   r0, r0, GPIO2_25_GPIO_INPUT_3                   ;
  SBBO  &r0, r22, 0, 4                                  ;
; write GPIO IN DATA INTO SHARED
  LDI32 r0, (GPIO2|GPIO_DATAIN)                         ;
  LBBO  &r1, r0, 0, 4                                   ;
  LDI32 r0, 0x03C00000                                  ;
  AND   r1, r1, r0                                      ;
  LSR   r1, r1, 22                                      ;
; set flag rising detect
  SET   r1, r1, GPIO_INPUT_DATARDY_FLAG                 ;
  SBBO  &r1, r20, SHD_GPIO_INPUT_DATA, 4                ;
  QBA   level_gpio_output_get                           ;

GPIO_OUTPUT_GET_MODE0:
; clr flag gpio_output
  LBBO  &r0, r20, SHD_GPIO_OUTPUT_GET_FLAGS, 4          ;
  CLR   r0, r0, GPIO_OUTPUT_GET_MODE0_FLAG              ;
  SBBO  &r0, r20, SHD_GPIO_OUTPUT_GET_FLAGS, 4          ;
; read GPIO_DATAOUT
  LDI32 r0, (GPIO0|GPIO_DATAOUT)                        ;
  LBBO  &r1, r0, 0, 4                                   ;
; write GPIO DATAOUT INTO SHARED
  LDI32 r0, 0xF00                                       ; mascara desde bit8 a bit11 porque en el registro GPIO_DATAOUT estan en esa posicion
  AND   r1, r1, r0                                      ;
  LSR   r1, r1, 4                                       ; se desplazan 4 posiciones para que los datos queden a partir del bit4
; set flag data ready
  SET   r1, r1, GPIO_OUTPUT_GET_DATARDY_FLAG            ; bit12-> flag gpio_output get complete
  SBBO  &r1, r20, SHD_GPIO_OUTPUT_GET_DATA, 4           ; Cargamos valores de gpio_out en los bits 4-7 y el flag data ready en bit12
  QBA   level_gpio_output_set                           ;

GPIO_OUTPUT_SET_MODE0:
; clr flag gpio_write
  LBBO  &r0, r20, SHD_GPIO_OUTPUT_SET_FLAGS, 4          ;
  CLR   r0, r0, GPIO_OUTPUT_SET_MODE0_FLAG              ; bit4 flag gpio_output set
  SBBO  &r0, r20, SHD_GPIO_OUTPUT_SET_FLAGS, 4          ;
; read gpio_output to set
  LBBO  &r0, r20, SHD_GPIO_OUTPUT_SET_DATA, 4           ;
  AND   r1, r0, 0x0F                                    ; extrae los primeros 4 bits (0-3) en r1. Flag de mascaras
  AND   r2, r0, 0xF0                                    ; extrae los bits 4-7 en r2. Pin state
  LSR   r2, r2, 4                                       ; desplaza los bits 4-7 a la posicion menos significativa en r2
  LDI32 r3, 0                                           ; inicializa el contador de bits (0 a 3)
  LDI32 r4, GPIO_OUT_BASE                               ; direccion base GPIO_OUT_BASE
check_bits_w:
  AND   r5, r2, 1                                       ; extrae el bit menos significativo de r2 (state 0 o 1)
  QBBS  gpio_write_out_funct, r1, r3                    ; qbbs myLabel r1, r3. Branch if( r1&(1<<r3) ) ojo r3 es un 1 desplazado
GPIO_OUTPUT_SET_MODE0_A:
  LSR   r2, r2, 1                                       ; desplaza r2 a la derecha para procesar el siguiente bit de state
  LSL   r4, r4, 1                                       ; desplaza hacia el proximo bit de GPIO_out1,...GPIO_out2....
  ADD   r3, r3, 1                                       ; count++
  QBGT  check_bits_w, r3, 4                             ; cuatro salidas. qbgt myLabel, r3, r4. Branch if r4 > r3
; set flag data complete
  ZERO  &r0, 4                                          ; shared erase
  SET   r0, r0, GPIO_OUTPUT_SET_DATARDY_FLAG            ; shared bit12-> flag write complete
  SBBO  &r0, r20, SHD_GPIO_OUTPUT_SET_DATA, 4           ;
  QBA   level_motor_get                                 ;

gpio_write_out_funct:
  QBBS  write_out_set, r5, 0                            ; jamp si bit0 de r5 es set
  QBBC  write_out_clr, r5, 0                            ; jamp si bit0 de r5 es clr
write_out_set:
  LDI32 r6, (GPIO0|GPIO_SETDATAOUT)                     ; load addr for GPIO Set data r6
  SBBO  &r4, r6, 0, 4                                   ; write r4 to the r6 address valu
  QBA   GPIO_OUTPUT_SET_MODE0_A                         ;
write_out_clr:
  LDI32 r6, (GPIO0|GPIO_CLRDATAOUT)                     ; load addr for GPIO Clear data.
  SBBO  &r4, r6, 0, 4                                   ; write r4 to the r6 address
  QBA   GPIO_OUTPUT_SET_MODE0_A                         ;

MOTOR_GET_MODE0:
; clr flag motor config get
  LBBO  &r0, r20, SHD_MOTOR_GET_FLAGS, 4                ;
  CLR   r0,r0, MOTOR_GET_MODE0_FLAG                     ; bit5 flag get state
  SBBO  &r0, r20, SHD_MOTOR_GET_FLAGS, 4                ;
; read MOTOR
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for DATAOUT
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
; write GPIO IN DATA INTO SHARED
  LSR   r2, r1, 2         				; r2 = bits 2–5 en posición 0–3
  AND   r2, r2, 0xF       				; enmascara solo los 4 bits bajos
  LSR   r3, r1, 14        				; r3 = bits 14–17 en posición 0–3
  AND   r3, r3, 0xF       				; enmascara solo los 4 bits bajos
  LSL   r2, r2, 4         				; ahora r2 contiene los bits 2–5 en posición 4–7
  LSL   r3, r3, 8         				; r3 contiene los bits 14–17 en posición 8–11
  OR    r4, r2, r3        				; combina ambos en r4
; set flag data ready
  SET   r4, r4, MOTOR_GET_DATARDY_FLAG                  ; bit12-> flag get
  SBBO  &r4, r20, SHD_MOTOR_GET_DATA, 4                 ; Cargamos valores enable-dir de todos los motores bit4-bit7 y el flag complete
  QBA   level_motor_set                                 ;

MOTOR_GET_MODE1:
  LBBO  &r0, r20, SHD_MOTOR_GET_FLAGS, 4                ;
  QBBC  level_motor_set, r0, MOTOR_GET_MODE1_FLAG ;
; clear IRQ_GPIO_IRQSTATUS_1
  LBBO  &r0, r22, 0, 4                                  ;
  SET   r0, r0, GPIO2_7_MOTOR_TRIGGER                   ;
  SBBO  &r0, r22, 0, 4                                  ;
; read MOTOR
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ;
  LBBO  &r1, r0, 0, 4                                   ;
; write GPIO IN DATA INTO SHARED
  LSR   r2, r1, 2                                       ; r2 = bits 2–5 en posición 0–3
  AND   r2, r2, 0xF                                     ; enmascara solo los 4 bits bajos
  LSR   r3, r1, 14                                      ; r3 = bits 14–17 en posición 0–3
  AND   r3, r3, 0xF                                     ; enmascara solo los 4 bits bajos
  LSL   r2, r2, 4                                       ; ahora r2 contiene los bits 2–5 en posición 4–7
  LSL   r3, r3, 8                                       ; r3 contiene los bits 14–17 en posición 8–11
  OR    r4, r2, r3                                      ; combina ambos en r4
; set flag data ready
  SET   r4, r4, MOTOR_GET_DATARDY_FLAG                  ; shared bit12-> flag motor_config get
  SBBO  &r4, r20, SHD_MOTOR_GET_DATA, 4                 ; Cargamos valores enable-dir de todos los motores bit4-bit7 y el flag complete
  QBA   level_motor_set                                 ;

MOTOR_SET_MODE0:
; clr flag motor set
  LBBO  &r0, r20, SHD_MOTOR_SET_FLAGS, 4                ; Shared
  CLR   r0,r0, MOTOR_SET_MODE0_FLAG                     ; bit5 flag motor
  SBBO  &r0, r20, SHD_MOTOR_SET_FLAGS, 4                ;
; Cargamos los step
  LBBO  &r10, r20, SHD_MOTOR_STEP_PERIOD_A, 4           ; read shared STEP_PERIOD_A
  LBBO  &r12, r20, SHD_MOTOR_STEP_PERIOD_B, 4           ; read shared STEP_PERIOD_B
  LBBO  &r14, r20, SHD_MOTOR_STEP_PERIOD_C, 4           ; read shared STEP_PERIOD_C
  LBBO  &r16, r20, SHD_MOTOR_STEP_PERIOD_D, 4           ; read shared STEP_PERIOD_C
; load data
  LBBO  &r0, r20, SHD_MOTOR_SET_DATA, 4                 ;
  AND   r1, r0, 0x0F                                    ; extrae los priomeros 4 bits (0-3) en r1. Flag de mascaras
  LDI32 r2, 0xFF0                                       ; mascara bit 4-11
  AND   r2, r0, r2                                      ; extrae los bits 4-11 en r2. Pins enable-dir
  LSR   r2, r2, 4                                       ; desplaza los bits 4-11 a la posicion menos significativa en r2
  LDI32 r3, 0                                           ; inicializa el contador de bits (0 a 3)
  LDI32 r4, GPIO_MOTOR_ENA_BASE                         ; direccion base gpio MOTOR_ENA_Base
  LDI32 r5, GPIO_MOTOR_DIR_BASE                         ; direccion base gpio MOTOR_DIR_Base
check_bits_m:
  AND   r6, r2, 0x11                                    ; mascara inicial Bit0 -> Enable Bit4-> Dir
  QBBS  motor_config, r1, r3                            ; qbbs myLabel r2, r4. Branch if( r2&(1<<r4) )
MOTOR_SET_MODE0_A1:
  LSR   r2, r2, 1                                       ; desplaza r2 a la derecha para procesar
  LSL   r4, r4, 1                                       ; desplaza a la siguiente Ena
  LSL   r5, r5, 1                                       ; desplaza a la siguente Dir
  ADD   r3, r3, 1                                       ; count++
  QBGT  check_bits_m, r3, 4                             ; cuatro salidas. qbgt myLabel, r3, r4. Branch if r4 > r3
; set flag motor config complete
  ZERO  &r0, 4                                          ; shared erase
  SET   r0, r0, MOTOR_SET_DATARDY_FLAG                  ; shared bit12-> flag motor config complete
  SBBO  &r0, r20, SHD_MOTOR_SET_DATA, 4                 ;
  QBA   level_motorA_step_time                          ;

motor_config:
  QBBS  enable_set, r6, 0                               ;
  QBBC  enable_clr, r6, 0                               ;
MOTOR_SET_MODE0_A2:
  QBBS  dir_set, r6, 4                                  ;
  QBBC  dir_clr, r6, 4                                  ;
enable_set:
  LDI32 r7, (GPIO2|GPIO_CLRDATAOUT)                     ; A4988 ENABLE CLR
  SBBO  &r4, r7, 0, 4                                   ; write r4 to the r7 address value
  QBA   MOTOR_SET_MODE0_A2                              ;
enable_clr:
  LDI32 r7, (GPIO2|GPIO_SETDATAOUT)                     ; load addr for GPIO Set data r7
  SBBO  &r4, r7, 0, 4                                   ; write r4 to the r6 address value
  QBA   MOTOR_SET_MODE0_A2                              ;
dir_set:
  LDI32 r7, (GPIO2|GPIO_SETDATAOUT)                     ; load addr for GPIO Set data r7
  SBBO  &r5, r7, 0, 4                                   ; write r5 to the r7 address value
  QBA   MOTOR_SET_MODE0_A1                              ;
dir_clr:
  LDI32 r7, (GPIO2|GPIO_CLRDATAOUT)                     ; load addr for GPIO Set data r7
  SBBO  &r5, r7, 0, 4                                   ; write r5 to the r7 address value
  QBA   MOTOR_SET_MODE0_A1                              ;

STEP_PERIOD_A:
  SUB   r11, r11, 1                                     ; se resta 1 a r10. r10 fue cargado cuando se habilito el motorA
  QBEQ  TOGGLE_PIN_A, r11, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_motorB_step_time                          ; sino volvemos al loop principal
TOGGLE_PIN_A:
  XOR   r30, r30, OUTPUT_MA_S                           ; toggle GPIO2_14_MA_S
  MOV   r11, r10			                ;
  QBA   level_motorB_step_time                          ; volvemos al loop principal

STEP_PERIOD_B:
  SUB   r13, r13, 1                                     ; se resta 1 a r11. r11 fue cargado cuando se habilito el motorB
  QBEQ  TOGGLE_PIN_B, r13, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_motorC_step_time                          ; sino volvemos al loop principal
TOGGLE_PIN_B:
  XOR   r30, r30, OUTPUT_MB_S                           ; toggle GPIO2_15_MB_S
  MOV   r13, r12					;
  QBA   level_motorC_step_time                          ; volvemos al loop principal

STEP_PERIOD_C:
  SUB   r15, r15, 1                                     ; se resta 1 a r12. r12 fue cargado cuando se habilito el motorC
  QBEQ  TOGGLE_PIN_C, r15, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_motorD_step_time                          ; sino volvemos al loop principal
TOGGLE_PIN_C:
  XOR   r30, r30, OUTPUT_MC_S                           ; toggle GPIO2_16_MC_S
  MOV   r15, r14				        ;
  QBA   level_motorD_step_time                          ; volvemos al loop principal

STEP_PERIOD_D:
  SUB   r17, r17, 1                                     ; se resta 1 a r13. r13 fue cargado cuando se habilito el motorD
  QBEQ  TOGGLE_PIN_D, r17, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_gpio_input                                ; sino volvemos al loop principal
TOGGLE_PIN_D:
  XOR   r30, r30, OUTPUT_MD_S                           ; toggle GPIO2_17_MD_S
  MOV   r17, r16					;
  QBA   level_gpio_input                                ; volvemos al loop principal

  LDI32  R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)         ;
  HALT                                                  ; halt the pru program
