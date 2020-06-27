// running as boolean in class?
#include <mpi.h>
#include <thread>
#include "tourist.h"

	// Sent once by main thread
#define LAUNCH 0
	// Costume request message
#define COSTUME_REQ 1 // SREQ
	// Boat request message
#define BOAT_REQ 2 // BREQ
	// Costume accept message
#define COSTUME_ACK 3
	// Boat accept message
#define BOAT_ACK 4
	// Sent by process when can't fit in boad
	// Boat sent to Cruise
#define CRUISE 5
	// Sent by process when boat arrives in harbor
	// Releases all the resources
#define CRUISE_END 6

/*
	// 3) Lista ID wszystkich procesów
	std::vector<int> process_list;
	
	// 4) Liczba dostępnych strojów
	int costumes;
	
	// 5) Stan posiadania stroju (posiada / nie posiada)
	bool have_costume;
	
	// 6) Lista łodzi
	std::vector<s_boat> boats_list;
	
	// 8) Stan zegara Lamporta
	int clock;
	std::mutex clock_mutex;
	bool running;
	
*/


Tourist::Tourist(int costumes, int boats, int tourists, int max_capacity) {
	//state = "PENDING";
	
	size = MPI::COMM_WORLD.Get_size();
	rank = MPI::COMM_WORLD.Get_rank();
	
	// srand(time(nullptr) + rank);
	
	// std::vector<int> process_list;
	
	this.costumes = costumes;
	
	have_costume = 0;
	
	boats_list = null; // zastanowić się nad dodaniem wskaźnika
	
	clock = 0;
	
}

void Tourist::createMonitorThread() {
	new std::thread(&Tourist::monitorThread, this);
}

void Tourist::monitorThread() {
	while(running) {
		s_request result;
		MPI_Status status;
		MPI_Recv(&result, sizeof(s_request), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		
		clock_mutex.lock();
		clock = std::max(clock, result.clock) + 1;
		clock_mutex.unlock();
		
		switch(status.MPI_TAG) {
			case LAUNCH:
				// status: INIT -> PENDING
				break;
				
			case COSTUME_REQ:
				
				break;
				
			case COSTUME_ACK:
				
				break;
				
			case BOAT_REQ:
			
				break;
			
			case BOAT_ACK:
			
				break;
			
			case CRUISE:
			
				break;
			
			case CRUISE_END:
			
				break;
			
			default:
				break;
			
			
		}
	}
}


/** ****DONE****
 * Function creates and returns a new request.
 */
s_request Tourist::create_request(int value) {
	s_request request;
	request.value = value;
	request.sender_id = rank;
	
	clock_mutex.lock();
	clock++;
	clock_mutex.unlock();
	
	return request;
}


void Tourist::runPerformThread() {
	while(running) {
		// perform all tasks
		// 1. wait to spawn
		
		// 2. Require an costume
		
		s_request costume_request = create_request(x);
		// wait_for_ack = request.clock;
		
		
		// 3. require an boat
		// 4. cruise (?)
		// 5. end cruise and release resources
	}
}


void Tourist::broadcastRequest(s_request *request, int request_type) {
	for (int i = 0; i < size; i++)
		if (i != rank)
			MPI_Send(request, sizeof(s_request), MPI_BYTE, i, request_type, MPI_COMM_WORLD);
}

