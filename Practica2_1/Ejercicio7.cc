#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>

const size_t MAXBUFFER = 80;
#define MAXLISTEN 3

std::condition_variable numCv;
std::mutex numMutex;
int numListen = 0;

class MyThread {
public:
	MyThread(int clientesd, int id) : clientesd_(clientesd), id_(id) {};
	
	void connections() {
		bool activo = true;

		while(activo) {
			char buffer[MAXBUFFER];

	                int bytes = recv(clientesd_, (void *) buffer, MAXBUFFER - 1, '\0');
			if (bytes < 1) {
				std::cout << "Conexión terminada\n";
				activo = false;
			}
			buffer[bytes] = '\0';

			send(clientesd_, (void *) buffer, bytes, 0);
		}

		close(clientesd_);

		numMutex.lock();
		numListen--;
		
		if(numListen < MAXLISTEN){
			numCv.notify_all();
		}

		numMutex.unlock();
	}

private:
	int clientesd_;
	int id_;
};

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Uso: ./Ejercicio7 [IP] [puerto]\n";
		return -1;
	}

	struct addrinfo hints;
	struct addrinfo * res;

	memset((void *) &hints, 0, sizeof(struct addrinfo));
	
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int rc = getaddrinfo(argv[1], argv[2], &hints, &res);
	
	if (rc != 0) {
		std::cerr << "[getaddrinfo]: " << gai_strerror(rc) << std::endl;
		return -1;
	}

	int sd = socket(res->ai_family, res->ai_socktype, 0);

	if (sd == -1) {
		std::cerr << "ERROR: [socket] no creado\n";
		return -1;
	}

	if (bind(sd, res->ai_addr, res->ai_addrlen) == -1) {
		std::cerr << "ERROR: fallo en la asignacion de addr a socket\n";
		return -1;
	}

	freeaddrinfo(res);
	
	if (listen(sd, MAXLISTEN) == -1) {
		std::cerr << "ERROR: fallo en listen\n";
		return -1;
	}
	
	//int id = 0;
	while(true) {
		struct sockaddr cliente;
		socklen_t clientelen = sizeof(struct sockaddr);

		{
			std::unique_lock<std::mutex> lck(numMutex);
		        while(numListen >= MAXLISTEN){
				std::cout << "No se puede conectar, el servidor está lleno...\n";
				numCv.wait(lck);
			}
		}

		int clientesd = accept(sd, (struct sockaddr *) &cliente, &clientelen);
		if (clientesd == -1) {
	                std::cerr << "ERROR: fallo en accept\n";
			close(sd);
	                return -1;
		}

		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];

		getnameinfo((struct sockaddr *) &cliente, clientelen, host, NI_MAXHOST,
			serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
		std::cout << "Conexión desde " << host << " " << serv << "\n";

		numMutex.lock();
		numListen++;
		numMutex.unlock();

		MyThread* thrd = new MyThread(clientesd, numListen);
		std::thread([&thrd]() { thrd->connections(); delete thrd; }).detach();
	}

	close(sd);
	return 0;
}
