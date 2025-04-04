; Raul Emilio Romero
; 27-01-2025
; cd /sys/class/remoteproc/remoteproc1
; echo 'am335x-pru0-rc-encoder-fw' > firmware
; echo 'start'> state
; echo 'stop' > state
; cd /sys/kernel/debug/remoteproc/remoteproc1
; sudo cat regs

; Este programa registra 1024 muestras por canal a una frecuencia de muestreo de fs=172kHz
; tiene dos flanco configurables para verificar que el tiempo total de registro es de aprox 5,8 ms

; PRU setup definitions

; memory
  .asg C28, CONST_PRUSHAREDRAM         		       ; shared mem addr contant table
  .asg 64,  CNT_OFFSET	                               ; valor original 64

  .asg 0x00000000, RAM0_ADDR_BASE                      ; local addr ram0. In linux space addr is 0x4a300000
  .asg 0x00002000, RAM1_ADDR_BASE                      ; local addr ram1. In linux space addr is 0x4a302000

  .asg 0x00000000, RAM0_INDEX_0                        ;
  .asg 0x00001000, RAM0_INDEX_N  		       ;

  .asg 0x04, OFFSET_4_BYTES                            ; el tamanio del dato es 4 bytes
  .asg 0x1000, SAMPLES                                 ; Muestras a guardar en cada iteracion 0x1000 -> 4096
  .asg "50000", SAMPLE_PERIOD                          ; fs aprox 99799 -> 1KHz. 1000

  .asg 0x00010000, SHARED_ADDR                         ; local addr shared. In linux space addr is 0x4a310000
  .asg 0x20, SHD_8    		                       ; shared[8] bit0 flag linux. No usamos shared[0] para evitar condicion de carrera
  .asg 0x24, SHD_9				       ; Shared[9] bit0-> flag data ready RAM[0-4095] bit1-> flag data ready RAM[4096-8191]

  .asg 0x22000, PRU0_CTRL
  .asg 0x24000, PRU1_CTRL                              ; page 19

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

  .asg r30.t5, pruout_fs_sample_test		       ; debug-> usamos esta salida para validad fs. config-pin P9_27 pruout

  .asg 32, PRU0_R31_VEC_VALID                          ; allows notification of program completion
  .asg 3,  PRU_EVTOUT_0                                ; the event number that is sent back

  .clink
  .global start

start:
; registros fijos
  ; r3-> addr base RAM0
  ; r5-> N cantidad de muestras por ciclo de muestreo. Default N = 4096
  ; r6-> bit0 toggle RAM[0-4095] o RAM[4096-8191]

; inicializacion
  LDI32	r3, RAM0_ADDR_BASE		               ; registro que almancena la direccion base, co r4 nos desplazamos por toda la RAM0
  ZERO  &r4, 4                                         ; offset inicial en ram0[0]
  LDI32	r5, SAMPLES				       ; cantidad de muestras por iteracion
  LDI32 r6, 0					       ; flag para determinar que index se carga INDEX_0 O INDEX_N
  CLR   r30, pruout_fs_sample_test                     ; debug-> pru0_pru_r30_5 as an output P9_27

SETUP:
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

; debug-------------------------------------------------
  LDI32 r20, 0x01                                        ; forzamos manualmente un loop de medicion continuo
  SBCO  &r20,  CONST_PRUSHAREDRAM, (CNT_OFFSET+SHD_8), 4 ; load flag shared[8] bit0
; debug-------------------------------------------------

MAIN_LOOP:
  LBCO &r0,  CONST_PRUSHAREDRAM, (CNT_OFFSET+SHD_8), 4 ; load flag shared[8] bit0
  QBBS ADC_READ_CH, r0, 0                              ; jump is adc_read_flag is set shared[8] bit0
  QBA  MAIN_LOOP				       ;

ADC_READ_CH:
  SET r30, pruout_fs_sample_test		       ; debug-> para medir la frecuencia de cada iteracion

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
;  QBA   CLEAR_IRQ_END_OF_SECUENCE                      ; detectada la interrupcion porque termino la secuencia seguimos adelante

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
  ADD   r4, r4, OFFSET_4_BYTES                         ; desplazamos de a 4 bytes.
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

  CLR r30, pruout_fs_sample_test                       ; debug-> tiempo de cada conversion 4 muestras

; frecuencia de muestreo
  LDI32	r0, SAMPLE_PERIOD			       ;
LEVEL_SAMPLE_PERIOD:
  SUB  r0, r0, 1                                       ; index--
  QBNE LEVEL_SAMPLE_PERIOD, r0, 0        	       ;

  QBNE DATA_RDY_SET_FLAG, r4, r5                       ; llegamos a la cantidad de muestras por iteracion
  QBA  ADC_READ_CH				       ; si no llegamos seguimos midiendo de a cuatro muestras

DATA_RDY_SET_FLAG:
  LBCO &r0, CONST_PRUSHAREDRAM, (CNT_OFFSET+SHD_9), 4  ; read shared[8]
  CLR  r0, r0, 0                                       ; clear flag bit0 RAM0[0]-RAM0[4095]
  CLR  r0, r0, 1                                       ; clear flag bit1 RAM0[4096]-RAM0[8191]
  SBCO &r0, CONST_PRUSHAREDRAM, (CNT_OFFSET+SHD_9), 4  ; cargamos set flag bit1 en shared[8]

  XOR  r6.b0, r6.b0, 0                                 ; toggle ram0 addr initial index bit0
  QBBC SET_RAM_INDEX_N, r6, 0                          ; jump is clr bit0
  QBBS SET_RAM_INDEX_0, r6, 0                          ; jUmp is set bit0

SET_RAM_INDEX_N:
  LBCO &r0, CONST_PRUSHAREDRAM, (CNT_OFFSET+SHD_9), 4  ; read shared[9]
  SET  r0, r0, 0                                       ; activamos el flag bit0 indicando que hay datos para consumir de RAM0[0]-RAM0[4095]
  SBCO &r0, CONST_PRUSHAREDRAM, (CNT_OFFSET+SHD_9), 4  ; cargamos set flag bit0 en shared[9]
  LDI32 r4, RAM0_INDEX_N                               ; Cargamos index N para seguir cargando la memoria desde la direccion 0x1000
  QBA   MAIN_LOOP                                      ;
SET_RAM_INDEX_0:
  LBCO &r0, CONST_PRUSHAREDRAM, (CNT_OFFSET+SHD_9), 4  ; read shared[9]
  SET  r0, r0, 1                                       ; activamos el flag bit1 indicando que hay datos para consumir de RAM0[4096]-RAM0[8191]
  SBCO &r0, CONST_PRUSHAREDRAM, (CNT_OFFSET+SHD_9), 4  ; cargamos set flag bit1 en shared[9]
  LDI32 r4, RAM0_INDEX_0			       ; cargamos el indix 0 para escribir desde el principio de la memoria
  QBA   MAIN_LOOP                                      ;

  LDI32   R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)       ;
  HALT      					       ; we should never actually get here
