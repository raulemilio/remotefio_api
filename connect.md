## 🔐 Conexión por SSH a BBB

### Encendido
- Conectar alimentación y luego USB
- Dentro de la BBB
  ```bash
  ip a
  ```
- Seleccionar eth0 o similar
- Con esto ya se puede acceder por ethernet (sin USB)

### Apagado
- Control C
- sudo shutdown now
- Desconectar el cable de alimentación
- Desconectar USB

### Accesos:
- USB:  
  ```bash
  ssh debian@192.168.7.2
  ```
- Ethernet:  
  ```bash
  ssh debian@192.168.0.55 (depende la asignación del router)
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
