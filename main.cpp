#include <mpi.h>
#include "tourist.h"

#define COSTUMES 4
#define BOATS 2
#define TOURISTS 2

int main(int argc, char **argv) {
	printf("0. Zero\n");
	
	MPI::Init_thread(MPI_THREAD_MULTIPLE);
	
	printf("1. Init_thread\n");
	
	Tourist tourist(COSTUMES, BOATS, TOURISTS);
	printf("2. new tourist()\n");
	tourist.createMonitorThread();
	printf("3. createMonitorThread()\n");
	tourist.runPerformThread();
	printf("4. runPerformThread()\n");
	
	MPI::Finalize();
	
	printf("5. Finalize\n");
}