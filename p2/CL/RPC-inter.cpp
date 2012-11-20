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
    cout << endl << "[maq_inter abortado]" << endl;
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

int basico(string pedido, string archivo_proveedores) {

    int error = 0;
    CLIENT *cl;                 /* manejo de RPC */
    int *inv; 
    char *servidor;          /* hostname */
    char **resp_consulta;   /* valor de retorno de la consulta */
    consulta = new priority_queue<producto, vector<producto>, comparacionProductos>;
    
    // servirse de mas de un proveedor para satisfacer pedido
    int cantidad_solicitada; 
    int cantidad_acumulada = 0; 


    int pos_separador = pedido.find("&");
    string nombre_producto = pedido.substr(0, pos_separador);
    stringstream s(pedido.substr(pos_separador+1, 
                           pedido.length()));
    s >> cantidad_solicitada;



    inicializar_tabla_proveedores(archivo_proveedores);

    map<string, vendedor*>::const_iterator proov_iter;    

    for (proov_iter = tabla_proveedores.begin(); 
            proov_iter != tabla_proveedores.end(); ++proov_iter) {

        string hostname = proov_iter->second->direccion.c_str();
        servidor = new char[hostname.length()+1]; 
        strcpy(servidor, hostname.c_str());

        /* manejo de conexion con el cliente */
        if ((cl = clnt_create(servidor, PROVEEDOR_PROG, PROVEEDOR_VERS, (char*)"udp")) == NULL) {
            cerr << "Error de conexión con el proveedor '" << proov_iter->first << endl;
            delete servidor;
            error = -1;
            continue;
        }

        delete servidor;

        if ( (inv = inicializar_tabla_productos_1(NULL, cl))==NULL){
            cerr << "Error al leer inventario" << endl;
            error = -1;            
            continue;
        }

        char * texto_consulta = new char[(nombre_producto).length()+1];
        strcpy(texto_consulta, (nombre_producto).c_str());

        if ((resp_consulta = consultar_inventario_1(&texto_consulta, cl))==NULL){
            cerr << "Error al realizar la consulta" << endl;
            error = -1;
            delete texto_consulta;
            continue;
        }

        delete texto_consulta;

        if (*resp_consulta[0] != '0') {
            // si el proveedor tiene el producto se almacenan los datos
            producto* p = mensaje_a_producto(*resp_consulta, nombre_producto, proov_iter->first);
            consulta->push(*p);
        }
        clnt_destroy(cl);         /* finalizo la conexion con el cliente */
    }

    while (!consulta->empty() && cantidad_acumulada <= cantidad_solicitada){
        producto p = consulta->top(); 
        compra.push_back(p);
        cantidad_acumulada += p.cantidad;
        consulta->pop();
    }    
    
    return error;
}

int avanzado(string mensaje, string archivo_proveedores) {
    int error = 0; 
    CLIENT *cl;                 /* manejo de RPC */
    char *servidor;          /* hostname */
    char **resp_pedido;     /* valor de retorno del pedido */
    int *inv; 

    // primero se ejecuta una consulta y se obtiene el producto al mejor precio
    error = basico(mensaje, archivo_proveedores);

    vector<producto>::const_iterator it;

    for (it = compra.begin(); it != compra.end(); ++it) {
        producto p = *it;        

        vendedor* v = tabla_proveedores[p.nombre_vendedor];

        servidor = new char[(v->direccion).length()+1];
        strcpy(servidor, v->direccion.c_str());

        /* manejo de conexion con el cliente */
        if ((cl = clnt_create(servidor, PROVEEDOR_PROG, PROVEEDOR_VERS, (char*)"udp")) == NULL) {
            cerr << "Error de conexión con el proveedor '" << p.nombre_vendedor << endl;
            error = -1;
            delete servidor;
            return error; 
        }
        delete servidor;

        char * pedido = new char[mensaje.length()+1];
        strcpy(pedido, mensaje.c_str());

        cout << "Realizar pedido: " << pedido << endl;

        if ((resp_pedido = realizar_pedido_1(&pedido, cl))==NULL){
            error = -1;
            cerr << "Error al realizar el pedido" << endl;
        } 

        delete pedido;

        if  ((inv = actualizar_inventario_1(NULL, cl))==NULL){
            error = -1;
            cerr << "Error al realizar el pedido" << endl;
        }

        clnt_destroy(cl);  

       }
    return error;

}


int main(int argc, char** argv) {
    signal(SIGINT, terminar); // escucha la señal SIGINT

    if (argc != 5) {
        return imprimir_uso();
    }

    string archivo_proveedores = argv[2]; 

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

        avanzado(mensaje_recibido, archivo_proveedores);

        string mensaje_compra; 

        // Ver compras realizadas 
        if (compra.empty()){
            mensaje_compra = "0"; 
        }

        vector<producto>::const_iterator it;
        
        for (it = compra.begin(); it != compra.end(); ++it) {
            producto p = *it;
            
            stringstream cant;
            cant << p.cantidad;
            
            stringstream prec;
            prec << p.precio;

            mensaje_compra += p.nombre_vendedor + "&" + cant.str() + "&" + prec.str() + "|";
        }
        compra.clear();

        bzero(buffer, 256);
        strcpy(buffer, mensaje_compra.c_str());

        cout << string(buffer) << endl; 

        if (!write(newsockfd, buffer, 255)) {
            cerr << "Error al escribir" << endl;
        }

        cout << string(buffer) << endl; 

        close(newsockfd);        

    }
}
