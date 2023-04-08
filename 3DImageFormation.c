#include "imageFormationUtils.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>


int fnWritePPM(char* fileNm, struct point3d* img, int N, int width, int height)
{
	FILE* fp;
    unsigned char **pixels;
    pixels = (unsigned char**)calloc(height, sizeof(unsigned char*));
	for(int i=0; i<height; i++){
		pixels[i] = (unsigned char*)calloc(width * 3, sizeof(unsigned char));
	}

	fp = fopen(fileNm, "w");
	if(fp == NULL){
		fprintf(stderr, "Failed\n");
		return 0;
	}

	fprintf(fp, "%c%c\n", 'P', '3');
	fprintf(fp, "%d %d\n" , width, height);
	fprintf(fp, "%d\n", 255);

    for(int i=0;i<N;i++){
        int x = img[i].x;
        int y = img[i].y;
        pixels[x][y*3] = img[i].r;
        pixels[x][y*3+1] = img[i].g;
        pixels[x][y*3+2] = img[i].b;
    }
	for(int i=0; i<height; i++){
		for(int j=0; j<width*3; j+=3){
			fprintf(fp, "%d ", pixels[i][j]);
			fprintf(fp, "%d ", pixels[i][j+1]);
			fprintf(fp, "%d ", pixels[i][j+2]);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
	return 1;
}

void pinholeProjection(struct point3d *pointsInImage, struct point3d *points, int N, float F){
    for(int i = 0; i < N; i++){
        pointsInImage[i].x = points[i].x / (1 + points[i].z/F);
        pointsInImage[i].y = points[i].y / (1 + points[i].z/F);
        pointsInImage[i].r = points[i].r;
        pointsInImage[i].g = points[i].g;
        pointsInImage[i].b = points[i].b;
    }
}

void matmul(float *transM, float *P, float *result){
    for(int i=0;i<4;i++){
        float sum=0;
        for(int j=0;j<4;j++){
            sum += transM[i*4+j] * P[j];
        }
        result[i] = sum;
    }
    // printf("in %f %f %f\n",result[0],result[1],result[2]);
}

void orthgonalProjection(struct point3d *pointsInImage, struct point3d *points, int N, float F){
    for(int i = 0; i < N; i++){
        pointsInImage[i].x = points[i].x;
        pointsInImage[i].y = points[i].y;
        pointsInImage[i].r = points[i].r;
        pointsInImage[i].g = points[i].g;
        pointsInImage[i].b = points[i].b;
    }
}

void hideBack(struct point3d *pointsInImage, struct point3d *points, int N, float F){
    float zSum=0;
    for(int i = 0; i < N; i++){
        zSum += points[i].z;
    }

    float zMean = zSum/N;
    for(int i = 0; i < N; i++){
        if(points[i].z <= zMean){
            pointsInImage[i].x = points[i].x / (1 + points[i].z/F);
            pointsInImage[i].y = points[i].y / (1 + points[i].z/F);
            pointsInImage[i].r = points[i].r;
            pointsInImage[i].g = points[i].g;
            pointsInImage[i].b = points[i].b;
        }
    }
}

int main(int argc, char* argv[])
{
    struct point3d *points;
    struct point3d *pointsInImage;
    int N_v = 0;
    float F=0.1;
    // float F=10; // #7
    points = readOff(argv[1], &N_v);

    // rigid transformation
    float TransMatrix[16];
    // computeTrans(0.1, 1.74, 0.2, 0, 0.52, 0.15, TransMatrix);
    computeTrans(0, 0, 0, 0, 0, 0, TransMatrix);
    // computeTrans(-0.05,0.0,4.75,0.5,0,0, TransMatrix); // #7
    float Temp[4];
    float result[4];
    for(int i=0; i<N_v;i++){
        Temp[0] = points[i].x;
        Temp[1] = points[i].y;
        Temp[2] = points[i].z;
        Temp[3] = 1;
        matmul(TransMatrix,Temp,result);
        points[i].x = result[0];
        points[i].y = result[1];
        points[i].z = result[2];
    }


    pointsInImage = (struct point3d *) malloc(N_v * sizeof(struct point3d));
    pinholeProjection(pointsInImage, points, N_v, F);
    // orthgonalProjection(pointsInImage, points, N_v, F);
    // hideBack(pointsInImage, points, N_v, F); // #7
    centerThePCL(pointsInImage, N_v);

    float A_u=0.0005,A_v=0.0005;
    // float A_u=0.001,A_v=0.001;
    // float A_u=0.002,A_v=0.002; // #7
    float u_0=500,v_0=500;

    for(int i=0;i<N_v;i++){
        float x = pointsInImage[i].x/A_u + u_0;
        pointsInImage[i].x = x;
        float y = pointsInImage[i].y/A_v + v_0;
        pointsInImage[i].y = y;
    }
    
    fnWritePPM("test.ppm", pointsInImage, N_v, 1000, 1000);
} 
