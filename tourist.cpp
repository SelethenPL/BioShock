// running as boolean in class?
#include <mpi.h>
#include <chrono>
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

#define PENDING 100
#define SUIT_CRITICAL 101
#define BOAT_CRITICAL 102
#define ON_BOAT 103

Tourist::Tourist(int costumes, int boats, int tourists, int max_capacity) {
	
	size = MPI::COMM_WORLD.Get_size();
	rank = MPI::COMM_WORLD.Get_rank();
	
	// this.state = PENDING;
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
		
		// TODO: Add interactions
		if (state == PENDING) {
			switch(status.MPI_TAG) {
				case COSTUME_REQ:
					addToLamportVector(&costume_queue, &result);
					s_request ack = create_request(0);
					MPI_Send(&ack, sizeof(s_request), MPI_BYTE, status.sender_id, COSTUME_ACK, MPI_COMM_WORLD);
					break;
					
				case BOAT_REQ: // (-1, -1)
					addToLamportVector(&boat_queue, &result);
					s_request ack = create_request(-1);
					MPI_Send(&ack, sizeof(s_request), MPI_BYTE, status.sender_id, BOAT_ACK, MPI_COMM_WORLD);
					break;
					
				default:
					break;
			}		
		}
		if (state == SUIT_CRITICAL) {
			switch(status.MPI_TAG) {
				case COSTUME_ACK:
					// dłuższy opis
					// ?????
					ack_mutex.lock();
					ack++;
					ack_mutex.unlock();
					break;
					
				case COSTUME_REQ:
					// send costume_ack (priorytety)
					break;
					
				case BOAT_REQ: // (-1, -1)
					addToLamportVector(&boat_queue, &result);
					s_request ack = create_request(-1);
					MPI_Send(&ack, sizeof(s_request), MPI_BYTE, status.sender_id, BOAT_ACK, MPI_COMM_WORLD);
					break;
					
				default:
					break;
			}
		}
		if (state == BOAT_CRITICAL) {
			switch(status.MPI_TAG) {
				case COSTUME_ACK:
					// stop request til costume is released
					// ????
					break;
				case COSTUME_REQ:
					// stop request (priority of time)
					// else add to lamport_vector
					break;
				case BOAT_REQ:
					// update array of boat state
					break;
				default:
					break;
			}
		}
		if (state == ON_BOAT) {
			switch(status.MPI_TAG) {
				case COSTUME_REQ:
					// stop request til costume is released
					// ????
					break;
				case BOAT_REQ:
					addToLamportVector(&boat_queue, &result);
					int a1 = 0; // boat id
					int a2 = 0; // size requested for a boat
					s_request ack = create_request(a1, a2); // (boat id, size)
					MPI_Send(&ack, sizeof(s_request), MPI_BYTE, status.sender_id, BOAT_ACK, MPI_COMM_WORLD);
					break;
				case CRUISE_END:
					// change statuses of boat
					// state = PENDING;
					break;
				default:
					break;
			}
		}
	}
}


/** >>Done<<
 * Function creates and returns a new request with one value.
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
/** >>Done<<
 * Function creates and returns a new request with two values.
 */
s_request Toutist::create_request(int value, int value2) {
	s_request request = create_request(value);
	request.value2 = value2;
	
	return request;
}


void Tourist::runPerformThread() {
	while(running) {
		// perform all tasks
		// 1. initialization
		
		// 2. wait to spawn
		
		std::this_thread::sleep_for(std::chrono::milliseconds((rand()%5000) + 2000));
		state = SUIT_CRITICAL;
		
		// 3. sending costume request
		
		printf("[Rank: %d|Clock: %d]: %s\n", rank, clock, "Request for costume");
		ack = 0;
		s_request costume_request = create_request(0);
		broadcastRequest(&costume_request, COSTUME_REQ);
		
		// 4. receive costume ACK for every proc -> get a costume
		
		while(true) {
			// algorithm for costume request
			ack_mutex.lock();
			
			// check for costumes variable
			if (ack > 10) {
				printf("[Rank: %d|Clock: %d]: %s\n", rank, clock, "Costume received!");
				state = BOAT_CRITICAL;
				have_costume = 1;
				break;
			};
			
			ack_mutex.unlock();
		}
		
		// 5. sending boat request
		
		int x = (rand()%5000) + 2000; // place needed on boat
		
		printf("[Rank: %d|Clock: %d]: %s\n", rank, clock, "Request for boat");
		ack = 0;
		s_request boat_request = create_request(x);
		broadcastRequest(&boat_request, BOAT_REQ);
		
		// 6. receive boat ACK
		
		while(true) {
			ack_mutex.lock();
			
			// TODO: update with boats
			if (ack > 10) {
				printf("[Rank: %d|Clock: %d]: %s\n", rank, clock, "Boat received!");
				printf("Left %d space on boat id: %d.", 0, 0); 
				state = ON_BOAT;
				break;
			}
			
			ack_mutex.unlock();
		}
		
		// 7. iterate via all boats -> get a boat
		
		// 8. cruise
		// 9. release the boat
		// 10. release costume and start over
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
	lamport -> edit_mutex.lock();
	std::vector<s_request>::iterator it;
	
	for (it = lamport -> lamport_vector.begin(); it < lamport -> lamport_vector.end(); it++) {
		if (it->clock > request->clock) {
			break;
		}
		// setting priority for the same times
		if (it->clock == request->clock && it->sender_id > request->sender_id) {
			break;
		}
	}
	lamport -> lamport_vector.insert(it, *request);
	lamport -> edit_mutex.unlock();
}

/** >>Done<< (as above)
 * Removing first spotted request in local list of events.
 */
void Tourist::removeFromLamportVector(s_lamport_vector *lamport, int sender) {
	lamport -> edit_mutex.lock();
	std::vector<s_request>::iterator it;
	
	for (it = lamport -> lamport_vector.begin(); it < lamport -> lamport_vector.end(); it++) {
		if (it->sender_id == sender) {
			lamport -> lamport_vector.erase(it);
			break;
		}
	}
	lamport -> edit_mutex.unlock();
}

