DIRSRC:= src/

CC:=mpicc

all: clean compilar run

compilar: 
	@read -p "¿Cuántos procesos quieres utilizar? " numero; \
	$(CC) $(DIRSRC)pract2.c -o pract2 -lX11 -DNUMPROCESOS=$$numero 

run:
	mpirun -np 1 --hostfile $(DIRSRC)hostfile ./pract2

clean:
	rm -rf pract2