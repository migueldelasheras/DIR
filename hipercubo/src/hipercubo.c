#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FILENAME "datos.dat"
#define MAX_NUM 1000
#define MAX_LINE 1000

int leerFichero(int *numeros);
void obtenerVecinos(int rank, int *vecinos);
int obtenerMaximo(int rank, int bufferNum, int *vecinos);

int main(int argc, char **argv){

    int rank,size;
    MPI_Status status;

    int maximo;

    int bufferNum;

    int procesos = pow(2,D);

    int vecinos[D];

    int seguir=1;

    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank==0)
    {
        
        if (procesos!=size)
        {

            printf("Son necesarios %d procesos para una red hipercubo de dimensión %d \n",procesos,D);
            seguir=0;
            MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);

        }else{

            int *numeros = malloc(MAX_NUM * sizeof(int));

            int numTotales = leerFichero(numeros);

            if (procesos!=numTotales)
            {
                printf("La cantidad de numeros del fichero es incorrecta. Se necesitan %d numeros para una red hipercubo de dimensión %d\n", procesos, D);
                seguir=0;
                MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);
            }else{
                
                MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);
                for (int i = 1; i < numTotales; i++)
                {
                    bufferNum=numeros[i];
                    MPI_Send(&bufferNum, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                }
                bufferNum=numeros[0];
                free(numeros);

            }
            
        }
        
    }

    MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (seguir!=0)
    {
        if(rank!=0) MPI_Recv(&bufferNum, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		obtenerVecinos(rank, vecinos);
        maximo=obtenerMaximo(rank, bufferNum, vecinos);

        if(rank==0) printf("Soy el rank %d. El mayor numero de la red es: %d\n", rank, maximo);
    }
    
    MPI_Finalize();

}

int leerFichero(int *numeros){

    FILE *fp;
    char line[MAX_LINE];
    int numTotales=0;
    char *num;

    if ((fp = fopen(FILENAME,"r"))==NULL)
    {
        fprintf(stderr, " No se ha podido abrir el archivo\n");
        return 0;
    }

    fgets(line, MAX_LINE, fp); //copiamos todos los numeros en la variable char line
    fclose(fp);

    numeros[numTotales++]= atof(strtok(line,",")); //leemos el primer numero y despues leemos el resto con strtok(null)
    while( (num = strtok(NULL, "," )) != NULL ){
		numeros[numTotales++]=atof(num);
	}

    return numTotales;

}

void obtenerVecinos(int rank, int *vecinos){
    //se le aplica XOR y desplazamiento de i posiciones para obtener el vecino de la dimension i
    for(int i=0;i<D;i++){
        vecinos[i] = rank ^ (1 << i);  
    }
}

int obtenerMaximo(int rank, int bufferNum, int *vecinos){

    int maximo=bufferNum;

    MPI_Status status;

    for (int i = 0; i < D; i++)
    {

        MPI_Send(&maximo,1, MPI_INT, vecinos[i], i, MPI_COMM_WORLD);

        MPI_Recv(&bufferNum, 1, MPI_INT, vecinos[i], i, MPI_COMM_WORLD, &status);

        if(bufferNum>maximo) maximo=bufferNum;
    }

    return maximo;

}