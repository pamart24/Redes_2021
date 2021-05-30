#include "Serializable.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define MAX_NAME 80

class Jugador: public Serializable
{
public:
    Jugador(const char * _n, int16_t _x, int16_t _y):x(_x),y(_y)
    {
        strncpy(name, _n, MAX_NAME);
    };

    virtual ~Jugador(){};

    void to_bin()
    {
        _size = (sizeof(int16_t) * 2) + (sizeof(char) * MAX_NAME);
	alloc_data(_size);

	char* temp = _data;
	memcpy(temp, name, MAX_NAME * sizeof(char));
	temp += MAX_NAME * sizeof(char);
	memcpy(temp, &x, sizeof(int16_t));
	temp += sizeof(int16_t);
	memcpy(temp, &y, sizeof(int16_t));
    }

    int from_bin(char * data)
    {
        char* temp = data;
	memcpy(name, temp, MAX_NAME * sizeof(char));
	temp += MAX_NAME * sizeof(char);
	memcpy(&x, temp, sizeof(int16_t));
	temp += sizeof(int16_t);
	memcpy(&y, temp, sizeof(int16_t));

        return 0;
    }
	
    int16_t getX() const { return x; }
    int16_t getY() const { return y; }

    char name[MAX_NAME];

private:
    int16_t x;
    int16_t y;
};

int main(int argc, char **argv)
{
    Jugador one_r("", 0, 0);
    Jugador one_w("Player_ONE", 123, 987);

    // 1. Serializar el objeto one_w
    // 2. Escribir la serialización en un fichero
    // 3. Leer el fichero
    // 4. "Deserializar" en one_r
    // 5. Mostrar el contenido de one_r
	
    //1
    one_w.to_bin();

    //2
    int fd = open("./player1_data.txt", O_CREAT | O_TRUNC | O_RDWR, 0666);
    ssize_t tam = write(fd, one_w.data(), one_w.size());

    if(tam != one_w.size())
	std::cout << "WARNING: no all info was saved\n";

    close(fd);

    //3
    fd = open("./player1_data.txt", O_RDONLY, 0666);
    char buffer[tam];

    if (read(fd, &buffer, tam) == -1) {
	std::cerr << "ERROR: error while reading: ./player1_data.txt\n";
	return -1;
    }
    close(fd);

    //4
    one_r.from_bin(buffer);

    //5
    std::cout << "Nombre de one_r: " << one_r.name << "| x: " << one_r.getX() << ", y: " << one_r.getY() << "\n";

    return 0;
}
