
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

## NODE RED

1. To start Node-RED, run the following command in the terminal:
   ```bash
   node-red
   ```
2. Access Node-RED from your browser at: `http://127.0.0.1:1880/`
3. Install the necessary packages:
   - node-red-dashboard
   - Led packages

## Debian Image

The recommended Debian image for BeagleBone Black is:
```
am335x-eMMC-flasher-debian-11.7-iot-armhf-2023-09-02-4gb
```
You can also find more installation details in this video: [YouTube Link](https://www.youtube.com/watch?v=i1zY1ylp_3c)

### To see the IP for Ethernet:

```bash
ip addr show eth0
```

### SSH Connection:

```bash
$ ssh debian@192.168.7.2
```

The default password is `temppwd`.

### Copying Folders from BBB to Local:

**Important:** Run the following command from a local terminal (not from the server):
```bash
$ scp -r debian@192.168.7.2:/home/debian/remotefio_api /home/rome/Escritorio/
```

**Do not delete the folder locally** as it is linked to Git.

### Copying Folders from Local to BBB:

```bash
$ scp -r /home/rome/Escritorio/BBRremote debian@192.168.7.2:/home/debian/
```

### To remove SSH key for reconnection:

```bash
$ ssh-keygen -f "/home/rome/.ssh/known_hosts" -R "192.168.7.2"
```

## PRU (Cap15.pagina 667 libro BB)

### The PRU Code Generation Tools:

Download the PRU development framework from:
[PRU-CGT Tool](https://www.ti.com/tool/download/PRU-CGT)

Note: No installation needed as it is already included in the Debian image with version 2.3.3.

Check the installed version with:
```bash
$ clpru --compiler_revision
```

To find the location of `clpru`:
```bash
$ whereis clpru
```

## librobotcontrol (Robotic Control Library)

### To Reproduce:

```bash
git clone https://github.com/beagleboard/librobotcontrol
cd librobotcontrol
make
sudo make install
rc_test_drivers
```

Change the version in the PRU firmware folder's `Makefile` to v6.0:
```bash
LIBS=--library=/usr/lib/ti/pru-software-support-package-v6.0/lib/rpmsg_lib.lib
INCLUDE=--include_path=/usr/lib/ti/pru-software-support-package-v6.0/include --include_path=/usr/lib/ti/pru-software-support-package-v6.0/include/am335x --include_path=$(PRU_CGT)/include
```

Follow the project construction methodology as per the template: [Template Link](https://old.beagleboard.org/static/librobotcontrol/project_template.html)

## Compiling and Running ONLY PRU0

1. Open SSH terminals:

```bash
$ ssh debian@192.168.7.2
```

2. Configure GPIO pins:
   ```bash
   config-pin P8_16 pruin
   ```
   For more configuration options:
   ```bash
   config-pin -l P9_11
   ```
   To see the current configuration:
   ```bash
   config-pin -q P9_11
   ```

3. To export the pin and configure it:
```bash
echo "30" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio30/direction
echo "1" > /sys/class/gpio/gpio30/value  # Set high
echo "0" > /sys/class/gpio/gpio30/value  # Set low
```

4. Compile and load PRU firmware.

5. Check PRU state:
```bash
$ echo 'start' > /sys/class/remoteproc/remoteproc1/state
$ echo 'stop' > /sys/class/remoteproc/remoteproc1/state
```

### View PRU Registers:
```bash
$ sudo cat /sys/kernel/debug/remoteproc/remoteproc1/regs
```

## Remoteproc

- To view Linux memory space for the PRU:
```bash
$ ls -l /sys/bus/platform/drivers/pru-rproc
```

- Compiling and debugging steps:

1. Save the firmware `.asm` file.
2. In the first terminal:
   ```bash
   $ sudo make
   ```
3. Install the firmware:
   ```bash
   $ sudo make install
   ```
4. In the second terminal, run the main file:
   ```bash
   $ sudo ./main
   ```

### Important Troubleshooting:
- When there are issues during compilation, ensure the BeagleBone is connected to the internet.
- If encountering the error `am335x-pru0-rc-encoder-fw failed to load`, initialize the shared memory:
```assembly
zero &r0, 4
SBCO &r0, CONST_PRUSHAREDRAM, CNT_OFFSET, 4
LBCO &r0, CONST_PRUSHAREDRAM, CNT_OFFSET, 4
```

## MQTT

### Install Mosquitto (Optional for testing connection):

```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
```

To activate the service:
```bash
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

### Test MQTT locally:

In one terminal:
```bash
$ mosquitto_sub -h localhost -t "test/topic"
```

In another terminal:
```bash
$ mosquitto_pub -h localhost -t "test/topic" -m "¡Hola desde BeagleBone!"
```

### Test MQTT between BeagleBone and Server:

1. Ping the server:
```bash
$ ping 192.168.0.171
```

2. On the notebook, open MQTT Explorer and connect to `tcp://192.168.0.171:1883`.
3. Subscribe to `test/topic` and send a message.

### Test MQTT Project:

Run the test MQTT project:
```bash
$ ./test_mqtt
```

## ADC Circuit

Based on:
- Book: "Analog Engineer's Circuit" (2023) Pg. 163
- Book: "Analog Engineer's Circuit" (2024) Pg. 160
- BeagleBook-Tool Pg. 443 (445 has protection for ADC inputs)
- Pg. 290 GIO

