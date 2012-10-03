#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "lib/string_lib.cpp"

using namespace std;

map<string, int> tabla_producto_cantidad;
// relaciona nombre del producto a la cantidad que se desea ordenar

map<string, string>tabla_proveedor_direccion;
// relaciona nombre del proveedor a direccion
// no se encuentra <tuple> en Mac OS X, habría sido más cómodo usar
// un solo map

map<string, int>tabla_proveedor_puerto;

int imprimir_uso() {
    cerr << "Uso: ordenes -[a|b] -f [archivo de pedidos] -d "
        << "[archivo de proveedores]" << endl;
    return 1; // reportar error a la consola
}

void inicializar_tabla_productos(string archivo_pedidos) {
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
                istringstream s(linea.substr(pos_separador + 1, linea.length()));
                s >> cantidad_producto; // no se verifica formato del archivo

                tabla_producto_cantidad[nombre_producto] = cantidad_producto;
            }
        }
    }
    else {
        cerr << "ERROR: No se pudo abrir el archivo de pedidos." << endl;
        exit(1);
    }
    datos.close();
}

void imprimir_tabla_productos() {
    map<string, int>::const_iterator pos;
    cout << "{";
    for (pos = tabla_producto_cantidad.begin(); pos != tabla_producto_cantidad.end(); ++pos) {
        cout << "\"" << pos->first << "\": ";
        cout << pos->second << ", ";
    }
    cout << "}" << endl;
}

void imprimir_tabla_proveedores() {
    map<string, string>::const_iterator pos;
    cout << "{";
    for (pos = tabla_proveedor_direccion.begin(); 
         pos != tabla_proveedor_direccion.end(); ++pos) {
        cout << "\"" << pos->first << "\": <";
        cout << pos->second << ", ";
        cout << tabla_proveedor_puerto[pos->first] << ">, ";
    }
    cout << "}" << endl;
}

void inicializar_tablas_proveedor(string archivo_proveedores) {
    ifstream datos;
    datos.open(archivo_proveedores.c_str());

    if (datos.is_open()) {
        string linea;
        string resto;
        string nombre_proveedor;
        string direccion_proveedor;
        int puerto_proveedor;
        while (datos.good()) {
            getline(datos, linea);
            if (linea.substr(0, 1) == "#") {
                // comentarios
                continue;
            }
            if (linea != "") {

                // se llena la tabla de pedidos
                int pos_separador = linea.find("&");
                nombre_proveedor = trim(linea.substr(0, pos_separador));
                resto = linea.substr(pos_separador + 1, linea.length());

                int pos_separador_2 = resto.find("&");
                direccion_proveedor = trim(resto.substr(0, pos_separador_2));

                istringstream s(resto.substr(pos_separador_2 + 1, resto.length()));
                s >> puerto_proveedor;

                tabla_proveedor_direccion[nombre_proveedor] = direccion_proveedor;
                tabla_proveedor_puerto[nombre_proveedor] = puerto_proveedor;
            }
        }
    }
    else {
        cerr << "ERROR: No se pudo abrir el archivo de proveedores." << endl;
        exit(1);
    }
    datos.close();
}

int basico(string archivo_pedidos, string archivo_proveedores) {
    inicializar_tabla_productos(archivo_pedidos);
    inicializar_tablas_proveedor(archivo_proveedores);

    imprimir_tabla_productos();
    imprimir_tabla_proveedores();
    //print_map_2(tabla_proveedores);

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
