/*
	Proyecto 1 - CI4835
	Autores:
		Alejandro Machado 07-41138
		Maria Leonor Pacheco 07-41302
*/

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <iomanip>

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>

#include "lib/producto.h"
#include "lib/error.h"

using namespace std;

// socket file descriptor
int sockfd;

int imprimir_uso() {
    cerr << "Uso: RPC-inter -d [proveedores] -p [puerto]" << endl;
    return 1; // reportar error a la consola
}

/* Establece una conexión al puerto como servidor */
int conectar(int puerto) {
    struct sockaddr_in servidor;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero((char *) &servidor, sizeof(servidor));
    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY;
    servidor.sin_port = htons(puerto);

    if (bind(sockfd, (struct sockaddr *) &servidor, 
                sizeof(servidor)) < 0) {
        return -1;
    }

    return sockfd;
}

/* Finaliza el servidor apropiadamente luego de recibir SIGINT (CTRL-C) */
void terminar(int signal) {
    close(sockfd);
    cout << endl << "[servidor abortado]" << endl;
    exit(0);
}

int main(int argc, char** argv) {
    signal(SIGINT, terminar); // escucha la señal SIGINT

    if (argc != 5) {
        return imprimir_uso();
    }

    int puerto;
    istringstream s(argv[4]);
    s >> puerto;

    int sockfd;
    sockfd = conectar(puerto);
    if (sockfd < 0) {
        cerr << "Error de conexion" << endl;
    }

    listen(sockfd, 5);

    struct sockaddr_in cliente;
    socklen_t clilen = sizeof(cliente);

    while (true) {
        int newsockfd = accept(sockfd, (struct sockaddr*) &cliente, &clilen);
        if (newsockfd < 0) {
            imprimir_error_socket(puerto);
            exit(1);
        }
        // se ha establecido una conexión

        char buffer[256];
        bzero(buffer, 256);
        if (!read(newsockfd, buffer, 255)) {
            cerr << "Error al leer" << endl;
        }
        string mensaje_recibido = string(buffer);

        cout << "[cliente: " << mensaje_recibido << "]" << endl;

        /* El primer caracter del mensaje codifica si se está realizando una
         * consulta (C) o un pedido (P) */

#if 0
        if (mensaje_recibido[0] == 'C') {
            string nombre_producto = mensaje_recibido.substr(1, 
                                      mensaje_recibido.length());
            producto* p = tabla_productos[nombre_producto];

            if (!p) { // no existe tal producto
                tabla_productos.erase(nombre_producto);
                bzero(buffer, 256);
                buffer[0] = '0'; // se envía la cantidad en el inventario (0)
            }
            else {
                stringstream s;
                s << p->cantidad << '&' << p->precio;
                // se envía la cantidad en el inventario y el precio
                bzero(buffer, 256);
                strcpy(buffer, s.str().c_str());
            }

            if (!write(newsockfd, buffer, 255)) {
                cerr << "Error al escribir" << endl;
            }
        }
        else if (mensaje_recibido[0] == 'P') {
            string nombre_producto;
            int cantidad_solicitada;
            int pos_separador = mensaje_recibido.find("&");

            nombre_producto = mensaje_recibido.substr(1, pos_separador - 1);
            stringstream s(mensaje_recibido.substr(pos_separador + 1, 
                           mensaje_recibido.length()));
            s >> cantidad_solicitada;

            string mensaje;
            if (tabla_productos[nombre_producto]->cantidad 
                    >= cantidad_solicitada) {
                tabla_productos[nombre_producto]->cantidad -= cantidad_solicitada;
                mensaje = "OK&" + nombre_producto;
                // se acepta la compra
            }
            else {
                mensaje = "NO&" + nombre_producto;
                /* se rechaza la compra
                 si el cliente realizó la consulta correctamente antes del
                 pedido, esto no debería ocurrir */
            }
        }
#endif
        close(newsockfd);
    }
}
