; Raul Emilio Romero
; 31-03-2025
; cd /sys/class/remoteproc/remoteproc1
; echo 'am335x-pru0-analog-fw' > firmware
; echo 'start'> state
; echo 'stop' > state
; cd /sys/kernel/debug/remoteproc/remoteproc1
; sudo cat regs

; Este programa registra 1024 muestras por canal a una frecuencia de muestreo de fs=172kHz
; tiene dos flanco configurables para verificar que el tiempo total de registro es de aprox 5,8 ms

; PRU setup definitions

; memory
  .asg 0x00000000, RAM0_ADDR_BASE                       ; local addr ram0. In linux space addr is 0x4a300000
  .asg 0x00010000, SHD_ADDR_BASE	       	        ;

  .asg 0x00000000, RAM0_BUFFER0_INDEX0                  ; arranca en 0x0
  .asg 0x00001000, RAM0_BUFFER1_INDEX0  	        ; arranca en 0x1000

  .asg 0x02, OFFSET_2_BYTES                             ; el tamanio del dato es 2 bytes

  .asg 0xA0, SHD_FLAGS_INDEX                            ; SHD[40] flags linux bit0-> no trigger bit1-> trigger (asc flanc adq)
  .asg 0xA4, SHD_DATARDY_FLAGS_INDEX                    ; SHD[41] data ready flag bit0 [ram0-ram4095] bit1 [ram4096-ram8191]
  .asg 0xA8, SHD_SAMPLE_PERIOD_INDEX		        ; SHD[42] periodo de muestreo
  .asg 0xAC, SHD_BUFFER_SIZE_INDEX	                ; SHD[43] Cantidad de muestras por iteracion

  .asg 0x22000, PRU0_CTRL
  .asg 0x24000, PRU1_CTRL                               ; page 19

; gpio bank
  .asg 0x44e07000, GPIO0                                ; GPIO Bank 0

  .asg 0x190, GPIO_CLRDATAOUT                           ; for clearing the GPIO registers
  .asg 0x194, GPIO_SETDATAOUT                           ; for setting the GPIO registers
  .asg 0x13C, GPIO_DATAOUT                              ; for read-write the GPIO registers
  .asg 0x138, GPIO_DATAIN                               ; for reading the GPIO registers
  .asg 0x38,  GPIO_IRQSTATUS_SET_1                      ; habilita las interrupciones por set
  .asg 0x148, GPIO_RISINGDETECT                         ; habilita las interrupciones por flanco asc
  .asg 0x30,  GPIO_IRQSTATUS_1                          ; get-set irq

  .asg 0x130, GPIO_CTRL                                 ; enable GPIO port
  .asg 0x00,  GPIO_CTRL_ENABLE                          ;
  .asg 0x134, GPIO_OE                                   ;
  .asg 23,    ADC_INPUT_TRIGGER 			; set gpio p8_13 GPIO0[23] bit23


; registros del modulo clk
  .asg 0x44e00400, CM_WKUP                             ; addr base. TRM pag.179
  .asg 0xBC, CM_WKUP_ADC_TSC_CLKCTRL                   ; clk offset TRM pag.1315
  .asg 0x02, CM_WKUP_ADC_TSC_CLKCTRL_ENABLE            ; TRM pag.1369

; end-of-sequence interrupt
  .asg 0x28, ADC_TSC_IRQSTATUS                         ;
  .asg 0x02, ADC_TSC_IRQSTATUS_END_OF_SECUENCE         ; cuidado aca el bit es el 0x02 pero debe configurarse asi para

  .asg 0x30, ADC_TSC_IRQENABLE_CLR                     ;
  .asg 0xFFFFFFFF, ADC_TSC_IRQENABLE_CLR_ALL_MASK      ; deshabilitar todas las interrupciones para luego  habilitar solo las que usamos

  .asg 0x2C, ADC_TSC_IRQENABLE_SET                     ;
  .asg 0x00, ADC_TSC_IRQENABLE_SET_RESET_MASK          ;

  .asg 0x02, ADC_TSC_IRQENABLE_SET_END_OF_SECUENCE     ; aviso que termino la secuencia de medicion
  ;.asg 0x0F, ADC_TSC_IRQENABLE_SET_FIFO_0_THRESHOLD    ; aviso que todos los datos estan disponibles en FIFO

; registros especificos del modulo ADC_TSC
  .asg 0x44e0d000, ADC_TSC                             ; TRM pag.180
  .asg 0x40, ADC_TSC_CTRL                              ; registro de control del modulo
  .asg 0x00, ADC_TSC_CTRL_ENABLE	               ;
  .asg 0x01, ADC_TSC_CTRL_STEP_ID_TAG                  ;
  .asg 0x02, ADC_TSC_CTRL_STEPCONFIG_PROTECT           ;

  .asg 0x48, ADC_TSC_ADCRANGE                          ;
  .asg (0xFFF<<16), ADC_TSC_ADCRANGE_HIGH_RANGE_DATA   ; cargamos 4095 en los bits 27-16. TRM pag. 1859

  .asg 0x4C, ADC_TSC_CLKDIV                            ; clk
  .asg 0x00, ADC_TSC_CLKDIV_ADC_CLKDIV                 ; valor a cargar para la division del clk. Cargando 0x0 el valor minimo es 1

  .asg 0x54, ADC_TSC_STEPENABLE                        ; registro para habilitar los steps
  .asg 0x3A, ADC_TSC_STEPENABLE_ENABLE_MASK            ; un canal (0000 0010 -> 0x02) Cuatro canales (0001 1110 -> 0x1E). 0011 1010 -> 0x3A
  .asg 0x00, ADC_TSC_STEPENABLE_DISABLE_MASK           ; se desahbilitan todos los canales

  .asg 0x64, ADC_TSC_STEPCONFIG_1                      ; ch0
  .asg 0x00, ADC_TSC_STEPCONFIG_1_MASK                 ; SW mode, single ended, one shot mode, fifo 0, channel 0 P9_39 (AIN0)

  .asg 0x68, ADC_TSC_STEPDELAY_1                       ; TRM pag. 1859
  .asg 0x00, ADC_TSC_STEPDELAY_1_MASK                  ; OpenDly0 SampleDly0

  .asg 0x74, ADC_TSC_STEPCONFIG_3                      ; ch2
  .asg 0x00100000, ADC_TSC_STEPCONFIG_3_MASK           ; SW mode, single ended, one shot mode, fifo 0, channel 3 P9_37 (AIN2)

  .asg 0x78, ADC_TSC_STEPDELAY_3                       ; TRM pag. 1859
  .asg 0x00, ADC_TSC_STEPDELAY_3_MASK                  ; OpenDly0 SampleDly0

  .asg 0x7C, ADC_TSC_STEPCONFIG_4                      ; ch3
  .asg 0x00180000, ADC_TSC_STEPCONFIG_4_MASK           ; SW mode, single ended, one shot mode, fifo 0, channel 4 P9_38 (AIN3)

  .asg 0x80, ADC_TSC_STEPDELAY_4                       ; TRM pag. 1859
  .asg 0x00, ADC_TSC_STEPDELAY_4_MASK                  ; OpenDly0 SampleDly0

  .asg 0x84, ADC_TSC_STEPCONFIG_5                      ; ch5
  .asg 0x00200000, ADC_TSC_STEPCONFIG_5_MASK           ; SW mode, single ended, one shot mode, fifo 0, channel 5 P9_33 (AIN4)

  .asg 0x88, ADC_TSC_STEPDELAY_5                       ; TRM pag. 1859
  .asg 0x00, ADC_TSC_STEPDELAY_5_MASK                  ; openDly0 SampleDly0

  .asg 0xE4, ADC_TSC_FIFO_0_COUNT                      ; registro de conteo de datos en FIFO0
  .asg 0x100, ADC_TSC_FIFO_0_DATA                      ; registro FIFO0
  .asg 0xFFF, MASK_12_BIT                              ; FIFO0_DATA

; functions flags
  .asg 0, ADC_NOTRIGGER_FLAG                           ; SHD[20] bit0
  .asg 1, ADC_TRIGGER_FLAG                             ; SHD[20] bit1

;  .asg r30.t5, pruout_fs_sample_test		       ; debug-> usamos esta salida para validad fs. config-pin P9_27 pruout

  .asg 32, PRU0_R31_VEC_VALID                          ; allows notification of program completion
  .asg 3,  PRU_EVTOUT_0                                ; the event number that is sent back

; Debug
  .asg 9480, SAMPLES_PERIOD_DEBUG		       ; Fs = 10 kHz
  .asg 0x200, BUFFER_SIZE_DEBUG		               ; Muestras a guardar en cada iteracion por canal maximo 0x200 -> 512

  .clink
  .global start

start:
; registros fijos
  ; r3-> addr base RAM0
  ; r4-> variable de recorrido de memoria
  ; r5-> limite buffer0
  ; r6-> limite buffer1
  ; r10-> addr base SHD
  ; r11-> sample_period
  ; r12-> buffer_count
  ; r16-> irq addr

; inicializacion
  LDI32 r0, 0					       ;
  SBBO  &r0, r10, SHD_FLAGS_INDEX, 4                   ; borramos todos los flags
  LDI32	r3, RAM0_ADDR_BASE		               ; registro que almancena la direccion base, con r4 nos desplazamos por toda la RAM0
  ZERO  &r4, 4                                         ; offset inicial en RAM0[0]
  LDI32	r5, 0  				               ; tamanio maximo buffer0 recordar que va de a 4 bytes 0x0000 - 0x0FFC por canal 1024
  LDI32 r6, 0				               ; tamanio maximo de ram1 buffer1                      0x1000 - 0x1FFC por canal 1024
  LDI32 r10, SHD_ADDR_BASE
  LDI32 r11, 0					       ; sample_period
  LDI32 r12, 0					       ; buffer count
;  CLR   r30, pruout_fs_sample_test                     ; debug-> pru0_pru_r30_5 as an output P9_27

SETUP:
;GPIO0
  LDI32 r0, (GPIO0|GPIO_CTRL)                          ; load GPIO0 control register address
  LDI32 r1, GPIO_CTRL_ENABLE                           ; load control enable value
  SBBO  &r1, r0, 0, 4                                  ; write enable value to GPIO0 control register

  LDI32 r0, (GPIO0|GPIO_OE)                            ; load GPIO1 output enable register address
  SET   r1, r1, ADC_INPUT_TRIGGER                      ; cargamos la entrada P8_13 GPI01[23] como entrada
  SBBO  &r1, r0, 0, 4                                  ; write output configuration to GPIO0

; rising detect
  LDI32 r0, (GPIO0|GPIO_RISINGDETECT)                  ; load addr for GPIO0
  LBBO  &r1, r0, 0, 4                                  ; Load the values at r0 into r1.
  SET   r1, r1, ADC_INPUT_TRIGGER                      ;
  SBBO  &r1, r0, 0, 4                                  ; Load the values at r0 into r1.

; enable irq set_1
  LDI32 r0, (GPIO0|GPIO_IRQSTATUS_SET_1)               ; load addr for GPIO0
  LBBO  &r1, r0, 0, 4                                  ; Load the values at r0 into r1.
  SET   r1, r1, ADC_INPUT_TRIGGER                      ;
  SBBO  &r1, r0, 0, 4                                  ; Load the values at r0 into r1.

; ADC MODULE
; CM_WKUP
; Habilitar el relor del modulo ADC en CM_WKUP
  LDI32 r0, (CM_WKUP|CM_WKUP_ADC_TSC_CLKCTRL)          ; cargamos en r0 la addr del registro CLKCTRL del modulo CM_WKUP
  LDI32 r1, CM_WKUP_ADC_TSC_CLKCTRL_ENABLE    	       ; cargamos el valor 0x02 en r1 que habilita en modulo CM_WKUP
  SBBO  &r1, r0, 0, 4                                  ; cargamos a donde apunta r0 el valor contenido en r1

; ADC_TSC module-> disabe BIT0
  LDI32 r0, (ADC_TSC|ADC_TSC_CTRL)                     ; load addr
  LBBO  &r1, r0, 0, 4                                  ; read value mem
  CLR   r1, r1, ADC_TSC_CTRL_ENABLE	 	       ; ojo para realiazar
  SBBO  &r1, r0, 0, 4                                  ; TRM pag. 1849 0b0110

; ADC_TSC module -> Enable StepConfig step ID tag BIT1
  LDI32 r0, (ADC_TSC|ADC_TSC_CTRL)                     ; load addr
  LBBO  &r1, r0, 0, 4                                  ; read value mem
  CLR   r1, r1, ADC_TSC_CTRL_STEP_ID_TAG               ; enable-> set | disable-> clr
  SBBO  &r1, r0, 0, 4                                  ; TRM pag. 1849

; ADC_TSC module -> Enable StepConfig Protect Are not protected (writable) BIT2. Tenemos que habilitar la escritura antes stepconfig
  LDI32 r0, (ADC_TSC|ADC_TSC_CTRL)                     ; load addr
  LBBO  &r1, r0, 0, 4                                  ; read value mem
  SET   r1, r1, ADC_TSC_CTRL_STEPCONFIG_PROTECT        ;
  SBBO  &r1, r0, 0, 4                                  ; TRM pag. 1849

; step 1 config
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPCONFIG_1)             ; load addr
  LDI32 r1, ADC_TSC_STEPCONFIG_1_MASK                  ;
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro

; step 1 SampleDelay-OpenDelay
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPDELAY_1)              ; load addr
  LDI32 r1, ADC_TSC_STEPDELAY_1_MASK                   ;
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro

; step 3 config
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPCONFIG_3)             ; load addr
  LDI32 r1, ADC_TSC_STEPCONFIG_3_MASK                  ;
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro

; step 3 SampleDelay-OpenDelay
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPDELAY_3)              ; load addr
  LDI32 r1, ADC_TSC_STEPDELAY_3_MASK                   ;
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro

; step 4 config
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPCONFIG_4)             ; load addr
  LDI32 r1, ADC_TSC_STEPCONFIG_4_MASK                  ;
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro

; step 4 SampleDelay-OpenDelay
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPDELAY_4)              ; load addr
  LDI32 r1, ADC_TSC_STEPDELAY_4_MASK                   ;
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro

; step 5 config
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPCONFIG_5)             ; load addr
  LDI32 r1, ADC_TSC_STEPCONFIG_5_MASK                  ;
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro

; step 5 SampleDelay-OpenDelay
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPDELAY_5)              ; load addr
  LDI32 r1, ADC_TSC_STEPDELAY_5_MASK                   ;
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro

; configurar el divisor del reloj
  LDI32 r0, (ADC_TSC|ADC_TSC_CLKDIV)                   ; load addr for ADS_TSC_CLKDIV
  LDI32 r1, ADC_TSC_CLKDIV_ADC_CLKDIV                  ; valor minimo
  SBBO  &r1, r0, 0, 4                                  ; escribir en TSC_ADC_CLKDIV

; definir el rango de medicion 0-4095
  LDI32 r0, (ADC_TSC|ADC_TSC_ADCRANGE)                 ; load addr for ADS_TSC_ADCRANGE
  LDI32 r1, ADC_TSC_ADCRANGE_HIGH_RANGE_DATA           ; 0 - 4095
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro

; IRQ DESAHBILITAR TODAS LAS INTERRUPCIONES
  LDI32 r0, (ADC_TSC|ADC_TSC_IRQENABLE_CLR)            ; load addr
  LDI32 r1, ADC_TSC_IRQENABLE_CLR_ALL_MASK	       ;
  SBBO  &r1, r0, 0, 4                                  ; TRM pag. 1851

; IRQ ENABLE END_OF_SECUENCE
  LDI32 r0, (ADC_TSC|ADC_TSC_IRQENABLE_SET)            ; load addr
  LDI32 r1, ADC_TSC_IRQENABLE_SET_END_OF_SECUENCE      ; ojo para realiazar un clear hay que hacer un set del bit
  SBBO  &r1, r0, 0, 4                                  ; TRM pag. 1849

; IRQ Clear interrupt
; CLEAR_IRQ_END_OF_SECUENCE:
  LDI32 r0, (ADC_TSC|ADC_TSC_IRQSTATUS)                ; load addr
  LDI32 r1, ADC_TSC_IRQENABLE_CLR_ALL_MASK             ; ojo para realiazar un clear hay que hacer un set del bit
  SBBO  &r1, r0, 0, 4                                  ; TRM pag. 1849

; GPIO0
; clear IRQ_GPIO_IRQSTATUS_1
  LDI32 r0, (GPIO0|GPIO_IRQSTATUS_1)                   ;
  LBBO  &r1, r0, 0, 4                                  ;
  SET   r1, r1, ADC_INPUT_TRIGGER                      ;
  SBBO  &r1, r0, 0, 4                                  ;

;-------------------------------------------------------
; Debug

; CONFIG
;  LDI32 r0, 0xff                                      ; test ram0
;  SBBO  &r0, r11, 0, 4                                ; ram0[0]

;  LDI32 r0, SAMPLES_PERIOD_DEBUG	               ; expresar en hexa
;  SBBO  &r0, r10, SHD_SAMPLE_PERIOD_INDEX, 4           ;
;  LDI32 r0, BUFFER_SIZE_DEBUG                          ; 0x200 buffer maximo 512 por canal
;  SBBO  &r0, r10, SHD_BUFFER_SIZE_INDEX, 4             ;

; MODE 0
;  LBBO  &r0, r10, SHD_FLAGS_INDEX, 4                   ;
;  SET   r0, r0, ADC_MODE0_FLAG                         ;
;  SBBO  &r0, r10, SHD_FLAGS_INDEX, 4                   ;

; MODE 1
;  LBBO  &r0, r10, SHD_FLAGS_INDEX, 4                   ;
;  SET   r0, r0, ADC_MODE1_FLAG                         ;
;  SBBO  &r0, r10, SHD_FLAGS_INDEX, 4                   ;
;-------------------------------------------------------

  LDI32 r16, (GPIO0|GPIO_IRQSTATUS_1)                  ;

MAIN_LOOP:
  LBBO  &r15, r10, SHD_FLAGS_INDEX, 4                  ;
  QBBS  ADC_INIT, r15, ADC_NOTRIGGER_FLAG     	       ; jump is set bit0
  QBBS  ADC_GET_TRIGGER_GPIO, r15, ADC_TRIGGER_FLAG    ; jump is set bit1
  ; get irq status
  LBBO  &r0, r16, 0, 4
  QBBS  CLR_IRQ, r0, ADC_INPUT_TRIGGER
  QBA   MAIN_LOOP                                      ;

CLR_IRQ:
  LBBO  &r0, r16, 0, 4
  SET   r0, r0, ADC_INPUT_TRIGGER                      ;
  SBBO  &r0, r16, 0, 4                                 ;
  QBA   MAIN_LOOP

ADC_GET_TRIGGER_GPIO:
  ; get irq status
  LBBO  &r0, r16, 0, 4
  QBBC  MAIN_LOOP, r0, ADC_INPUT_TRIGGER               ; si se detecta flanco asc comenzamos la conversion
  QBA   ADC_INIT

ADC_INIT:
  LDI32  r4, 0                                         ; RAM0 index init 0
  LBBO  &r5, r10, SHD_BUFFER_SIZE_INDEX, 4             ; SHD[3] buffer por canal maximo 512
  LDI32 r7, 0xFFFC                                     ; mascara para asegurar multiplo de 4
  AND   r5, r5, r7                                     ; aseguramos multiplo de 4
  ADD   r6, r5, r5                                     ; r6 es doble de r5
  LBBO  &r11, r10, SHD_SAMPLE_PERIOD_INDEX, 4          ; read SHD[2] SAMPLES PERIOD
  LDI32 r12, 0x0                                       ; RAM0 buffer count
  QBA   ADC_READ_CH

ADC_READ_CH:

;  SET r30, pruout_fs_sample_test		       ; debug-> period de muestreo de 4 canales, ojo no es el tiempo de buffer

; habilitar steps ENABLE VER MANUAL TRM pag. 1839 (solo puede escribirse en STEPCONFIG si ADC_TSC esta disable )
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPENABLE)               ; load addr
  LDI32 r1, ADC_TSC_STEPENABLE_ENABLE_MASK             ;
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro. En esta instruccion la FSM del modulo arranca a medir

; ADC_TSC module-> enable
  LDI32 r0, (ADC_TSC|ADC_TSC_CTRL)                     ; load addr
  LBBO  &r1, r0, 0, 4                                  ; read value mem
  SET   r1, r1, ADC_TSC_CTRL_ENABLE                    ; ojo para realiazar
  SBBO  &r1, r0, 0, 4                                  ; TRM pag. 1849 0x00

; wait for interrupt
WAIT_IRQ_END_OF_SECUENCE:
  LDI32 r0, (ADC_TSC|ADC_TSC_IRQSTATUS)                ; load addr
  LBBO  &r1, r0, 0, 4                                  ; read value mem
  AND   r2, r1, ADC_TSC_IRQSTATUS_END_OF_SECUENCE      ; Comprobar el estado del bit 1
  QBEQ  WAIT_IRQ_END_OF_SECUENCE, r2, 0                ; hasta que la interruccion no pase a 1 no se avanza a la proxima instruccion
  QBA   CLEAR_IRQ_END_OF_SECUENCE                      ; detectada la interrupcion porque termino la secuencia seguimos adelante

; clear interrupt
CLEAR_IRQ_END_OF_SECUENCE:
  LDI32 r0, (ADC_TSC|ADC_TSC_IRQSTATUS)                ; load addr
  LBBO  &r1, r0, 0, 4                                  ; read value mem
  SET   r1, r1, ADC_TSC_IRQSTATUS_END_OF_SECUENCE      ; ojo para realiazar un clear hay que hacer un set del bit
  SBBO  &r1, r0, 0, 4                                  ; TRM pag. 1849

; read count FIFO0
  LDI32 r0, (ADC_TSC|ADC_TSC_FIFO_0_COUNT)             ; load addr for mem
  LBBO  &r1, r0, 0, 4                                  ; read r1 <-FIFO_0_COUNT r1 contiene la cantidad 4

ADC_READ_4SAMPLES:
  SUB   r1, r1, 1                                      ; index-- en cada iteracion:  3-2-1-0
  LDI32 r0, (ADC_TSC|ADC_TSC_FIFO_0_DATA)              ; load addr for mem
  LBBO  &r2, r0, 0, 4                                  ; load r2 <- FIFO_O_DATA .Se lee extrae un dato de la FIFO. Recordar mecanismo
  SBBO  &r2, r3, r4, 4	  			       ; load r2 -> ram[] memory. La primer pasada guarda en ram[0] y la ultima en ram[3]
  ADD   r4, r4, OFFSET_2_BYTES                         ; desplazamos de a 2 bytes.
  QBNE  ADC_READ_4SAMPLES, r1, 0                       ; en este loop se leen solo 4 datos

; disable ADC_TSC module
  LDI32 r0, (ADC_TSC|ADC_TSC_CTRL)                     ; load addr
  LBBO  &r1, r0, 0, 4                                  ; read value mem
  CLR   r1, r1, ADC_TSC_CTRL_ENABLE                    ; disable module
  SBBO  &r1, r0, 0, 4                                  ; TRM pag. 1849

; deshabilitar steps
  LDI32 r0, (ADC_TSC|ADC_TSC_STEPENABLE)               ; load addr
  LDI32 r1, ADC_TSC_STEPENABLE_DISABLE_MASK            ; usamos mascaras cuando queremos configurar muchos bit juntos
  SBBO  &r1, r0, 0, 4                                  ; escribir en registro. En esta instruccion la FSM del modulo arranca a medir

;  CLR r30, pruout_fs_sample_test                       ; debug-> tiempo de cada conversion 4 muestras

  MOV r0, r11					       ; no lo hacemos leyendo desde shared porque es muy lento
LEVEL_SAMPLE_PERIOD:
  SUB  r0, r0, 1                                       ; index--
  QBNE LEVEL_SAMPLE_PERIOD, r0, 0        	       ; Esperamos el tiempo de muestreo

  LBBO  &r15, r10, SHD_FLAGS_INDEX, 4                  ; Verificamos si se detuvo la adquisicion desde linux
  QBEQ MAIN_LOOP, r15, 0

  ADD  r12, r12, 1				       ; memory RAM0 count
  QBEQ SET_BUFFER0_FLAG, r5, r12                       ; llenamos buffer0
level_A1:
  QBEQ SET_BUFFER1_FLAG, r6, r12          	       ; llenamos buffer1
  QBA ADC_READ_CH				       ;

SET_BUFFER0_FLAG:
  LBBO &r0, r10, SHD_DATARDY_FLAGS_INDEX, 4            ; read RAM1[1] flags de iteracion
  SET  r0, r0, 0                                       ; activamos el flag bit0 indicando que hay datos para consumir de RAM0[0]-RAM0[4095]
  SBBO &r0, r10, SHD_DATARDY_FLAGS_INDEX, 4            ; cargamos set flag bit0 en RAM1[1]
  LDI32 r4, RAM0_BUFFER1_INDEX0		               ; 0x1000 4096 mitad de ram0
  QBA   level_A1                                       ;

SET_BUFFER1_FLAG:
  LBBO &r0, r10, SHD_DATARDY_FLAGS_INDEX, 4            ; read RAM1[1] flags de iteracion
  SET  r0, r0, 1                                       ; activamos el flag bit1 indicando que hay datos para consumir de RAM0[4096]-RAM0[8191]
  SBBO &r0, r10, SHD_DATARDY_FLAGS_INDEX, 4            ; cargamos set flag bit1 en RAM1[1]
  LDI32 r4, RAM0_BUFFER0_INDEX0	                       ; 0x0 inicio de ram0
  LDI32 r12, 0x0                                       ; buffer count va desde 0 a tamanio de buffer x 2
  QBA   ADC_READ_CH				       ; se llenaron los dos buffers

  LDI32   R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)       ;
  HALT      					       ; we should never actually get here
