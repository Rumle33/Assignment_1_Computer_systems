//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"

//Function to invert pixels of an image (negative)
void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]){
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
      output_image[x][y][c] = 255 - input_image[x][y][c];
      }
    }
  }
}

void grayscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]){
for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image[x][y][0] = ((input_image[x][y][0]+input_image[x][y][1]+input_image[x][y][2])/3);
      output_image[x][y][1] = output_image[x][y][0];
      output_image[x][y][2] = output_image[x][y][1];

    }
  }
}
void binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]){
for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
     if(input_image[x][y][0]>THRESHOLD_FB){
      output_image[x][y][0] = 255;
      output_image[x][y][1] = 255;
      output_image[x][y][2] = 255;
     }
     else{
      output_image[x][y][0] = 0;
      output_image[x][y][1] = 0;
      output_image[x][y][2] = 0;
     }
    }
  }

}
void addCross(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y){
  for(int i = -10; i < 10; i++){
    for(int j = -1; j < 1; j++) {
      output_image[x+i][y+j][0] = 255;
      output_image[x+i][y+j][1] = 0;
      output_image[x+i][y+j][2] = 0;
      output_image[x+j][y+i][0] = 255;
      output_image[x+j][y+i][1] = 0;
      output_image[x+j][y+i][2] = 0;
    }
  }
  
}
  //Declaring the array to store the image (unsigned char = unsigned 8 bit)
  unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
  unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

//Main function
int main(int argc, char** argv)
{
  //argc counts how may arguments are passed
  //argv[0] is a string with the name of the program
  //argv[1] is the first command line argument (input image)
  //argv[2] is the second command line argument (output image)

  //Checking that 2 arguments are passed
  if (argc != 3)
  {
      fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
      exit(1);
  }

  printf("Example program - 02132 - A1\n");

  //Load image from file
  read_bitmap(argv[1], input_image);

  //Run inversion
  grayscale(input_image, output_image);

  binarize(input_image, output_image);
  for(int k = 0; k < 100;k++){
    int randx = rand()%900+25;
    int randy = rand()%900+25;
    addCross(output_image,randx,randy);
  }

  //Save image to file
  write_bitmap(output_image, argv[2]);

  printf("Done!\n");
  return 0;
}
