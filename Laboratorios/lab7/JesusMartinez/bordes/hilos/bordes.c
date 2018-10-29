#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "imagen.h"
#define DIMASK 3
#define NUM_HILOS 4

unsigned char * reservarMemoria (uint32_t width, uint32_t height);

unsigned char * RGBtoGray( unsigned char *imagenRGB, uint32_t width, uint32_t height );

void GraytoRGB( unsigned char *imagenGray, unsigned char *imagenRGB, uint32_t width, uint32_t height );

void brilloImagen(unsigned char *imagenGray, uint32_t width, uint32_t height);

void filtroImagen( unsigned char *imagenGray, unsigned char *imagenFiltro, uint32_t width, uint32_t height );

void * funHilo (void * arg);

// Variables Globales

unsigned char *imagenRGB, *imagenGray, *imagenFiltro;
uint32_t width, height;


int main()
{
	bmpInfoHeader info;
	
	int nhs[NUM_HILOS];
    int *res;
	register int nh;
    pthread_t tids[NUM_HILOS];
	
	imagenRGB = abrirBMP( "calle1.bmp", &info );
	
	displayInfo( &info );
    
    width = info.width;
    height = info.height;

    //ConvertirGray
	imagenGray = RGBtoGray( imagenRGB, info.width, info.height );
	//Bordes
    imagenFiltro = reservarMemoria(info.width, info.height);
	
	for(nh = 0; nh<NUM_HILOS; nh++)
	{
		nhs[nh] = nh;
		pthread_create(&tids[nh], NULL, funHilo, (void*)&nhs[nh]);
	}
	
	for(nh = 0; nh<NUM_HILOS; nh++)
	{
		pthread_join(tids[nh], (void**)&res);
		printf("Hilo %i terminado\n", *res);
	}

    //ConvertirRgb
	GraytoRGB( imagenFiltro, imagenRGB, info.width, info.height );
	   
	guardarBMP("calleSobel.bmp", &info, imagenRGB);
	
	free( imagenRGB );
	free( imagenGray );
	free( imagenFiltro );
	
	return 0;
}


void * funHilo (void * arg)
{
   	
	int nh = *(int*)arg;

	int tamBloquey = height/NUM_HILOS;
	int iniBloquey = nh * tamBloquey;
	int finBloquey = iniBloquey + tamBloquey;
	

    register int x, y, xm, ym;
    int conv1, conv2, indice, indicem, magnitud;
    char GradF [] = { 1, 0, -1,
                      2, 0, -2,
                      1, 0, -1};
                               
    char GradC [] = { -1, -2, -1,
                       0,  0,  0,
                       1,  2,  1};
    
    if( nh == 3 )
    {
        finBloquey -= DIMASK;
    }

    //printf("Hilo %i, tam %i, ini %i, fin %i\n", nh, tamBloquey, iniBloquey, finBloquey);
        
    for( y=iniBloquey; y<=finBloquey; y++ )
            for( x=0; x<=width-DIMASK; x++ )
            {
                conv1 = 0;
                conv2 = 0;
                indicem = 0;
                for(ym=0; ym<DIMASK; ym++)
                    for(xm=0; xm<DIMASK; xm++)
                    {
                        indice = (y+ym)*width+(x+xm);
                        conv1 += imagenGray[indice] * GradF[indicem];
                        conv2 += imagenGray[indice] * GradC[indicem++];
                        
                    }
                conv1 = conv1 / 4;
                conv2 = conv2 / 4;
                magnitud = sqrt((conv1*conv1) + (conv2*conv2));
                imagenFiltro[indice] = magnitud;
            }
	
    
	pthread_exit( arg );
}



unsigned char * reservarMemoria (uint32_t width, uint32_t height)
{
    unsigned char * imagen;
    
    imagen = (unsigned char *)malloc(width*height*sizeof(unsigned char));
    
    if(imagen == NULL)
    {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }

    return imagen;
}

void brilloImagen(unsigned char *imagenGray, uint32_t width, uint32_t height)
{
    register int p;
    unsigned short int pixel;
    
    for(p=0; p<width*height; p++)
    {
        pixel = imagenGray[p] + 80;
        imagenGray[p] = (pixel>255) ? 255 : (unsigned char)pixel;
    }
}

void GraytoRGB( unsigned char *imagenGray, unsigned char *imagenRGB, uint32_t width, uint32_t height )
{
    int x,y;
    int indiceRGB, indiceGray;
    
    for(y=0; y<height; y++)
    {
        for(x=0;x<width;x++)
        {
            indiceGray = (width * y) + x;
            indiceRGB = indiceGray * 3;
            
            imagenRGB[indiceRGB  ] = imagenGray[indiceGray];
            imagenRGB[indiceRGB+1] = imagenGray[indiceGray];
            imagenRGB[indiceRGB+2] = imagenGray[indiceGray];
        }
    }
    
}

unsigned char * RGBtoGray( unsigned char *imagenRGB, uint32_t width, uint32_t height )
{
    register int x, y;
    int indiceRGB, indiceGray;
    unsigned char grayLevel;
    unsigned char *imagenGray;
    
    imagenGray = (unsigned char *)malloc(width*height*sizeof(unsigned char));
    
    if(imagenGray == NULL)
    {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }
    
    for(y=0; y<height; y++)
    {
        for(x=0;x<width;x++)
        {
            indiceGray = (width * y) + x;
            indiceRGB = (indiceGray<<1) + indiceGray; //indiceGray * 3;
            
            //Calcular nivel de gris
            grayLevel = (imagenRGB[indiceRGB]*30 + imagenRGB[indiceRGB + 1]*59 +imagenRGB[indiceRGB + 2]*11) / 100;
            
            imagenGray[indiceGray] = grayLevel;
        }
    }
        
        return imagenGray;
}
