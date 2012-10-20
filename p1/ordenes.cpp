#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <queue>
#include <iomanip>

#include "lib/string_lib.h"
#include "lib/producto.h"
#include "lib/vendedor.cpp"

using namespace std;

map<string, int> tabla_pedidos;
vector<string> pedidos;
// relaciona nombre del producto a la cantidad que se desea ordenar

map<string, vendedor*> tabla_proveedores;

map<string, priority_queue<producto, 
             vector<producto>, comparacionProductos>* > tabla_consultas;

vector<producto> compra;

void insertar_en_tabla_consultas(producto* p) {
    if (!tabla_consultas[p->nombre]) {
        tabla_consultas[p->nombre] = new priority_queue<producto,
                                     vector<producto>, comparacionProductos>;
    }
    tabla_consultas[p->nombre]->push(*p);
}

void generar_reporte() {
    cout.precision(2);
    cout << left;
    map<string, priority_queue<producto, 
             vector<producto>, comparacionProductos>* >::const_iterator pos;
    cout << endl << "CONSULTA" << endl;
    cout << setw(30) << "PRODUCTO" << setw(20) << "PROVEEDOR" << setw(15) << "PRECIO UN."
            << setw(10) << "CANTIDAD" << setw(10) << "COSTO TOTAL" << endl;
    double total_consulta = 0.0;
    for (pos = tabla_consultas.begin(); 
         pos != tabla_consultas.end(); ++pos) {

        string nombre_producto = pos->first;
        int unidades_faltantes = tabla_pedidos[nombre_producto];
        int unidades_pedidas;

        while (!pos->second->empty() && unidades_faltantes > 0) {
            cout << setw(30) << nombre_producto << setw(20);
            producto p = pos->second->top();

            if (p.cantidad > tabla_pedidos[nombre_producto])
                unidades_pedidas = tabla_pedidos[nombre_producto];
            else
                unidades_pedidas = p.cantidad;
            p.cantidad = unidades_pedidas;

            unidades_faltantes -= p.cantidad;

            cout << p.nombre_vendedor << setw(15) << fixed
                 << p.precio << setw(10) << p.cantidad << setw(10) 
                 << fixed << p.cantidad * p.precio << endl;
            total_consulta += p.cantidad * p.precio;

            compra.push_back(p);

            pos->second->pop();
        }
    }
    cout << setw(30) << "TOTAL" << setw(20) << " " << setw(15) << " "
         << setw(10) << " " << setw(10) << fixed << total_consulta << endl;
    cout << endl;
}

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

int conectar(int puerto, string direccion) {
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1; // error abriendo el socket
    }

    server = gethostbyname(direccion.c_str());

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
    int error = 0;
    inicializar_tabla_pedidos(archivo_pedidos);
    inicializar_tabla_proveedores(archivo_proveedores);

    map<string, vendedor*>::const_iterator proov_iter;
    int puerto;
    string direccion;
    char buffer[256];

    for (proov_iter = tabla_proveedores.begin(); 
         proov_iter != tabla_proveedores.end(); ++proov_iter) {
        puerto = proov_iter->second->puerto;
        direccion = proov_iter->second->direccion;

        vector<string>::const_iterator pedid_iter;
        string pedido;

        int sockfd;
        for (pedid_iter = pedidos.begin(); 
             pedid_iter != pedidos.end(); ++pedid_iter) {
            sockfd = conectar(puerto, direccion);
            if (sockfd < 0) {
                cerr << "Error de conexión con el proveedor '"
                     << proov_iter->first << "' para consultar '"
                     << *pedid_iter << "' " << endl;
                error = 1;
                continue;
            }
            string mensaje = "C" + *pedid_iter;

            bzero(buffer, 256);
            strcpy(buffer, mensaje.c_str());
            //printf("%s\n", buffer);

            if (!write(sockfd, buffer, 255)) {
                cout << "error al escribir" << endl;
                return -1;
            }

            bzero(buffer, 256);

            if (!read(sockfd, buffer, 255)) {
                cout << "error al leer" << endl;
                exit(1);
            }
            cout << "[servidor <" << proov_iter->second->nombre << ">: "
                 << string(buffer) << "]" << endl;

            if (buffer[0] != '&') { // mensaje no vacío
                producto* p = mensaje_a_producto(buffer, *pedid_iter, proov_iter->first);
                insertar_en_tabla_consultas(p);
            }

            close(sockfd);
        }
    }
    generar_reporte();

    return error;
}

int avanzado(string archivo_pedidos, string archivo_proveedores) {
    int error = basico(archivo_pedidos, archivo_proveedores);

    vector<producto>::const_iterator it;
    int sockfd;
    char buffer[256];
    for (it = compra.begin(); it != compra.end(); ++it) {
        producto p = *it;
        vendedor* v = tabla_proveedores[p.nombre_vendedor];
        int puerto = v->puerto;
        string direccion = v->direccion;


        sockfd = conectar(puerto, direccion);
        if (sockfd < 0) {
            cerr << "Error de conexión con el proveedor '"
                 << p.nombre_vendedor << "' para solicitar '"
                 << p.nombre << "' " << endl;
            error = 1;
            continue;
        }

        stringstream s;
        s << p.cantidad;
        string mensaje = "P" + p.nombre + "&" + s.str();

        bzero(buffer, 256);
        strcpy(buffer, mensaje.c_str());

        if (!write(sockfd, buffer, 255)) {
            cout << "error al escribir" << endl;
            return -1;
        }

        bzero(buffer, 256);
        if (!read(sockfd, buffer, 255)) {
            cout << "error al leer" << endl;
            exit(1);
        }
        cout << "[servidor <" << p.nombre_vendedor << ">: " 
             << string(buffer) << "]" << endl;
    }

    close(sockfd);
    return error;
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
