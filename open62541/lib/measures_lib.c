#include "measures_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef _RPI_CCR
#pragma message "REMEMBER TO ENABLE CCR KERNEL MODULE"
static inline uint32_t GetTimeStamp(void)
{
  uint32_t cc = 0;
  __asm__ volatile ("mrc p15, 0, %0, c9, c13, 0":"=r" (cc));
  return cc;
}
#else
long GetTimeStamp() { //timestamp in us
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(long)1000000+tv.tv_usec;
}
#endif


void produce_measures_file(long * measures_vector, int n_measures, char *FILE_PATH){ //file delle misure in us
	FILE * file_pointer = fopen (FILE_PATH, "w");
	int i;
	for(i = 0; i < n_measures - 1; i++){
		fprintf(file_pointer, "%s %ld", ", ", *(measures_vector++));
	}
	fclose(file_pointer);
}

