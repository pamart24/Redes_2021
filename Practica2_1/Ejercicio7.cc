#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <thread>

const size_t MAXBUFFER = 80;
const int MAXLISTEN = 5;

class MyThread {
public:
	MyThread(int clientesd) : clientesd_(clientesd) {};
	
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
	}

private:
	int clientesd_;
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
	hints.ai_socktype = SOCK_DGRAM;

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
	
	if (listen(sd, 1) == -1) {
		std::cerr << "ERROR: fallo en listen\n";
		return -1;
	}

	while(true) {
		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];

		struct sockaddr cliente;
		socklen_t clientelen = sizeof(struct sockaddr);
		int clientesd = accept(sd, (struct sockaddr *) &cliente, &clientelen);

		if (clientesd == -1) {
	                std::cerr << "ERROR: fallo en accept\n";
			close(sd);
	                return -1;
		}

		getnameinfo(&cliente, clientelen, host, NI_MAXHOST, serv, NI_MAXSERV,
			NI_NUMERICHOST | NI_NUMERICSERV);
		std::cout << "Conexión desde " << host << " " << serv << "\n";

		MyThread* thrd = new MyThread(clientesd);
		std::thread([&thrd]() { thrd->connections(); delete thrd; }).detach();
	}

	close(sd);
	return 0;
}
