#ifndef PLEASEWORK_TOURIST_H
#define PLEASEWORK_TOURIST_H


#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

typedef struct {
	int id; // 1) numer na liście
	int capacity; // 2) pojemność
	int occupied; // 2.1) zajmowana pojemność
	int state; // 3) stan łodzi
	std::vector<int> tourists_list; // 4) lista pasażerów
} s_boat;

typedef struct {
	int id;
	int type; // 1) typ wiadomości
	int value; // 2) zawartość wiadomości
	int value2; // cd.
	int sender_id; // 3) numer id obiektu wysyłającego
	int clock;
} s_request;

class Tourist {
	
	// 1) Stan obecnego procesu
	int state;
	
	// 2) ID obecnego procesu
	int size;
	int rank;
	
	// 3) Lista ID wszystkich procesów
	std::vector<int> process_list;
	
	// 4) Liczba dostępnych strojów
	int costumes;
	
	// 5) Stan posiadania stroju (posiada / nie posiada)
	int have_costume;
	
	// 6) Lista łodzi
	std::vector<s_boat*> boats_list;
	
	// 7) Wstrzymanie żądania
	
	
	// 8) Stan zegara Lamporta
	int clock;
	std::mutex clock_mutex;
	int ack;
	int on_boat_ack;
	std::mutex ack_mutex;
	std::mutex event_mutex;
	
	// *) Lista zegarów lamporta
	std::vector<s_request> lamport_vector;
	std::mutex lamport_mutex;
	int request_id;
	
	// Wartości klienta
	int capacity;
	int boat_id;
	int last_request_clock;
	
	bool is_last_process;
	bool is_captain;
	bool running;
	
	void monitorThread();
	void broadcastRequest(s_request *request, int request_type);
	s_request create_request(int value);
	s_request create_request(int value, int value2);
	
	void setState(int value);
	
	bool handleResponse(s_request *result, int status, bool isExternal);
	void addToLamportVector(s_request *request);
	void removeFromLamportVector(int sender);
	void finish_cruise(int sig);
	void log(std::string msg);
	
public:
	Tourist(int costumes, int boats, int tourists);
	void createMonitorThread();
	void runPerformThread();
};

#endif