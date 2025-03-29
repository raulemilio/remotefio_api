# Read pru adc to arm linux space
Este ejemplo muetra como adquirir datos de AN0 (P9_39) a una frecuencia de 1 KHz y enviar estos datos al linux 
space. Los datos se intercambian por la ram0 que corresponde al PRU0. El rango de medicion es 0-4095 (0 V-1.8 V)
A su vez se cuenta con una salida pruout util para verificar la frecuencia de muestra con un oscisloscopio.

- Configuracion de pines (ver en BeagleboneBlackP9HeaderTable.pdf)
	PRUout. P9_27 (r30.t5)
	$ config-pin P9_27 pruout
	La entrada AN0 (P9_39) no hace falta configurar

- Circuito de prueba
	Conectar dos resistencias en serie de 33Kohm entre 0 V(P9_34) y 1.8 V (P9_32)
	Conectar la entrada AN0 al punto serie de las dos conexiones.
	La medicion es ese caso debe ser de 0x800, que corresponde a 2048 o 0.9 V
	
- pru_firmware
        $ sudo make
        $ sudo make install
        Estas dos acciones compila y cargan el firmware en el PRU, no hace falta hacer nada mas.
        Con estas acciones el PRU debe quedar programado
- Cargar y encender el PRU
        /sys/class/remoteproc/remoteproc1$ echo 'am335x-pru0-rc-encoder-fw' > firmware
        /sys/class/remoteproc/remoteproc1$ echo 'start' > state

- linux_space proyect
        volver al proyecto principal
        $ sudo make
        $ sudo make install
        $ sudo ./main

- Debug register
        /sys/kernel/debug/remoteproc/remoteproc1$ sudo cat regs

