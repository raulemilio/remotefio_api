COMMAND

SYSTEM
    BBRemote/cmds/system
	cliente -> {"task":"system","function":"gpio_input","action": "stop"}
	
	function puede tomar los valores: adc, gpio_input, gpio_output, motor, pru0, pru1, lcd
    "lcd" → "clean"
    "pru0", "pru1" → "start" o "stop"
    "adc", "gpio_input", "gpio_output", "motor" → "stop"

	BBRemote/rsp/system
	BBB -> RESPONSE -> {"task":"system", "status": "complete"}

ADC

GPIO_INPUT
MODE 0 get state
	BBRemote/cmds/gpio_input
	cliente -> {"task":"gpio_input","input":[0,1,2,3], "mode": 0}
	BBRemote/rsp/gpio_input
	BBB -> RESPONSE -> {"task":"gpio_input","input":[0,1,2,3],"state":[0,1,0,0]}
MODE 1 get state flag response
	BBRemote/cmds/gpio_input
	cliente -> {"task":"gpio_input","input":[0,1,2,3], "mode": 1}
	BBRemote/rsp/gpio_input
	BBB -> RESPONSE -> {"task":"gpio_input","input":[0,1,2,3],"state":[0,1,0,0]}

GPIO_OUTPUT
MODE 0 get state
	BBRemote/cmds/gpio_output
	cliente-> {"task":"gpio_output","output":[0,1,2,3], "mode": 0}
	BBRemote/rsp/gpio_output
	BBB -> RESPONSE -> {"task":"gpio_output","output":[0,1,2,3], "state": [0,0,0,0]}
MODE 1 set state
	BBRemote/cmds/gpio_2output
	cliente-> {"task":"gpio_output","output":[0,1,2,3], "state": [0,0,0,0], "mode": 1}
	BBRemote/rsp/gpio_output
	BBB -> RESPONSE -> {"task":"gpio_output", "status": "complete"}

MOTOR
MODE 0 get state
	BBRemote/cmds/motor
	cliente -> {"task":"motor", "motor":[0,1,2,3], "mode": 0}
	BBRemote/rsp/motor
    BBB-> RESPONSE -> {"task":"motor", "motor":[0,1,2,3],"ena":[1,1,0,1],"dir":[0,1,0,1],"step_time":[1000,1500,1200,1800]}
MODE 1 get state flag response
    BBRemote/cmds/motor
	cliente -> {"task":"motor", "motor":[0,1,2,3], "mode": 1}
	BBRemote/rsp/motor
    BBB-> RESPONSE -> RESPONSE -> {"task":"motor", "motor":[0,1,2,3],"ena":[1,1,0,1],"dir":[0,1,0,1],"step_time":[1000,1500,1200,1800], "mode": }
MODE 2 set state
    BBRemote/cmds/motor
	cliente -> {"task":"motor", "motor":[0,1,2,3],"ena":[1,1,0,1],"dir":[0,1,0,1],"step_time":[1000,1500,1200,1800], "mode": 2}
	BBRemote/rsp/motor
    BBB-> RESPONSE -> RESPONSE -> {"task":"motor", "status": "complete"}
