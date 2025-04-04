# Write-read mem
Este ejemplo muetra como cargar datos en las diferentes memorias del PRU0 y como leer los datos 
desde el linux space

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


scp debian@192.168.7.2:/home/debian/remotefio_api/test_mem_adc_4c/data_output.txt /home/rome/Escritorio/data.txt
