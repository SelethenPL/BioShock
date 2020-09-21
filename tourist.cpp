#include <mpi.h>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <cstdio>
#include <ctime>
#include <csignal>
#include <iostream>
#include <unistd.h>		
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

#define INIT 100
#define PENDING 101
#define SUIT_CRITICAL 102
#define BOAT_CRITICAL 103
#define ON_BOAT 104


Tourist::Tourist(int costumes, int boats, int tourists) {
	
	size = MPI::COMM_WORLD.Get_size();
	rank = MPI::COMM_WORLD.Get_rank();
	
	srand(time(NULL) * rank);
	this->state = INIT;
	for (int i = 0; i < tourists; i++) {
		this->process_list.push_back(i);
	}
	this->costumes = costumes;
	this->have_costume = 0;
	
	this->clock = 0;
	this->event_mutex.lock();
	this->running = true;
	this->request_id = 0;
	this->is_last_process = true;
	
	// proces madka
	for (int i = 0; i < boats; i++) {
		s_boat* boat = new s_boat();
		boat->id = i;
		boat->capacity = 10 + 2 * i; //rand()%10+10; // 10-20
		boat->state = 0;
		boat->occupied = 0;
		boats_list.push_back(boat);
	}
	
	this->state = PENDING;
	log("Created a tourist!");
}

void Tourist::createMonitorThread() {
	new std::thread(&Tourist::monitorThread, this);
}

/** >>Done<<
 * Function creates and returns a new request with one value.
 */
s_request Tourist::create_request(int value) {
	s_request request;
	request.value = value;
	request.sender_id = rank;
	
	clock_mutex.lock();
	
	request.id = request_id++;
	request.clock = clock++;
	
	clock_mutex.unlock();
	
	return request;
}

/** >>Done<<
 * Function creates and returns a new request with two values.
 */
s_request Tourist::create_request(int value, int value2) {
	s_request request = create_request(value);
	request.value2 = value2;
	return request;
}


bool Tourist::handleResponse(s_request *result, int status) {
	bool resolved = true;

	// Switch-case dla zachowań niezależnych od stanu procesu
	switch(status){
		case CRUISE: 
		{
			for (auto boat : boats_list) {
				if(boat->id == result->value){
					boat->state = 1;
					if(result->value2 == rank){
						log("Process " + std::to_string(this->rank) + " has become a captain of the boat with ID " + std::to_string(result->value));
						this->is_captain = true;
					}
				}
			}
			break;
		}
		case CRUISE_END:
		{
			log("Received info about boat " + std::to_string(result->value) + " 's return");
			for (auto boat : boats_list) {
				if(boat->id == result->value){
					boat->occupied = 0;
					boat->state = 0;
					boat->tourists_list.clear();
				}
			}
			break;
		}
	}

	// Switch-case dla zachowań zależnych od stanu procesu
	switch(state) {
		case PENDING:
		{
			switch(status) {
				case COSTUME_REQ:
				{
					s_request ack = create_request(0);
					log("Sending Costume ACK to " + std::to_string(result->sender_id));
					MPI_Send(&ack, sizeof(s_request), MPI_BYTE, result->sender_id, COSTUME_ACK, MPI_COMM_WORLD);
					break;
				}
				
				case BOAT_REQ: // (-1, -1)
				{
					s_request ack = create_request(-1, -1);
					log("Sending Boat ACK to " + std::to_string(result->sender_id));
					MPI_Send(&ack, sizeof(s_request), MPI_BYTE, result->sender_id, BOAT_ACK, MPI_COMM_WORLD);
					break;
				}
				
				default:
				{
					resolved = false;
					break;
				}
			}
			break;
		}
		
		case SUIT_CRITICAL:
		{
			switch(status) {
				case COSTUME_ACK:
				{
					ack_mutex.lock();
					if (result->value == 0)
						ack++;
					log("Acquired Costume ACK from " + std::to_string(result->sender_id));
					if (ack + costumes >= process_list.size()) {
						setState(BOAT_CRITICAL);
						event_mutex.unlock();
					}
					ack_mutex.unlock();
					break;
				}
				
				case COSTUME_REQ:
				{
					if (result->clock < clock || (result->clock == clock && rank > result->id)) { // send ok
						s_request ack = create_request(0);
						log("Sending Costume ACK to " + std::to_string(result->sender_id));
						MPI_Send(&ack, sizeof(s_request), MPI_BYTE, result->sender_id, COSTUME_ACK, MPI_COMM_WORLD);
					}
					else {
						log("Costume request from " + std::to_string(result->sender_id) + " suspended");
						addToLamportVector(result);
					}
					break;
				}
				
				case BOAT_REQ:
				{
					s_request ack = create_request(-1, -1);
					log("Sending Boat ACK to " + std::to_string(result->sender_id));
					MPI_Send(&ack, sizeof(s_request), MPI_BYTE, result->sender_id, BOAT_ACK, MPI_COMM_WORLD);
					break;
				}
				
				default:
				{
					resolved = false;
					break;
				}
			}
			break;
		}
		
		case BOAT_CRITICAL:
		{
			switch(status) {
				case COSTUME_REQ:
				{
					log("Costume request from " + std::to_string(result->sender_id) + " suspended");
					addToLamportVector(result);
					break;
				}
				
				case BOAT_REQ:
				{
					if (result->clock > clock) { // dodaj do tablicy
						log("Boat request from " + std::to_string(result->sender_id) + " suspended");
						addToLamportVector(result);
					}
					else {
						s_request ack = create_request(-1, -1);
						log("Sending Boat ACK to " + std::to_string(result->sender_id));
						MPI_Send(&ack, sizeof(s_request), MPI_BYTE, result->sender_id, BOAT_ACK, MPI_COMM_WORLD);
					}
					break;
				}
				
				case BOAT_ACK:
				{
					log("Received Boat ACK from " + std::to_string(result->sender_id) + " with values " + std::to_string(result->value) + ", "  + std::to_string(result->value2));
					for (auto x : boats_list) {
						if (x->id == result->value) {
							x->occupied += result->value2;
							x->tourists_list.push_back(result->sender_id);
						}
					}
					ack_mutex.lock();
					ack++;

					// jeśli jest jakiś proces, który nie zajął miejsca na łodzi to znaczy, że będzie proces, który da radę
					// uruchomić rejs.
					if (result->value == -1){			
						this->is_last_process = false;
					}
					if (ack + 1 == process_list.size()) {
						event_mutex.unlock();
					}
					ack_mutex.unlock();
					break;
				}
				
				case CRUISE_END:
				{
					ack = 0;
					s_request boat_request = create_request(capacity);
					boat_request.clock = last_request_clock;
					broadcastRequest(&boat_request, BOAT_REQ);
					break;
				}
				
				default:
				{
					resolved = false;
					break;
				}
			}
			break;
		}
		
		case ON_BOAT:
		{
			switch(status) {
				case COSTUME_REQ:
				{
					log("Costume request from " + std::to_string(result->sender_id) + " suspended");
					addToLamportVector(result);
					break;
				}
				
				case BOAT_REQ:
				{
					s_request ack = create_request(boat_id, capacity); // (boat id, size)
					MPI_Send(&ack, sizeof(s_request), MPI_BYTE, result->sender_id, BOAT_ACK, MPI_COMM_WORLD);
					break;
				}

				case CRUISE:
				{
					event_mutex.unlock();
					break;
				}
				
				case CRUISE_END:
				{
					if(result->value == boat_id){
						event_mutex.unlock();
					}
					break;
				}
				
				default:
				{
					resolved = false;
					break;
				}
			}
			break;
		}
		
		default:
		{
			resolved = false;
			break;
		}
	}
	return resolved;
}


/** 
 * Function to manage received MPI messages.
 */
void Tourist::monitorThread() {
	while(running) {
		s_request result;
		MPI_Status status;
		MPI_Recv(&result, sizeof(s_request), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		result.type = status.MPI_TAG;
		
		handleResponse(&result, result.type);

		clock_mutex.lock();
		result.id = request_id;
		request_id++;
		clock = std::max(clock, result.clock) + 1;
		clock_mutex.unlock();
	}
}

void Tourist::setState(int value) {
	if (!lamport_vector.empty()) {
		std::vector<s_request>::iterator it;
		for (it = lamport_vector.begin(); it < lamport_vector.end(); it++) {
			bool x = handleResponse(&(*it), it->type); 
			// TODO: change status (status not present in scope)
			if (x) {
				removeFromLamportVector(it->id);
			}
		}
	}
		
	state = value;
}

void Tourist::runPerformThread() {
	while(running) {
		// perform all tasks
		// 1. initialization
		this->is_captain = false;
		this->is_last_process = true;
		// 2. wait to spawn
		
		std::this_thread::sleep_for(std::chrono::milliseconds((rand()%10000) + 2000));
		setState(SUIT_CRITICAL);
		
		log("Requesting costume...");
		// if number of costumes is greater than or equal the size of process_list, 
		// then each process can have its individual costume, therefore requesting is not 
		// necessary
		if(costumes < process_list.size()) {
			ack = 0;
			s_request costume_request = create_request(0);
			broadcastRequest(&costume_request, COSTUME_REQ);
			
			// 4. receive costume ACK for every proc -> get a costume
			
			event_mutex.lock();
		}

		log("Received a costume!");
		setState(BOAT_CRITICAL);
		have_costume = 1;
		
		// 5. sending boat request
		
		capacity = (rand()%5) + 2; // place needed on boat
		
		log("Requesting a boat");
		ack = 0;
		s_request boat_request = create_request(capacity);
		last_request_clock = boat_request.clock;
		for(auto boat: boats_list){
			boat->occupied = 0;
			boat->tourists_list.clear();
		}
		broadcastRequest(&boat_request, BOAT_REQ);
		
		// 6. receive boat ACK -> get a boat
		// for - czekanie na CRUISE_END, które wysyła jeszcze raz request o dostęp do łodzi (ze starym clockiem).
		for (;;) {
			event_mutex.lock();
			log("Checking available space on boats. Requested space: " + std::to_string(capacity));
			for (auto boat : boats_list){
				printf("Boat info - id: %d, state: %d capacity: %d, occupied: %d, travellers: %ld\n", boat->id, boat->state, boat->capacity, boat->occupied, boat->tourists_list.size());
			}
			bool found = false;
			for (auto boat : boats_list) {
				// jeśli łódź jest w porcie i da radę na nią wejść
				if (boat->state == 0 && boat->capacity >= capacity + boat->occupied) {
					boat_id = boat->id;
					boat->tourists_list.push_back(this->rank);
					boat->occupied += capacity;
					found = true;
					break;
				}
				// jeśli łódź nie wypłynęła, ale dany proces nie zmieści się na pokładzie
				else if (boat->state == 0) {
					if(boat->tourists_list.size() > 0){
						log("Boat no. " + std::to_string(boat->id) + " is starting its cruise!");
						s_request cruise_request = create_request(boat->id, boat->tourists_list[0]);
						broadcastRequest(&cruise_request, CRUISE);
					}
				}
			}
			if (found) {
				break;
			}
		}
		log("Boat received! Occupying " + std::to_string(capacity) + " on boat " + std::to_string(boat_id));
		setState(ON_BOAT);

		// Jeśli żaden proces nie nadchodzi to aktywuj rejsy na łodziach sam z siebie.
		if(this->is_last_process){
			log("Looking for boats to dispatch...");
			for(auto boat : boats_list){
				// Jeśli pusta łódź jest w porcie i lista turystów nie jest pusta
				if(boat->state == 0 && !boat->tourists_list.empty()){
					log("Boat no. " + std::to_string(boat->id) + " is starting its cruise!");
					s_request cruise_request = create_request(boat->id, boat->tourists_list[0]);
					broadcastRequest(&cruise_request, CRUISE);
					boat->state = 1;

					if(boat->tourists_list[0] == this->rank){
						log("I have become a captain of the boat with id " + std::to_string(boat->id) + "!");
						this->is_captain = true;
					}
				}
			}
		}

		// Get pointer to proper boat
		s_boat* currentBoat;
		for(auto boat : boats_list){
			if(boat_id == boat->id){
				currentBoat = boat;
			}
		}
		// 7. wait for cruise launch (given it hasn't already been launched)
		if(currentBoat){
			if(currentBoat->state == 0){
				event_mutex.lock();
			}
		}

		// 8. end the cruise
		if (is_captain) {
			log("Waiting to announce end of the cruise");
			int sleep_time = rand() % 3 + 3;
			sleep(sleep_time);
			s_request cruise_end_req = create_request(boat_id);
			for(auto boat : boats_list){
				if(boat->id == boat_id){
					boat->occupied = 0;
					boat->state = 0;
					boat->tourists_list.clear();
				}
			}
			broadcastRequest(&cruise_end_req, CRUISE_END);
			log("Boat " + std::to_string(boat_id) + " has finished its cruise.");
		}
		else {
			log("Waiting for the cruise to end");
			event_mutex.lock();
		}

		// 9. release the boat
		boat_id = -1;

		// 10. release costume and start over
		have_costume = 0;
		log("Loop finished");
		setState(PENDING);
	}
}

/** >>Done<<
 * Function broadcasting request to every proccess.
 */
void Tourist::broadcastRequest(s_request *request, int request_type) {
	for (int i = 0; i < size; i++)
		if (i != rank) {
			MPI_Send(request, sizeof(s_request), MPI_BYTE, i, request_type, MPI_COMM_WORLD);
			//printf("Broadcast %d to %d\n", rank, i);
		}
}

/** >>Done<< (in theory should be alright)
 * Adding cached request to local list of events.
 */
void Tourist::addToLamportVector(s_request *request) {
	lamport_mutex.lock();
	std::vector<s_request>::iterator it;
	
	for (it = lamport_vector.begin(); it < lamport_vector.end(); it++) {
		if (it->clock > request->clock) {
			break;
		}
		// setting priority for the same times
		if (it->clock == request->clock && it->sender_id > request->sender_id) {
			break;
		}
	}
	lamport_vector.insert(it, *request);
	lamport_mutex.unlock();
}

/** >>Done<< (as above)
 * Removing first spotted request in local list of events.
 */
void Tourist::removeFromLamportVector(int id) {
	lamport_mutex.lock();
	std::vector<s_request>::iterator it;
	
	for (it = lamport_vector.begin(); it < lamport_vector.end(); it++) {
		if (it->id == id) {
			lamport_vector.erase(it);
			break;
		}
	}
	lamport_mutex.unlock();
}

void Tourist::log(std::string message){
	printf("[ID: %d|Clock: %d|State: %d]: %s\n", rank, clock, state, message.c_str());
}