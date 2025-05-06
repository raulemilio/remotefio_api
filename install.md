# Guía de Configuración - BeagleBone Black

## 📀 Imagen Debian



- Imagen eMMC: `am335x-eMMC-flasher-debian-11.7-iot-armhf-2023-09-02-4gb`  
- Video tutorial: [YouTube](https://www.youtube.com/watch?v=i1zY1ylp_3c)

- Imagen SD: am335x-debian-11.7-iot-armhf-2023-09-02-4gb.img.xz

- Grabador: en SD https://etcher.balena.io/#download-etcher

**Pasos de arranque:**
1. Conectar la micro USB con la imagen.
2. Presionar el botón **boot**.
3. Conectar la alimentación de **5V 1A** (no usar USB para alimentación).

---
### Deshabilitar video:
```bash
sudo nano /boot/uEnv.txt
```
Descomentar:
disable_uboot_overlay_emmc=1
disable_uboot_overlay_video=1
disable_uboot_overlay_audio=1
disable_uboot_overlay_wireless=1

## 🌐 INSTALACION

### Update:
solo actualizar los que requieren actualización
```bash
sudo apt update
sudo apt list --upgradable
```

## 📚 Librería `librobotcontrol`

### Instalación:
```bash
git clone https://github.com/beagleboard/librobotcontrol
cd librobotcontrol
make
sudo make install
rc_test_drivers
```
Versión: `1.0.5`

### Herramientas del PRU:
Ya vienen instaladas en la imagen Debian.

Comprobar versión:
```bash
clpru --compiler_revision
# Salida esperada: versión 2.3.3
```
### Configurar idioma es_AR.UTF-8
```bash
sudo apt install locales
sudo dpkg-reconfigure locales
```

### Librería `cJSON`:
```bash
sudo apt update
sudo apt install libcjson-dev
```

## 🌐 MQTT

### Instalación mosquitto:
```bash
sudo apt install libmosquitto-dev

---

## 🧠 Node-RED

### Iniciar Node-RED:
```bash
node-red
```

### Acceder:
`http://127.0.0.1:1880/`

### Paquetes recomendados:
- `node-red-dashboard`
- Paquetes para control de LEDs

---

## 🛠️ Compilar proyectos con PRU

Desde la carpeta principal del proyecto:
```bash
make
make clean
sudo make install
sudo ./execut_proyect
```

---

## 🔐 Conexión por SSH

### Accesos:
- USB:  
  ```bash
  ssh debian@192.168.7.2
  ```
- Ethernet:  
  ```bash
  ssh debian@192.168.0.55
  ```
**Contraseña:** `temppwd`

### Copiar archivos:

**Desde BBB a PC (ejecutar desde tu máquina local):**
```bash
scp -r debian@192.168.7.2:/home/debian/remotefio_api /home/rome/Escritorio/
```

**Desde PC a BBB:**
```bash
scp -r /home/rome/Escritorio/remotefio_api debian@192.168.7.2:/home/debian/
```

---

## 🗑️ Eliminar clave SSH conocida (en caso de reconexión fallida):
```bash
ssh-keygen -f "/home/rome/.ssh/known_hosts" -R "192.168.7.2"
```
