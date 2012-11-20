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
#include <vector>
#include <queue>
#include <iomanip>

#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>

#include "lib/string_lib.h"
#include "lib/producto.h"
#include "lib/vendedor.h"
#include "lib/error.h"


using namespace std;

map<string, int> tabla_pedidos;
// relaciona el nombre de cada producto a la cantidad que se desea ordenar

vector<string> pedidos;
// guarda los pedidos en el mismo orden que el archivo de texto

map<string, vendedor*> tabla_proveedores;
// relaciona el nombre de cada proveedor a sus datos

map<string, priority_queue<producto, 
             vector<producto>, comparacionProductos>* > tabla_consultas;
// almacena, para cada producto, una lista de los proveedores que lo ofrecen
// ordenada por el precio que ofrecen

vector<producto> compra;
// guarda los productos que se desean ordenar, incluyendo de qué proveedor se
// solicitarán

int imprimir_uso() {
    cerr << "Uso: RPC-ordenes -f [archivo de pedidos] -m "
        << "[maq_inter]" << endl;
    return 1;
}

/* Lee el archivo de texto que contiene los requerimientos del cliente
 * e inicializa la estructura de datos */
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

/* Lee el archivo de texto que contiene la lista de proveedores
 * e inicializa la estructura de datos */
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

/* inserta en la lista ordenada (cola de prioridades) la información de cada
* producto/proveedor. Si el producto no está en la tabla, se le crea una
* entrada */
void insertar_en_tabla_consultas(producto* p) {
    if (!tabla_consultas[p->nombre]) {
        tabla_consultas[p->nombre] = new priority_queue<producto,
                                     vector<producto>, comparacionProductos>;
    }
    tabla_consultas[p->nombre]->push(*p);
}

/* Intenta conectarse con el servidor a través del puerto y la dirección
 * especificados */
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

void escribir_encabezado_reporte(string tipo_reporte) {
    cout.precision(2);
    cout << left;
    cout << endl << tipo_reporte << endl;
    cout << setw(30) << "PRODUCTO" << setw(20) << "PROVEEDOR" << setw(15) << "PRECIO UN."
            << setw(10) << "CANTIDAD" << setw(10) << "COSTO TOTAL" << endl;
}

void escribir_pie_reporte(double total) {
    cout << setw(30) << "TOTAL" << setw(20) << " " << setw(15) << " "
         << setw(10) << " " << setw(10) << fixed << total << endl;
    cout << endl;
}

void generar_reporte_consulta() {
    escribir_encabezado_reporte("CONSULTA");

    map<string, priority_queue<producto, 
             vector<producto>, comparacionProductos>* >::const_iterator pos;
    double total = 0.0;
    for (pos = tabla_consultas.begin(); 
         pos != tabla_consultas.end(); ++pos) {

        string nombre_producto = pos->first;
        int unidades_faltantes = tabla_pedidos[nombre_producto];
        int unidades_pedidas;

        while (!pos->second->empty() && unidades_faltantes > 0) {
            producto p = pos->second->top();

            if (p.cantidad > tabla_pedidos[nombre_producto])
                unidades_pedidas = tabla_pedidos[nombre_producto];
            else
                unidades_pedidas = p.cantidad;
            p.cantidad = unidades_pedidas;

            unidades_faltantes -= p.cantidad;

            cout << setw(30) << nombre_producto << setw(20)
                 << p.nombre_vendedor << setw(15) << fixed
                 << p.precio << setw(10) << p.cantidad << setw(10) 
                 << fixed << p.cantidad * p.precio << endl;
            total += p.cantidad * p.precio;

            compra.push_back(p);

            pos->second->pop();
        }
    }
    escribir_pie_reporte(total);
}

void generar_reporte_compra() {
    escribir_encabezado_reporte("PEDIDOS SOLICITADOS");
    double total = 0.0;
    vector<producto>::const_iterator it;
    for (it = compra.begin(); it != compra.end(); ++it) {
        producto p = *it;

        cout << setw(30) << p.nombre << setw(20)
             << p.nombre_vendedor << setw(15) << fixed
             << p.precio << setw(10) << p.cantidad << setw(10) 
             << fixed << p.cantidad * p.precio << endl;
        total += p.cantidad * p.precio;
    }
    escribir_pie_reporte(total);
}

int main(int argc, char** argv) {
    if (argc != 5) {
        return imprimir_uso();
    }

    string archivo_pedidos = string(argv[2]);
    string archivo_intermedio = string(argv[4]);

    int error = 0;
    inicializar_tabla_pedidos(archivo_pedidos);

    int puerto = 41138;
    string direccion = "localhost";
    char buffer[256];

    map<string, int>::const_iterator pedid_iter;
    string nombre_pedido;
    string cantidad_pedido;

    int sockfd;
    for (pedid_iter = tabla_pedidos.begin(); 
            pedid_iter != tabla_pedidos.end(); ++pedid_iter) {
        sockfd = conectar(puerto, direccion);
        if (sockfd < 0) {
            cerr << "Error de conexión con el intermediario" << endl;
            error = 1;
            continue;
        }

        stringstream s;
        s << pedid_iter->second;
        cantidad_pedido = s.str();

        nombre_pedido = pedid_iter->first + "&" + cantidad_pedido;

        bzero(buffer, 256);
        strcpy(buffer, nombre_pedido.c_str());

        if (!write(sockfd, buffer, 255)) {
            cout << "error al escribir" << endl;
            return -1;
        }

        bzero(buffer, 256);

        if (!read(sockfd, buffer, 255)) {
            cout << "error al leer" << endl;
            exit(1);
        }
        cout << "[maq_inter: " << string(buffer) << "]" << endl;

        int numero_compras;
        char** compras = split(buffer, &numero_compras, (char*) "|");
        for (int i = 0; i < numero_compras; i++) {
            // compras de diferentes proveedores
            int n;
            char** productos = split(compras[i], &n, (char*) "&");
            int cant = str_to_int(productos[1]);
            double precio = str_to_double(productos[2]);
            producto p = producto(pedid_iter->first, cant, precio, productos[0]);
            compra.push_back(p);
        }

      //  if (buffer[0] != '0') {
      //  }

        close(sockfd);
    }

    generar_reporte_compra();
}

