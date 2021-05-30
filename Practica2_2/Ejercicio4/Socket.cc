#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char * address, const char * port):sd(-1)
{
    //Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
    //Con el resultado inicializar los miembros sd, sa y sa_len de la clase
    struct addrinfo hints;
    struct addrinfo* res;

    memset((void *) &hints, 0, sizeof(struct addrinfo));

    //UDP
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int rc = getaddrinfo(address, port, &hints, &res);
    if (rc != 0)
	fprintf(stderr, "ERROR: [getaddrinfo]: " << gai_strerror(rc) << "\n";

    sd = socket(res->ai_family, res->ai_socktype, 0);
    if (sd == -1)
	std::cerr << "ERROR: [socket] no creado\n";

    sa = *res->ai_addr;
    sa_len = res->ai_addrlen;

    freeaddrinfo(res);
}

Socket::Socket(struct sockaddr* _sa, socklen_t _sa_len) : sd(-1), sa(*_sa), sa_len(_sa_len) {
    sd = socket(sa.sa_family, SOCK_DGRAM, 0);
    bind();
}

int Socket::recv(Serializable &obj, Socket * &sock)
{
    struct sockaddr sa;
    socklen_t sa_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);

    if ( bytes <= 0 )
    {
        return -1;
    }

    if ( sock != 0 )
    {
        sock = new Socket(&sa, sa_len);
    }

    obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable& obj, const Socket& sock)
{
    //Serializar el objeto
    obj.to_bin();

    //Enviar el objeto binario a sock usando el socket sd
    ssize_t bytes = sendto(sock.sd, obj.data(), obj.size(), 0, &sock, sa, sock.sa_len);
    if (bytes < 1) {
	std::cerr << "ERROR: fallo al enviar mensaje\n";
	return -1;
    }
    return 0;
}

bool operator== (const Socket &s1, const Socket &s2)
{
    struct sockaddr_in* s1_in = (struct sockaddr_in *) &(s1.sa);
    struct sockaddr_in* s2_in = (struct sockaddr_in *) &(s2.sa);

    //Comparar los campos sin_family, sin_addr.s_addr y sin_port
    //de la estructura sockaddr_in de los Sockets s1 y s2
    //Retornar false si alguno difiere
    if (s1.sa.sa_family != s2.sa.sa_family || s1_in->sin_addr.s_addr != s2_in->sin_addr.s_addr ||
	s1_in->sin_port != s2_in->sin_port)
	return false;
    else return true;
};

bool operator!= (const Socket &s1, const Socket &s2) {
    return !(s1 == s2);
};

std::ostream& operator<<(std::ostream& os, const Socket& s)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    getnameinfo((struct sockaddr *) &(s.sa), s.sa_len, host, NI_MAXHOST, serv,
                NI_MAXSERV, NI_NUMERICHOST);

    os << host << ":" << serv;

    return os;
};

