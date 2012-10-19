#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "lib/string_lib.cpp"
#include "lib/vendedor.cpp"

using namespace std;

map<string, int> tabla_pedidos;
vector<string> pedidos;
// relaciona nombre del producto a la cantidad que se desea ordenar

map<string, vendedor*> tabla_proveedores;

int imprimir_uso() {
    cerr << "Uso: ordenes -[a|b] -f [archivo de pedidos] -d "
        << "[archivo de proveedores]" << endl;
    return 1;
}

void inicializar_tabla_pedidos(string archivo_pedidos) {
    ifstream datos;
    datos.open(archivo_pedidos.c_str());

    if (datos.is_open()) {
        string linea;
        string nombre_producto;
        int cantidad_producto;
        while (datos.good()) {
            getline(datos, linea);
            if (linea != "") {
                // se llena la tabla de pedidos
                int pos_separador = linea.find("&");
                nombre_producto = trim(linea.substr(0, pos_separador));
                pedidos.push_back(nombre_producto);
                istringstream s(linea.substr(pos_separador + 1, linea.length()));
                s >> cantidad_producto; // no se verifica formato del archivo

                tabla_pedidos[nombre_producto] = cantidad_producto;
            }
        }
    }
    else {
        cerr << "ERROR: No se pudo abrir el archivo de pedidos." << endl;
        exit(1);
    }
    datos.close();
}

void imprimir_tabla_pedidos() {
    map<string, int>::const_iterator pos;
    cout << "{";
    for (pos = tabla_pedidos.begin(); pos != tabla_pedidos.end(); ++pos) {
        cout << "\"" << pos->first << "\": ";
        cout << pos->second << ", ";
    }
    cout << "}" << endl;
}

void imprimir_tabla_proveedores() {
    map<string, vendedor*>::const_iterator pos;
    cout << "{";
    for (pos = tabla_proveedores.begin(); 
         pos != tabla_proveedores.end(); ++pos) {
        cout << "\"" << pos->first << "\": <";
        cout << pos->second->direccion << ", ";
        cout << pos->second->puerto << ">, ";
    }
    cout << "}" << endl;
}

void inicializar_tabla_proveedores(string archivo_proveedores) {
    ifstream datos;
    datos.open(archivo_proveedores.c_str());

    if (datos.is_open()) {
        string linea;
        while (datos.good()) {
            getline(datos, linea);
            if (linea.substr(0, 1) == "#") {
                // comentarios
                continue;
            }
            if (linea != "") {
                vendedor* v = string_a_vendedor(linea);
                tabla_proveedores[v->nombre] = v;
            }
        }
    }
    else {
        cerr << "ERROR: No se pudo abrir el archivo de proveedores." << endl;
        exit(1);
    }
    datos.close();
}

int conectar(int puerto, string hostname) {
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1; // error abriendo el socket
    }

    server = gethostbyname(hostname.c_str());

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // tipo de sockets = Internet
    serv_addr.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;
    serv_addr.sin_port = htons(puerto);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        return -2; // error de conexion
    }

    return sockfd;
}

int basico(string archivo_pedidos, string archivo_proveedores) {
    inicializar_tabla_pedidos(archivo_pedidos);
    inicializar_tabla_proveedores(archivo_proveedores);

    map<string, vendedor*>::const_iterator proov_iter;
    int puerto;
    string hostname;
    for (proov_iter = tabla_proveedores.begin(); 
         proov_iter != tabla_proveedores.end(); ++proov_iter) {
        puerto = proov_iter->second->puerto;
        hostname = proov_iter->second->direccion;

        cout << proov_iter->first << ": " << hostname
             << ", puerto " << puerto << endl;

        vector<string>::const_iterator pedid_iter;
        string producto;
        char buffer[256];

        int sockfd;
        for (pedid_iter = pedidos.begin(); 
             pedid_iter != pedidos.end(); ++pedid_iter) {
            sockfd = conectar(puerto, hostname);
            producto = *pedid_iter;

            bzero(buffer, 256);
            strcpy(buffer, producto.c_str());
            printf("%s\n", buffer);

            if (!write(sockfd, buffer, 255)) {
                cout << "error al escribir" << endl;
                return -1;
            }

            bzero(buffer, 256);

            if (!read(sockfd, buffer, 255)) {
                cout << "error al leer" << endl;
                exit(1);
            }
            printf("Mensaje del servidor: %s\n", buffer);

            close(sockfd);
        }
        cout << "ya lo cerre" << endl;
    }

    return 0;
}

int avanzado(string archivo_pedidos, string archivo_proveedores) {
    cout << "no implementado" << endl;
    return 1;
}

int main(int argc, char** argv) {
    if (argc != 6) {
        return imprimir_uso();
    }

    string uso = string(argv[1]);
    string archivo_pedidos = string(argv[3]);
    string archivo_proveedores = string(argv[5]);

    if (uso == "-a") {
        return avanzado(archivo_pedidos, archivo_proveedores);
    }
    else if (uso == "-b") {
        return basico(archivo_pedidos, archivo_proveedores);
    }
    else {
        return imprimir_uso();
    }
}
