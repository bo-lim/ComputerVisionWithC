#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define TRUE	1
#define FALSE	0
#define K 3

typedef struct {
	char	M, N;
	int		width;
	int		height;
	int		max;
	unsigned char **pixels;
} PPMImage;

typedef struct {
  int sum[3];
  int cnt;
} CenterMean;

typedef unsigned char regionNum;

/* read & write ppm file */
int fnReadPPM(char* fileNm, PPMImage* img);
void fnClosePPM(PPMImage* img);
int fnWritePPM(char* fileNm, PPMImage* img);

int main(int argc, char** argv)
{
  srand(time(NULL));
	PPMImage img;

  char imgName[10] = "frog.ppm";
	if(fnReadPPM(imgName, &img) != TRUE){
		return -1;
	}

  CenterMean cMean[K]={{{0,0,0},0},{{0,0,0},0},{{0,0,0},0}};
  
  /* Initial centers */
  unsigned char centers[K][3];

  regionNum** regDistLocMap = (regionNum**) calloc(img.height ,sizeof(regionNum*));
  for(int i=0; i<img.height; i++){
    regDistLocMap[i] = (regionNum*)calloc(img.width*3,sizeof(regionNum));
  }
  
  for(int i=0;i<K;i++){
    int rnd_row = rand()%768;
    int rnd_col = rand()%1024;
    centers[i][0] = img.pixels[rnd_row][rnd_col+0];
    centers[i][1] = img.pixels[rnd_row][rnd_col+1];
    centers[i][2] = img.pixels[rnd_row][rnd_col+2];
  }

  int stopFlag = 0;
  while(!stopFlag){
    /* Calculate the distances to the center 
        & Find the closest center */
    for(int i=0; i<img.height; i++){
      for(int j=0; j<img.width * 3; j+=3){
        int minC,minDistance=999999;
        for(int c=0;c<K;c++){
          int distance = sqrt(pow(img.pixels[i][j]-centers[c][0],2)+
          pow(img.pixels[i][j+1]-centers[c][1],2)+pow(img.pixels[i][j+2]-centers[c][2],2));
          // int distance = abs(img.pixels[i][j]-centers[c][0])+
          // abs(img.pixels[i][j+1]-centers[c][1])+abs(img.pixels[i][j+2]-centers[c][2]);
          if(distance<minDistance){
            minDistance= distance;
            minC= c;
          }
        }
        regDistLocMap[i][j]= minC;
        cMean[minC].cnt= cMean[minC].cnt+1;
        cMean[minC].sum[0]+= img.pixels[i][j];
        cMean[minC].sum[1]+= img.pixels[i][j+1];
        cMean[minC].sum[2]+= img.pixels[i][j+2];
      }
    }
    
    

    /* Calculate the mean of pixel values in each region 
      & Define new region centers */
    int prev_centers[K][3];
    int diff = 0;
    for(int c=0;c<K;c++){
      prev_centers[c][0]=centers[c][0];
      prev_centers[c][1]=centers[c][1];
      prev_centers[c][2]=centers[c][2];
      centers[c][0]= cMean[c].sum[0] / cMean[c].cnt;
      centers[c][1]= cMean[c].sum[1] / cMean[c].cnt;
      centers[c][2]= cMean[c].sum[2] / cMean[c].cnt;
      printf("New center[%d] : %d %d %d\n",c,centers[c][0],centers[c][1],centers[c][2]);
      // compare new centers and previous centers
      diff += abs(centers[c][0]-prev_centers[c][0]) + 
        abs(centers[c][1]-prev_centers[c][1]) + abs(centers[c][2]-prev_centers[c][2]);

      cMean[c].sum[0]=0;cMean[c].sum[1]=0;cMean[c].sum[2]=0;
      cMean[c].cnt = 0;
    }

    if(diff>0){stopFlag=0;printf("Continued...\n");}
    else{stopFlag=1;}
  }

  /* iterate until no moving */
  for(int i=0; i<img.height; i++){
    for(int j=0; j<img.width*3; j+=3){
      int region = regDistLocMap[i][j];
      img.pixels[i][j] = centers[region][0];
      img.pixels[i][j+1] = centers[region][1];
      img.pixels[i][j+2] = centers[region][2];
    }
  }

  

	if(fnWritePPM("result.ppm", &img) == TRUE){
		printf("Finished!\n");
	}
	fnClosePPM(&img);

	return 0;
}

int fnReadPPM(char* fileNm, PPMImage* img)
{
	FILE* fp;

	if(fileNm == NULL){
		fprintf(stderr, "Filename Error\n");
		return FALSE;
	}
	
	fp = fopen(fileNm, "rb");
	if(fp == NULL){
		fprintf(stderr, "Cannot open : %s\n", fileNm);
		return FALSE;
	}

	fscanf(fp, "%c%c\n", &img->M, &img->N);

	if(img->M != 'P' || img->N != '6'){
		fprintf(stderr, "No PPM format : %c%c\n", img->M, img->N);
		return FALSE;
	}

	fscanf(fp, "%d %d\n", &img->width, &img->height);
	fscanf(fp, "%d\n"   , &img->max                );

	img->pixels = (unsigned char**)calloc(img->height, sizeof(unsigned char*));

	for(int i=0; i<img->height; i++){
		img->pixels[i] = (unsigned char*)calloc(img->width * 3, sizeof(unsigned char));
	}
	for(int i=0; i<img->height; i++){
		for(int j=0; j<img->width * 3; j++){
			fread(&img->pixels[i][j], sizeof(unsigned char), 1, fp);
		}
	}
	fclose(fp);
	return TRUE;
}

void fnClosePPM(PPMImage* img)
{
	for(int i=0; i<img->height; i++){
		free(img->pixels[i]);
	}

	free(img->pixels);
}

int fnWritePPM(char* fileNm, PPMImage* img)
{
	FILE* fp;

	fp = fopen(fileNm, "w");
	if(fp == NULL){
		fprintf(stderr, "Failed\n");
		return FALSE;
	}

	fprintf(fp, "%c%c\n", 'P', '3');
	fprintf(fp, "%d %d\n" , img->width, img->height);
	fprintf(fp, "%d\n", 255);


	for(int i=0; i<img->height; i++){
		for(int j=0; j<img->width * 3; j+=3){
			fprintf(fp, "%d ", img->pixels[i][j]);
			fprintf(fp, "%d ", img->pixels[i][j+1]);
			fprintf(fp, "%d ", img->pixels[i][j+2]);

		}

		fprintf(fp, "\n");
	}

	fclose(fp);
	
	return TRUE;
}
