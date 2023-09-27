// To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
// To run (linux/mac): ./main.out example.bmp example_inv.bmp

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"

// Function to invert pixels of an image (negative)
void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
{
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
/*
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
*/
void addCross(unsigned char outimage[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y)
{
  for (int i = -5; i < 6; i++)
  {
    for (int j = -1; j < 1; j++)
    {
      outimage[x + i][y + j][0] = 255;
      outimage[x + i][y + j][1] = 0;
      outimage[x + i][y + j][2] = 0;
      outimage[x + j][y + i][0] = 255;
      outimage[x + j][y + i][1] = 0;
      outimage[x + j][y + i][2] = 0;
    }
  }
}
void flattenGrayscale(unsigned char image_array[BMP_WIDTH][BMP_HEIGTH], unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      image_array[x][y] = input_image[x][y][0]; // image already grayscale
    }
  }
}
void binarize(unsigned char image_array[BMP_WIDTH][BMP_HEIGTH])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      if (image_array[x][y] > THRESHOLD_FB)
      {
        image_array[x][y] = 0xFF;
      }
      else
      {
        image_array[x][y] = 0x00;
      }
    }
  }
}
// Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char image_array[BMP_WIDTH][BMP_HEIGTH];
unsigned char output_array[BMP_WIDTH][BMP_HEIGTH];
unsigned char (*in_image)[BMP_HEIGTH] = image_array;
unsigned char (*out_image)[BMP_HEIGTH] = output_array; // dont worry we reverse it
unsigned char cell_array[BMP_WIDTH][BMP_HEIGTH];
unsigned int count = 0;
unsigned int step = 0;
unsigned char neighbours = 0x00;
unsigned char eroded = 1;
unsigned char hasEdgePixels = 0x00;
unsigned char offset = 5;
void inflate(unsigned char image_array[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image[x][y][0] = image_array[x][y];
      output_image[x][y][1] = output_image[x][y][0];
      output_image[x][y][2] = output_image[x][y][1];
    }
  }
}
unsigned char hasCounted = 0;
const int excframe = 10;
void detect()
{
  for (int x = 0; x < BMP_WIDTH - excframe - 1; x++)
  {
    for (int y = 0; y < BMP_HEIGTH - excframe - 1; y++)
    {
      hasEdgePixels = 0x00;

      for (int i = 0; i < excframe; i++)
      {
        if (in_image[x + i][y] > 0 || in_image[x + i][y + excframe - 1] > 0 || in_image[x][y + i] > 0 || in_image[x + excframe - 1][y + i] > 0)
        {
          hasEdgePixels = 0xFF;
        }
      }
      if (hasEdgePixels == 0x00)
      {

        hasCounted = 0;
        for (int x1 = 1; x1 < excframe - 2; x1++)
        {
          for (int y1 = 1; y1 < excframe - 2; y1++)
          {
            if (hasCounted == 0 && in_image[x + x1][y + y1] > 0)
            {
              count++;
              hasCounted = 1;
              cell_array[x + 6][y + 6] = 0xFF;
            }
          }
        }
        if (hasCounted > 0)
        {
          for (int x2 = 1; x2 < excframe - 1; x2++)
          {
            for (int y2 = 1; y2 < excframe - 1; y2++)
            {
              in_image[x + x2][y + y2] = 0x00;
              out_image[x + x2][y + y2] = 0x00;
            }
          }
        }
      }
    }
  }
}
void Detect2()
{
  for (int x = offset; x < BMP_WIDTH + offset - excframe; x++)
  {
    for (int y = offset; y < BMP_HEIGTH + offset - excframe; y++)
    {
      if (in_image[x][y] > 0)
      {

        hasEdgePixels = 0x00;
        for (int edge = 0; edge < excframe; edge++)
        {
          if (in_image[x - offset][y - offset + edge] > 0 || in_image[x - offset + edge][y - offset] > 0 || in_image[x + excframe - offset][y - offset + edge] > 0 || in_image[x - offset + edge][y + excframe - offset] > 0)
          {
            hasEdgePixels = 0xff;
          }
        }
        if (hasEdgePixels == 0)
        {
          count++;
          cell_array[x + 2][y + 2] = 0xff;
          for (int wipe = 0; wipe < excframe; wipe++)
          {
            for (int xframe = 0; xframe < excframe; xframe++)
            {
              for (int yframe = 0; yframe < excframe; yframe++)
              {
                in_image[x - offset + xframe][y - offset + yframe] = 0x00;
                out_image[x - offset + xframe][y - offset + yframe] = 0x00;
              }
            }
          }
        }
      }
    }
  }
}
int erode(unsigned char image_array[BMP_WIDTH][BMP_HEIGTH])
{

  in_image = output_array;
  out_image = image_array;
  while (eroded > 0)
  {
    step++;
    eroded = 0;
    if (in_image == output_array)
    {
      in_image = image_array;
      out_image = output_array;
    }
    else
    {
      in_image = output_array;
      out_image = image_array;
    }

    for (int x = 0; x < BMP_WIDTH; x++)
    {
      for (int y = 0; y < BMP_HEIGTH; y++)
      {
        if ((x == 0 || x == (BMP_WIDTH - 1)) || (y == 0 || y == (BMP_HEIGTH - 1)))
        {
          out_image[x][y] = 0x00;
        }
        else if (in_image[x][y] > 0)
        {
          neighbours = 0;
          if (in_image[x + 1][y] > 0)
          {
            neighbours++;
          }
          if (in_image[x - 1][y] > 0)
          {
            neighbours++;
          }
          if (in_image[x][y + 1] > 0)
          {
            neighbours++;
          }
          if (in_image[x][y - 1] > 0)
          {
            neighbours++;
          }

          if (neighbours == 4)
          {
            out_image[x][y] = 0xFF;
          }
          else
          {
            out_image[x][y] = 0x00;
            eroded = 1;
          }
        }
        else
        {
          out_image[x][y] = 0x00;
        }
      }
    }
    // detect();
    Detect2();
    char basename[] = "test/step";
    char filename[20];
    inflate(in_image, output_image);
    snprintf(filename, sizeof(filename), "%s%d.bmp", basename, step);
    write_bitmap(output_image, filename);
  }
}

// Main function
int main(int argc, char **argv)
{
  // argc counts how may arguments are passed
  // argv[0] is a string with the name of the program
  // argv[1] is the first command line argument (input image)
  // argv[2] is the second command line argument (output image)

  // Checking that 2 arguments are passed
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
    exit(1);
  }

  printf("Example program - 02132 - A1\n");

  // Load image from file
  read_bitmap(argv[1], input_image);

  flattenGrayscale(image_array, input_image);
  binarize(image_array);
  inflate(image_array, output_image);

  erode(image_array);
  inflate(image_array, output_image);
  // Save image to file
  int crosscount = 0;
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      if (cell_array[x][y] > 0)
      {
        addCross(input_image, x, y);
        crosscount++;

        printf("%d %d \n", x, y);
      }
    }
  }
  write_bitmap(input_image, argv[2]);
  printf("Done!\n");
  printf("Count: %d", count);
  return 0;
}
