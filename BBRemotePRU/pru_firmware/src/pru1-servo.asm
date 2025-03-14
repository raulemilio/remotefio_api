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
; memory
  .asg 0x00000000, RAM0_ADDR           			; local addr ram0. In linux space addr is 0x4a300000
  .asg 0x00002000, RAM1_ADDR           			; local addr ram1. In linux space addr is 0x4a302000
  .asg 0x00010000, SHARED_ADDR         			; local addr shared. In linux space addr is 0x4a310000
  .asg 0x00, SHD_0    		       			; shared[0] flags linux
  .asg 0x04, SHD_1    		       			; shared[1] gpio_read bit4-> flag | bit0-bit3-> data_ch
  .asg 0x08, SHD_2    		       			; shared[2] gpio_write bit8-> flag write complete | bit0-bit7 flags_ch-state_ch
  .asg 0x0C, SHD_3    		       			; shared[3] motor bit12-> flag config complete | bit0-bit11 flags_st-enable-direction
  .asg 0x10, SHD_4		       			; shared[4] motor_A STEP_PERIOD_A
  .asg 0x14, SHD_5		      	 		; shared[5] motor_B STEP_PERIOD_B
  .asg 0x18, SHD_6		       			; shared[6] motor_C STEP_PERIOD_C
  .asg 0x1C, SHD_7		       			; shared[7] motor_D STEP_PERIOD_D
  .asg 0x0a, SHARED_MEM_SIZE          	 		; 10 en decimal
  .asg 0x04, OFFSET_MEM                			;

; gpio bank
  .asg 0x44e07000, GPIO0               			; GPIO Bank 0, See the AM335x TRM
  .asg 0x481ac000, GPIO2               			; GPIO Bank 2

  .asg 0x190, GPIO_CLRDATAOUT          			; for clearing the GPIO registers
  .asg 0x194, GPIO_SETDATAOUT          			; for setting the GPIO registers
  .asg 0x13C, GPIO_DATAOUT            		 	; for read-write the GPIO registers
  .asg 0x138, GPIO_DATAIN              			; for reading the GPIO registers

  .asg 0x130, GPIO_CTRL        	      			; enable GPIO port
  .asg 0x00,  GPIO_CTRL_ENABLE         			;
  .asg 0x134, GPIO_OE                  			; set GPIO input - output
  .asg 0xFC3C003F, GPIO_OE_OUTPUT      			; pines usados como salida bit6 a bit17 y bit22 al bit25 en 0
  .asg 0xF0,  GPIO_OE_INPUT            			; para todos lo pines usados como entrada bit8 al bit10 en 1

; gpio_read
  ; Pins
  ; P8_35 gpio0[8] -> bit8 input_0  no hace falta hacer: config-pin P8_35 gpio_pd
  ; P8_33 gpio0[9] -> bit9 input_1  no hace falta hacer: config-pin P8_33 gpio_pd
  ; P8_31 gpio0[10]-> bit10 input_2 no hace falta hacer: config-pin P8_31 gpio_pd
  ; P8_32 gpio0[11]-> bit11 input_3 no hace falta hacer: config-pin P8_32 gpio_pd

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
  ; P8_45 gpio2[6] -> bit6 output_MA_E no hace falta hacer: config-pin P8_45 gpio
  ; P8_46 gpio2[7] -> bit7 output_MA_D config-pin P8_46 gpio

  ; P8_43 gpio2[8] -> bit8 output_MB_E config-pin P8_43 gpio
  ; P8_44 gpio2[9] -> bit9 output_MB_D config-pin P8_44 gpio

  ; P8_41 gpio2[10]-> bit10 output_MC_E config-pin P8_41 gpio
  ; P8_42 gpio2[11]-> bit11 output_MC_D config-pin P8_42 gpio

  ; P8_39 gpio2[12]-> bit12 output_MD_E config-pin P8_39 gpio
  ; P8_40 gpio2[13]-> bit13 output_MD_D config-pin P8_40 gpio

  ; Step period (medio ciclo de la senial)
  .asg (1<<14), GPIO2_14_MA_S          			; P8_37 gpio2[14] output_MA_S no hace falta hacer config-pin
  .asg (1<<15), GPIO2_15_MB_S          			; P8_38 gpio2[15] output_MB_S
  .asg (1<<16), GPIO2_16_MC_S          			; P8_36 gpio2[16] output_MC_S
  .asg (1<<17), GPIO2_17_MD_S          			; P8_34 gpio2[17] output_MD_S

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
  LDI32 r0, (GPIO0|GPIO_CTRL) 	       			; load GPIO0 control register address
  LDI32 r1, GPIO_CTRL_ENABLE           			; load control enable value
  SBBO  &r1, r0, 0, 4                  			; write enable value to GPIO0 control register

  LDI32 r0, (GPIO0|GPIO_OE)            			; load GPIO0 output enable register address
  LDI32 r1, GPIO_OE_INPUT              			; set GPIO0 as input
  SBBO  &r1, r0, 0, 4                  			; write input configuration to GPIO0

  LDI32 r0, (GPIO2|GPIO_CTRL)          			; load GPIO2 control register address
  LDI32 r1, GPIO_CTRL_ENABLE           			; load control enable value
  SBBO  &r1, r0, 0, 4                  			; write enable value to GPIO2 control register

  LDI32 r0, (GPIO2|GPIO_OE)            			; load GPIO2 output enable register address
  LDI32 r1, GPIO_OE_OUTPUT             			; set GPIO2 as output
  SBBO  &r1, r0, 0, 4                  			; write output configuration to GPIO2

  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load GPIO2 output
  ZERO  &r1, 4                                          ;
  SBBO  &r1, r0, 0 , 4                                  ; Cargamos todas las salidas en cero

; debug ---------------------------------------------------------------------------------------
; get flag function
;  LBBO &r0, r20, SHD_0, 4                               ; load shared[0]
; set flag function probe
;  SET  r0, r0, 0                          	         ; set flag gpio_read bit0
;  SET  r0, r0, 1                          	         ; set flag gpio_write bit1
;  SET  r0, r0, 2			  	         ; set flaf motor bit2
; set flag set in shared[0]
;  SBBO  &r0, r20, SHD_0, 4                              ;
; gpio read
;  LBBO  &r21, r20, SHD_1, 4                             ; valores de las entradas gpio_read bit0-bit3
; gpio write
;  LDI32 r22, 0xFF			                 ; debug-> Cargamos gpio_write state->1100 pins->1101
;  SBBO  &r22, r20, SHD_2, 4                             ;
; motor
;  LDI32 r23, 0x47D                       		 ; direction-enable 01 00 01 11  motor->1101 (0100 0111 1101)
;  LDI32 r24, 0xFFF					 ; Cargamos un STEP_PERIOD_A de prueba en decimal 4096  aprox 2.8 ms
;  LDI32 r25, 0xFFF					 ; Cargamos un STEP_PERIOD_B de prueba
;  LDI32 r26, 0xFF					 ; Cargamos un STEP_PERIOD_C de prueba
;  LDI32 r27, 0xF					 ; Cargamos un STEP_PERIOD_D de prueba en decimal 15 aprox  10 us
;  SBBO  &r23, r20, SHD_3, 4 		 		 ; cargamos enable-dir
;  SBBO  &r24, r20, SHD_4, 4 		 		 ; cargamos STEP_PERIOD_A
;  SBBO  &r25, r20, SHD_5, 4 		 		 ; cargamos STEP_PERIOD_B
;  SBBO  &r26, r20, SHD_6, 4 		 		 ; cargamos STEP_PERIOD_C
;  SBBO  &r27, r20, SHD_7, 4 				 ; cargamos STEP_PERIOD_D
; debug ----------------------------------------------------------------------------------------

MAIN_LOOP:
; get state gpio_read function flag
  LBBO  &r0, r20, SHD_0, 4		  		; read shared[0]
  QBBS  GPIO_READ, r0, 0                                ; jump is gpio_read_flag is set shared[0] bit0
level_A:
; get state gpio_write function flag
  LBBO  &r0, r20, SHD_0, 4  				; read shared[0]
  QBBS  GPIO_WRITE, r0, 1                               ; jump is gpio_write_flag is set shared[0] bit1
level_B:
; get state motor function flag
  LBBO  &r0, r20, SHD_0, 4 		 		; read shared[0]
  QBBS  MOTOR, r0, 2                       	        ; jump is motor_flag is set shared[0] bit2
level_C:
; get enable MA
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2 Set data r0
  LBBO  &r1, r0, 0, 4				        ; load value into r1 r0 addr
  QBBS  STEP_PERIOD_A, r1, 6                            ; jump is output_MA_E is set bit6
  ZERO  &r10, 4  					; borramos r10 STEP_PERIOD_A
  SBBO  &r10, r20, SHD_4, 4 				; borramos shared[4] STEP_PERIOD_A
level_C1:
  QBBS  STEP_PERIOD_B, r1, 8				; jump is output_MB_E is set bit8
  ZERO  &r11, 4                                         ; borramos r11 STEP_PERIOD_B
  SBBO  &r11, r20, SHD_5, 4 				; borramos shared[5] STEP_PERIOD_B
level_C2:
  QBBS  STEP_PERIOD_C, r1, 10                           ; jump is output_MC_E is set bit10
  ZERO  &r12, 4                                         ; borramos r12 STEP_PERIOD_C
  SBBO  &r12, r20, SHD_6, 4 				; borramos shared[6] STEP_PERIOD_C
level_C3:
  QBBS  STEP_PERIOD_D, r1, 12                           ; jump is output_MD_E is set bit12
  ZERO  &r13, 4                                         ; borramos r13 STEP_PERIOD_D
  SBBO  &r13, r20, SHD_7, 4 				; borramos shared[7] STEP_PERIOD_D
level_D:

  QBA   MAIN_LOOP			   	        ; program loop

GPIO_READ:
; clr flag gpio_read
  LBBO  &r0, r20, SHD_0, 4  				; shared[0]
  CLR   r0,r0, 0					; bit0 flag gpio_read
  SBBO  &r0, r20, SHD_0, 4  				;
; read GPIO IN
  LDI32 r0, (GPIO0|GPIO_DATAIN)                  	; load addr for DATAIN
  LBBO  &r1, r0, 0, 4                      	        ; Load the values at r0 into r1.
; write GPIO IN DATA INTO SHARED[1]
  LDI32 r0, 0xF00			   	        ; mascara desde bit8 a bit11 porque en el registro GPIO_DATAIN estan en esa posicion
  AND	r1, r1, r0                         	        ;
  LSR   r1, r1, 8			   	        ; se desplazan 8 posiciones para que los datos queden a partir del bit 0
; set flag data ready
  SET   r1, r1, 12               	       		; shared[1] bit12-> flag data ready
  SBBO  &r1, r20, SHD_1, 4 		 		; Cargamos valores de las entradas gpio_read bit0-bit3 y el flag data ready
  QBA	level_A		                                ;

GPIO_WRITE:
; clr flag gpio_write
  LBBO  &r0, r20, SHD_0, 4  				; Shared[0]
  CLR   r0,r0, 1                                        ; bit1 flag gpio_write
  SBBO  &r0, r20, SHD_0, 4  				;
; read gpio_output to set
  LBBO  &r0, r20, SHD_2, 4  				; read shared[2]
  AND   r1, r0, 0x0F      	  		        ; extrae los primeros 4 bits (0-3) en r1. Flag de mascaras
  AND   r2, r0, 0xF0      		   	        ; extrae los bits 4-7 en r2. Pin state
  LSR   r2, r2, 4         		   	        ; desplaza los bits 4-7 a la posicion menos significativa en r2
  LDI32	r3, 0            	           	        ; inicializa el contador de bits (0 a 3)
  LDI32 r4, GPIO_OUT_BASE			   	; direccion base GPIO_OUT_BASE
check_bits_w:
  AND   r5, r1, 1  			   	        ; extrae el bit menos significativo de r1 (state 0 o 1)
  QBBS  gpio_write_out_funct, r1, r3       	        ; qbbs myLabel r1, r3. Branch if( r1&(1<<r3) ) ojo r3 es un 1 desplazado
level_A1:
  LSR   r2, r2, 1			   		; desplaza r2 a la derecha para procesar el siguiente bit de state
  LSL   r4, r4, 1                          		; desplaza hacia el proximo bit de GPIO_out1,...GPIO_out2....
  ADD   r3, r3, 1			   		; count++
  QBGT  check_bits_w, r3, 4                		; cuatro salidas. qbgt myLabel, r3, r4. Branch if r4 > r3
; set flag data complete
  ZERO  &r0, 4						; shared[2] erase
  SET   r0, r0, 12                                      ; shared[2] bit12-> flag write complete
  SBBO  &r0, r20, SHD_2, 4  				;
  QBA	level_B		         	         	;

gpio_write_out_funct:
  QBBS  write_out_set, r5, 0		   		; jamp si bit0 de r5 es set
  QBBC  write_out_clr, r5, 0               	        ; jamp si bit0 de r5 es clr
write_out_set:
  LDI32 r6, (GPIO2|GPIO_SETDATAOUT)        	        ; load addr for GPIO Set data r6
  SBBO  &r4, r6, 0, 4                      		; write r4 to the r6 address valu
  QBA	level_A1			   		;
write_out_clr:
  LDI32 r6, (GPIO2|GPIO_CLRDATAOUT)        		; load addr for GPIO Clear data.
  SBBO  &r4, r6, 0, 4                      		; write r4 to the r6 address
  QBA	level_A1			   		;

MOTOR:
; Cargamos los step
  LBBO  &r10, r20, SHD_4, 4  				; read shared[4] STEP_PERIOD_A
  LBBO  &r11, r20, SHD_5, 4  				; read shared[5] STEP_PERIOD_B
  LBBO  &r12, r20, SHD_6, 4  				; read shared[6] STEP_PERIOD_C
  LBBO  &r13, r20, SHD_7, 4  				; read shared[7] STEP_PERIOD_C
; clr flag motor
  LBBO  &r0, r20, SHD_0, 4 		 		; Shared[0]
  CLR   r0,r0, 2                                        ; bit2 flag motor
  SBBO  &r0, r20, SHD_0, 4  				;
  LBBO  &r0, r20, SHD_3, 4  				; read shared[3]
  AND   r1, r0, 0x0F                       		; extrae los primeros 4 bits (0-3) en r1. Flag de mascaras
  LDI32 r2, 0xFF0			   		; mascara bit 4-11
  AND   r2, r0, r2                         		; extrae los bits 4-11 en r2. Pins enable-dir
  LSR   r2, r2, 4                          		; desplaza los bits 4-11 a la posicion menos significativa en r2
  LDI32 r3, 0                              		; inicializa el contador de bits (0 a 3)
  LDI32 r4, GPIO_MOTOR_ENA_BASE             		; direccion base gpio MOTOR_ENA_Base
  LDI32	r5, GPIO_MOTOR_DIR_BASE		   		; direccion base gpio MOTOR_DIR_Base
check_bits_m:
  AND   r6, r2, 0x03                       		; extrae los dos primeros  bits. Bit0 -> Enable Bit1-> Dir
  QBBS  motor_config, r1, r3               		; qbbs myLabel r2, r4. Branch if( r2&(1<<r4) )
level_B1:
  LSR   r2, r2, 2                          		; desplaza r2 a la derecha para procesar los siguientes 2 bits dir-ena
  LSL	r4, r4, 2			   		; desplaza a la siguiente Ena
  LSL	r5, r5, 2			   		; desplaza a la siguente Dir
  ADD   r3, r3, 1                          		; count++
  QBGT  check_bits_m, r3, 4                		; cuatro salidas. qbgt myLabel, r3, r4. Branch if r4 > r3
; set flag motor config complete
  ZERO  &r0, 4						; shared[3] erase
  SET   r0, r0, 12                      		; shared[3] bit12-> flag motor config complete
  SBBO  &r0, r20, SHD_3, 4  		                ;
  QBA 	level_C				    		;

motor_config:
  QBBS  enable_set, r6, 0		    	 	;
  QBBC  enable_clr, r6, 0                   		;
level_B2:
  QBBS  dir_set, r6, 1			    		;
  QBBC  dir_clr, r6, 1    		    		;
enable_set:
  LDI32 r7, (GPIO2|GPIO_SETDATAOUT)         		; load addr for GPIO Set data r7
  SBBO  &r4, r7, 0, 4                       		; write r4 to the r7 address value
  QBA	level_B2			    		;
enable_clr:
  LDI32 r7, (GPIO2|GPIO_CLRDATAOUT)         		; load addr for GPIO Set data r7
  SBBO  &r4, r7, 0, 4                       		; write r4 to the r6 address value
  QBA	level_B2			    		;
dir_set:
  LDI32 r7, (GPIO2|GPIO_SETDATAOUT)         		; load addr for GPIO Set data r7
  SBBO  &r5, r7, 0, 4                       		; write r5 to the r7 address value
  QBA	level_B1		            		;
dir_clr:
  LDI32 r7, (GPIO2|GPIO_CLRDATAOUT)         		; load addr for GPIO Set data r7
  SBBO  &r5, r7, 0, 4                       		; write r5 to the r7 address value
  QBA	level_B1			    		;

STEP_PERIOD_A:
  SUB   r10, r10, 1					; se resta 1 a r10. r10 fue cargado cuando se habilito el motorA
  QBEQ  TOGGLE_PIN_A, r10, 0				; toggle pin step cuando la cuenta llega a 0
  QBA   level_C1					; sino volvemos al loop principal
TOGGLE_PIN_A:
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, GPIO2_14_MA_S 				; GPIO2_14_MA_S
  XOR   r1, r1, r2				        ; toggle GPIO2_14_MA_S
  SBBO  &r1, r0, 0 , 4					; actualizamos la salida GPIO2_14_MA_S
  LBBO  &r10, r20, SHD_4, 4  		                ; cargamos nuevamente el valor de shared[4] para arrancar de nuevo la cuenta
  QBA   level_C1					; volvemos al loop principal

STEP_PERIOD_B:
  SUB   r11, r11, 1                                     ; se resta 1 a r11. r11 fue cargado cuando se habilito el motorB
  QBEQ  TOGGLE_PIN_B, r11, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_C2                                        ; sino volvemos al loop principal
TOGGLE_PIN_B:
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, GPIO2_15_MB_S                               ; GPIO2_15_MB_S
  XOR   r1, r1, r2                                      ; toggle GPIO2_15_MB_S
  SBBO  &r1, r0, 0 , 4                                  ; actualizamos la salida GPIO2_15_MB_S
  LBBO  &r11, r20, SHD_5, 4  		                ; cargamos nuevamente el valor de shared[5] para arrancar de nuevo la cuenta
  QBA   level_C2                                        ; volvemos al loop principal

STEP_PERIOD_C:
  SUB   r12, r12, 1                                     ; se resta 1 a r12. r12 fue cargado cuando se habilito el motorC
  QBEQ  TOGGLE_PIN_C, r12, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_C3                                        ; sino volvemos al loop principal
TOGGLE_PIN_C:
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, GPIO2_16_MC_S                               ; GPIO2_16_MC_S
  XOR   r1, r1, r2                                      ; toggle GPIO2_16_MC_S
  SBBO  &r1, r0, 0 , 4                                  ; actualizamos la salida GPIO2_16_MC_S
  LBBO  &r12, r20, SHD_6, 4                             ; cargamos nuevamente el valor de shared[6] para arrancar de nuevo la cuenta
  QBA   level_C3                                        ; volvemos al loop principal

STEP_PERIOD_D:
  SUB   r13, r13, 1                                     ; se resta 1 a r13. r13 fue cargado cuando se habilito el motorD
  QBEQ  TOGGLE_PIN_D, r13, 0                            ; toggle pin step cuando la cuenta llega a 0
  QBA   level_D                                         ; sino volvemos al loop principal
TOGGLE_PIN_D:
  LDI32 r0, (GPIO2|GPIO_DATAOUT)                        ; load addr for GPIO2
  LBBO  &r1, r0, 0, 4                                   ; Load the values at r0 into r1.
  LDI32 r2, GPIO2_17_MD_S                               ; GPIO2_17_MD_S
  XOR   r1, r1, r2                                      ; toggle GPIO2_17_MD_S
  SBBO  &r1, r0, 0 , 4                                  ; actualizamos la salida GPIO2_17_MD_S
  LBBO  &r13, r20, SHD_7, 4                             ; cargamos nuevamente el valor de shared[7] para arrancar de nuevo la cuenta
  QBA   level_D                                         ; volvemos al loop principal

  LDI32   R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)        ;

  HALT    					        ; halt the pru program
