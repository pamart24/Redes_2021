#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Usage: ./Ejercicio1 [IP | domain | host]\n";
		return -1;
	}

	struct addrinfo hints;
	struct addrinfo * res;

	memset((void *) &hints, 0, sizeof(struct addrinfo));
	
	hints.ai_family = AF_UNSPEC;
	//hints.ai_socktype = SOCK_STREAM;


	int rc = getaddrinfo(argv[1], argv[2], &hints, &res);
	
	if (rc != 0) {
		std::cerr << "[getaddrinfo]: " << gai_strerror(rc) << std::endl;
		return -1;
	}

	for (addrinfo* i = res; i != NULL; i = i->ai_next) {
		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];

		getnameinfo(i->ai_addr, i->ai_addrlen, host, NI_MAXHOST,
			serv, NI_MAXSERV, NI_NUMERICHOST);

		std::cout << "Host: " << host << " Port: " << serv <<
			" Type: " << i->ai_socktype << std::endl;
	}

	freeaddrinfo(res);
	return 0;
}
