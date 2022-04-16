#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "datos.dat"
#define MAX_NUM 1024
#define MAX_LINE 1024
#define L 2

int leerFichero(int *numeros);
void obtenerVecinos(int rank, int *vecinoN, int *vecinoS, int *vecinoE, int *vecinoO);
double obtenerMinimo(int rank, int numero, int vecinoN, int vecinoS, int vecinoE, int vecinoO);

int main(int argc, char **argv){

    int rank, size;
    MPI_Status status;

    //menor numero de la red
    int minimo;

    //numero que corresponde a cada nodo
    int miNumero;

    //vecinos
    int vecinoN, vecinoS, vecinoE, vecinoO;

    //variable para seguir o no
    int seguir=1;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank==0)
    {
        if (L*L!=size)
        {

            printf("Son necesarios %d procesos para una red toroide de lado %d \n",L*L,L);
            seguir=0;
            MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        }else{

            /* reservamos un espacio de memoria para los numeros. como no sabemos la cantidad de numeros que 
            hay; definimos la cantidad maxima de numeros que queremos usar*/
            int *numeros = malloc(MAX_NUM * sizeof(int));

            int numTotales = leerFichero(numeros);

            if(L*L!=numTotales){

                printf("La cantidad de numero del fichero es incorrecta. Se necesitan %d numeros para una red toroide de lado %d", L*L, L);
                seguir=0;
                MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);

            }else{

                MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);
                for (int i = 0; i < numTotales; ++i)
                {
                    miNumero=numeros[i];
                    MPI_Send(&miNumero, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                }
                free(numeros);
                
            }
        }
    }

    //esperamos un broadcast para poder continuar con la ejecucion
    MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (seguir!=0)
    {
        MPI_Recv(&miNumero, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        printf("Soy el rank %d. El numero es: %d\n", rank, miNumero);
        
        obtenerVecinos(rank, &vecinoN, &vecinoS, &vecinoE, &vecinoO);

        minimo= obtenerMinimo(rank, miNumero, vecinoN, vecinoS, vecinoE, vecinoO);

        if(rank==0) printf("Soy el rank %d. El menor es: %d\n", rank, minimo);
    }
    
    MPI_Finalize();
    return 0;
}

int leerFichero(int *numeros){

    FILE *fp;
    char line[MAX_LINE];
    int numTotales=0;
    char *num;

    if ((fp = fopen(FILENAME,"r"))==NULL)
    {
        fprintf(stderr, " No se ha podido abrir el archivo");
        return 0;
    }

    fgets(line, 200, fp); //copiamos todos los numeros en la variable char line
    fclose(fp);

    numeros[numTotales++]= atof(strtok(line,",")); //leemos el primer numero y despues leemos el resto con strtok(null)
    while( (num = strtok(NULL, "," )) != NULL ){
		numeros[numTotales++]=atof(num);
	}

    return numTotales;

}

//método para obtener los ranks de los vecinos correspondientes a mi rank
void obtenerVecinos( int rank, int *vecinoN, int *vecinoS, int *vecinoE, int *vecinoO){

    int fila, columna;

    //como las filas se enumeran de abajo a arriba y las columnas de izquierda a derecha:
    fila = rank/L;
    columna = rank%L;

    if(fila==0){
		//Obtengo el vecino inferior, correspondiente al norte de la red
		*vecinoS = ((L-1)*L)+columna;
	}else{
		*vecinoS = ((fila-1)*L)+columna;
	}

	//Si la fila es la superior
	if(fila==L-1){
		//Obtengo el vecino superior, correspondiente al sur de la red
		*vecinoN = columna;
	}else{
		*vecinoN = ((fila+1)*L)+columna;
	}

	//Si la columna es la izquierda
	if(columna==0){
		//Obtengo el vecino izquierdo, correspondiente al oeste de la red
		*vecinoO = (fila*L)+(L-1);
	}else{
		*vecinoO = (fila*L)+(columna-1);
	}

	//Si la columna es la derecha
	if(columna==L-1){
		//Obtengo el vecino derecho, correspondiente al este de la red
		*vecinoE = (fila*L);
	}else{
		*vecinoE = (fila*L)+(columna+1);
	}
    
}

double obtenerMinimo(int rank, int numero, int vecinoN, int vecinoS, int vecinoE, int vecinoO){

    int minimo= __INT_MAX__;

    MPI_Status status;

    /*calculamos el numero menor por fila enviando al vecino este el minimo y recibiendo del vecino izquierdo
    su minimo */
    for (int i = 0; i < L; i++)
    {
        if(numero<minimo) minimo=numero; //si mi numero es menor que minimo, lo cambio
    
        //enviamos el minimo a mi vecino este(derecho)
        MPI_Send(&minimo, 1, MPI_INT, vecinoE, i, MPI_COMM_WORLD);

        //recibimos el minimo del vecino oeste(izquierdo)
        MPI_Recv(&numero, 1, MPI_INT, vecinoO, i, MPI_COMM_WORLD, &status);

        //Si es menor el numero que me ha enviado, lo cambio por el minimo
        if(numero<minimo) minimo=numero;
    }

    /*calculamos el numero menor por columna enviando al vecino norte el minimo y recibiendo del vecino
    sur su minimo */
    for(int i=0; i<L; i++){

		//Envio mi minimo a mi vecino superior
		MPI_Send(&minimo, 1, MPI_INT, vecinoN, i, MPI_COMM_WORLD);

		//Recibo el minimo de mi vecino inferior
		MPI_Recv(&numero, 1, MPI_INT, vecinoS, i, MPI_COMM_WORLD, &status);

		if(numero<minimo) minimo=numero;
		
	}

    return minimo;

}

