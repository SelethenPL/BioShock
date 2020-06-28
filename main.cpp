#include <mpi.h>
#include "tourist.h"

#define COSTUMES 1
#define BOATS 1
#define TOURISTS 1
#define MAX_CAPACITY 5

typedef struct {
	int id; // 1) numer na liście
	int capacity; // 2) pojemność
	int occupied; // 2.1) zajmowana pojemność
	int state; // 3) stan łodzi
	std::vector<int> tourists_list; // 4) lista pasażerów
} s_boat;

typedef struct {
	int id;
	int value; // 1) typ wiadomości
	int value2; // 2) zawartość wiadomości
	int sender_id; // 3) numer id obiektu wysyłającego
	int clock;
} s_request;


int main(int argc, char **argv) {
	MPI::Init_thread(MPI_THREAD_MULTIPLE);
	
	Tourist tourist(COSTUMES, BOATS, TOURISTS, MAX_CAPACITY);
	tourist.createMonitorThread();
	tourist.runPerformedThread();
	
	MPI::Finalize();
}