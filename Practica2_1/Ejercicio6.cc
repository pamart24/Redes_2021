#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>

#include <thread>

const size_t MAXBUFFER = 80;
#define MAX_THREAD 5

class MsgThread{
private:
	int sd;
	int id;
public:
	MsgThread(int sd_, int i_) : sd(sd_), id(i_){}
	void haz_mensaje(){
		time_t tiempo;
		size_t tam;
		char buffer[MAXBUFFER];
		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];
		
		struct sockaddr cliente;	
		socklen_t clientelen = sizeof(struct sockaddr);

		while(true){	
			int bytes = recvfrom(sd, (void *) buffer, MAXBUFFER - 1,
				'\0', &cliente, &clientelen);
			
			if (bytes == -1) {
				std::cerr << "ERROR: no se reciben bytes\n";
	                	return;
			}
	
			buffer[bytes] = '\0';
	
			getnameinfo(&cliente, clientelen, host, NI_MAXHOST,
					serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	
			std::cout << bytes << " bytes de " << host << ":" << serv << "\n";
	
			switch(buffer[0]) {
				//Devolvemos la hora
				case 't':
					time(&tiempo);
					tam = strftime(buffer, MAXBUFFER - 1, "%T %p",
						localtime(&tiempo));
	
					sendto(sd, buffer, tam, 0, &cliente, clientelen);
					buffer[tam] = '\0';
					break;
				//Devolvemos la fecha
				case 'd':
					time(&tiempo);
					tam = strftime(buffer, MAXBUFFER - 1, "%F",
	                                        localtime(&tiempo));
	
					sendto(sd, buffer, tam, 0, &cliente, clientelen);
					buffer[tam] = '\0';
					break;
				//Comando no soportado
				default:
					std::cout << "Comando no soportado " << buffer[0] << "\n";
					break;
			}
		}
	}
};

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Uso: ./Ejercicio2 [IP] [puerto]\n";
		return -1;
	}

	struct addrinfo hints;
	struct addrinfo * res;

	memset((void *) &hints, 0, sizeof(struct addrinfo));
	
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

	
	for(int i = 0; i < MAX_THREAD; i++){
		MsgThread* th = new MsgThread(sd, i);
		std::thread([&th]() {
		th->haz_mensaje();
		delete th;
		}).detach();
	}	
	
	std::string q = "_";
	while(q != "q"){
		std::cin >> q;
	}
	
	close(sd);
	return 0;
}
