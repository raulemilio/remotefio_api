; Raul Emilio Romero
;       27-01-2025

; PRU setup definitions
;       .asg C4,     CONST_SYSCFG
        .asg C28,    CONST_PRUSHAREDRAM			; AM335x_PRU.cmd

        .asg 0x22000, PRU0_CTRL
        .asg 0x24000, PRU1_CTRL                         ; page 19
;	.asg 0x28, CTPPR0                               ; page 75

        .asg 0x000, PRU0_RAM
        .asg 0x020, PRU1_RAM
        .asg 0x100, SHARED_RAM                          ; This is so prudebug can find it.
        .asg 64, CNT_OFFSET

	.asg 0x00000000, ram0_addr			; Estamos usando ram0 para guardar los datos

        ; Resgistro GPIO
        .asg 0x44e07000, GPIO0                          ; GPIO Bank 0, See the AM335x TRM
        .asg 0x190, GPIO_CLRDATAOUT                     ; for clearing the GPIO registers offset
        .asg 0x194, GPIO_SETDATAOUT                     ; for setting the GPIO registers
        .asg 0x138, GPIO_DATAIN                         ; for reading the GPIO registers
        .asg 1<<30, GPIO0_30                            ; P9_11/P2.05 gpio0[30] Output - bit 30

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
        .asg 0x00, ADC_TSC_STEPCONFIG_1_MASK            ; SW mode, single ended, one shot mode, fifo 0, channel 0
	;.asg 0x20800000, ADC_TSC_STEPCONFIG_1_MASK            ; SW mode, differential pair enable, one shot mode, fifo 0, ch0_positivo - ch1_negativo

        .asg 0x68, ADC_TSC_STEPDELAY_1                  ; TRM pag. 1859
        .asg 0x00, ADC_TSC_STEPDELAY_1_MASK             ; OpenDly0 SampleDly0

        .asg 0x4C, ADC_TSC_CLKDIV                       ; clk
        .asg 0x00, ADC_TSC_CLKDIV_ADC_CLKDIV            ; valor a cargar para la division del clk. Cargando 0x0 el valor minimo es 1

        .asg 0x48, ADC_TSC_ADCRANGE                     ;
        .asg (0xFFF<<16), ADC_TSC_ADCRANGE_HIGH_RANGE_DATA; Cargamos 4095 en los bits 27-16. TRM pag. 1859

        .asg 0x54, ADC_TSC_STEPENABLE                   ; Registro para habilitar los steps
        .asg 0x02, ADC_TSC_STEPENABLE_ENABLE_MASK       ; Se habilita solo el canal 1 (0000 0010) por ahora
        .asg 0x00, ADC_TSC_STEPENABLE_DISABLE_MASK      ; Se desahbilitan todos los canales

        .asg 0xE4, ADC_TSC_FIFO_0_COUNT                 ; Registro de conteo de datos en FIFO0
        .asg 0x100, ADC_TSC_FIFO_0_DATA                 ; Registro FIFO0
        .asg 0xFFF, MASK_12_BIT                         ; FIFO0_DATA

        ; end-of-sequence interrupt
        .asg 0x28, ADC_TSC_IRQSTATUS                    ;
        .asg 0x02, ADC_TSC_IRQSTATUS_END_OF_SECUENCE    ; Cuidado aca el bit es el 0x02 pero debe configurarse asi para

        .asg 0x30, ADC_TSC_IRQENABLE_CLR		;
	.asg 0xFFFFFFFF, ADC_TSC_IRQENABLE_CLR_ALL_MASK	; Desahbilitar todas las interrupciones para luego  habilitar solo las que usamos

	.asg 0x2C, ADC_TSC_IRQENABLE_SET                ;
	.asg 0x00, ADC_TSC_IRQENABLE_SET_RESET_MASK	;

        .asg 0x02, ADC_TSC_IRQENABLE_SET_END_OF_SECUENCE; aviso que termino la secuencia de medicion
;	.asg 0x0F, ADC_TSC_IRQENABLE_SET_FIFO_0_THRESHOLD; aviso que todos los datos estan disponibles en FIFO

	.asg r30.t5, pruout_fs_sample_test		 ; Debug-> usamos esta salida para validad fs. config-pin P9_27 pruout

	.asg "99700", DELAY_S				 ; fs aprox 1KHz para un canal
        .clink
        .global start

start:

; inicializacion de shared mem
        zero    &r0, 4                                  ; Inicializa r0 en 0
	zero    &r1, 4                                  ; Inicializa r1 en 0
	LDI32	r5, ram0_addr				; no usar este registro para otra cosa

;	SBCO    &r0, CONST_PRUSHAREDRAM, CNT_OFFSET, 4  ; Escribe el nuevo valor e
;	SBCO    &r1, CONST_PRUSHAREDRAM, CNT_OFFSET+4, 4  ; Escribe el nuevo valor e

;	LBCO    &r2, CONST_PRUSHAREDRAM, CNT_OFFSET, 4  ; Escribe el nuevo valor e
;	LBCO    &r3, CONST_PRUSHAREDRAM, CNT_OFFSET+4, 4 ; Escribe el nuevo valor e

	CLR     r30, pruout_fs_sample_test               ; pru0_pru_r30_5 as an output P9_27
;-----------------------------------------------------------------------------------------------------------------------------------------

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
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849
;        LBBO    &r2, r0, 0, 4                           ; Debug->0b0110

; ADC_TSC module -> Enable StepConfig step ID tag BIT1
        LDI32   r0, (ADC_TSC | ADC_TSC_CTRL)            ; load addr
        LBBO    &r1, r0, 0, 4                           ; read value mem
        SET     r1, r1, ADC_TSC_CTRL_STEP_ID_TAG        ; ojo
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849
;        LBBO    &r2, r0, 0, 4                           ; Debug->

; ADC_TSC module -> Enable StepConfig Protect Are not protected (writable) BIT2
        LDI32   r0, (ADC_TSC | ADC_TSC_CTRL)            ; load addr
        LBBO    &r1, r0, 0, 4                           ; read value mem
        SET     r1, r1, ADC_TSC_CTRL_STEPCONFIG_PROTECT ;
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849
;        LBBO    &r2, r0, 0, 4                           ; Debug->

; Step config Importante STEPCONFIG_PROTECT debe ser SET para configurar STEP
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPCONFIG_1)    ; load addr
        LDI32   r1, ADC_TSC_STEPCONFIG_1_MASK           ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro
;        LBBO    &r2, r0, 0, 4                           ; Debug

; Step 1 SampleDelay-OpenDelay
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPDELAY_1)     ; load addr
        LDI32   r1, ADC_TSC_STEPDELAY_1_MASK            ;
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro
;        LBBO    &r2, r0, 0, 4				; Debug

; Configurar el divisor del reloj
        LDI32   r0, (ADC_TSC | ADC_TSC_CLKDIV)          ; load addr for ADS_TSC_CLKDIV
        LDI32   r1, ADC_TSC_CLKDIV_ADC_CLKDIV           ; valor minimo
        SBBO    &r1, r0, 0, 4                           ; Escribir en TSC_ADC_CLKDIV
;        LBBO    &r2, r0, 0, 4                           ; Debug->

; Definir el rango de medicion 0-4095
        LDI32   r0, (ADC_TSC | ADC_TSC_ADCRANGE)        ; load addr for ADS_TSC_ADCRANGE
        LDI32   r1, ADC_TSC_ADCRANGE_HIGH_RANGE_DATA    ; 0-4095
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro
;        LBBO    &r2, r0, 0, 4				; Debug

; IRQ DESAHBILITAR TODAS LAS INTERRUPCIONES
        LDI32   r0, (ADC_TSC | ADC_TSC_IRQENABLE_CLR)   ; load addr
        LDI32   r1, ADC_TSC_IRQENABLE_CLR_ALL_MASK	;
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1851
;        LBBO    &r2, r0, 0, 4                           ; Debug-> ojo aca el resultado es 0x00

;IRQ ENABLE END_OF_SECUENCE
        LDI32   r0, (ADC_TSC | ADC_TSC_IRQENABLE_SET)    ; load addr
        LDI32   r1, ADC_TSC_IRQENABLE_SET_END_OF_SECUENCE; ojo para realiazar un clear hay que hacer un set del bit
        SBBO    &r1, r0, 0, 4                            ; TRM pag. 1849
;        LBBO    &r2, r0, 0, 4                            ; Debug->

; IRQ Clear interrupt
;CLEAR_IRQ_END_OF_SECUENCE:
        LDI32   r0, (ADC_TSC | ADC_TSC_IRQSTATUS)       ; load addr
        LDI32   r1, ADC_TSC_IRQENABLE_CLR_ALL_MASK      ; ojo para realiazar un clear hay que hacer un set del bit
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849
;        LBBO    &r2, r0, 0, 4                           ; Debug-> verificamos reset modulo 0x0
;-----------------------------------------------------------------------------------------------------------------------------------------

;Get sample loop
GET_SAMPLE_LOOP:
	SET r30, pruout_fs_sample_test			; Debug->

; Habilitar step1 ENABLE VER MANUAL TRM pag. 1839 (solo puede escribirse en STEPCONFIG si ADC_TSC esta disable )
       LDI32   r0, (ADC_TSC | ADC_TSC_STEPENABLE)      ; load addr
       LDI32   r1, ADC_TSC_STEPENABLE_ENABLE_MASK      ;
       SBBO    &r1, r0, 0, 4                           ; Escribir en registro. En esta instruccion la FSM del modulo arranca a medir
;       LBBO    &r2, r0, 0, 4                           ; Debug->

; ADC_TSC module-> enable
        LDI32   r0, (ADC_TSC | ADC_TSC_CTRL)            ; load addr
        LBBO    &r1, r0, 0, 4                           ; read value mem
        SET     r1, r1, ADC_TSC_CTRL_ENABLE             ; ojo para realiazar
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849
;        LBBO    &r2, r0, 0, 4                           ; Debug-> verificamos reset modulo 0x0

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
;        LBBO    &r2, r0, 0, 4                            ; Debug-> verificamos reset modulo 0x0

; Read count FIFO0
        LDI32   r0, (ADC_TSC | ADC_TSC_FIFO_0_COUNT)    ; load addr for mem
        LBBO    &r1, r0, 0, 4                           ; read r1->FIFO_0_COUNT
        MOV     r2, r1                                  ; copiamos r1(N) en r2(index) Notar que para copiar registro no utilice LDI32
        LDI32   r3, MASK_12_BIT                         ;

GET_SAMPLE_LOOP_FOR_START:
        SUB     r2, r2, 1                               ; index--
        LDI32   r0, (ADC_TSC | ADC_TSC_FIFO_0_DATA)     ; load addr for mem
        LBBO    &r4, r0, 0, 4                           ; read r3->FIFO_O_DATA
        AND     r4,r4,r3                                ; mask para guardar solo 12 bits 0x0000_0fff->1.8V | 0x0000_0800->0.9V
	SBBO    &r4, r5, 0, 4  				; write to ram[0] memory
        QBNE    GET_SAMPLE_LOOP_FOR_START, r2, 0        ;

; Disable ADC_TSC module
        LDI32   r0, (ADC_TSC | ADC_TSC_CTRL)            ; load addr
        LBBO    &r1, r0, 0, 4                           ; read value mem
        CLR     r1, r1, ADC_TSC_CTRL_ENABLE             ; disable module
        SBBO    &r1, r0, 0, 4                           ; TRM pag. 1849
;        LBBO    &r2, r0, 0, 4                           ; Debug->

; Deshabilitar step1
        LDI32   r0, (ADC_TSC | ADC_TSC_STEPENABLE)      ; load addr
        LDI32   r1, ADC_TSC_STEPENABLE_DISABLE_MASK     ; Usamos mascaras cuando queremos configurar muchos bit juntos
        SBBO    &r1, r0, 0, 4                           ; Escribir en registro. En esta instruccion la FSM del modulo arranca a medir
;        LBBO    &r2, r0, 0, 4                           ; Debug->

       CLR r30, pruout_fs_sample_test                   ; Debug-> Dede estar en esta posicion antes del delay

	LDI32	r2, DELAY_S				;
DELAY_SAMPLE:
        SUB     r2, r2, 1                               ; index--
        QBNE    DELAY_SAMPLE, r2, 0        ;
;-----------------------------------------------------------------------------------------------------------------------------------------
        QBA     GET_SAMPLE_LOOP                         ; volvemos a realizar una medicion

        HALT    ; we should never actually get here
