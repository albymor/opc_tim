#include <sys/time.h>
#include <stdint.h>

#ifdef _RPI_CCR
uint32_t GetTimeStamp();
#else
long GetTimeStamp();
#endif

void produce_measures_file(long * measures_vector, int n_measures, char *FILE_PATH);
