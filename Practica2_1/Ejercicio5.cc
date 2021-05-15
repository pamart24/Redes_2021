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
		std::cerr << "Uso: ./Ejercicio5 [IP] [puerto]\n";
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

	int servidorsd = connect(sd, (struct sockaddr *) res->ai_addr, res->ai_addrlen);

	if(servidorsd == 1) {
		std::cerr << "ERROR: fallo al conectarse con el servidor\n";
		close(sd);
		close(servidorsd);
		return -1;
	}

	freeaddrinfo(res);

	bool conectado = true;
	char buffer[MAXBUFFER];

	while (conectado) {
		std::cin >> buffer;
		
		//He intentado comparar con 'Q' el carácter del buffer[bytes - 1],
		//pero por alguna razón no me guarda bien el num de bytes del mensaje
		//int bytes = recv(servidorsd, (void *) buffer, MAXBUFFER - 1,'\0'); -> primer intento
		//ssize_t bytes = send(servidorsd, buffer, (MAXBUFFER - 1)*sizeof(char), 0); -> segundo intento (sin necesidad de hacer el siguiente 'send')
		//std::cout << "Numero de bytes: " << bytes << "\n"; -> da siempre -1
		//if (buffer[bytes] == 'Q')
		//	conectado = false;
		
		send(servidorsd, buffer, MAXBUFFER - 1, 0);
		
		//Lo haremos entonces solo para 'Q' como primer y único carácter
		if (buffer[0] == 'Q' && buffer[1] == '\0')
			conectado = false;
		else {
			recv(servidorsd, buffer, MAXBUFFER - 1, 0);
			std::cout << buffer << "\n";
		}
	}

	close(servidorsd);
	close(sd);
	return 0;
}
