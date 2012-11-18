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

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>

#include <rpc/rpc.h>
#include "proveedor.h"       /* archivo generado por RPC */

#include "lib/string_lib.h"
#include "lib/producto.h"
#include "lib/vendedor.h"
#include "lib/error.h"

using namespace std;

// socket file descriptor
int sockfd;

map<string, int> tabla_pedidos;
// relaciona el nombre de cada producto a la cantidad que se desea ordenar

vector<string> pedidos;
// guarda los pedidos en el mismo orden que el archivo de texto

map<string, vendedor*> tabla_proveedores;
// relaciona el nombre de cada proveedor a sus datos

priority_queue<producto, vector<producto>, comparacionProductos>* consulta;
// almacena una lista de los proveedores que ofrecen un producto

vector<producto> compra;
// guarda los productos que se desean ordenar, incluyendo de qué proveedor se
// solicitarán

int imprimir_uso() {
    cerr << "Uso: RPC-inter -d [proveedores] -p [puerto]" << endl;
    return 1; // reportar error a la consola
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
	cout << archivo_proveedores << endl; 
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

int basico(string pedido, string archivo_proveedores) {
    int error = 0;
    CLIENT *cl;                 /* manejo de RPC */
    int *inv; 
    char *servidor;          /* hostname */
    char **resp_consulta;   /* valor de retorno de la consulta */
    consulta = new priority_queue<producto, vector<producto>, comparacionProductos>;

    inicializar_tabla_proveedores(archivo_proveedores);

    map<string, vendedor*>::const_iterator proov_iter;    

    for (proov_iter = tabla_proveedores.begin(); 
            proov_iter != tabla_proveedores.end(); ++proov_iter) {

        string hostname = proov_iter->second->direccion.c_str();
        servidor = new char[hostname.length()+1]; 
        strcpy(servidor, hostname.c_str());
        cout << "Conectar con: " << servidor << endl;

        /* manejo de conexion con el cliente */
        if ((cl = clnt_create(servidor, PROVEEDOR_PROG, PROVEEDOR_VERS, (char*)"udp")) == NULL) {
            cerr << "Error de conexión con el proveedor '" << proov_iter->first << endl;
            error = 1;
            free(servidor);
            continue;
        }

        free(servidor);

        vector<string>::const_iterator pedid_iter;

        if ( (inv = inicializar_tabla_productos_1(NULL, cl))==NULL){
            cerr << "Error al leer inventario" << endl;
            error = -1;
            continue;
        }

        char * texto_consulta = new char[(pedido).length()+1];
        strcpy(texto_consulta, (pedido).c_str());

        if ((resp_consulta = consultar_inventario_1(&texto_consulta, cl))==NULL){
            cerr << "Error al realizar la consulta" << endl;
            error = -1;
        }

        delete texto_consulta;

        if (*resp_consulta[0] != '0') {
            // si el proveedor tiene el producto se almacenan los datos
            producto* p = mensaje_a_producto(*resp_consulta, *pedid_iter, proov_iter->first);
            consulta->push(*p);
        }
        clnt_destroy(cl);         /* finalizo la conexion con el cliente */
    }
    //delete consulta;
    return 1;
}

int avanzado(string pedido, string archivo_proveedores) {

    CLIENT *cl;                 /* manejo de RPC */
    char *servidor;          /* hostname */
    char **resp_pedido;     /* valor de retorno del pedido */
    int *inv; 

    // primero se ejecuta una consulta
    int error = basico(pedido, archivo_proveedores);

    vector<producto>::const_iterator it;

    // ya se tiene la orden de compra, se lleva a cabo
    for (it = compra.begin(); it != compra.end(); ++it) {

        producto p = *it;
        vendedor* v = tabla_proveedores[p.nombre_vendedor];

        char * servidor = new char[(v->direccion).length()+1];
        strcpy(servidor, v->direccion.c_str());

        cout << "Conectando con: " << servidor << endl; 

        /* manejo de conexion con el cliente */
        if ((cl = clnt_create(servidor, PROVEEDOR_PROG, PROVEEDOR_VERS, (char*)"udp")) == NULL) {
            cerr << "Error de conexión con el proveedor '" << p.nombre_vendedor << endl;
            error = 1;
            free(servidor);
            continue;
        }
        free(servidor);

        stringstream s;
        s << p.cantidad;
        string mensaje = p.nombre + "&" + s.str();

        char * pedido = new char[mensaje.length()+1];
        strcpy(pedido, mensaje.c_str());

        cout << "Realizar pedido: " << pedido << endl; 

        if ((resp_pedido = realizar_pedido_1(&pedido, cl))==NULL){
            error = -1;
            cerr << "Error al realizar el pedido" << endl;
        }

        free(pedido);

        if  ((inv = actualizar_inventario_1(NULL, cl))==NULL){
            error = -1;
            cerr << "Error al realizar el pedido" << endl;
        }

        clnt_destroy(cl);  

    }

    generar_reporte_compra();
    return error;
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
