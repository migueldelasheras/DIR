/* Pract2  RAP 09/10    Javier Ayllon*/

#include <openmpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h> 
#include <assert.h>   
#include <unistd.h>

#define NIL (0)   
#define IMAGEN "src/foto.dat"  
#define TAMANIO 400  

/*Variables Globales */

XColor colorX;
Colormap mapacolor;
char cadenaColor[]="#000000";
Display *dpy;
Window w;
GC gc;

/*Funciones auxiliares */

void initX() {

      dpy = XOpenDisplay(NIL);
      assert(dpy);

      int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
      int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

      w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
                                     400, 400, 0, blackColor, blackColor);
      XSelectInput(dpy, w, StructureNotifyMask);
      XMapWindow(dpy, w);
      gc = XCreateGC(dpy, w, 0, NIL);
      XSetForeground(dpy, gc, whiteColor);
      for(;;) {
            XEvent e;
            XNextEvent(dpy, &e);
            if (e.type == MapNotify)
                  break;
      }


      mapacolor = DefaultColormap(dpy, 0);

}

void dibujaPunto(int x,int y, int r, int g, int b) {

        sprintf(cadenaColor,"#%.2X%.2X%.2X",r,g,b);
        XParseColor(dpy, mapacolor, cadenaColor, &colorX);
        XAllocColor(dpy, mapacolor, &colorX);
        XSetForeground(dpy, gc, colorX.pixel);
        XDrawPoint(dpy, w, gc,x,y);
        XFlush(dpy);

}

//Método para recibir y dibujar el punto
void imprimirPunto(MPI_Comm commPadre){
  int punto[5];
  for(unsigned i=0; i<TAMANIO*TAMANIO; i++){
    //recibimos el punto
    MPI_Recv(&punto,5,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,commPadre,MPI_STATUS_IGNORE);
    //imprimimos el punto
    dibujaPunto(punto[0],punto[1],punto[2],punto[3],punto[4]);
  }
}

/* Programa principal */

int main (int argc, char *argv[]) {

  int rank,size;
  MPI_Comm commPadre;
  int tag;
  MPI_Status status;

  int buf[5]; //buffer del punto
  
  int codeError[NUMPROCESOS];


  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_get_parent( &commPadre );
  if ( (commPadre==MPI_COMM_NULL) && (rank==0) )  {
    
    initX();

	  /* Codigo del maestro */

    //codigos de error de los procesos trabajadores

    //Creacion de los procesos trabajadores
    MPI_Comm_spawn("pract2",MPI_ARGV_NULL,NUMPROCESOS,MPI_INFO_NULL,0,MPI_COMM_WORLD,&commPadre,codeError);

	  //recibimos e imprimimos los puntos
    imprimirPunto(commPadre);
    printf("Imagen renderizada. Se mostrará durante 10 segundos\n");
    sleep(10);
        
  } else {

    /* Codigo de todos los trabajadores */
    int filasPorTrabajador = TAMANIO/NUMPROCESOS;
    int bloquePorTrabajador = filasPorTrabajador * TAMANIO * 3 * sizeof(unsigned char);

    //definimos la fila de inicio y final del rank
    int inicio = rank * filasPorTrabajador;
    int final = inicio + filasPorTrabajador;

    /* El archivo sobre el que debemos trabajar es foto.dat */

    //manejador del archivo foto.dat
    MPI_File fp;
    //Abrimos el archivo en modo solo lectura
    MPI_File_open(MPI_COMM_WORLD,IMAGEN,MPI_MODE_RDONLY,MPI_INFO_NULL, &fp);
    //Le asinamos al rank su parte correspondiente del archivo
    MPI_File_set_view(fp,rank*bloquePorTrabajador,MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR,"native",MPI_INFO_NULL);

    if(rank==NUMPROCESOS-1) final=TAMANIO;

    unsigned char color[3];

    //comenzamos la lectura de la vista asignada al rank
    for(unsigned i=inicio;i<final;i++){
      for(unsigned j=0; j<TAMANIO;j++){

        MPI_File_read(fp,color,3,MPI_UNSIGNED_CHAR,&status);

        //Creamos el punto
        //intercambiamos las posiciones de i y j para que la imagen no salga girada
        buf[0]=j;
        buf[1]=i;
        buf[2]=(int)color[0];
        buf[3]=(int)color[1];
        buf[4]=(int)color[2];
        
        //Enviamos el punto al proceso maestro
        MPI_Send(&buf,5,MPI_INT,0,1,commPadre);

      }
    }

    //Cerramos el archivo foto.dat
    MPI_File_close(&fp);

  }

  MPI_Finalize();

}

