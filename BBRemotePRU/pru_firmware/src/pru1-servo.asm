; Raul Emlio Romero
;
; cd /sys/class/remoteproc/remoteproc1
; echo 'am335x-pru1-rc-servo-fw' > firmware
; echo 'start' > state
; echo 'stop' > state
; cd /sys/kernel/debug/remoteproc/remoteproc1
; sudo cat regs
;
; PRU definitions

; pru1Control
  .asg 0x22000, PRU0_CTRL
  .asg 0x24000, PRU1_CTRL                               ; page 19
  .asg 0x28, CTPPR0                                     ; page 75

; memory
  .asg 0x00000000, RAM0_ADDR           			; local addr ram0. In linux space addr is 0x4a300000
  .asg 0x00002000, RAM1_ADDR           			; local addr ram1. In linux space addr is 0x4a302000
  .asg 0x00010000, SHARED_ADDR         			; local addr shared. In linux space addr is 0x4a310000

  .asg 0x00, SHD_0    		       			; shared[0] flags linux
  .asg 0x04, SHD_1    		       			; shared[1] gpio_read get
  .asg 0x08, SHD_2    		       			; shared[2] gpio_read send rising detect
  .asg 0x0C, SHD_3    		       			; shared[3] gpio_write get-set
  .asg 0x10, SHD_4		       			; shared[4] motor get-set
  .asg 0x14, SHD_5		      	 		; shared[5] motor_A STEP_PERIOD_A
  .asg 0x18, SHD_6		       			; shared[6] motor_B STEP_PERIOD_B
  .asg 0x1C, SHD_7		       			; shared[7] motor_C STEP_PERIOD_C
  .asg 0x20, SHD_8					; shared[8] motor_D STEP_PERIOD_D
  .asg 0x24, SHD_9					; shared[9] send enable-dir MA y MB rising detect
  .asg 0x28, SHD_10
  .asg 0x0a, SHARED_MEM_SIZE          	 		; 10 en decimal
  .asg 0x04, OFFSET_MEM                			;

; gpio bank
  .asg 0x44e07000, GPIO0               			; GPIO Bank 0, See the AM335x TRM
  .asg 0x481ac000, GPIO2               			; GPIO Bank 2

  .asg 0x190, GPIO_CLRDATAOUT          			; for clearing the GPIO registers
  .asg 0x194, GPIO_SETDATAOUT          			; for setting the GPIO registers
  .asg 0x13C, GPIO_DATAOUT            		 	; for read-write the GPIO registers
  .asg 0x138, GPIO_DATAIN              			; for reading the GPIO registers
  .asg 0x38,  GPIO_IRQSTATUS_SET_1			; habilita las interrupciones por set
  .asg 0x148, GPIO_RISINGDETECT				; habilita las interrupciones por flanco asc
  .asg 0x30,  GPIO_IRQSTATUS_1				; get-set irq
  .asg 0x150, GPIO_DEBOUNCENABLE			; antirebote enable register
  .asg 0x54,  GPIO_DEBOUNCINGTIME			; configure debouncingtime register
  .asg 0xD,  DEBOUNCETIME				; Debouncing Value = (DEBOUNCETIME + 1) * 31 microseconds
  .asg 0xF00, GPIO0_LEVELDETECT_MASK		        ; bits 8-11 estas entradas estan configuradas con antirebote
  .asg 0x08800000, GPIO0_RISINGDETECT_MASK              ; bit23 y bit 27
  .asg 0x01, GPIO2_RISINGDETECT_MASK			; pin P8_18 GPIO2_1 flanco asc detect para funcion get gpio read send

  .asg 0x130, GPIO_CTRL        	      			; enable GPIO port
  .asg 0x00,  GPIO_CTRL_ENABLE         			;
  .asg 0x134, GPIO_OE                  			; set GPIO input - output
  .asg 0xFC3C003F, GPIO2_OE_IN_OUT      		; pines usados como salida bit6 a bit17 y bit22 al bit25 en 0
  .asg 0x0CC00F00, GPIO0_OE_IN_OUT       	        ; pines usados como entrada bits8-10, bits22-23 y bits26-27 en 1

; gpio_read
  ; Pins
  ; P8_35 gpio0[8] -> bit8 input_0  no hace falta hacer: config-pin P8_35 gpio_pd
  ; P8_33 gpio0[9] -> bit9 input_1  no hace falta hacer: config-pin P8_33 gpio_pd
  ; P8_31 gpio0[10]-> bit10 input_2 no hace falta hacer: config-pin P8_31 gpio_pd
  ; P8_32 gpio0[11]-> bit11 input_3 no hace falta hacer: config-pin P8_32 gpio_pd

  ; MOTOR_A hardware pins
  .asg 22, INPUT_MA_DISABLE				; P8_19 gpio0[22]-> bit22 input_MA_E set disable motor
  .asg 23, INPUT_MA_TD					; P8_13 gpio0[23]-> bit23 input_MA_D set toogle direction
  ; MOTOR_B hardware pins
  .asg 26, INPUT_MB_DISABLE				; P8_14 gpio0[26]-> bit26 input_MB_E set disable motor
  .asg 27, INPUT_MB_TD					; P8_17 gpio0[27]-> bit27 input_MB_D set toogle direction
  .asg 1, INPUT_GPIO_READ_ASC				; P8_18 gpio2[1]-> bit1 input gpio read send rising detect

; gpio_write
  .asg  (1<<22),  GPIO_OUT_BASE	       			;
  ; Pins
  ; P8_27 gpio2[22]-> bit22 output_0 no hace falta hacer: config-pin P8_27 gpio
  ; P8_29 gpio2[23]-> bit23 output_1 no hace falta hacer: config-pin P8_29 gpio
  ; P8_28 gpio2[24]-> bit24 output_2 no hace falta hacer: config-pin P8_28 gpio
  ; P8_30 gpio2[25]-> bit25 output_3 no hace falta hacer: config-pin P8_30 gpio

; motor
  ; Enable-Direction
  .asg (1<<6), GPIO_MOTOR_ENA_BASE     			;
  .asg (1<<7), GPIO_MOTOR_DIR_BASE     			;

  ; Pins
  .asg 6, OUTPUT_MA_E					; P8_45 gpio2[6] -> bit6 output_MA_E no hace falta hacer: config-pin P8_45 gpio
  .asg 7, OUTPUT_MA_D					; P8_46 gpio2[7] -> bit7 output_MA_D config-pin P8_46 gpio

  .asg 8, OUTPUT_MB_E					; P8_43 gpio2[8] -> bit8 output_MB_E config-pin P8_43 gpio
  .asg 9, OUTPUT_MB_D					; P8_44 gpio2[9] -> bit9 output_MB_D config-pin P8_44 gpio

  .asg 10, OUTPUT_MC_E					; P8_41 gpio2[10]-> bit10 output_MC_E config-pin P8_41 gpio
  .asg 11, OUTPUT_MC_D					; P8_42 gpio2[11]-> bit11 output_MC_D config-pin P8_42 gpio

  .asg 12, OUTPUT_MD_E					; P8_39 gpio2[12]-> bit12 output_MD_E config-pin P8_39 gpio
  .asg 13, OUTPUT_MD_D					; P8_40 gpio2[13]-> bit13 output_MD_D config-pin P8_40 gpio

  ; Step period (medio ciclo de la senial)
  .asg (1<<14), OUTPUT_MA_S          			; P8_37 gpio2[14] output_MA_S no hace falta hacer config-pin
  .asg (1<<15), OUTPUT_MB_S          			; P8_38 gpio2[15] output_MB_S
  .asg (1<<16), OUTPUT_MC_S          			; P8_36 gpio2[16] output_MC_S
  .asg (1<<17), OUTPUT_MD_S          			; P8_34 gpio2[17] output_MD_S

; functions flags
  .asg 0, GPIO_READ_FLAG			        ; shared[0] bit0
  .asg 1, GPIO_INSENDSTATE_FLAG                         ; shared[0] bit1
  .asg 2, GPIO_WRITESET_FLAG                            ; shared[0] bit2
  .asg 3, GPIO_WRITEGET_FLAG                            ; shared[0] bit3
  .asg 4, MOTOR_CONFIGSET_FLAG                          ; shared[0] bit4
  .asg 5, MOTOR_CONFIGGET_FLAG                          ; shared[0] bit5

  .asg 12, GPIO_READDATARDY_FLAG			; shared[1] bit12
  .asg 12, GPIO_INSENDSTATERISING_FLAG                  ; shared[2] bit12
  .asg 12, GPIO_WRITESETCOMP_FLAG			; shared[3] bit12
  .asg 12, GPIO_WRITEGETCOMP_FLAG			; shared[3] bit12
  .asg 12, MOTOR_CONFIGSETCOMP_FLAG			; shared[4] bit12
  .asg 12, MOTOR_CONFIGGETCOMP_FLAG			; shared[4] bit12


; disable-toogle direction to linux msg
  .asg 0, SHD_MA_DISABLE				; shared[10] set bit0 MA disable
  .asg 1, SHD_MA_TD                                     ; shared[10] set bit1 MA toggle dir
  .asg 2, SHD_MB_DISABLE                                ; shared[10] set bit2 MB disable
  .asg 3, SHD_MB_TD                                     ; shared[10] set bit3 MB toggle dir

  .asg 32, PRU0_R31_VEC_VALID          			; allows notification of program completion
  .asg 3,  PRU_EVTOUT_0                			; the event number that is sent back

  .clink
  .global start

start:
; Registro fijos
  LDI32 r20, SHARED_ADDR				; shared_addr_base
  ; r10-> STEP_PERIOD_A
  ; r11-> STEP_PERIOD_B
  ; r12-> STEP_PERIOD_C
  ; r13-> STEP_PERIOD_D
  ; r21-r25-> DEBUG

SETUP:
; borrado de momoria
  ZERO  &r0, 4                         			; zero put register
  LDI32 r1, 0   	               			; offset_mem
  LDI32 r2, 0			       			; Count mem
mem_init:
  SBBO	&r0, r20, r1, 4          			;
  ADD   r1, r1, 4        	       			; cada posicion de memoria ocupa 4 bytes
  ADD	r2, r2, 1		       			; count++
  QBGT  mem_init, r2, SHARED_MEM_SIZE  			; qbgt myLabel, r3, r4. Branch if r4 > r3

gpio_config:
;GPIO0
  LDI32 r0, (GPIO0|GPIO_CTRL) 	       			; load GPIO0 control register address
  LDI32 r1, GPIO_CTRL_ENABLE           			; load control enable value
  SBBO  &r1, r0, 0, 4                  			; write enable value to GPIO0 control register

  LDI32 r0, (GPIO0|GPIO_OE)            			; load GPIO0 output enable register address
  LDI32 r1, GPIO0_OE_IN_OUT           			; set GPIO0 as input
  SBBO  &r1, r0, 0, 4                  			; write input configuration to GPIO0

  ; solo pines toggle configurados con flanco asc
  LDI32 r0, (GPIO0|GPIO_RISINGDETECT)                   ; load addr for GPIO0
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r1, GPIO0_RISINGDETECT_MASK                     ; input toggles bit23 y bit27
  SBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.

  ; enable irq set_1
  LDI32 r0, (GPIO0|GPIO_IRQSTATUS_SET_1)                ; load addr for GPIO0
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r1, GPIO0_RISINGDETECT_MASK                     ; input toggles bit23 y bit27
  SBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.

  ; configuracion antirebote
;  LDI32 r0, (GPIO0|GPIO_DEBOUNCINGTIME)                 ; load addr for GPIO0
;  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
;  LDI32 r1, GPIO_DEBOUNCINGTIME                         ; debouncingtime 0-255
;  SBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.

  ; Muy importante primero hay que definir el debouncingtime y luego habilitar el modulo
;  LDI32 r0, (GPIO0|GPIO_DEBOUNCENABLE)                  ; load addr for GPIO0
;  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
;  LDI32 r1, 0x0;
;  LDI32 r1, GPIO0_LEVELDETECT_MASK                      ; input GPIO0 bits 8-11
;  SBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.

;GPIO2
  LDI32 r0, (GPIO2|GPIO_CTRL)          			; load GPIO2 control register address
  LDI32 r1, GPIO_CTRL_ENABLE           			; load control enable value
  SBBO  &r1, r0, 0, 4                  			; write enable value to GPIO2 control register

  LDI32 r0, (GPIO2|GPIO_OE)            			; load GPIO2 output enable register address
  LDI32 r1, GPIO2_OE_IN_OUT            			; set GPIO2 as output ojo aca tambien estamos configurando las entradas
  SBBO  &r1, r0, 0, 4                  			; write output configuration to GPIO2

  ; GPIO2_1 pin8.18 se configura como entrada
  ; solo un pin configurados con flanco asc
  LDI32 r0, (GPIO2|GPIO_RISINGDETECT)                   ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r1, GPIO2_RISINGDETECT_MASK                     ; input bit1
  SBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.

  ; enable irq set_1
  LDI32 r0, (GPIO2|GPIO_IRQSTATUS_SET_1)                ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r1, GPIO2_RISINGDETECT_MASK                     ; input bit1
  SBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.

  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load GPIO2 output
  ZERO  &r1, 4                                          ;
  SBBO  &r1, r0, 0 , 4                                  ; Cargamos todas las salidas en cero

; debug ---------------------------------------------------------------------------------------
; get flag function
  LBBO &r0, r20, SHD_0, 4                               ; load shared[0]
; set flag function probe
;  SET  r0, r0, 0                          	         ; set flag gpio_read bit0
  SET  r0, r0, 2                          	         ; set flag gpio_write set  bit2
  SET  r0, r0, 4			  	         ; set flaf motor config set bit4
  SBBO  &r0, r20, SHD_0, 4                              ; set flag set in shared[0]

; gpio read
;  LBBO  &r21, r20, SHD_1, 4                             ; valores de las entradas gpio_read bit0-bit3

; gpio write
  LDI32 r22, 0x6F			                 ; debug-> Cargamos gpio_write state->1100 pins->1101
  SBBO  &r22, r20, SHD_3, 4                              ;
; motor
  LDI32 r23, 0xFFF                       		 ; direction-enable 11 11 11 11  motor->1111 (1111 1111 1111)
  LDI32 r24, 0xFFF					 ; Cargamos un STEP_PERIOD_A de prueba en decimal 4096  aprox 2.8 ms
  LDI32 r25, 0xFF					 ; Cargamos un STEP_PERIOD_B de prueba
  LDI32 r26, 0xFFF					 ; Cargamos un STEP_PERIOD_C de prueba
  LDI32 r27, 0xFF					 ; Cargamos un STEP_PERIOD_D de prueba en decimal 15 aprox  10 us
  SBBO  &r23, r20, SHD_4, 4 		 		 ; cargamos enable-dir
  SBBO  &r24, r20, SHD_5, 4 		 		 ; cargamos STEP_PERIOD_A
  SBBO  &r25, r20, SHD_6, 4 		 		 ; cargamos STEP_PERIOD_B
  SBBO  &r26, r20, SHD_7, 4 		 		 ; cargamos STEP_PERIOD_C
  SBBO  &r27, r20, SHD_8, 4 				 ; cargamos STEP_PERIOD_D
; debug ----------------------------------------------------------------------------------------

MAIN_LOOP:
; get state gpio_read function flag
  LBBO  &r0, r20, SHD_0, 4		  		; read shared[0]
  QBBS  GPIO_READ, r0, GPIO_READ_FLAG                   ; jump is gpio_read_flag is set shared[0] bit0
level_A:
  LBBO  &r0, r20, SHD_0, 4                              ; read shared[0]
  QBBS  GPIO_INSENDSTATE, r0, GPIO_INSENDSTATE_FLAG     ; jump is gpio_send_state_flag is set shared[0] bit1
level_B:
; set state gpio_write function flag
  LBBO  &r0, r20, SHD_0, 4  				; read shared[0]
  QBBS  GPIO_WRITE_SET, r0, GPIO_WRITESET_FLAG          ; jump is gpio write flag set state is set shared[0] bit2
level_C:
; get state gpio_write get state
  LBBO  &r0, r20, SHD_0, 4				; read shared[0]
  QBBS  GPIO_WRITE_GET, r0, GPIO_WRITEGET_FLAG		; jum is gpio write flag get state is set shared[0] bit3
level_D:
; set state motor function flag
  LBBO  &r0, r20, SHD_0, 4 		 		; read shared[0]
  QBBS  MOTOR_CONFIG_SET, r0, MOTOR_CONFIGSET_FLAG      ; jump is motor flag config set is set shared[0] bit4
level_E:
; get state motor function flag
  LBBO  &r0, r20, SHD_0, 4				; read shared[0]
  QBBS  MOTOR_CONFIG_GET,r0, MOTOR_CONFIGGET_FLAG	; jump is motor flag config get is set bit5
level_F:
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2 Set data r0
  LBBO  &r1, r0, 0, 4				        ; load value into r1 r0 addr
  QBBS  STEP_PERIOD_A, r1, OUTPUT_MA_E                  ; jump is output_MA_E is set bit6
  QBBC  DISABLE_OUTPUT_MA, r1, OUTPUT_MA_E		; jump si output_MA_DIS borramos todas las salidas
level_F1:
  QBBS  STEP_PERIOD_B, r1, OUTPUT_MB_E		        ; jump is output_MB_E is set bit8
  QBBC  DISABLE_OUTPUT_MB, r1, OUTPUT_MB_E              ; jump si output_MB_DIS borramos todas las salidas
level_F2:
  QBBS  STEP_PERIOD_C, r1, OUTPUT_MC_E                  ; jump is output_MC_E is set bit10
  QBBC  DISABLE_OUTPUT_MC, r1, OUTPUT_MC_E              ; jump si output_MC_DIS borramos todas las salidas
level_F3:
  QBBS  STEP_PERIOD_D, r1, OUTPUT_MD_E                  ; jump is output_MD_E is set bit12
  QBBC  DISABLE_OUTPUT_MD, r1, OUTPUT_MD_E              ; jump si output_MD_DIS borramos todas las salidas
level_G:
  LDI32 r0, (GPIO0|GPIO_DATAIN)                         ; load addr for GPIO0 Set data r0
  LBBO  &r1, r0, 0, 4					;
  QBBS  DISABLE_MA, r1, INPUT_MA_DISABLE		; Desabilita el motor_A si GPIO0_22 is set
  ; clr bit0
  LBBO  &r0, r20, SHD_9, 4                              ; shared[9]
  CLR   r0,r0, SHD_MA_DISABLE                           ; clr bit0
  SBBO  &r0, r20, SHD_9, 4                              ;
level_G1:
  LDI32 r0, (GPIO0|GPIO_IRQSTATUS_1)                    ; load addr for GPIO0 Set data r0
  LBBO  &r1, r0, 0, 4                                   ;
  QBBS  TOGGLE_DIR_MA, r1, INPUT_MA_TD                  ; toggle direction el motor_MA si flanco GPIO0_23 is set
  ; clr bit1
  LBBO  &r0, r20, SHD_9, 4                              ; shared[9]
  CLR   r0,r0, SHD_MA_TD                                ; clr bit1
  SBBO  &r0, r20, SHD_9, 4                              ;
level_H:
  LDI32 r0, (GPIO0|GPIO_DATAIN)                         ; load addr for GPIO0 Set data r0
  LBBO  &r1, r0, 0, 4                                   ;
  QBBS  DISABLE_MB, r1, INPUT_MB_DISABLE                ; Desabilita el motor_MB si GPIO0_26 is set
  ; clr bit2
  LBBO  &r0, r20, SHD_9, 4                              ; shared[9]
  CLR   r0,r0, SHD_MB_DISABLE                           ; clr bit2
  SBBO  &r0, r20, SHD_9, 4                              ;
level_H1:
  LDI32 r0, (GPIO0|GPIO_IRQSTATUS_1)                    ; load addr for GPIO0 Set data r0
  LBBO  &r1, r0, 0, 4                                   ;
  QBBS  TOGGLE_DIR_MB, r1, INPUT_MB_TD                  ; toggle direction el motor_MB si flanco GPIO0_27 is set
  ; clr bit3
  LBBO  &r0, r20, SHD_9, 4                              ; shared[9]
  CLR   r0,r0, SHD_MA_TD                                ; clr bit3
  SBBO  &r0, r20, SHD_9, 4                              ;
level_I:

  QBA   MAIN_LOOP			   	        ; program loop

GPIO_READ:
; clr flag gpio_read
  LBBO  &r0, r20, SHD_0, 4  				; shared[0]
  CLR   r0,r0, GPIO_READ_FLAG			        ; bit0 flag gpio_read
  SBBO  &r0, r20, SHD_0, 4  				;
; read GPIO IN
  LDI32 r0, (GPIO0|GPIO_DATAIN)                  	; load addr for DATAIN
  LBBO  &r1, r0, 0, 4                      	        ; Load the values at r0 into r1.
; write GPIO IN DATA INTO SHARED[1]
  LDI32 r0, 0xF00			   	        ; mascara desde bit8 a bit11 porque en el registro GPIO_DATAIN estan en esa posicion
  AND	r1, r1, r0                         	        ;
  LSR   r1, r1, 8			   	        ; se desplazan 8 posiciones para que los datos queden a partir del bit 0
; set flag data ready
  SET   r1, r1, GPIO_READDATARDY_FLAG               	; shared[1] bit12-> flag data ready
  SBBO  &r1, r20, SHD_1, 4 		 		; Cargamos valores de las entradas gpio_read bit0-bit3 y el flag data ready
  QBA	level_A		                                ;

GPIO_INSENDSTATE:
; no clr flag shared[0]para que quede continuamente leyendo
; get irq status
  LDI32 r0, (GPIO2|GPIO_IRQSTATUS_1)                    ; load addr for GPIO2 rising detect bit1 set data r0
  LBBO  &r1, r0, 0, 4                                   ;
  QBBS  level_A1, r1, INPUT_GPIO_READ_ASC 	        ; si se detecta flanco asc enviamos los estados de gpio0 input bit1
  QBA   level_B						;
level_A1:
; clear IRQ_GPIO_IRQSTATUS_1
  LDI32 r0, (GPIO2|GPIO_IRQSTATUS_1)                    ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ;
  SET   r1, r1, INPUT_GPIO_READ_ASC                     ; bit1
  SBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
; read GPIO IN
  LDI32 r0, (GPIO0|GPIO_DATAIN)                         ; load addr for DATAIN
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
; write GPIO IN DATA INTO SHARED[2]
  LDI32 r0, 0xF00                                       ; mascara desde bit8 a bit11 porque en el registro GPIO_DATAIN estan en esa posicion
  AND   r1, r1, r0                                      ;
  LSR   r1, r1, 8                                       ; se desplazan 8 posiciones para que los datos queden a partir del bit 0
; set flag rising detect
  SET   r1, r1, GPIO_INSENDSTATERISING_FLAG             ; shared[2] bit12-> flag rising detect
  SBBO  &r1, r20, SHD_2, 4                              ; Cargamos valores de las entradas gpio_read bit0-bit3 y el flag
  QBA   level_B                                         ;

GPIO_WRITE_SET:
; clr flag gpio_write
  LBBO  &r0, r20, SHD_0, 4  				; Shared[0]
  CLR   r0, r0, GPIO_WRITESET_FLAG                      ; bit2 flag gpio_write set
  SBBO  &r0, r20, SHD_0, 4  				;
; read gpio_output to set
  LBBO  &r0, r20, SHD_3, 4  				; read shared[3]
  AND   r1, r0, 0x0F      	  		        ; extrae los primeros 4 bits (0-3) en r1. Flag de mascaras
  AND   r2, r0, 0xF0      		   	        ; extrae los bits 4-7 en r2. Pin state
  LSR   r2, r2, 4         		   	        ; desplaza los bits 4-7 a la posicion menos significativa en r2
  LDI32	r3, 0            	           	        ; inicializa el contador de bits (0 a 3)
  LDI32 r4, GPIO_OUT_BASE			   	; direccion base GPIO_OUT_BASE
check_bits_w:
  AND   r5, r2, 1  			   	        ; extrae el bit menos significativo de r2 (state 0 o 1)
  QBBS  gpio_write_out_funct, r1, r3       	        ; qbbs myLabel r1, r3. Branch if( r1&(1<<r3) ) ojo r3 es un 1 desplazado
level_B1:
  LSR   r2, r2, 1			   		; desplaza r2 a la derecha para procesar el siguiente bit de state
  LSL   r4, r4, 1                          		; desplaza hacia el proximo bit de GPIO_out1,...GPIO_out2....
  ADD   r3, r3, 1			   		; count++
  QBGT  check_bits_w, r3, 4                		; cuatro salidas. qbgt myLabel, r3, r4. Branch if r4 > r3
; set flag data complete
  ZERO  &r0, 4						; shared[3] erase
  SET   r0, r0, GPIO_WRITESETCOMP_FLAG                  ; shared[3] bit12-> flag write complete
  SBBO  &r0, r20, SHD_3, 4  				;
  QBA	level_C		         	         	;

gpio_write_out_funct:
  QBBS  write_out_set, r5, 0		   		; jamp si bit0 de r5 es set
  QBBC  write_out_clr, r5, 0               	        ; jamp si bit0 de r5 es clr
write_out_set:
  LDI32 r6, (GPIO2|GPIO_SETDATAOUT)        	        ; load addr for GPIO Set data r6
  SBBO  &r4, r6, 0, 4                      		; write r4 to the r6 address valu
  QBA	level_B1			   		;
write_out_clr:
  LDI32 r6, (GPIO2|GPIO_CLRDATAOUT)        		; load addr for GPIO Clear data.
  SBBO  &r4, r6, 0, 4                      		; write r4 to the r6 address
  QBA	level_B1			   		;

GPIO_WRITE_GET:
; clr flag gpio_read
  LBBO  &r0, r20, SHD_0, 4                              ; shared[0]
  CLR   r0, r0, GPIO_WRITEGET_FLAG                      ; bit3 flag gpio_write get state
  SBBO  &r0, r20, SHD_0, 4                              ;
; read GPIO_DATAOUT
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for DATAOUT
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
;  SBBO  &r1, r20, SHD_10, 4
; write GPIO DATAOUT INTO SHARED[3]
  LDI32 r0, 0x03C00000                                  ; mascara desde bit22 a bit25 porque en el registro GPIO_DATAOUT estan en esa posicion
  AND   r1, r1, r0                                      ;
  LSR   r1, r1, 18                                      ; se desplazan 18 posiciones para que los datos queden a partir del bit4
  SBBO  &r1, r20, SHD_10, 4
; set flag data ready
  SET   r1, r1, GPIO_WRITEGETCOMP_FLAG                  ; shared[3] bit12-> flag gpio_write get complete
  SBBO  &r1, r20, SHD_3, 4                              ; Cargamos valores de gpio_out 22-25 en los bits 4-7 y el flag data ready en bit12
  QBA   level_D                                         ;

MOTOR_CONFIG_SET:
; Cargamos los step
  LBBO  &r10, r20, SHD_5, 4  				; read shared[5] STEP_PERIOD_A
  LBBO  &r11, r20, SHD_6, 4  				; read shared[6] STEP_PERIOD_B
  LBBO  &r12, r20, SHD_7, 4  				; read shared[7] STEP_PERIOD_C
  LBBO  &r13, r20, SHD_8, 4  				; read shared[8] STEP_PERIOD_C
; clr flag motor set
  LBBO  &r0, r20, SHD_0, 4 		 		; Shared[0]
  CLR   r0,r0, MOTOR_CONFIGSET_FLAG                     ; bit4 flag motor
  SBBO  &r0, r20, SHD_0, 4  				;
; load data
  LBBO  &r0, r20, SHD_4, 4  				; read shared[4]
  ;SBBO  &r0, r20, SHD_10, 4; debug
  AND   r1, r0, 0x0F                       		; extrae los priomeros 4 bits (0-3) en r1. Flag de mascaras
  LDI32 r2, 0xFF0			   		; mascara bit 4-11
  AND   r2, r0, r2                         		; extrae los bits 4-11 en r2. Pins enable-dir
  LSR   r2, r2, 4                          		; desplaza los bits 4-11 a la posicion menos significativa en r2
  LDI32 r3, 0                              		; inicializa el contador de bits (0 a 3)
  LDI32 r4, GPIO_MOTOR_ENA_BASE             		; direccion base gpio MOTOR_ENA_Base
  LDI32	r5, GPIO_MOTOR_DIR_BASE		   		; direccion base gpio MOTOR_DIR_Base
check_bits_m:
  AND   r6, r2, 0x03                       		; extrae los dos primeros  bits. Bit0 -> Enable Bit1-> Dir
  QBBS  motor_config, r1, r3               		; qbbs myLabel r2, r4. Branch if( r2&(1<<r4) )
level_D1:
  LSR   r2, r2, 2                          		; desplaza r2 a la derecha para procesar los siguientes 2 bits dir-ena
  LSL	r4, r4, 2			   		; desplaza a la siguiente Ena
  LSL	r5, r5, 2			   		; desplaza a la siguente Dir
  ADD   r3, r3, 1                          		; count++
  QBGT  check_bits_m, r3, 4                		; cuatro salidas. qbgt myLabel, r3, r4. Branch if r4 > r3
; set flag motor config complete
  ZERO  &r0, 4						; shared[4] erase
  SET   r0, r0, MOTOR_CONFIGSETCOMP_FLAG                ; shared[4] bit12-> flag motor config complete
  SBBO  &r0, r20, SHD_4, 4  		                ;
  QBA 	level_E				    		;

motor_config:
  QBBS  enable_set, r6, 0		    	 	;
  QBBC  enable_clr, r6, 0                   		;
level_D2:
  QBBS  dir_set, r6, 1			    		;
  QBBC  dir_clr, r6, 1    		    		;
enable_set:
  LDI32 r7, (GPIO2|GPIO_SETDATAOUT)         		; load addr for GPIO Set data r7
  SBBO  &r4, r7, 0, 4                       		; write r4 to the r7 address value
  QBA	level_D2			    		;
enable_clr:
  LDI32 r7, (GPIO2|GPIO_CLRDATAOUT)         		; load addr for GPIO Set data r7
  SBBO  &r4, r7, 0, 4                       		; write r4 to the r6 address value
  QBA	level_D2			    		;
dir_set:
  LDI32 r7, (GPIO2|GPIO_SETDATAOUT)         		; load addr for GPIO Set data r7
  SBBO  &r5, r7, 0, 4                       		; write r5 to the r7 address value
  QBA	level_D1		            		;
dir_clr:
  LDI32 r7, (GPIO2|GPIO_CLRDATAOUT)         		; load addr for GPIO Set data r7
  SBBO  &r5, r7, 0, 4                       		; write r5 to the r7 address value
  QBA	level_D1			    		;

MOTOR_CONFIG_GET:
; clr flag motor config get
  LBBO  &r0, r20, SHD_0, 4                              ; shared[0]
  CLR   r0,r0, MOTOR_CONFIGGET_FLAG                     ; bit5 flag motor_config get state
  SBBO  &r0, r20, SHD_0, 4                              ;
; read GPIO IN
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for DATAOUT
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
; write GPIO IN DATA INTO SHARED[3]
  LDI32 r0, 0x03FC0                                     ; mascara desde bit6-13 para sacar enable-dir de los cuatro motores
  AND   r1, r1, r0                                      ;
  LSR   r1, r1, 2                                       ; se desplazan 2 posiciones para que los datos queden a partir del bit4
; set flag data ready
  SET   r1, r1, MOTOR_CONFIGGETCOMP_FLAG                ; shared[3] bit12-> flag motor_config get
  SBBO  &r1, r20, SHD_4, 4                              ; Cargamos valores enable-dir de todos los motores bit4-bit7 y el flag complete
  QBA   level_F                                         ;

STEP_PERIOD_A:
  SUB   r10, r10, 1					; se resta 1 a r10. r10 fue cargado cuando se habilito el motorA
  QBEQ  TOGGLE_PIN_A, r10, 0				; toggle pin step cuando la cuenta llega a 0
  QBA   level_F1					; sino volvemos al loop principal
TOGGLE_PIN_A:
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, OUTPUT_MA_S 				; GPIO2_14_MA_S
  XOR   r1, r1, r2				        ; toggle GPIO2_14_MA_S
  SBBO  &r1, r0, 0 , 4					; actualizamos la salida GPIO2_14_MA_S
  LBBO  &r10, r20, SHD_5, 4  		                ; cargamos nuevamente el valor de shared[5] para arrancar de nuevo la cuenta
  QBA   level_F1					; volvemos al loop principal

STEP_PERIOD_B:
  SUB   r11, r11, 1                                     ; se resta 1 a r11. r11 fue cargado cuando se habilito el motorB
  QBEQ  TOGGLE_PIN_B, r11, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_F2                                        ; sino volvemos al loop principal
TOGGLE_PIN_B:
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, OUTPUT_MB_S                                 ; GPIO2_15_MB_S
  XOR   r1, r1, r2                                      ; toggle GPIO2_15_MB_S
  SBBO  &r1, r0, 0 , 4                                  ; actualizamos la salida GPIO2_15_MB_S
  LBBO  &r11, r20, SHD_6, 4  		                ; cargamos nuevamente el valor de shared[6] para arrancar de nuevo la cuenta
  QBA   level_F2                                        ; volvemos al loop principal

STEP_PERIOD_C:
  SUB   r12, r12, 1                                     ; se resta 1 a r12. r12 fue cargado cuando se habilito el motorC
  QBEQ  TOGGLE_PIN_C, r12, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_F3                                        ; sino volvemos al loop principal
TOGGLE_PIN_C:
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, OUTPUT_MC_S                                 ; GPIO2_16_MC_S
  XOR   r1, r1, r2                                      ; toggle GPIO2_16_MC_S
  SBBO  &r1, r0, 0 , 4                                  ; actualizamos la salida GPIO2_16_MC_S
  LBBO  &r12, r20, SHD_7, 4                             ; cargamos nuevamente el valor de shared[7] para arrancar de nuevo la cuenta
  QBA   level_F3                                        ; volvemos al loop principal

STEP_PERIOD_D:
  SUB   r13, r13, 1                                     ; se resta 1 a r13. r13 fue cargado cuando se habilito el motorD
  QBEQ  TOGGLE_PIN_D, r13, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_G                                         ; sino volvemos al loop principal
TOGGLE_PIN_D:
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, OUTPUT_MD_S                                 ; GPIO2_17_MD_S
  XOR   r1, r1, r2                                      ; toggle GPIO2_17_MD_S
  SBBO  &r1, r0, 0 , 4                                  ; actualizamos la salida GPIO2_17_MD_S
  LBBO  &r13, r20, SHD_8, 4                             ; cargamos nuevamente el valor de shared[8] para arrancar de nuevo la cuenta
  QBA   level_G                                         ; volvemos al loop principal

DISABLE_OUTPUT_MA:
  LDI32 r0, (GPIO2|GPIO_CLRDATAOUT)                     ; recordar que hay que hacer un set para clr
  LDI32 r1, 0x0						; ponemos en estado bajo las salidas
  SET   r1, r1, OUTPUT_MA_D                             ;
  SET   r1, r1, 14	         			; OUTPUT_MA_S lo hacemos asi para no agregar mas instrucciones
  SBBO  &r1, r0, 0, 4                                   ;
  QBA   level_F1					;

DISABLE_OUTPUT_MB:
  LDI32 r0, (GPIO2|GPIO_CLRDATAOUT)                     ; recordar que hay que hacer un set para clr
  LDI32 r1, 0x0                                         ; ponemos en estado bajo las salidas
  SET   r1, r1, OUTPUT_MB_D                             ;
  SET   r1, r1, 15                                      ; OUTPUT_MB_S
  SBBO  &r1, r0, 0, 4                                   ;
  QBA   level_F2                                        ;

DISABLE_OUTPUT_MC:
  LDI32 r0, (GPIO2|GPIO_CLRDATAOUT)                     ; recordar que hay que hacer un set para clr
  LDI32 r1, 0x0                                         ; ponemos en estado bajo las salidas
  SET   r1, r1, OUTPUT_MC_D                             ;
  SET   r1, r1, 16                                      ; OUTPUT_MC_S
  SBBO  &r1, r0, 0, 4                                   ;
  QBA   level_F3                                        ;

DISABLE_OUTPUT_MD:
  LDI32 r0, (GPIO2|GPIO_CLRDATAOUT)                     ; recordar que hay que hacer un set para clr
  LDI32 r1, 0x0                                         ; ponemos en estado bajo las salidas
  SET   r1, r1, OUTPUT_MD_D                             ;
  SET   r1, r1, 17                                      ; OUTPUT_MD_S
  SBBO  &r1, r0, 0, 4                                   ;
  QBA   level_G                                         ;

DISABLE_MA:
  LDI32 r0, (GPIO2|GPIO_CLRDATAOUT)                     ;
  LDI32 r1, GPIO_MOTOR_ENA_BASE                         ; direccion base gpio MOTOR_ENA_Base Motor_MA
  SBBO  &r1, r0, 0, 4                                   ;
; set flag shared[9] bit0
  LBBO  &r0, r20, SHD_9, 4                              ; Shared[9] avisamos a linux
  SET   r0, r0, SHD_MA_DISABLE                          ; bit0 flag motor_MA disable
  SBBO  &r0, r20, SHD_9, 4                              ;
  QBA   level_G1                                        ;

TOGGLE_DIR_MA:
; toggle drection pins
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, GPIO_MOTOR_DIR_BASE                         ;
  XOR   r1, r1, r2                                      ; toggle
  SBBO  &r1, r0, 0 , 4                                  ; actualizamos dir
; clear IRQ_GPIO_IRQSTATUS_1
  LDI32 r0, (GPIO0|GPIO_IRQSTATUS_1)                    ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ;
  SET   r1, r1, INPUT_MA_TD                             ;
  SBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
; set flag shared[9] bit1
  LBBO  &r0, r20, SHD_9, 4                              ; Shared[9] avisamos a linux
  SET   r0, r0, SHD_MA_TD                               ; bit1 flag direction motor_MA toggle
  SBBO  &r0, r20, SHD_9, 4                              ;
  QBA   level_H                                         ; volvemos al loop principal

DISABLE_MB:
  LDI32 r0, (GPIO2|GPIO_CLRDATAOUT)                     ;
  LDI32 r1, GPIO_MOTOR_ENA_BASE                         ; direccion gpio MOTOR_ENA_Base Motor_MB
  LSL   r1, r1, 2                                       ; hay que moverse 2 posiciones Enable motor_MB
  SBBO  &r1, r0, 0, 4                                   ;
; set flag shared[8] bit2
  LBBO  &r0, r20, SHD_9, 4                              ; Shared[9] avisamos a linux
  SET   r0, r0, SHD_MB_DISABLE                          ; bit2 flag motor_MB disable
  SBBO  &r0, r20, SHD_9, 4                              ;
  QBA   level_H1                                        ;

TOGGLE_DIR_MB:
; toggle direction pins
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, GPIO_MOTOR_DIR_BASE                         ;
  LSL   r2, r2, 2                                       ;
  XOR   r1, r1, r2                                      ; toggle
  SBBO  &r1, r0, 0 , 4                                  ; actualizamos la salida
; clear IRQ_GPIO_IRQSTATUS_1
  LDI32 r0, (GPIO0|GPIO_IRQSTATUS_1)                    ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ;
  SET   r1, r1, INPUT_MB_TD                             ;
  SBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
; set flag shared[9] bit3
  LBBO  &r0, r20, SHD_9, 4                              ; Shared[9] avisamos a linux
  SET   r0, r0, SHD_MB_TD                               ; bit3 flag direction motor_MB toggle
  SBBO  &r0, r20, SHD_9, 4                              ;
  QBA   level_I                                         ; volvemos al loop principal

  LDI32   R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)        ;
  HALT    					        ; halt the pru program
