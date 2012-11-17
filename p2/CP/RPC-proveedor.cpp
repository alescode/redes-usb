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
#include <iomanip>

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>

#include "lib/producto.h"

#include "inventario.h"

#include <rpc/rpc.h>        
#include "proveedor.h" /* este archivo es generado por rpcgen */

using namespace std;

/* Relaciona el nombre de cada producto con 
 * sus datos en el inventario */
map<string, producto*> tabla_productos;
string archivo_inventario = INVENTARIO;

int * imprimir_inventario_1_svc( void * nada, struct svc_req *req) {
	static int error = 0;
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
	return &error;
}

/* Lee el archivo de texto que contiene los datos del inventario
 * e inicializa la estructura de datos */
int * inicializar_tabla_productos_1_svc( void * nada, struct svc_req *req) {
	static int error = 0; 
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
		error = -1; 
        cerr << "ERROR: No se pudo abrir el archivo de productos." << endl;
        exit(1);
    }
    datos.close();
	return &error;
}

/* Actualiza el archivo de texto de los datos del inventario
 * con el estado actual de la estructura de datos */
int * actualizar_inventario_1_svc( void * nada, struct svc_req *req) {
	static int error = 0;
	ofstream datos;
	datos.open(archivo_inventario.c_str()); 
	string linea; 

    map<string, producto*>::const_iterator pos;

	if (datos.is_open()) {

   		for (pos = tabla_productos.begin();
         	pos != tabla_productos.end(); ++pos) {

			stringstream s;
			stringstream r;
        	s << pos->second->cantidad;
			r << pos->second->precio; 

			linea = pos->first + " & " + s.str() + 
					" & " + r.str() + "\n"; 
			datos << linea; 
		}
	} else  {
		error = -1;
        cerr << "ERROR: No se pudo abrir el archivo de productos." << endl;
        exit(1);
    }
    datos.close();
	return &error;
}

/* Envia el mensaje de respuesta a la consulta  */
char **consultar_inventario_1_svc(char **nombre, struct svc_req *req){
	string nombre_producto = *nombre;
	cout << "consultando: " << endl; 
	static char * mensaje; 
	producto* p = tabla_productos[nombre_producto];
	
	if (!p) { // no existe tal producto
    	tabla_productos.erase(nombre_producto);
		mensaje = new char[2];
		mensaje[0] = '0'; // se envía la cantidad en el inventario (0)
		mensaje[1] = 0;
		return &mensaje; 
	} else {
		stringstream s;
		// se envía la cantidad en el inventario y el precio
        s << p->cantidad << '&' << p->precio;
		mensaje = new char[s.str().length()+1];
		strcpy(mensaje, s.str().c_str());
		return &mensaje;
	}

}

char **realizar_pedido_1_svc(char **pedido, struct svc_req *req) {
	string mensaje_recibido = *pedido; 
	string nombre_producto;
	int cantidad_solicitada;
	int pos_separador = mensaje_recibido.find("&");
	string aux; 

	nombre_producto = mensaje_recibido.substr(0, pos_separador);
	stringstream s(mensaje_recibido.substr(pos_separador+1, 
                           mensaje_recibido.length()));

	string cant = (mensaje_recibido.substr(pos_separador+1, 
                           mensaje_recibido.length()));
	s >> cantidad_solicitada;

	static char * mensaje;

	if (tabla_productos[nombre_producto]->cantidad 
		>= cantidad_solicitada) {

		tabla_productos[nombre_producto]->cantidad -= cantidad_solicitada;
		aux = "OK&" + nombre_producto;
		mensaje = new char[aux.length()+1];
		strcpy(mensaje, aux.c_str());
		// se acepta la compra
    }
	else {
		aux = "NO&" + nombre_producto;
		mensaje = new char[aux.length()+1];
		strcpy(mensaje, aux.c_str());
		/* se rechaza la compra
		 * si el cliente realizó la consulta correctamente antes del
		 * pedido, esto no debería ocurrir */
	}
	return &mensaje;

}

