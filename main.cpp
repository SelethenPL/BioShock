#include <mpi.h>
#include "tourist.h"

#define COSTUMES 1
#define BOATS 1
#define TOURISTS 1
#define MAX_CAPACITY 5


int main(int argc, char **argv) {
	MPI::Init_thread(MPI_THREAD_MULTIPLE);
	
	
	
	Tourist tourist(COSTUMES, BOATS, TOURISTS, MAX_CAPACITY);
	tourist.createMonitorThread();
	tourist.runPerformedThread();
	
	MPI::Finalize();
}