DIRSRC := src/
LADO := 0

CC := gcc
 
all: clean generarDatos exeDatos toroide run
	
generarDatos:
	$(CC) -g $(DIRSRC)generarDatos.c -o generarDatos

exeDatos:
	@read -p "Número de datos del fichero datos.dat: " numero; \
	./generarDatos $$numero

toroide: 
	@read -p "¿Cuál será el lado del toroide?: " lado; \
	mpicc -g $(DIRSRC)toroide.c -o toroide -DL=$$lado

run: 
	@read -p "¿Cuántos procesos quieres lanzar?: " procesos; \
	mpirun -n $$procesos --hostfile $(DIRSRC)hostfile toroide

clean:
	rm -rf toroide datos.dat generarDatos