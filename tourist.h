
#include <vector>
#include <mutex>


typedef struct {
	int id; // 1) numer na liście
	int capacity; // 2) pojemność
	int state; // 3) stan łodzi
	std::vector<int> tourists_list; // 4) lista pasażerów
} s_boat;

typedef struct {
	int value; // 1) typ wiadomości
	int value2; // 2) zawartość wiadomości
	int sender_id; // 3) numer id obiektu wysyłającego
	int clock;
} s_request;

typedef struct {
	std::mutex edit_mutex;
	std::mutex action_mutex;
	std::vector<s_request> lamport_vector;
} s_lamport_vector;

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
	
	bool running;
	
	void monitorThread();
	void broadcastRequest(s_request *request, int request_type);
	s_request create_request(int value);
	
	void addToLamportVector();
	void removeFromLamportVector();
	
public:
	Tourist(int costumes, int boats, int tourists, int max_capacity);
	void createMonitorThread();
	boid runPerformThread();
	
}