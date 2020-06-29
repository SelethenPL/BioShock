#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

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
	std::vector<s_boat> boats_list;
	
	// 7) Wstrzymanie żądania
	
	
	// 8) Stan zegara Lamporta
	int clock;
	std::mutex clock_mutex;
	int ack;
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
	
	bool is_captain;
	bool running;
	
	void monitorThread();
	void broadcastRequest(s_request *request, int request_type);
	s_request create_request(int value);
	s_request create_request(int value, int value2);
	
	void setState(int value);
	
	void handleResponse(s_request *request, MPI_Status status);
	void finishCruise(int sig);
	void addToLamportVector(s_request *request);
	void removeFromLamportVector(int sender);
	
public:
	Tourist(int costumes, int boats, int tourists, int max_capacity);
	void createMonitorThread();
	void runPerformThread();
}