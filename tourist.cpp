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


Tourist::Tourist(int costumes, int boats, int tourists, int max_capacity) {
	
	size = MPI::COMM_WORLD.Get_size();
	rank = MPI::COMM_WORLD.Get_rank();
	
	// this.state = 0; // "PENDING"
	// this.process_list = null; // ???
	// this.costumes = costumes;
	// this.have_costume = 0;
	// this.boats_list = null; // ???
	// need to change for pointer, to let every process use the same?
	
	// this.clock = 0;
	// this.clock_mutex = new mutex; // ???
	// this.running = 0; // ???
}

void Tourist::createMonitorThread() {
	new std::thread(&Tourist::monitorThread, this);
}

/** TODO: add proper actions to tags
 * Function to manage received MPI messages.
 */
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


/** >>Done<<
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

/** >>Done<<
 * Function broadcasting request to every proccess.
 */
void Tourist::broadcastRequest(s_request *request, int request_type) {
	for (int i = 0; i < size; i++)
		if (i != rank)
			MPI_Send(request, sizeof(s_request), MPI_BYTE, i, request_type, MPI_COMM_WORLD);
}

/** >>Done<< (in theory should be alright)
 * Adding request to local list of events.
 */
void Tourist::addToLamportVector(s_lamport_vector *lamport, s_request *request) {
	vector -> edit_mutex.lock();
	std::vector<s_request>::iterator it;
	
	for (it = lamport -> vector.begin(); it < lamport -> vector.end(); it++) {
		if (it->clock > request->clock) {
			break;
		}
		// setting priority for the same times
		if (it->clock == request->clock && it->sender_id > request->sender_id) {
			break;
		}
	}
	vector -> vector.insert(it, *request);
	vector -> edit_mutex.unlock();
}

/** >>Done<< (as above)
 * Removing first spotted request in local list of events.
 */
void Tourist::removeFromLamportVector(s_lamport_vector *lamport, int sender) {
	vector -> edit_mutex.lock();
	std::vector<s_request>::iterator it;
	
	for (it = lamport -> vector.begin(); it < lamport -> vector.end(); it++) {
		if (it->sender_id == sender) {
			vector -> vector.erase(it);
			break;
		}
	}
	vector -> edit_mutex.unlock();
}

