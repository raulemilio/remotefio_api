# 📘 Configuración de Pines – BeagleBone Black

## 🔌 Alimentación

| Señal | Pin   | Descripción                             |
|-------|--------|-----------------------------------------|
| GND   | P9_1   | Tierra común                            |
| 3.3V  | P9_3   | Salida de 3.3V                          |
| 5V    | P9_5   | Entrada 5V externos (por ficha)        |

---

## 📟 LCD

| Señal   | Pin      | Descripción                                   |
|---------|----------|-----------------------------------------------|
| SCL     | P9_19    | Línea I2C (verde)                              |
| SDA     | P9_20    | Línea I2C                                     |
| GND     | P9_1     | Tierra común                                   |
| VCC     | P9_5     | Alimentación 5V                                |
| Bus     | `/dev/i2c-1` | Interfaz I2C                              |
| Config  |          | Driver configurado para 2x16, funciona en 4x20 |

---

## 📈 Canales Analógicos (ADC)

| Canal      | Pin     | Descripción             |
|------------|---------|-------------------------|
| vref_1.8V  | P9_32   | Referencia de 1.8V      |
| AIN0       | P9_39   | Canal analógico 0       |
| AIN2       | P9_37   | Canal analógico 3       |
| AIN3       | P9_38   | Canal analógico 4       |
| AIN4       | P9_33   | Canal analógico 5       |

---

## 📥 GPIO – Entradas

| Señal        | Pin     | Descripción         |
|--------------|---------|---------------------|
| gpio_input0  | P8_35   | Entrada digital     |
| gpio_input1  | P8_33   | Entrada digital     |
| gpio_input2  | P8_31   | Entrada digital     |
| gpio_input3  | P8_32   | Entrada digital     |

---

## 📤 GPIO – Salidas

| Señal     | Pin     | Descripción           |
|-----------|---------|-----------------------|
| output_0  | P8_27   | Salida digital        |
| output_1  | P8_29   | Salida digital        |
| output_2  | P8_28   | Salida digital        |
| output_3  | P8_30   | Salida digital        |

---

## ⚙️ Control de Motores

### Salidas de Control

| Motor | Pin     | Señal           |
|-------|---------|-----------------|
| MA    | P8_45   | output_MA_E     |
| MA    | P8_46   | output_MA_D     |
| MB    | P8_43   | output_MB_E     |
| MB    | P8_44   | output_MB_D     |
| MC    | P8_41   | output_MC_E     |
| MC    | P8_42   | output_MC_D     |
| MD    | P8_39   | output_MD_E     |
| MD    | P8_40   | output_MD_D     |

### Período del Paso (Step)

| Motor | Pin     | Señal           |
|-------|---------|-----------------|
| MA    | P8_37   | output_MA_S     |
| MB    | P8_38   | output_MB_S     |
| MC    | P8_36   | output_MC_S     |
| MD    | P8_34   | output_MD_S     |

---

## ⏱️ Trigger y Detección de Flanco Ascendente

### Entradas Generales

| Señal              | Pin     | Descripción                           |
|--------------------|---------|---------------------------------------|
| adc trigger        | P9_14   | Entrada trigger ADC                   |
| gpio rising detect | P8_07   | Detección de flanco ascendente GPIO  |
| motor rising detect| P8_08   | Detección de flanco ascendente motor |

### Triggers Hardware – Motor A

| Señal      | Pin     | Descripción             |
|------------|---------|-------------------------|
| input_MA_E | P8_19   | Deshabilitar motor A    |
| input_MA_D | P8_13   | Cambiar dirección motor A |

### Triggers Hardware – Motor B

| Señal      | Pin     | Descripción             |
|------------|---------|-------------------------|
| input_MB_E | P8_14   | Deshabilitar motor B    |
| input_MB_D | P8_17   | Cambiar dirección motor B |

---

> 💡 Este documento puede ser actualizado para incluir nuevas asignaciones o expandirse con descripciones funcionales y diagramas de conexión.

