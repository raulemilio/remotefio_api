
# Imagen debian
	am335x-eMMC-flasher-debian-11.7-iot-armhf-2023-09-02-4gb
	https://www.youtube.com/watch?v=i1zY1ylp_3c

- Conectar la micro Usb con la imagen presionar el boton boot y luego conectar la alimentacion de 5 V 1 A (no hacer por USB)

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

# MQTT
- Intalar mosquitto
		sudo apt update
		sudo apt upgrade -y
		sudo apt install -y mosquitto mosquitto-clients
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
		
en /etc/mosquitto agregar al archivo mosquitto.config -> listener 1883	

Con usb mosquitto funciona perfecto para usarlo con ether hay que:
en un terminal de la maquina broker $ ip a
^Crome@rome-HP-Laptop-15-bs0xx:~ip a a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: eno1: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc fq_codel state DOWN group default qlen 1000
    link/ether f4:30:b9:dd:6b:5e brd ff:ff:ff:ff:ff:ff
    altname enp1s0
3: wlo1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
    link/ether f8:da:0c:35:29:5f brd ff:ff:ff:ff:ff:ff
    altname wlp2s0
    inet 192.168.0.171/24 brd 192.168.0.255 scope global dynamic noprefixroute wlo1
       valid_lft 94sec preferred_lft 94sec
    inet6 fe80::fe3f:78b0:4b07:faed/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever

Ejemplo en local y con wifi el broker address es: 192.168.0.171, esta dirección es la que hay que cargar en el archivo de direccion de broker del programa. Por ejemplo para BBRemoteIO es en mqtt_calback.h		

# PRU (Cap15.pagina 667 libro BB)
## The PRU Code Generation Tools
En este enlace esta el framework de desarrollo https://www.ti.com/tool/download/PRU-CGT 
pero no hace falta instalarlo porque ya viene instalado en la imagen debian en su última versión
$ clpru --compiler_revision
-- versión 2.3.3

Librerias cJSON
sudo apt update
sudo apt install libcjson-dev

Compilar proyectos con pru
Entrar a la carpeta principal del proyecto
-- volver a la carpeta principal 
	-- make
	-- make clean
	-- sudo make install
	-- sudo ./execut_proyect
Entrar a la carpeta principal del proyecto pru
-- volver a la carpeta principal 
	-- make
	-- make clean
	-- sudo make install
	-- sudo ./execut_proyect


# Conexión por ssh
  ->$ ssh debian@192.168.7.2 (usb)
  ->$ ssh debian@192.168.0.55 (ethernet)
  ->$ luego pass: temppwd
  
- Copiar carpetas desde BBB a local 
    !!!!Importante: copiar el comando siguiente desde un terminal local (no desde BBB)!!!!!
		$ scp -r debian@192.168.7.2:/home/debian/remotefio_api /home/rome/Escritorio/
  - No borrar la carpeta  del local ya que está vinculada al git

-  Copiar carpeta desde local a BBB (terminal local)
	scp -r /home/rome/Escritorio/remotefio_api debian@192.168.7.2:/home/debian/
	
- Escritorio/voltage_data.txt
	
  # borrar clave ssh para reconcexi+ón
  ssh-keygen -f "/home/rome/.ssh/known_hosts" -R "192.168.7.2"
