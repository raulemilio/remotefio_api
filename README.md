
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

## PRU (Cap15.pagina 667 libro BB)

### The PRU Code Generation Tools:

Download the PRU development framework from:
[PRU-CGT Tool](https://www.ti.com/tool/download/PRU-CGT)

Follow the project construction methodology as per the template: [Template Link](https://old.beagleboard.org/static/librobotcontrol/project_template.html)

## Compiling and Running ONLY PRU0

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

