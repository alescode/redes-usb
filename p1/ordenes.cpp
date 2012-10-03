#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "lib/string_lib.cpp"
#include "lib/vendedor.cpp"

using namespace std;

map<string, int> tabla_pedidos;
// relaciona nombre del producto a la cantidad que se desea ordenar

map<string, vendedor*> tabla_proveedores;

int imprimir_uso() {
    cerr << "Uso: ordenes -[a|b] -f [archivo de pedidos] -d "
        << "[archivo de proveedores]" << endl;
    return 1; // reportar error a la consola
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

int basico(string archivo_pedidos, string archivo_proveedores) {
    inicializar_tabla_pedidos(archivo_pedidos);
    inicializar_tabla_proveedores(archivo_proveedores);

    imprimir_tabla_pedidos();
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
