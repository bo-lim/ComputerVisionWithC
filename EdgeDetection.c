#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define n 3
#define ARSIZE 256
#define THRESHOLD 200

/* Util.c Util.h*/
typedef unsigned char bit;
typedef unsigned char gray;

char pm_getc(FILE* file);
bit pm_getbit(FILE* file);
unsigned char pm_getrawbyte(FILE* file);
int pm_getint( FILE* file);
void pm_erreur(char *);
void print_img(char filename[50],gray* map, int pgmraw, int cols, int rows, int maxval);


int main(int argc, char* argv[])
    {
    /* Options */
    char* filename ="images/boat.pgm";
    int filtersize = n*n;
    // scharr operators
    int filter_h1[] = {-3,0,3,-10,0,10,-3,0,3};
    int filter_h2[] = {-3,-10,-3,0,0,0,3,10,3};
    int div_filter = 16;

    FILE* ifp;
    gray* graymap;
    gray* gradmap;
    double* anglemap;
    gray* resmap;

    int ich1, ich2, rows, cols, maxval=255, pgmraw;
    int i, j;
    
    /* Opening */
    ifp = fopen(filename,"rb");
    if (ifp == NULL) {
      printf("error in opening file %s\n", filename);
      exit(1);
    }

    /*  Magic number reading */
    ich1 = getc( ifp );
    if ( ich1 == EOF )
        pm_erreur( "EOF / read error / magic number" );
    ich2 = getc( ifp );
    if ( ich2 == EOF )
        pm_erreur( "EOF /read error / magic number" );
    if(ich2 != '2' && ich2 != '5')
      pm_erreur(" wrong file type ");
    else
      if(ich2 == '2')
    pgmraw = 0;
      else pgmraw = 1;

    /* Reading image dimensions */
    cols = pm_getint( ifp );
    rows = pm_getint( ifp );
    maxval = pm_getint( ifp );

    int pad = (int)(n/2);
    int newcols = cols + pad*2;
    int newrows = rows + pad*2;

    /* Memory allocation  */
    graymap = (gray *) malloc(newcols * newrows * sizeof(gray));
    gradmap = (gray *) malloc((cols) * (rows) * sizeof(gray));
    anglemap = (double *) malloc((cols) * (rows) * sizeof(double));
    resmap = (gray *) malloc((cols) * (rows) * sizeof(gray));
    /* Reading */
    for(i=pad; i < rows+pad; i++)
      for(j=pad; j < cols+pad ; j++)
        if(pgmraw)
          graymap[i * cols + j] = pm_getrawbyte(ifp) ;
        else
          graymap[i * cols + j] = pm_getint(ifp);
    /* Padding */
    //up
    for(i=pad;i>0;i--){
      for(j=pad;j<cols+pad;j++){
          graymap[(i-1)*cols+j] = graymap[pad*cols + j];
      }
    }
    //down
    for(i=pad+rows-1;i<newrows-1;i++){
      for(j=pad;j<cols+pad;j++){
          graymap[(i+1)*cols + j] = graymap[i*cols + j];
      }
    }
    //left
    for(j=pad;j>0;j--){
      for(i=0;i<rows+2;i++){
          graymap[i*cols+j-1] = graymap[i*cols+j];
      }
    }
    //right
    for(j=pad+cols-1;j<newcols-1;j++){
      for(i=0;i<rows+2;i++){
          graymap[i*cols+(j+1)] = graymap[i*cols+j];
      }
    }

    /* Filtering */
    for(i=0;i<rows;i++){
      for(j=0;j<cols;j++){
        int tmp = 0, filter_idx = 0;
        int grad_y = 0, grad_x = 0;

        for(int p_r=pad*(-1);p_r<=pad;p_r++){
          for(int p_c=pad*(-1);p_c<=pad;p_c++){
            grad_x += graymap[(i+p_r)*cols+j+p_c]*filter_h1[filter_idx];
            grad_y += graymap[(i+p_r)*cols+j+p_c]*filter_h2[filter_idx];
            filter_idx += 1;
          }
        }
        gradmap[i*cols+j] = sqrt(pow(grad_x/div_filter,2)+pow(grad_y/div_filter,2));
        anglemap[i*cols+j] = atan2(grad_y,grad_x)*180./3.14;
        if(anglemap[i*cols+j]<0){anglemap[i*cols+j] += 180;}
      }
    }

    /* Edge Detection */
    for(i=0;i<rows;i++){
      for(j=0;j<cols;j++){
        int left,right;
        // 0 degrees
        if (((0 <= anglemap[i*cols+j]) && (anglemap[i*cols+j]< 22.5))||
            ((157.5 <= anglemap[i*cols+j]) &&(anglemap[i*cols+j]<= 180))){
          left = gradmap[i*cols+j - 1];
          right = gradmap[i*cols+j + 1];
        }

        // 45 degrees
        if (22.5 <= anglemap[i*cols+j] &&  anglemap[i*cols+j] < 67.5){
            left = gradmap[(i - 1)*cols+j - 1];
            right = gradmap[(i + 1)*cols+j + 1];
        }

        // 90 degrees
        if (67.5 <= anglemap[i*cols+j] && anglemap[i*cols+j] < 112.5){
          left = gradmap[(i - 1)*cols+ j];
          right = gradmap[(i + 1)*cols+ j];
        }
        
        //135 degrees
        if(112.5 <= anglemap[i*cols+j] && anglemap[i*cols+j] < 157.5){
            left = gradmap[(i - 1)*cols+ j + 1];
            right = gradmap[(i + 1)*cols+ j - 1];
        }

        if((gradmap[i*cols+ j] > left) && (gradmap[i*cols+ j] > right)){
            resmap[i*cols+j] = gradmap[i*cols+ j];
        }
        else{
          resmap[i*cols+j]=0;
        }
      }
    }

    double high_threshold=60, low_threshold=30;
    int weak_pixel_val = 80, strong_pixel_val = 255;
    int strong_x, strong_y, weak_x, weak_y;
    for(i=0;i<rows;i++){
      for(j=0;j<cols;j++){
        if(resmap[i*cols+j]>high_threshold){
          resmap[i*cols+j]=strong_pixel_val;
        }else{
          if(resmap[i*cols+j]>low_threshold){
            resmap[i*cols+j]=weak_pixel_val;
          }
        }
      }
    }

    for(i=0;i<rows;i++){
      for(j=0;j<cols;j++){
        if(resmap[i*cols+j]==weak_pixel_val){
          if((resmap[(i+1)*cols+(j-1)]==strong_pixel_val)||
          (resmap[(i+1)*cols+j]==strong_pixel_val)||(resmap[(i+1)*cols+(j+1)]==strong_pixel_val)||
          (resmap[(i)*cols+(j-1)]==strong_pixel_val)||(resmap[(i)*cols+(j+1)]==strong_pixel_val)||
          (resmap[(i-1)*cols+(j-1)]==strong_pixel_val)||(resmap[(i-1)*cols+(j)]==strong_pixel_val)
          ||(resmap[(i-1)*cols+(j+1)]==strong_pixel_val)){
          resmap[i*cols+j]=strong_pixel_val;
          }else{
            resmap[i*cols+j]=0;
          }
        }
      }
    }

    /* Printing Images */
    print_img("gradient.pgm",gradmap, pgmraw, cols, rows, maxval);
    print_img("result.pgm",resmap, pgmraw, cols, rows, maxval);

    return 0;
}

void print_img(char filename[50],gray* map, int pgmraw, int cols, int rows, int maxval){
  FILE* new = fopen(filename, "w+");
  /* Writing */
  if(pgmraw)
    fprintf(new,"P2\n");
  else
    fprintf(new,"P5\n");

  fprintf(new,"%d %d \n", cols, rows);
  fprintf(new,"%d\n",maxval);

  for(int i=0; i < rows; i++)
    for(int j=0; j < cols ; j++)
      if(pgmraw)
        fprintf(new,"%d ", map[i * cols + j]);
      else
        fprintf(new,"%c",map[i * cols + j]);
    /* Closing */
    fclose(new);
}

char pm_getc(FILE* file)
    {
    int ich;
    char ch;

    ich = getc( file );
    if ( ich == EOF )
        pm_erreur("EOF / read error" );
    ch = (char) ich;

    if ( ch == '#' )
        {
        do
            {
            ich = getc( file );
            if ( ich == EOF )
                pm_erreur("EOF / read error" );
            ch = (char) ich;
            }
        while ( ch != '\n' && ch != '\r' );
        }

    return ch;
    }

bit pm_getbit(FILE* file)
    {
    char ch;

    do
        {
        ch = pm_getc( file );
        }
    while ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' );

    if ( ch != '0' && ch != '1' )
        pm_erreur("read error : != [0|1]" );

    return ( ch == '1' ) ? 1 : 0;
    }

unsigned char pm_getrawbyte(FILE* file)
    {
    int iby;

    iby = getc( file );
    if ( iby == EOF )
        pm_erreur("EOF / read error " );
    return (unsigned char) iby;
    }

int pm_getint( FILE* file)
    {
    char ch;
    int i;

    do
        {
        ch = pm_getc( file );
        }
    while ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' );

    if ( ch < '0' || ch > '9' )
        pm_erreur( "read error : != [0-9]" );

    i = 0;
    do
        {
        i = i * 10 + ch - '0';
        ch = pm_getc( file );
        }
    while ( ch >= '0' && ch <= '9' );

    return i;
    }

void pm_erreur(char *texte)
{
    fprintf(stderr, "\n%s \n\n", texte);
    exit(1);
}

