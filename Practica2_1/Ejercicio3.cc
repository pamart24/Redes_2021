#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>

const size_t MAXBUFFER = 80;

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cerr << "Uso: ./Ejercicio3 [IP] [puerto] [comando]\n";
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

	freeaddrinfo(res);

	char buffer[MAXBUFFER];

	if (sendto(sd, argv[3], strlen(argv[3]) + 1, 0, res->ai_addr, res->ai_addrlen) == -1) {
		std::cerr << "ERROR: fallo en [sendTo]\n";
		return -1;
	}

	if (recvfrom(sd, buffer, MAXBUFFER - 1, 0, res->ai_addr, &res->ai_addrlen) == -1) {
		std::cerr << "ERROR: fallo en [recvfrom]\n";      
                return -1;
        }
	std::cout << buffer << "\n";

	close(sd);
	return 0;
}
