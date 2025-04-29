LCD pin
dev/i2c-1
VCC P9_5 (5 V externos por ficha)
GND P9_1
SCL P9_19 verde
SDA P9_20
El driver está configurado para 2 lineas por 16 caracteres pero
se puede usar perfectamente con 4 lineas y 20 caracteres.

Analog channels
channel 0 P9_39 (AIN0)
channel 3 P9_37 (AIN2)
channel 4 P9_38 (AIN3)
channel 5 P9_33 (AIN4)

gpio_read
P8_35 gpio_input0
P8_33 gpio_input1
P8_31 gpio_input2
P8_32 gpio_input3

gpio_write
P8_27 output_0 
P8_29 output_1 
P8_28 output_2 
P8_30 output_3

MOTOR
P8_45 output_MA_E
P8_46 output_MA_D
P8_43 output_MB_E
P8_44 output_MB_D
P8_41 output_MC_E
P8_42 output_MC_D 
P8_39 output_MD_E 
P8_40 output_MD_D 

Step period (medio ciclo de la senial)
P8_37 output_MA_S
P8_38 output_MB_S
P8_36 output_MC_S
P8_34 output_MD_S

TRIGGER RESING DETECT
P8_07 input gpio_input send rising detect
P8_08 input motor send rising detect
TRIGGER MOTOR_A hardware pins
P8_19 input_MA_E set disable motor
P8_13 input_MA_D set toogle direction
TRIGGER MOTOR_B hardware pins
P8_14 input_MB_E set disable motor
P8_17 input_MB_D set toogle direction

COMMAND

SYSTEM
    BBRemote/cmds/system
	cliente -> {"task":"system","function":"adc","action": "stop"}
	
	function puede tomar los valores: adc, 
									  gpio_input, 
									  gpio_output, 
									  motor,
									  pru0, pru1,
									  lcd, 
									  all_functions
    "lcd" → "clean"
    "pru0", "pru1" → "start" o "stop"
    "adc", "gpio_input", "gpio_output", "motor" → "stop"

	BBRemote/rsp/system
	BBB -> RESPONSE -> {"task":"system", "status": "complete"}

ADC
datos: buffer_size maximo por canal 1024 | minimo 8 
       num_samples debe ser multiplo de buffer 
       recordar que cada vuelta re graban dos buffer 
       Ts * num_samples >= 20 ms
       
MODE 0 get n samples
BBRemote/cmds/adc
cliente -> {"task":"adc","sample_rate":10, "buffer_size":16,"num_samples": 32,"enable_external_trigger":0, "mode": 0}

	BBRemote/rsp/adc
	BBB -> RESPONSE -> s
						{ "task": "adc",
							"channels": {
										"ch0": [dato1, dato2, dato3],
										"ch1": [dato1, dato2, dato3],
										"ch2": [dato1, dato2, dato3],
										"ch3": [dato1, dato2, dato3]
										}
						}
						
MODE 1 get state continus samples
	BBRemote/cmds/adc
	cliente -> {"task":"adc","fs":1000,"samples": 1000, "mode": 0}
	BBRemote/rsp/adc
	BBB -> RESPONSE -> 	{ "task": "adc", "fs":1000, "resolution":4096, "vref": 1.8,
							"channels": {
										"ch0": [dato1, dato2, dato3],
										"ch1": [dato1, dato2, dato3],
										"ch2": [dato1, dato2, dato3],
										"ch3": [dato1, dato2, dato3]
										}
						}

GPIO_INPUT
MODE 0-1-2 get state
	BBRemote/cmds/gpio/gpio_input
	cliente -> {"task":"gpio_input","input":[0,1,2,3], "mode": 0}
	BBRemote/rsp/gpio/gpio_input
	BBB -> RESPONSE -> {"task":"gpio_input","input":[0,1,2,3],"state":[0,1,0,0], "ts": 1723234234443}

GPIO_OUTPUT GET
MODE 0 get state
	BBRemote/cmds/gpio/gpio_output_get
	cliente-> {"task":"gpio_output_get","output":[0,1,2,3], "mode": 0}
	BBRemote/rsp/gpio/gpio_output_get
	BBB -> RESPONSE -> {"task":"gpio_output_get","output":[0,1,2,3], "state": [0,0,0,0], "ts": 1723234234443}
GPIO_OUTPUT SET
MODE 0 set state
	BBRemote/cmds/gpio/gpio_output_set
	cliente-> {"task":"gpio_output_set","output":[0,1,2,3], "state": [0,0,0,0], "mode": 0}
	BBRemote/rsp/gpio_output
	BBB -> RESPONSE -> {"task":"gpio_output_set", "status": "complete"}

MOTOR_GET
MODE 0-1-2 get state
	BBRemote/cmds/motor/motor_get
	cliente -> {"task":"motor_get", "motor":[0,1,2,3], "mode": 0}
	BBRemote/rsp/motor/motor_get
    BBB-> RESPONSE -> RESPONSE -> {"task":"motor_get","motor":[0,1,2,3],"ena":[1,1,0,1],"dir":[0,1,0,1],"rpm": [60, 150, 120, 180],"step_per_rev": [200, 200, 200, 200],"micro_step": [1,1,16,2],"mode":1,"ts": 1723234234443}
    
MOTOR_SET
MODE 0 set state
    BBRemote/cmds/motor/motor_set
NEMA 17 200 vueltas por step_per_rev
A4988 micro_step: 1 (full), 2 (half), 4 (quarter), 8 (eigth), 16 (sixteenth)
cliente ->
{"task": "motor_set","motor": [0, 1, 2, 3],"ena": [1, 1, 1, 1],"dir": [0, 0, 0, 0],"rpm": [60, 150, 120, 180],"step_per_rev": [200, 100, 200, 100],"micro_step": [1,4,16,2],"mode": 0}

BBRemote/rsp/motor/motor_set
BBB-> RESPONSE -> RESPONSE -> {"task":"motor_set", "status": "complete"}

