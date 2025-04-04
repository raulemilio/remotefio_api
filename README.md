# BBRemoteIO - BeagleBone Black Remote I/O System
BBRemoteIO es un sistema de adquisición y control remoto basado en BeagleBone Black, diseñado para gestionar entradas y salidas de hardware a través de mensajes MQTT. Este módulo electrónico permite la integración con sensores, actuadores y otros dispositivos mediante la lectura de ADC, control de GPIOs y manejo de motores. Además, soporta la ejecución en paralelo de múltiples tareas y la integración con la PRU para adquisición de datos de alta velocidad. Ideal para aplicaciones en automatización, monitoreo remoto y laboratorios de instrumentación.

## Overview
BBRemoteIO is a remote I/O system for the BeagleBone Black, designed to execute tasks based on MQTT messages. The system can handle multiple tasks in parallel, including ADC data acquisition, GPIO input/output control, motor control, and interaction with an LCD display.

## Features
- **ADC Sampling**: Supports continuous sampling and triggered sampling.
- **GPIO Input/Output**: Reads input states and sets output states.
- **Motor Control**: Enables setting and querying motor states.
- **PRU Integration**: Uses the PRU for high-speed ADC data acquisition.
- **LCD Display**: Future module for logging and status display.
- **Parallel Execution**: Designed to handle multiple MQTT tasks without blocking.

## MQTT Command Structure
### SYSTEM
**Topic:** `BBRemote/cmds/system`
#### Client Command:
```json
{"task":"system","function":"all_functions","action": "stop"}
```
- `function` values:
  - `adc`
  - `gpio_input_m0`, `gpio_input_m1`, `gpio_input_m2`
  - `gpio_output_m0`, `gpio_output_m1`
  - `motor_m0`, `motor_m1`, `motor_m2`, `motor_m3`
  - `pru0`, `pru1`
  - `lcd`
  - `functions`
- Actions:
  - `lcd`: `clean`
  - `pru0`, `pru1`: `start` or `stop`
  - `adc`, `gpio_input`, `gpio_output`, `motor`: `stop`

**Response Topic:** `BBRemote/rsp/system`
#### BBB Response:
```json
{"task":"system", "status": "complete"}
```

### ADC
#### MODE 0: Get N Samples
**Command Topic:** `BBRemote/cmds/adc`
```json
{"task":"adc","fs":1000,"num_samples": 10000, "buffer_size":4096, "mode": 0}
```
**Response Topic:** `BBRemote/rsp/adc`
```json
{"task": "adc","channels": {"ch0": [dato1, dato2, dato3],"ch1": [dato1, dato2, dato3],"ch2": [dato1, dato2, dato3],"ch3": [dato1, dato2, dato3]}}
```

#### MODE 1: Continuous Sampling
**Command Topic:** `BBRemote/cmds/adc`
```json
{"task":"adc","fs":1000,"samples": 1000, "mode": 0}
```
**Response Topic:** `BBRemote/rsp/adc`
```json
{"task": "adc","channels": {"ch0": [dato1, dato2, dato3],"ch1": [dato1, dato2, dato3],"ch2": [dato1, dato2, dato3],"ch3": [dato1, dato2, dato3]}}
```

### GPIO INPUT
#### MODE 0: Get State
**Command Topic:** `BBRemote/cmds/gpio_input`
```json
{"task":"gpio_input","input":[0,1,2,3], "mode": 0}
```
**Response Topic:** `BBRemote/rsp/gpio_input`
```json
{"task":"gpio_input","input":[0,1,2,3],"state":[0,1,0,0]}
```

#### MODE 1: Trigger on Input Pin
**Command Topic:** `BBRemote/cmds/gpio_input`
```json
{"task":"gpio_input","input":[0,1,2,3], "mode": 1}
```
**Response Topic:** `BBRemote/rsp/gpio_input`
```json
{"task":"gpio_input","input":[0,1,2,3],"state":[0,1,0,0]}
```

#### MODE 2: Trigger on Input Channels
**Command Topic:** `BBRemote/cmds/gpio_input`
```json
{"task":"gpio_input","input":[0,1,2,3], "mode": 2}
```
**Response Topic:** `BBRemote/rsp/gpio_input`
```json
{"task":"gpio_input","input":[0,1,2,3],"state":[0,1,0,0]}
```

### GPIO OUTPUT
#### MODE 0: Get State
**Command Topic:** `BBRemote/cmds/gpio_output`
```json
{"task":"gpio_output","output":[0,1,2,3], "mode": 0}
```
**Response Topic:** `BBRemote/rsp/gpio_output`
```json
{"task":"gpio_output","output":[0,1,2,3], "state": [0,0,0,0]}
```

#### MODE 1: Set State
**Command Topic:** `BBRemote/cmds/gpio_output`
```json
{"task":"gpio_output","output":[0,1,2,3], "state": [0,0,0,0], "mode": 1}
```
**Response Topic:** `BBRemote/rsp/gpio_output`
```json
{"task":"gpio_output", "status": "complete"}
```

### MOTOR
#### MODE 0: Get State
**Command Topic:** `BBRemote/cmds/motor`
```json
{"task":"motor", "motor":[0,1,2,3], "mode": 0}
```
**Response Topic:** `BBRemote/rsp/motor`
```json
{"task":"motor", "motor":[0,1,2,3],"ena":[1,1,0,1],"dir":[0,1,0,1],"step_time":[1000,1500,1200,1800]}
```

#### MODE 1: Trigger on Input Pin
**Command Topic:** `BBRemote/cmds/motor`
```json
{"task":"motor", "motor":[0,1,2,3], "mode": 1}
```
**Response Topic:** `BBRemote/rsp/motor`
```json
{"task":"motor", "motor":[0,1,2,3],"ena":[1,1,0,1],"dir":[0,1,0,1],"step_time":[1000,1500,1200,1800], "mode":1}
```

#### MODE 2: Trigger on ena, dir, MA, MB
**Command Topic:** `BBRemote/cmds/motor`
```json
{"task":"motor", "motor":[0,1,2,3], "mode": 2}
```
**Response Topic:** `BBRemote/rsp/motor`
```json
{"task":"motor", "motor":[0,1,2,3],"ena":[1,1,0,1],"dir":[0,1,0,1],"step_time":[1000,1500,1200,1800], "mode":1}
```

#### MODE 3: Set State
**Command Topic:** `BBRemote/cmds/motor`
```json
{"task":"motor", "motor":[0,1,2,3],"ena":[1,1,1,1],"dir":[0,0,0,0],"step_time":[200000,1500,1200,1800], "mode": 3}
```
**Response Topic:** `BBRemote/rsp/motor`
```json
{"task":"motor", "status": "complete"}




# Imagen debian
	am335x-eMMC-flasher-debian-11.7-iot-armhf-2023-09-02-4gb
	https://www.youtube.com/watch?v=i1zY1ylp_3c

# para ver ip para ethernet ip addr show eth0 

# Conexión por ssh
  ->$ ssh debian@192.168.7.2
  ->$ luego pass: temppwd
  
- Copiar carpetas desde BBB a local 
    !!!!Importante: copiar el comando siguiente desde un terminal local (no desde el server)!!!!!
		$ scp -r debian@192.168.7.2:/home/debian/remotefio_api /home/rome/Escritorio/
  - No borrar la carpeta  del local ya que está vinculada al git

-  Copiar carpeta desde local a BBB (terminal local)
	scp -r /home/rome/Escritorio/BBRremote debian@192.168.7.2:/home/debian/
	
- Escritorio/voltage_data.txt
	
  # borrar clave ssh para reconcexi+ón
  ssh-keygen -f "/home/rome/.ssh/known_hosts" -R "192.168.7.2"
  
# PRU (Cap15.pagina 667 libro BB)
## The PRU Code Generation Tools
En este enlace esta el framework de desarrollo https://www.ti.com/tool/download/PRU-CGT 
pero no hace falta instalarlo porque ya viene instalado en la imagen debian en su última versión
$ clpru --compiler_revision
-- versión 2.3.3

Para ver donde esta instalado 
$ whereis clpru
clpru: /usr/bin/clpru

# librobotcontrol (Robotic control librery)
To Reproduce
Steps to reproduce the behavior:
git clone https://github.com/beagleboard/librobotcontrol
cd librobotcontrol
make
sudo make install
rc_test_drivers
1.0.5

En las carpertas del pru_firmware cambiar en el Makefile agregar la versión v6.0

LIBS=--library=/usr/lib/ti/pru-software-support-package-v6.0/lib/rpmsg_lib.lib
INCLUDE=--include_path=/usr/lib/ti/pru-software-support-package-v6.0/include \
--include_path=/usr/lib/ti/pru-software-support-package-v6.0/include/am335x \
--include_path=$(PRU_CGT)/include

## Basamos la construcción de los proyectos en esta libreria
seguimos la metodologia propueta para los template (https://old.beagleboard.org/static/librobotcontrol/project_template.html)
-----------------------------------------------------------------------------------------------------------------------------
# Compilación y ejecución SOLO PRU0

## Abrir terminales por ssh
		$ ssh debian@192.168.7.2
## configurar los pines gpio 
		config-pin P8_16 pruin (ejemplo ¿, para mas ver libro)
		Para ver configuracion posible
		config-pin -l P9_11
		Para ver la configuracion actual
		config-pin -q P9_11
		GPIO OUT:
		config-pin P9_11 gpio
		GPIO IN pull dowmn
		config-pin P9_11 gpio_pd
		
		/user/bin/
		$ sudo ./config-pru-pins.sh 
		
	# Exportar el pin GPIO P9_11 (GPIO0_30)
	echo "30" > /sys/class/gpio/export

	# Configurar el pin como salida
	echo "out" > /sys/class/gpio/gpio30/direction

	# Poner el pin en alto (set)
	echo "1" > /sys/class/gpio/gpio30/value

	# Poner el pin en bajo (clr)
	echo "0" > /sys/class/gpio/gpio30/value

	# Liberar el pin (opcional)
	echo "30" > /sys/class/gpio/unexport

		
## compliaciones y ejecutar
- Primer terminal (nano) abrir el archivo .asm del proyecto con nano (editar)

- Segundo terminal(make) abrir /remotefio_api/proyectoX/pru_firmware
		aca ejecutamos y cargamos el firmware del PRU 
			$ sudo make 
			$ sudo make install
- Quinto terminal (state) Ojo aca remoteproc1 es el PRU0 . Vamos a /sys/class/remoteproc/remoteproc1
			$ echo 'am335x-pru0-rc-encoder-fw' > firmware (nombre del firmware)	
			$ echo 'am335x-pru1-rc-servo-fw' > firmware
			MPORTANTE: primero hay que cargar el firmware sino queda cargado el anterior
			$ cat state
					offline (el firmware no esta ejecutandose sobre el PRU. Para cargarlo hay que ejecutar el comando start)
			$ echo 'start' > state (ojo aca mirar bien el comando sino puede no arrancar el PRU y pensar que hay problemas donde no los hay)
			$ echo 'stop' > state
			IMPORTANTE: si el estado es offline ejecutar el comando stop da error dado que no tiene sentido
- Cuarto terminal (debag). vamos a /sys/kernel/debug/remoteproc/remoteproc1
			$ sudo cat regs
			IMPORTANTE:  Si el estado del PRU es running no puede accederse a los registros
-------------------------------------------------------------------------------------------
Entrar a la carpeta principal del proyecto
-- volver a la carpeta principal 
	-- make
	-- make clean
	-- sudo make install
	-- sudo ./execut_proyect
--------------------------------------------------------------------------------------------------------------------
# nano
'' signo de pregunta ? al lado la tecla 0
ver numero de lineas alt+N
para matar procesos:
ps (busca procesos)
kill (número de proceso lo mata)
tabulador de x espacios (en este caso 2)
$ nano -T2 
alt+u (deshacer) la tecla alt en nano aparece como M
Ctrl+e: (Rehace)
mayús+cursor (seleccionar)
Ctrl+k (borrar seleccion)
copiar alt+6

# Remoteproc 
- Ver espacio de memoria linux space
	$ /sys/bus/platform/drivers/pru-rproc# ls -l
		lrwxrwxrwx 1 root root 0 Sep 6 05:54 4a334000.pru ...
		lrwxrwxrwx 1 root root 0 Sep 6 05:54 4a338000.pru ...
- Para compilar y debagguer (no hace falta hacerlo asi, si se hace bien el proceso como (*))
		Guardar en archivo .asm
		(primer terminal) ~/remotefio_api/share_mem/pru_firmware$ sudo make
		(primer terminal)~/remotefio_api/share_mem/pru_firmware$ sudo make install 
		(segundo terminal)(linux space) ~/remotefio_api/share_mem$ sudo ./main (no importa si falla)
		(tercer terminal)/sys/kernel/debug/remoteproc/remoteproc1$ sudo cat regs
		
		Lo importenes que luego de compilar y grabar en el PRU hay que ejecutar
		el main. Luego de eso ya se pueden ver los registros con cat regs 
- ver recursos 
	$ /sys/kernel/debug/remoteproc/remoteproc1# ls -l
- ver pines 
	$ /sys/devices/platform/ocp

# make
- Error al compilar que tiene que ver con el ajuste en los tiempos y que se produce por no tener la beagleboard black conectada a internet (actualemte por ethernet)
Problema:
		debian@BeagleBone:~/remotefio_api/share_mem/pru_firmware$ make       
		make: Warning: File 'fw/am335x-pru0-rc-encoder-fw' has modification time 8936 s in the future
		CC	src/main_pru0.c
		CC	src/pru0-encoder.asm
		CC	src/main_pru1.c
		CC	src/pru1-servo.asm
		Generated: fw/am335x-pru0-rc-encoder-fw fw/am335x-pru1-rc-servo-fw
		make: warning:  Clock skew detected.  Your build may be incomplete.
Solución:
	Conectar a internet y luego->
		debian@BeagleBone:~/remotefio_api/share_mem/pru_firmware$ make clean 
		PRU Firmware Cleanup Complete
		debian@BeagleBone:~/remotefio_api/share_mem/pru_firmware$ make
		CC	src/main_pru0.c
		CC	src/pru0-encoder.asm
		LD	build/main_pru0.object build/pru0-encoder.object
		CC	src/main_pru1.c
		CC	src/pru1-servo.asm
		LD	build/main_pru1.object build/pru1-servo.object
		Generated: fw/am335x-pru0-rc-encoder-fw fw/am335x-pru1-rc-servo-fw
		debian@BeagleBone:~/remotefio_api/share_mem/pru_firmware$ sudo make install 
		[sudo] password for debian: 
		PRU Firmware Install Complete
		
# sudo ./main
Problema: 
	debian@BeagleBone:~/remotefio_api/share_mem$ sudo ./main 
	ERROR in rc_encoder_pru_init, am335x-pru0-rc-encoder-fw failed to load
	attempting to stop PRU0
	ERROR: failed to run rc_encoder_pru_init
Solución:
Inicializar la mem shared 
; Inicializacion de shared mem
        zero    &r0, 4                                    ; Clear r0
        SBCO    &r0, CONST_PRUSHAREDRAM, CNT_OFFSET, 4    ; write 0 to shared memory[0] es importante para que salte error
        LBCO    &r0, CONST_PRUSHAREDRAM, CNT_OFFSET, 4    ; lee el valor de shared[0] y lo carga en r0

# Asembly
- COPIA DESDE MEMORIA A REGISTRO
		Cargamos una dirección de memoria en r5 y copiamos en r6 el contenido al que apunta esa dirección (r5)
		LDI32 r5, (GPIO0|GPIO_DATAIN); load read addr for DATAOUT (GPIO0-> dirección base, GPIO_DATAIN-> registro particular de GPIO0)
		LBBO  &r6, r5, 0, 4 ; Load the value at r5 into r6. Con la dirección almacenada en r5 copiamos el contenido al que apunta en r6
- COPIA DESDE REGISTRO A MEMORIA CON EL USO DE CONST_TABLE
		Cargamos en un registro en valor (0xFF) y ese valor se almacena en memoria shared. 
		CONST_PRUSHAREDRAM-> direccion base. CNT_OFFSET-> offset 
		LDI32 r6,  0xFF                                      ; escribimos en r6 0xFF. LDI32 carga inmediata
		SBCO  &r6, CONST_PRUSHAREDRAM, CNT_OFFSET+12, 4      ; write 0 to shared[3]enable_PRU=0 Notar en este caso se usa la const-table 
															 ; si no fuera asi la instruccion seria SBBO
- COPIA DESDE REGISTRO A MEMORIA SIN USO DE CONST_TABLE
		LDI32 r1, (GPIO0|GPIO_SETDATAOUT)   ; load addr for GPIO Set data r1
		LDI32 r2, GPIO0_30					; write GPIO0_30 to r2
		SBBO  &r2, r1, 0, 4					; write r2 to the r1 address value - LED ON
- COPIA ENTRE REGISTROS
		MOV r0, r2			; copia r2 en r0														 
- SET-CLEAR
		SET r30, r30.t5 ; set the output P9_27/P2.34 high
		CLR r30, r30.t5 ; set the output P9_27/P2.34 low

# MQTT
- Intalar mosquitto (en realidad no hace falta instalarlo pero es util para hacer las primeras pruebas de conexion con el server)
		sudo apt update
		sudo apt upgrade -y
		sudo apt install -y mosquitto mosquitto-clients
		Este si es necesario ya que lo usamos como libreria (la libreria PaloMQTT no funciona como esperamos)
		sudo apt-get update
		sudo apt-get install libmosquitto-dev mosquitto mosquitto-clients
- Activar servicio
		sudo systemctl status mosquitto
		Si no esta activo ->
		sudo systemctl enable mosquitto
		sudo systemctl start mosquitto
- Prueba local
		En un terminal $ mosquitto_sub -h localhost -t "test/topic"
		En otro terminal $ mosquitto_pub -h localhost -t "test/topic" -m "¡Hola desde BeagleBone!"

- Prueba beagleboar y servidor notebook HP (tcp://192.168.0.171:1883)
		HTTP Desde beagleboard hacer
		$ ping 192.168.0.171
		MQTT
		En notebook abrir MQTT Explorer coneción local 1883
		En beagleboard $ mosquitto_sub -h 192.168.0.171 -t "test/topic"
		Desde MQTT Explorer enviar mensaje al topic "test/topic"
		Si la conexión no se establece, en la notebook ir a:
		sudo nano /etc/mosquitto/mosquitto.conf
		agregar las lineas:
				listener 1883
				allow_anonymous true

- Instalar libreria Paho (MQTT para c programming)
		$sudo apt update
		$sudo apt install -y libpaho-mqtt-dev

- Probar el proyecto test_mqtt 
		En beagleboard $ ./test_mqtt
		En notebook abrir MQTT Explorer localhost
		
# circuito analógico del adc

- Basado en: 
			- Book Analog Engineer's Circuit (2023) Pag. 163
			- book Analog Engineer's Circuit (2024) Pag. 160
			- beaglebook-tool Pag.443 (445 esta la protección para las entradas ADC)
			-  Pag. 290 GIO
			
			
# procedimiento para armar un repo a partir del template

- Crear una carpeta de proyecto
- Descargar el template .zip de githup TSSE_romero_TP2  (luego hacer uno genérico)
- Descomprimir la carpeta en el directorio del nuevo proyecto y borrar el .zip 
- mover los arcuivos descargados al directorio raiz del repo nuevo
- crear un repo remoto en GITHUP
- git int
- git add --all
- git commit -m "primer commint"
- git branch -M main
- git remote add origin git@github.com:NUEVO_REPO.git

# Para trabajar en el proyecto

- Se puede utilizar VSCODE es práctico
- para generar la documentación: $ make doc
- Formato de codigo antes de mandar al repo: $ pre-commit run --all-files

# cargar al remoto por consola
- git add --all
- git commit -m "commit"
- git push origin main


























