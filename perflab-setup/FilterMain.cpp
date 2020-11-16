#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"

using namespace std;

#include "rdtsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  } else {
    cerr << "Bad input in readFilter:" << filename << endl;
    exit(-1);
  }
}

//Almost all changes here

double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output)
{

  long long cycStart, cycStop;

  cycStart = rdtscll();

  output -> width = input -> width;
  output -> height = input -> height;
//Make local variables out of function calls, so there's less loop overhead for bounds checking 
int fsize= filter -> getSize();
int height= input -> height-1;
int width= input -> width-1;
int div= filter -> getDivisor();

//create accumulators to greatly increase paralellism. 
int v1, v2, v3, v4, v5, v6, v7, v8, v9;
    
//locally store the filter matrix, so that filter->get function calls aren't done in larger loop.
int filt[fsize][fsize]; //always 3x3, but this works for any size.
for (int i = 0; i < fsize; i++) {
    for (int j = 0; j < fsize; j++) {
          filt[i][j]=filter->get(i,j);
          }
    }

//Eliminate the two filter loops by knowing what values of the filter apply to what values of row and col.
//Eliminate the plane loop by doing all plane calculations in each iteration.
//reorder loops so they are in order of access, and easier to understand.
//This is loop unrolling

//Using openmp allows easy creation of a multi-threaded program.
//I had to change makefile to CXXFLAGS= -g -O4 -fno-omit-frame-pointer -Wall -fopenmp
//pragma omp parallel for divides loop iterations between the parallel threads, better using multiple cores. 
#pragma omp parallel for
for(int row = 1; row < height; row++){ //change to row++ or col++
    for(int col = 1; col < width; col++){ 

//output -> color[plane][row][col] = 0; //Unnecessary
          
//v1 to v3 accumulates all the values of plane 0, with the filter applied to the corresponding input pixels.
//v4 to v6 accumulates all the values of plane 1, with the filter applied to the corresponding input pixels.
//v7 to v9 accumulates all the values of plane 2, with the filter applied to the corresponding input pixels.
//each var when summed with itself holds the value for a particular column.

//do all calculations for row-1
v1=(input->color[0][row-1][col-1]*filt[0][0]);
v2=(input->color[0][row-1][col]*filt[0][1]);
v3=(input->color[0][row-1][col+1]*filt[0][2]);
v4=(input->color[1][row-1][col-1]*filt[0][0]);
v5=(input->color[1][row-1][col]*filt[0][1]);
v6=(input->color[1][row-1][col+1]*filt[0][2]);
v7=(input->color[2][row-1][col-1]*filt[0][0]);
v8=(input->color[2][row-1][col]*filt[0][1]);
v9=(input->color[2][row-1][col+1]*filt[0][2]);

//do all calculations for row
v1+=(input->color[0][row][col-1]*filt[1][0]);
v2+=(input->color[0][row][col]*filt[1][1]);
v3+=(input->color[0][row][col+1]*filt[1][2]);
v4+=(input->color[1][row][col-1]*filt[1][0]);
v5+=(input->color[1][row][col]*filt[1][1]);
v6+=(input->color[1][row][col+1]*filt[1][2]);
v7+=(input->color[2][row][col-1]*filt[1][0]);
v8+=(input->color[2][row][col]*filt[1][1]);
v9+=(input->color[2][row][col+1]*filt[1][2]);

//do all calculations for row+1
v1+=(input->color[0][row+1][col-1]*filt[2][0]);
v2+=(input->color[0][row+1][col]*filt[2][1]);
v3+=(input->color[0][row+1][col+1]*filt[2][2]);
v4+=(input->color[1][row+1][col-1]*filt[2][0]);
v5+=(input->color[1][row+1][col]*filt[2][1]);
v6+=(input->color[1][row+1][col+1]*filt[2][2]);
v7+=(input->color[2][row+1][col-1]*filt[2][0]);
v8+=(input->color[2][row+1][col]*filt[2][1]);
v9+=(input->color[2][row+1][col+1]*filt[2][2]);
        
//The final pixel is computed from the sum of the 3x3 filter applied to 3x3 input pixels for each color
 
//change to conditional notation, which is supposed to be faster.
//only divide by divisor if it is greater than 1.
div>1? (v3=(v1+v2+v3)/div) : (v3=v1+v2+v3);
div>1? (v6=(v4+v5+v6)/div) : (v6=v4+v5+v6);
div>1? (v9=(v7+v8+v9)/div) : (v9=v7+v8+v9);

v3<0? (v3=0):(v3=v3);
v6<0? (v6=0):(v6=v6);
v9<0? (v9=0):(v9=v9);

v3>255? (v3=255):(v3=v3);
v6>255? (v6=255):(v6=v6);
v9>255? (v9=255):(v9=v9);

output->color[0][row][col]=v3;
output->color[1][row][col]=v6;
output->color[2][row][col]=v9;
      }
    }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}

//OLD inefficient code. Disregard. 
/*
double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output)
{

  long long cycStart, cycStop;

  cycStart = rdtscll();

  output -> width = input -> width;
  output -> height = input -> height;


  for(int col = 1; col < (input -> width) - 1; col = col + 1) {
    for(int row = 1; row < (input -> height) - 1 ; row = row + 1) {
      for(int plane = 0; plane < 3; plane++) {

	output -> color[plane][row][col] = 0;

	for (int j = 0; j < filter -> getSize(); j++) {
	  for (int i = 0; i < filter -> getSize(); i++) {	
	    output -> color[plane][row][col]
	      = output -> color[plane][row][col]
	      + (input -> color[plane][row + i - 1][col + j - 1] 
		 * filter -> get(i, j) );
	  }
	}
	
	output -> color[plane][row][col] = 	
	  output -> color[plane][row][col] / filter -> getDivisor();

	if ( output -> color[plane][row][col]  < 0 ) {
	  output -> color[plane][row][col] = 0;
	}

	if ( output -> color[plane][row][col]  > 255 ) { 
	  output -> color[plane][row][col] = 255;
	}
      }
    }
  }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}
*/