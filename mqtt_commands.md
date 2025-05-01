# 📡 Interfaz MQTT – Comandos y Formato JSON

Este documento describe los comandos MQTT utilizados por el sistema BeagleBone Black, incluyendo tópicos, payloads en JSON, modos de operación y respuestas esperadas.

---

## 🧠 SYSTEM

**Tópico de comando:**  
`BBRemote/cmds/system`

**Formato del mensaje:**
```json
{ "task": "system", "function": "adc", "action": "stop" }
```

**Valores posibles de `function`:**
- `"adc"`
- `"gpio_input"`
- `"gpio_output"`
- `"motor"`
- `"pru0"`, `"pru1"` → `"start"` o `"stop"`
- `"lcd"` → `"clean"`
- `"all_functions"`

**Tópico de respuesta:**  
`BBRemote/rsp/system`

**Respuesta:**
```json
{ "task": "system", "status": "complete" }
```

---

## 🔍 ADC (Conversión Analógica-Digital)

**Restricciones:**
- `buffer_size` por canal: **mínimo 8**, **máximo 1024**
- `num_samples` debe ser múltiplo de `buffer_size`
- `Ts * num_samples >= 20 ms`

### Modo 0 – Obtener N muestras

**Tópico de comando:**  
`BBRemote/cmds/adc`

```json
{ "task": "adc", "sample_rate": 10, "buffer_size": 16, "num_samples": 32, "enable_external_trigger": 0, "mode": 0 }
```

**Tópico de respuesta:**  
`BBRemote/rsp/adc`

```json
{
  "task": "adc",
  "channels": {
    "ch0": [dato1, dato2, dato3],
    "ch1": [dato1, dato2, dato3],
    "ch2": [dato1, dato2, dato3],
    "ch3": [dato1, dato2, dato3]
  }
}
```

### Modo 1 – Adquisición continua

```json
{ "task": "adc", "fs": 1000, "samples": 1000, "mode": 1 }
```

**Respuesta:**
```json
{
  "task": "adc",
  "fs": 1000,
  "resolution": 4096,
  "vref": 1.8,
  "channels": {
    "ch0": [...],
    "ch1": [...],
    "ch2": [...],
    "ch3": [...]
  }
}
```

---

## 🔌 GPIO INPUT

**Tópico:**  
`BBRemote/cmds/gpio/gpio_input`

**Modo:**
- `0`: On demand
- `1`: Trigger GPIO
- `2`: Trigger GPIO inputs

```json
{ "task": "gpio_input", "input": [0,1,2,3], "mode": 0 }
```

**Respuesta:**  
`BBRemote/rsp/gpio/gpio_input`

```json
{ "task": "gpio_input", "input": [0,1,2,3], "state": [0,1,0,0], "ts": 1723234234443 }
```

---

## 🔧 GPIO OUTPUT

### GET – Estado actual

**Tópico:**  
`BBRemote/cmds/gpio/gpio_output_get`

```json
{ "task": "gpio_output_get", "output": [0,1,2,3], "mode": 0 }
```

**Respuesta:**  
`BBRemote/rsp/gpio/gpio_output_get`

```json
{ "task": "gpio_output_get", "output": [0,1,2,3], "state": [0,0,0,0], "ts": 1723234234443 }
```

### SET – Cambiar estado

**Tópico:**  
`BBRemote/cmds/gpio/gpio_output_set`

```json
{ "task": "gpio_output_set", "output": [0,1,2,3], "state": [0,0,0,0], "mode": 0 }
```

**Respuesta:**  
`BBRemote/rsp/gpio_output`

```json
{ "task": "gpio_output_set", "status": "complete" }
```

---

## ⚙️ MOTOR CONTROL

### GET – Obtener estado

**Modos:**
- `0`: On demand
- `1`: Trigger GPIO motor
- `2`: Trigger motor inputs

**Tópico:**  
`BBRemote/cmds/motor/motor_get`

```json
{ "task": "motor_get", "motor": [0,1,2,3], "mode": 0 }
```

**Respuesta:**  
`BBRemote/rsp/motor/motor_get`

```json
{
  "task": "motor_get",
  "motor": [0,1,2,3],
  "ena": [1,1,0,1],
  "dir": [0,1,0,1],
  "rpm": [60,150,120,180],
  "step_per_rev": [200,200,200,200],
  "micro_step": [1,1,16,2],
  "mode": 1,
  "ts": 1723234234443
}
```

### SET – Configurar estado

**Tópico:**  
`BBRemote/cmds/motor/motor_set`

**Nota:**
- NEMA 17: 200 pasos/rev
- A4988 `micro_step`: 1 (full), 2 (half), 4 (quarter), 8 (eighth), 16 (sixteenth)

```json
{
  "task": "motor_set",
  "motor": [0,1,2,3],
  "ena": [1,1,1,1],
  "dir": [0,0,0,0],
  "rpm": [60,150,120,180],
  "step_per_rev": [200,100,200,100],
  "micro_step": [1,4,16,2],
  "mode": 0
}
```

**Respuesta:**  
`BBRemote/rsp/motor/motor_set`

```json
{ "task": "motor_set", "status": "complete" }
```

---

> ⚠️ Este protocolo puede actualizarse con nuevas funcionalidades. Asegúrate de mantener sincronizada esta documentación con el firmware.
