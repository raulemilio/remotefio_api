# 🧰 Configuración de Pines – BeagleBone Black

Este documento detalla la asignación de pines para los distintos componentes del sistema.

---

## ⚡ Alimentación
| Descripción   | Pin     |
|---------------|---------|
| GND           | P9_1    |
| 3.3V          | P9_3    |
| 5V externos   | P9_5    |

---

## 🖥️ Pantalla LCD (I2C)
| Señal | Pin    |
|-------|--------|
| SDA   | P9_20 |
| SCL   | P9_19 |
| VCC   | P9_6  |
| GND   | P9_2  |
| Bus   | `/dev/i2c-1` |

**Nota:** Driver configurado para 2x16, compatible con 4x20.

---

## 🔍 Canales Analógicos
| Canal       | Pin   | Descripción                   |
|-------------|--------|------------------------------|
| VREF (1.8V) | P9_32 |                               |
| ADC_GND     | P9_34 | Se conecta a P8_1 (Comun GND) |
| AIN0        | P9_39 |                               |
| AIN2        | P9_37 |                               |
| AIN3        | P9_38 |                               |
| AIN4        | P9_33 |                               |

---

## 📥 GPIO Input
| Señal       | Pin    |
|-------------|--------|
| gpio_input0 | P8_27  |
| gpio_input1 | P8_29  |
| gpio_input2 | P8_28  |
| gpio_input3 | P8_30  |

---

## 📤 GPIO Output
| Señal      | Pin    |
|------------|--------|
| output_0   | P8_12  |
| output_1   | P8_11  |
| output_2   | P8_16  |
| output_3   | P8_15  |
---

## 🚗 Control de Motores
| Señal         | Pin    |
|---------------|--------|
| MA_E (ENA)    | P8_07  |
| MB_E          | P8_08  |
| MC_E          | P8_10  |
| MD_E          | P8_09  |
| MA_D (DIR)    | P8_37  |
| MB_D          | P8_38  |
| MC_D          | P8_36  |
| MD_D          | P8_34  |
---

## ⏱️ Step Period
| Señal   | Pin    |
|---------|--------|
| MA_S    | P8_41  |
| MB_S    | P8_42  |
| MC_S    | P8_39  |
| MD_S    | P8_40  |

---

## 🚨 Trigger y Detectores
| Función                       | Pin   |
|------------------------------|--------|
| ADC input trigger            | P8_13  |
| Gpio input trigger           | P8_45  |
| Motor input trigger          | P8_46  |

---

## 📊 Configuración de Conexiones

### Conexiones de Entradas Analógicas

**FICHA AI GX16-5**
| Pin | Descripción |
| --- | ----------- |
| 1   | AIN0        |
| 2   | AIN1        |
| 3   | AIN2        |
| 4   | AIN3        |
| 5   | GND         |

---

### Conexiones de Entradas Digitales

**FICHA DI GX16-5**
| Pin | Descripción |
| --- | ----------- |
| 1   | DI0         |
| 2   | DI1         |
| 3   | DI2         |
| 4   | DI3         |
| 5   | GND         |

---

### Conexiones de Salidas Digitales

**FICHA DO GX16-5**
| Pin | Descripción |
| --- | ----------- |
| 1   | DO0         |
| 2   | DO1         |
| 3   | DO2         |
| 4   | DO3         |
| 5   | GND         |

---

### Conexiones de Motores

**FICHA MA GX16-4**
| Pin | Descripción |
| --- | ----------- |
| 1   | 1B          |
| 2   | 1A          |
| 3   | 2A          |
| 4   | 2B          |

**FICHA MB GX16-4**
| Pin | Descripción |
| --- | ----------- |
| 1   | 1B          |
| 2   | 1A          |
| 3   | 2A          |
| 4   | 2B          |

**FICHA MC GX16-4**
| Pin | Descripción |
| --- | ----------- |
| 1   | 1B          |
| 2   | 1A          |
| 3   | 2A          |
| 4   | 2B          |

**FICHA MD GX16-4**
| Pin | Descripción |
| --- | ----------- |
| 1   | 1B          |
| 2   | 1A          |
| 3   | 2A          |
| 4   | 2B          |

---

### Conexiones de Alimentación de Motores

**FICHA ALIMENTACION MOTORES GX16-5**
| Pin | Descripción   |
| --- | ------------- |
| 1   | 12 V 3 A      |
| 2   | NC            |
| 3   | NC            |
| 4   | NC            |
| 5   | GND           |

---

### Conexiones de Trigger

**FICHA TRIGGER GX16-5**
| Pin | Descripción                              |
| --- | ---------------------------------------- |
| 1   | Analog channels input trigger           |
| 2   | Gpio input trigger                      |
| 3   | Motor input trigger                     |
| 4   | NC                                      |
| 5   | GND                                     |
