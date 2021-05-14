#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>

const size_t MAXBUFFER = 80;

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Uso: ./Ejercicio3 [IP] [puerto]\n";
		return -1;
	}

	struct addrinfo hints;
	struct addrinfo * res;

	memset((void *) &hints, 0, sizeof(struct addrinfo));
	
	hints.ai_family = AF_UNSPEC;
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

	if (listen(sd, 1) == -1) {
		std::cerr << "ERROR: fallo en listen\n";
		return -1;
	}

	bool activo = true;
	char buffer[MAXBUFFER];
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];

	struct sockaddr cliente;
	socklen_t clientelen = sizeof(struct sockaddr);
	int clientesd = accept(sd, (struct sockaddr *) &cliente, &clientelen);

	if (clientesd == -1) {
                std::cerr << "ERROR: fallo en accept\n";
                return -1;
	}

	getnameinfo(&cliente, clientelen, host, NI_MAXHOST, serv, NI_MAXSERV,
		NI_NUMERICHOST | NI_NUMERICSERV);
	std::cout << "Conexión desde " << host << " " << serv << "\n";
	
	while (activo) {
		int bytes = recvfrom(clientesd, (void *) buffer, MAXBUFFER - 1, '\0',
			&cliente, &clientelen);
		if (bytes < 1) {
			std::cout << "Conexión terminada\n";
			activo = false;
		}
		buffer[bytes] = '\0';	

		//Hacer eco al cliente de lo que se recibe
		sendto(clientesd, buffer, bytes, 0, &cliente, clientelen);
		//send(clientesd, (void *) buffer, bytes, 0);
	}

	close(clientesd);
	close(sd);
	return 0;
}
