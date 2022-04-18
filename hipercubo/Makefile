DIRSRC := src/
LADO := 0

CC := gcc
 
all: clean generarDatos exeDatos hipercubo run
	
generarDatos:
	$(CC) -g $(DIRSRC)generarDatos.c -o generarDatos

exeDatos:
	@read -p "Número de datos del fichero datos.dat: " numero; \
	./generarDatos $$numero

hipercubo: 
	@read -p "¿Cuál será la dimensión del hipercubo?: " dimension; \
	mpicc -g $(DIRSRC)hipercubo.c -lm -o hipercubo -DD=$$dimension

run: 
	@read -p "¿Cuántos procesos quieres lanzar?: " procesos; \
	mpirun -n $$procesos --hostfile $(DIRSRC)hostfile hipercubo

clean:
	rm -rf hipercubo datos.dat generarDatos