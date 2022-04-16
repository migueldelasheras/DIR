#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv){
    int num=atoi(argv[1]);
    char filename[]="datos.dat";
    FILE *fp;

    if((fp = fopen(filename, "w"))==NULL){
        fprintf(stderr, " No se ha podido abrir el archivo");
        exit(EXIT_FAILURE);
    }

    srand (time(NULL));

    for (unsigned i = 0; i < num; i++)
    {
        int numero = rand() % 1000;
        fprintf(fp, "%d", numero);
        if(i!=num-1) fputs(",",fp);
    }
    fclose(fp);
}