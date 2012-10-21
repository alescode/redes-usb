#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iomanip>

#include "lib/producto.h"
#include "lib/error.h"

using namespace std;

map<string, producto*> tabla_productos;

int sockfd;

int imprimir_uso() {
    cerr << "Uso: proveedor -f [archivo de inventario] -p [puerto]" << endl;
    return 1; // reportar error a la consola
}

void imprimir_inventario() {
    cout << left;
    cout << endl << "INVENTARIO" << endl;
    cout << setw(30) << "PRODUCTO" << setw(10) << "CANTIDAD" << endl;

    map<string, producto*>::const_iterator pos;
    for (pos = tabla_productos.begin();
         pos != tabla_productos.end(); ++pos) {

        cout << setw(30) << pos->first
             << setw(10) << pos->second->cantidad << endl;
    }
    cout << endl;
}

void inicializar_tabla_productos(string archivo_inventario) {
    ifstream datos;
    datos.open(archivo_inventario.c_str());

    if (datos.is_open()) {
        string linea;
        while (datos.good()) {
            getline(datos, linea);
            if (linea.substr(0, 1) == "#") {
                // comentarios
                continue;
            }
            if (linea != "") {
                producto* p = cargar_producto(linea);
                tabla_productos[p->nombre] = p;
            }
        }
    }
    else {
        cerr << "ERROR: No se pudo abrir el archivo de productos." << endl;
        exit(1);
    }
    datos.close();
}

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

void terminar(int signal) {
    close(sockfd);
    cout << endl << "[servidor abortado]" << endl;
    exit(0);
}

int main(int argc, char** argv) {
    signal(SIGINT, terminar);

    if (argc != 5) {
        return imprimir_uso();
    }
    cout.precision(2);

    string uso = string(argv[1]);
    string archivo_inventario = string(argv[2]);

    int puerto;
    istringstream s(argv[4]);
    s >> puerto;

    inicializar_tabla_productos(archivo_inventario);

    int sockfd;
    sockfd = conectar(puerto);
    if (sockfd < 0) {
        cerr << "Error de conexion" << endl;
    }

    listen(sockfd, 5);

    struct sockaddr_in cliente;
    socklen_t clilen = sizeof(cliente);

    imprimir_inventario();
    while (true) {
        int newsockfd = accept(sockfd, (struct sockaddr*) &cliente, &clilen);
        if (newsockfd < 0) {
            imprimir_error_socket(puerto);
        }

        char buffer[256];
        bzero(buffer, 256);
        if (!read(newsockfd, buffer, 255)) {
            cerr << "Error al leer" << endl;
        }
        string mensaje_recibido = string(buffer);

        cout << "[cliente: " << mensaje_recibido << "]" << endl;

        if (mensaje_recibido[0] == 'C') {
            string nombre_producto = mensaje_recibido.substr(1, 
                                      mensaje_recibido.length());
            producto* p = tabla_productos[nombre_producto];

            if (!p) {
                tabla_productos.erase(nombre_producto);
                bzero(buffer, 256);
                buffer[0] = '0';
            }
            else {
                stringstream s;
                s << p->cantidad << '&' << p->precio;
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

            tabla_productos[nombre_producto]->cantidad -= cantidad_solicitada;

            string mensaje = "OK&" + nombre_producto;
            bzero(buffer, 256);
            strcpy(buffer, mensaje.c_str());

            if (!write(newsockfd, buffer, 255)) {
                cerr << "Error al escribir" << endl;
            }

            imprimir_inventario();
        }
        close(newsockfd);
    }
}
