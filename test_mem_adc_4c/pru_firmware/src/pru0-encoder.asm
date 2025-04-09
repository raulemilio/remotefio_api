; Raul Emilio Romero
;       27-01-2025
; Este programa registra 1024 muestras por canal a una frecuencia de muestreo de fs=172kHz
; tiene dos flanco configurables para verificar que el tiempo total de registro es de aprox 5,8 ms

; PRU setup definitions
        .asg C28,    CONST_PRUSHAREDRAM			; AM335x_PRU.cmd

        .asg 0x22000, PRU0_CTRL
        .asg 0x24000, PRU1_CTRL                         ; page 19

        .asg 0x000, PRU0_RAM
        .asg 0x100, SHARED_RAM                          ; This is so prudebug can find it.
        .asg 64, CNT_OFFSET

	.asg 0x00000000, ram0_addr			; Estamos usando ram0 para guardar los datos
	.asg 0x04, ram0_offset				; Cada dato tiene 4 bytes

        ; Registros del modulo clk
        .asg 0x44e00400, CM_WKUP                        ; addr base. TRM pag.179
        .asg 0xBC, CM_WKUP_ADC_TSC_CLKCTRL              ; clk offset TRM pag.1315
        .asg 0x02, CM_WKUP_ADC_TSC_CLKCTRL_ENABLE       ; TRM pag.1369

        ; Registros especificos del modulo ADC_TSC
        .asg 0x44e0d000, ADC_TSC                        ; TRM pag.180
        .asg 0x40, ADC_TSC_CTRL                         ; Registro de control del modulo
        .asg 0x00, ADC_TSC_CTRL_ENABLE			;
        .asg 0x01, ADC_TSC_CTRL_STEP_ID_TAG             ;
        .asg 0x02, ADC_TSC_CTRL_STEPCONFIG_PROTECT      ;

        .asg 0x64, ADC_TSC_STEPCONFIG_1                 ; ch0
        .asg 0x00, ADC_TSC_STEPCONFIG_1_MASK            ; SW mode, single ended, one shot mode, fifo 0, channel 0 P9_39 (AIN0)
	;.asg 0x20800000, ADC_TSC_STEPCONFIG_1_MASK     ; SW mode, differential pair enable, one shot mode, fifo 0, ch0_positivo - ch1_negativo
        .asg 0x68, ADC_TSC_STEPDELAY_1                  ; TRM pag. 1859
        .asg 0x00, ADC_TSC_STEPDELAY_1_MASK             ; OpenDly0 SampleDly0

        .asg 0x74, ADC_TSC_STEPCONFIG_3                 ; ch2
        .asg 0x00100000, ADC_TSC_STEPCONFIG_3_MASK      ; SW mode, single ended, one shot mode, fifo 0, channel 3 P9_37 (AIN2)

        .asg 0x78, ADC_TSC_STEPDELAY_3                  ; TRM pag. 1859
        .asg 0x00, ADC_TSC_STEPDELAY_3_MASK             ; OpenDly0 SampleDly0

        .asg 0x7C, ADC_TSC_STEPCONFIG_4                 ; ch3
        .asg 0x00180000, ADC_TSC_STEPCONFIG_4_MASK      ; SW mode, single ended, one shot mode, fifo 0, channel 4 P9_38 (AIN3)

        .asg 0x80, ADC_TSC_STEPDELAY_4                  ; TRM pag. 1859
        .asg 0x00, ADC_TSC_STEPDELAY_4_MASK             ; OpenDly0 SampleDly0

        .asg 0x84, ADC_TSC_STEPCONFIG_5                 ; ch5
        .asg 0x00200000, ADC_TSC_STEPCONFIG_5_MASK      ; SW mode, single ended, one shot mode, fifo 0, channel 5 P9_33 (AIN4)
        .asg 0x88, ADC_TSC_STEPDELAY_5                  ; TRM pag. 1859
        .asg 0x00, ADC_TSC_STEPDELAY_5_MASK             ; OpenDly0 SampleDly0

        .asg 0x4C, ADC_TSC_CLKDIV                       ; clk
        .asg 0x00, ADC_TSC_CLKDIV_ADC_CLKDIV            ; valor a cargar para la division del clk. Cargando 0x0 el valor minimo es 1

        .asg 0x48, ADC_TSC_ADCRANGE                     ;
        .asg (0xFFF<<16), ADC_TSC_ADCRANGE_HIGH_RANGE_DATA; Cargamos 4095 en los bits 27-16. TRM pag. 1859

	.asg 0x54, ADC_TSC_STEPENABLE                   ; Registro para habilitar los steps
        .asg 0x3A, ADC_TSC_STEPENABLE_ENABLE_MASK	; Un canal (0000 0010 -> 0x02) Cuatro canales (0001 1110 -> 0x1E). 0011 1010 -> 0x3A
	.asg 0x00, ADC_TSC_STEPENABLE_DISABLE_MASK      ; Se desahbilitan todos los canales

        .asg 0xE4, ADC_TSC_FIFO_0_COUNT                 ; Registro de conteo de datos en FIFO0
        .asg 0x100, ADC_TSC_FIFO_0_DATA                 ; Registro FIFO0
        .asg 0xFFF, MASK_12_BIT                         ; FIFO0_DATA

        ; end-of-sequence interrupt
        .asg 0x28, ADC_TSC_IRQSTATUS                    ;
        .asg 0x02, ADC_TSC_IRQSTATUS_END_OF_SECUENCE    ; Cuidado aca el bit es el 0x02 pero debe configurarse asi para

        .asg 0x30, ADC_TSC_IRQENABLE_CLR		;
	.asg 0xFFFFFFFF, ADC_TSC_IRQENABLE_CLR_ALL_MASK	; Deshabilitar todas las interrupciones para luego  habilitar solo las que usamos

	.asg 0x2C, ADC_TSC_IRQENABLE_SET                ;
	.asg 0x00, ADC_TSC_IRQENABLE_SET_RESET_MASK	;

        .asg 0x02, ADC_TSC_IRQENABLE_SET_END_OF_SECUENCE; aviso que termino la secuencia de medicion
;	.asg 0x0F, ADC_TSC_IRQENABLE_SET_FIFO_0_THRESHOLD; aviso que todos los datos estan disponibles en FIFO

	.asg r30.t5, pruout_fs_sample_test		 ; Debug-> usamos esta salida para validad fs. config-pin P9_27 pruout

	.asg 0x04, OFFSET_4_BYTES		         ; EL tamanio del dato es 4 bytes. Para 2 bytes 0x02
	.asg 0x4000, N_SAMPLE				 ; Muestras a guardar en cada iteracion 0x1000 -> 4096 | 0x4000 -> 16384
	.asg 0x100, DELAY_S				 ; fs aprox 99799->1KHz. 1000->

        .clink
        .global start

start:
; INICIALIZACION
	LDI32	r5, PRU0_RAM				; no usar este registro para otra cosa cananl 0
        ZERO    &r6, 4                                  ; Para que el primer dato sea en ram0[0]
	LDI32	r7, N_SAMPLE				;
	CLR     r30, pruout_fs_sample_test               ; pru0_pru_r30_5 as an output P9_27

; ADC CONFIG---------------------------------------------------------------------------------------------------------------------------------
; CM_WKUP
; Habilitar el relor del modulo ADC en CM_WKUP
        LDI32   r0, (CM_WKUP | CM_WKUP_ADC_TSC_CLKCTRL) ; Cargamos en r0 la addr del registro CLKCTRL del modulo CM_WKUP
        LDI32   r1, CM_WKUP_ADC_TSC_CLKCTRL_ENABLE    	; Cargamos el valor 0x02 en r1 que habilita en modulo CM_WKUP
        SBBO    &r1, r0, 0, 4                           ; Cargamos a donde apunta r0 el valor contenido en r1
;	LBBO    &r2, r0, 0, 4 				; Debug-> verificamos el valor cargado. Cargamos el valor apuntado por r0 en r2

; ADC_TSC module-> disabe BIT0
        LDI32   r0, (ADC_TSC | ADC_TSC_CTRL)            ; load addr
        LBBO    &r1, r0, 0, 4                           ; read value mem
        CLR     r1, r1, ADC_TSC_CTRL_ENABLE	 	; ojo para realiazar
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849 0b0110

; ADC_TSC module -> Enable StepConfig step ID tag BIT1
        LDI32   r0, (ADC_TSC | ADC_TSC_CTRL)            ; load addr
        LBBO    &r1, r0, 0, 4                           ; read value mem
        CLR     r1, r1, ADC_TSC_CTRL_STEP_ID_TAG        ; enable-> SET | disable-> CLR
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849

; ADC_TSC module -> Enable StepConfig Protect Are not protected (writable) BIT2. Tenemos que habilitar la escritura antes stepconfig
        LDI32   r0, (ADC_TSC | ADC_TSC_CTRL)            ; load addr
        LBBO    &r1, r0, 0, 4                           ; read value mem
        SET     r1, r1, ADC_TSC_CTRL_STEPCONFIG_PROTECT ;
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849

; Step 1 config
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPCONFIG_1)    ; load addr
        LDI32   r1, ADC_TSC_STEPCONFIG_1_MASK           ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro

; Step 1 SampleDelay-OpenDelay
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPDELAY_1)     ; load addr
        LDI32   r1, ADC_TSC_STEPDELAY_1_MASK            ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro

; Step 3 config
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPCONFIG_3)    ; load addr
        LDI32   r1, ADC_TSC_STEPCONFIG_3_MASK           ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro

; Step 3 SampleDelay-OpenDelay
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPDELAY_3)     ; load addr
        LDI32   r1, ADC_TSC_STEPDELAY_3_MASK            ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro

; Step 4 config
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPCONFIG_4)    ; load addr
        LDI32   r1, ADC_TSC_STEPCONFIG_4_MASK           ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro

; Step 4 SampleDelay-OpenDelay
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPDELAY_4)     ; load addr
        LDI32   r1, ADC_TSC_STEPDELAY_4_MASK            ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro

; Step 5 config
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPCONFIG_5)    ; load addr
        LDI32   r1, ADC_TSC_STEPCONFIG_5_MASK           ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro

; Step 5 SampleDelay-OpenDelay
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPDELAY_5)     ; load addr
        LDI32   r1, ADC_TSC_STEPDELAY_5_MASK            ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro

; Configurar el divisor del reloj
        LDI32   r0, (ADC_TSC | ADC_TSC_CLKDIV)          ; load addr for ADS_TSC_CLKDIV
        LDI32   r1, ADC_TSC_CLKDIV_ADC_CLKDIV           ; valor minimo
        SBBO    &r1, r0, 0, 4                           ; Escribir en TSC_ADC_CLKDIV

; Definir el rango de medicion 0-4095
        LDI32   r0, (ADC_TSC | ADC_TSC_ADCRANGE)        ; load addr for ADS_TSC_ADCRANGE
        LDI32   r1, ADC_TSC_ADCRANGE_HIGH_RANGE_DATA    ; 0-4095
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro

;-IRQ-------------------------------------------------------------------------------------------------------------------------------------
; IRQ DESAHBILITAR TODAS LAS INTERRUPCIONES
        LDI32   r0, (ADC_TSC | ADC_TSC_IRQENABLE_CLR)   ; load addr
        LDI32   r1, ADC_TSC_IRQENABLE_CLR_ALL_MASK	;
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1851

;IRQ ENABLE END_OF_SECUENCE
        LDI32   r0, (ADC_TSC | ADC_TSC_IRQENABLE_SET)    ; load addr
        LDI32   r1, ADC_TSC_IRQENABLE_SET_END_OF_SECUENCE; ojo para realiazar un clear hay que hacer un set del bit
        SBBO    &r1, r0, 0, 4                            ; TRM pag. 1849

; IRQ Clear interrupt
;CLEAR_IRQ_END_OF_SECUENCE:
        LDI32   r0, (ADC_TSC | ADC_TSC_IRQSTATUS)       ; load addr
        LDI32   r1, ADC_TSC_IRQENABLE_CLR_ALL_MASK      ; ojo para realiazar un clear hay que hacer un set del bit
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849

; LOOP SAMPLE---------------------------------------------------------------------------------------------------------------------------------

;	SET r30, pruout_fs_sample_test                  ; Debug-> para el tiempo total de adquisicion

;Get sample loop
GET_SAMPLE_LOOP:

	SET r30, pruout_fs_sample_test			; Debug-> para ver cada iteracion

; Habilitar steps ENABLE VER MANUAL TRM pag. 1839 (solo puede escribirse en STEPCONFIG si ADC_TSC esta disable )
       LDI32   r0, (ADC_TSC | ADC_TSC_STEPENABLE)      ; load addr
       LDI32   r1, ADC_TSC_STEPENABLE_ENABLE_MASK      ;
       SBBO    &r1, r0, 0, 4                           ; Escribir en registro. En esta instruccion la FSM del modulo arranca a medir

; ADC_TSC module-> enable
        LDI32   r0, (ADC_TSC | ADC_TSC_CTRL)            ; load addr
        LBBO    &r1, r0, 0, 4                           ; read value mem
        SET     r1, r1, ADC_TSC_CTRL_ENABLE             ; ojo para realiazar
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849 0x00

; Wait for interrupt
WAIT_IRQ_END_OF_SECUENCE:
        LDI32   r0, (ADC_TSC | ADC_TSC_IRQSTATUS)         ; load addr
        LBBO    &r1, r0, 0, 4                             ; read value mem
        AND     r2, r1, ADC_TSC_IRQSTATUS_END_OF_SECUENCE ; Comprobar el estado del bit 1
        QBEQ    WAIT_IRQ_END_OF_SECUENCE, r2, 0           ; hasta que la interruccion no pase a 1 no se avanza a la proxima instruccion
        QBA     CLEAR_IRQ_END_OF_SECUENCE                 ; Detectada la interrupcion porque termino la secuencia seguimos adelante

; Clear interrupt
CLEAR_IRQ_END_OF_SECUENCE:
        LDI32   r0, (ADC_TSC | ADC_TSC_IRQSTATUS)        ; load addr
        LBBO    &r1, r0, 0, 4                            ; read value mem
        SET     r1, r1, ADC_TSC_IRQSTATUS_END_OF_SECUENCE; ojo para realiazar un clear hay que hacer un set del bit
        SBBO    &r1, r0, 0, 4                            ; TRM pag. 1849

; Read count FIFO0
        LDI32   r0, (ADC_TSC | ADC_TSC_FIFO_0_COUNT)    ; load addr for mem
        LBBO    &r1, r0, 0, 4                           ; read r1 <-FIFO_0_COUNT

GET_SAMPLE_LOOP_FOR_START:
        SUB     r1, r1, 1                               ; index--
        LDI32   r0, (ADC_TSC | ADC_TSC_FIFO_0_DATA)     ; load addr for mem
        LBBO    &r4, r0, 0, 4                           ; load r4 <- FIFO_O_DATA
	SBBO    &r4, r5, r6, 4	  			; load r4 -> ram[] memory. La primer pasada guarda en ram[0] y la ultima en ram[3]
	ADD     r6, r6, OFFSET_4_BYTES                  ; sumamos de a 4 bytes. Desde 0 hasta 4096
        QBNE    GET_SAMPLE_LOOP_FOR_START, r1, 0        ;

; Disable ADC_TSC module
        LDI32   r0, (ADC_TSC | ADC_TSC_CTRL)            ; load addr
        LBBO    &r1, r0, 0, 4                           ; read value mem
        CLR     r1, r1, ADC_TSC_CTRL_ENABLE             ; disable module
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849

; Deshabilitar steps
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPENABLE)      ; load addr
        LDI32   r1, ADC_TSC_STEPENABLE_DISABLE_MASK     ; Usamos mascaras cuando queremos configurar muchos bit juntos
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro. En esta instruccion la FSM del modulo arranca a medir

        CLR r30, pruout_fs_sample_test                  ; Debug-> tiempo de cada conversion

	LDI32	r2, DELAY_S				;

DELAY_SAMPLE:
        SUB     r2, r2, 1                               ; index--
        QBNE    DELAY_SAMPLE, r2, 0        		;
;-----------------------------------------------------------------------------------------------------------------------------------------

 	QBNE    GET_SAMPLE_LOOP, r6, r7     		;
        LDI32   r6, 0					;
        QBA     GET_SAMPLE_LOOP
;	CLR r30, pruout_fs_sample_test			; Debug-> tiempode conversion total

	HALT    ; we should never actually get here
