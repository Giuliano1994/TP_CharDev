Comandos utilizados en la compilacion y prueba del Modulo

# Compilar el código fuente y generar el ejecutable o los módulos específicos que hayas configurado en tu archivo Makefile.
make

# registro el driver
sudo insmod charDev.ko

# Utilidad que muestra los mensajes del kernel. 
dmrsg | tail

# Eliminar (descargar) un módulo del kernel en ejecución.
sudo rmmod charDev.ko

# Para ver los mensajes relacionados con la descarga del módulo.
dmrsg | tail

# Limpia los archivos generados durante el proceso de compilación.  Elimina los archivos objeto y otros archivos generados durante la compilación, dejando solo los archivos fuente. 
make clean


# Listar todos los módulos del kernel cargados actualmente.
lsmod