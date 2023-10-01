// To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
// To run (linux/mac): ./main.out example.bmp example_inv.bmp

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"
#include <time.h>
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char image_array[BMP_WIDTH][BMP_HEIGTH];
unsigned char output_array[BMP_WIDTH][BMP_HEIGTH];
unsigned char (*in_image)[BMP_HEIGTH] = image_array;
unsigned char (*out_image)[BMP_HEIGTH] = output_array; // these two are reversed for each iteration in erosion
unsigned char cell_array[BMP_WIDTH][BMP_HEIGTH];       // the binary representation (0x00 and 0xFF) of the image without color channels
unsigned int count = 0;                                // Detect2(), how many cells we have counted
unsigned int step = 0;                                 // erode(), how many steps of erosion we have done, used for the images showing erosion
unsigned char neighbours = 0x00;                       // erode(), represents the neighbours of a cell
unsigned char eroded = 1;                              // erode(), used to see if the current cell already has been eroded
unsigned char hasEdgePixels = 0x00;                    // Detect2(), if this is larger than 0x00, then there has been detected a cell in the detection frame
unsigned char offset = 5;                              // how much we want to offset the (0,0) coordinate of the detection frame up and left, (x-offset,y-offset).
unsigned int threshold = THRESHOLD_FB;                 // the default value of the threshold without otsus method as declared in the header
unsigned int maxIntensity = 220;                       // maximum intensity for otsus method
unsigned char hasCounted = 0;                          // detect(), dont count the cells in the frame more than once
const int excframe = 10;                               // Detect2(), the size of the exclusion frame
unsigned char debug = 0x00;                            // output debug stuff
unsigned char timeOut = 0xFF;                          // output run time

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
      if (image_array[x][y] > threshold)
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
void binarize2(unsigned char image_array[BMP_WIDTH][BMP_HEIGTH])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      if (input_image[x][y][0] > threshold)
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

void otsu(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
{
  int histogram[256]; // otsus method for calculating threshold
  double sum = 0.0;
  double sumB = 0.0;
  double var = 0;
  double var_max = 0;
  double mean1 = 0.0;
  double mean2 = 0.0;
  int q1 = 0;
  int q2 = 0;
  // clean histograms memory, maybe redundandt
  for (int i = 0; i < 256; i++)
  {
    histogram[i] = 0;
  }
  // now we count how many pixels there are of each intensity(as images are grayscale, we can just choose one of the three channels)
  for (int row = 0; row < BMP_WIDTH; row++)
  {
    for (int col = 0; col < BMP_HEIGTH; col++)
    {
      histogram[input_image[row][col][0]]++;
    }
  }
  // now otsus method
  for (int i = 0; i <= maxIntensity; i++)
  {
    sum += i * histogram[i];
  }
  for (int t = 0; t <= maxIntensity; t++)
  {
    q1 += histogram[t];
    if (q1 == 0)
    {
      continue;
    }

    q2 = (950 * 950) - q1;
    sumB += t * histogram[t];
    mean1 = sumB / q1;
    mean2 = (sum - sumB) / q2;
    var = q1 * q2 * ((mean1 - mean2) * (mean1 - mean2));
    if (var > var_max)
    {
      threshold = t;
      var_max = var;
    }
  }
}
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
    if (debug > 0x00)
    { // debug to see the steps of erosion
      char basename[] = "test/step";
      char filename[20];
      inflate(in_image, output_image);
      snprintf(filename, sizeof(filename), "%s%d.bmp", basename, step);
      write_bitmap(output_image, filename);
    }
  }
}

// Main function
int main(int argc, char **argv)
{
  clock_t start, end;
  double cpu_time_used;
  start = clock();
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

  // Load image from file
  read_bitmap(argv[1], input_image);
  otsu(input_image);
  flattenGrayscale(image_array, input_image);
  binarize(image_array);
  erode(image_array);
  inflate(image_array, output_image);
  // add crosses
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      if (cell_array[x][y] > 0)
      {
        addCross(input_image, x, y);

        printf("%d %d \n", x, y);
      }
    }
  }
  write_bitmap(input_image, argv[2]);
  printf("Done!\n");
  printf("Count: %d", count);
  end = clock();
  cpu_time_used = end - start;
  if (timeOut > 0)
  {
    printf("Total time used: %f ms\n", cpu_time_used * 1000.0 / CLOCKS_PER_SEC);
  }
  return 0;
}
