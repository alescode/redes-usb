/*
	Proyecto 2 - CI4835
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

#include <rpc/rpc.h>           
#include "proveedor.h"       /* archivo generado por RPC */


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
    cerr << "Uso: RPC-ordenes -f [archivo de pedidos] -d "
        << "[archivo de proveedores]" << endl;
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

        while (!pos->second->empty() && unidades_faltantes > 0) {
        	producto p = pos->second->top();
			int unidades_a_pedir;
			int inventario = p.cantidad;

			if (inventario >= unidades_faltantes)
				unidades_a_pedir = unidades_faltantes;
			else 
				unidades_a_pedir = inventario;

			unidades_faltantes -= unidades_a_pedir;
			p.cantidad = unidades_a_pedir;

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

int basico(string archivo_pedidos, string archivo_proveedores) {

	int error = 0;
	CLIENT *cl;        	 	/* manejo de RPC */
	int *inv; 
    char *servidor;      	/* hostname */
    char **resp_consulta;   /* valor de retorno de la consulta */

	inicializar_tabla_pedidos(archivo_pedidos);
    inicializar_tabla_proveedores(archivo_proveedores);

    map<string, vendedor*>::const_iterator proov_iter;	

	for (proov_iter = tabla_proveedores.begin(); 
         proov_iter != tabla_proveedores.end(); ++proov_iter) {

		string hostname = proov_iter->second->direccion.c_str();
        servidor = new char[hostname.length()+1]; 
		strcpy(servidor, hostname.c_str());

		/* manejo de conexion con el cliente */
		if ((cl = clnt_create(servidor, PROVEEDOR_PROG, PROVEEDOR_VERS, "udp")) == NULL) {
            cerr << "Error de conexión con el proveedor '" << proov_iter->first << endl;
            error = 1;
			free(servidor);
            continue;
		}
		

		vector<string>::const_iterator pedid_iter;

		for (pedid_iter = pedidos.begin(); 
             pedid_iter != pedidos.end(); ++pedid_iter) {

			if ( (inv = inicializar_tabla_productos_1(NULL, cl))==NULL){
				cerr << "Error al leer inventario" << endl;
				error = -1;
				continue;
			}

			char * consulta = new char[(*pedid_iter).length()+1];
			strcpy(consulta, (*pedid_iter).c_str());

			if ((resp_consulta = consultar_inventario_1(&consulta, cl))==NULL){
				cerr << "Error al realizar la consulta" << endl;
				error = -1;
			}

            cout << "[servidor " << servidor << ": " << *resp_consulta << "]" << endl;
		    free(servidor);
			free(consulta);

            if (*resp_consulta[0] != '0') {
                // si el proveedor tiene el producto se almacenan los datos
                producto* p = mensaje_a_producto(*resp_consulta, *pedid_iter, proov_iter->first);
                insertar_en_tabla_consultas(p);
            }

		}		

    	clnt_destroy(cl);         /* finalizo la conexion con el cliente */

	}

  	generar_reporte_consulta();
	return error;

}
	
				
int avanzado(string archivo_pedidos, string archivo_proveedores) {

	CLIENT *cl;        	 	/* manejo de RPC */
    char *servidor;      	/* hostname */
    char **resp_pedido;     /* valor de retorno del pedido */
	int *inv; 

    // primero se ejecuta una consulta
    int error = basico(archivo_pedidos, archivo_proveedores);

    vector<producto>::const_iterator it;

    // ya se tiene la orden de compra, se lleva a cabo
    for (it = compra.begin(); it != compra.end(); ++it) {

        producto p = *it;
        vendedor* v = tabla_proveedores[p.nombre_vendedor];
		
		char * servidor = new char[(v->direccion).length()+1];
		strcpy(servidor, v->direccion.c_str());

		/* manejo de conexion con el cliente */
		if ((cl = clnt_create(servidor, PROVEEDOR_PROG, PROVEEDOR_VERS, "udp")) == NULL) {
            cerr << "Error de conexión con el proveedor '" << p.nombre_vendedor << endl;
            error = 1;
			free(servidor);
            continue;
		}
		
        stringstream s;
        s << p.cantidad;
        string mensaje = p.nombre + "&" + s.str();

		char * pedido = new char[mensaje.length()+1];
		strcpy(pedido, mensaje.c_str());

		if ((resp_pedido = realizar_pedido_1(&pedido, cl))==NULL){
			error = -1;
			cerr << "Error al realizar el pedido" << endl;
		}

        cout << "[servidor " << servidor << ": " << *resp_pedido << "]" << endl;
		free(servidor);
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

    if (argc != 5) {
        return imprimir_uso();
    }


	string archivo_pedidos = string(argv[2]);
    string archivo_proveedores = string(argv[4]);

    avanzado(archivo_pedidos, archivo_proveedores);

}	
