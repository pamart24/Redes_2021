#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <thread>

const size_t MAXBUFFER = 80;
const int MAXTHREAD = 5;

class MyThread {
public:
	MyThread(int id, int sd) : id_(id), sd_(sd) {};
	
	void message() {
		bool activo = true;
		time_t tiempo;
		size_t tam;
		char buffer[MAXBUFFER];
		char host[NI_MAXHOST];
		char serv[NI_MAXSERV];

		struct sockaddr cliente;
		socklen_t clientelen = sizeof(struct sockaddr);

		while(activo) {
			sleep(3);

	                int bytes = recvfrom(sd_, (void *) buffer, MAXBUFFER - 1,
       		                 '\0', &cliente, &clientelen);
       		        if (bytes == -1) {
                        	std::cerr << "ERROR: no se reciben bytes\n";
                        	return;
                	}

	                buffer[bytes] = '\0';

	                getnameinfo(&cliente, clientelen, host, NI_MAXHOST,
	                                serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

	                std::cout << bytes << " bytes de " << host << ":" << serv << ":"
				  << std::this_thread::get_id() << "\n";

	                switch(buffer[0]) {
				//Devolvemos la hora
				case 't':
	                                time(&tiempo);
	                                tam = strftime(buffer, MAXBUFFER - 1, "%T %p",
	                                        localtime(&tiempo));

	                                sendto(sd_, buffer, tam, 0, &cliente, clientelen);
	                                buffer[tam] = '\0';
	                                break;
				//Devolvemos la fecha
				case 'd':
	                                time(&tiempo);
	                                tam = strftime(buffer, MAXBUFFER - 1, "%F",
	                                        localtime(&tiempo));

	                                sendto(sd_, buffer, tam, 0, &cliente, clientelen);
	                                buffer[tam] = '\0';
	                                break;
	                        //Cerramos el proceso del servidor
	                      	case 'q':
	                                std::cout << "Saliendo...\n";
	                                activo = false;
	                                break;
	                        //Comando no soportado
	                       	default:
	                                std::cout << "Comando no soportado " << buffer[0] << "\n";
	                                break;
	                }
		}
	}

private:
	int id_;
	int sd_;
};

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Uso: ./Ejercicio6 [IP] [puerto]\n";
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
	
	for (int id = 0; id < MAXTHREAD; ++id) {
		MyThread* thrd = new MyThread(sd, id);
		std::thread([&thrd]() { thrd->message(); delete thrd; }).detach();
	}

	//Leemos entradas hasta que se procese una 'q'
	std::string q;
	std::cin >> q;
	while (q != "q") {
		std::cin >> q;
	}

	close(sd);
	return 0;
}
