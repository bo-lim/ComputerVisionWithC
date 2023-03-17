#include <stdlib.h>
#include <stdio.h>

#define n 3
#define ARSIZE 256

typedef unsigned char bit;
typedef unsigned char gray;

char pm_getc(FILE* file);
bit pm_getbit(FILE* file);
unsigned char pm_getrawbyte(FILE* file);
int pm_getint( FILE* file);
void pm_erreur(char *);


void mk_binomial_filter(int filter[]){
  int i,j;
  /* mk 1d filter */
  int rowFilter[n];
  rowFilter[0] = 1;rowFilter[1] = 1;rowFilter[n-1] = 1;
  
  for (i=3;i<n+1;i++){
    int tmpfilter[i];
    // side
    tmpfilter[0]= 1;tmpfilter[i-1]= 1;
    // calc
    for(j=0;j<i-2;j++){
      tmpfilter[j+1] = rowFilter[j]+rowFilter[j+1];
    }
    // copy
    for(j=0;j<i;j++){
      rowFilter[j] = tmpfilter[j];
    }
  }

  /* mk 2d filter */
  for(i=0;i<n;i++){
      for(j=0;j<n;j++){
          if(i==0){filter[j] = rowFilter[j];}
          else{
              if(j==0){filter[i*n] = rowFilter[i];}
              else{
                  filter[i*n+j] = rowFilter[j] * rowFilter[i];
              }
          }
      }
  }
}

int get_median(int data[],int filtersize){
  int i,j;
  for(i=1;i<filtersize;i++){
    int now = data[i];
    for(j=i-1;data[j]>now && j<-1;j--){
      data[j+1] = data[j];
    }
    data[j+1] = now;
  }
  int mid = (int)(filtersize/2);
  return data[mid];
}

void mk_histogram(gray* map,int* histogram, int rows,int cols){
  int idx = 0;

  for(int i=0;i<rows;i++){
    for(int j=0;j<cols;j++){
      int idx = map[i*cols + j];
      histogram[idx]+=1;
    }
  }
  for(int i=0;i<ARSIZE;i++){
    printf("%d ",histogram[i]);
  }
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

int main(int argc, char* argv[])
    {
    /* Options */
    int binomial_filter = 1; // 0 : median filter
    int showing_histogram = 1;
    char* filename ="imagename.pgm";
    int filtersize = n*n;
    int filter[filtersize];
    int filtersum = 0;

    FILE* ifp;
    gray* graymap;
    gray* newmap;
    gray* stretmap;
    gray* equalmap;

    int ich1, ich2, rows, cols, maxval=255, pgmraw;
    int i, j;

    int histogram[ARSIZE]={0,};
    int stret_histogram[ARSIZE]={0,};
    int cumul_histogram[ARSIZE]={0,};
    int equal_histogram[ARSIZE]={0,};
    int min=-1,max=-1;


    if(binomial_filter){
      mk_binomial_filter(filter);
      for (int i=0;i<filtersize;i++){
          filtersum += filter[i];
      }
    }
    
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
    newmap = (gray *) malloc((cols) * (rows) * sizeof(gray));
    stretmap = (gray *) malloc((cols) * (rows) * sizeof(gray));
    equalmap = (gray *) malloc((cols) * (rows) * sizeof(gray));

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
    if(binomial_filter){
      for(i=0;i<rows;i++){
        for(j=0;j<cols;j++){
          int tmp = 0, filter_idx = 0;
          for(int p_r=pad*(-1);p_r<=pad;p_r++){
            for(int p_c=pad*(-1);p_c<=pad;p_c++){
              tmp += (graymap[(i+p_r)*cols+j+p_c]*filter[filter_idx++]);
            }
          }
          newmap[i*cols+j] = (int)(tmp/filtersum);
        }
      }
    }
    else{//median filter
      for(i=0;i<rows;i++){
        for(j=0;j<cols;j++){
          int data[filtersize];
          int idx = 0;
          for(int p_r=pad*(-1);p_r<=pad;p_r++){
            for(int p_c=pad*(-1);p_c<=pad;p_c++){
              data[idx++] = graymap[(i+p_r)*cols+j+p_c];
            }
          }
          newmap[i*cols + j] = get_median(data,filtersize);
        }
      }
    }

    /* Print Histogram */
    printf("Intensity Histogram : \n");
    mk_histogram(newmap, histogram, rows,cols);


    /* Histogram Stretching */
    // find max min
    for(i=0;i<ARSIZE;i++){
      if(histogram[i]){
        if(min<0)min=i;
        max=i;
      }
    }
    int gap = max-min;
    // modify image map
    for(i=0;i<rows;i++){
        for(j=0;j<cols;j++){
          stretmap[i*cols + j]=(newmap[i*cols + j]-min)*maxval/gap;
        }
    }

    if(showing_histogram){
      // for showing histogram
      printf("\n\nStretched Histogram : \n");
      mk_histogram(stretmap,stret_histogram, rows,cols);
    }


    /* Histogram Equalization*/
    // mk cumulative histogram
    cumul_histogram[0] = histogram[0];
    for(i=1;i<ARSIZE;i++){
      cumul_histogram[i] = cumul_histogram[i-1] + histogram[i];
    }

    // modify image map
    for(i=0;i<rows;i++){
        for(j=0;j<cols;j++){
          int value = newmap[i*cols + j];
          equalmap[i*cols+j]= maxval*cumul_histogram[value]/cumul_histogram[ARSIZE-1];
        }
    }

    if(showing_histogram){
      // for showing histogram
      printf("\n\nHistogram Equalization :\n");
      mk_histogram(equalmap,equal_histogram, rows,cols);
    }

    /* Printing Images */
    print_img("filter.pgm",newmap, pgmraw, cols, rows, maxval);
    print_img("stretching.pgm",stretmap, pgmraw, cols, rows, maxval);
    print_img("equalization.pgm",equalmap, pgmraw, cols, rows, maxval);

    return 0;
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

