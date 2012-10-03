#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "lib/string_lib.cpp"

using namespace std;

map<string, int>tabla_producto_inventario;

map<string, double>tabla_producto_precio;

int imprimir_uso() {
    cerr << "Uso: proveedor -f [archivo de inventario] -p [puerto]" << endl;
    return 1; // reportar error a la consola
}

void inicializar_tablas_productos(string archivo_inventario) {
    ifstream datos;
    datos.open(archivo_inventario.c_str());

    if (datos.is_open()) {
        string linea;
        string resto;
        string nombre_producto;
        int inventario_producto;
        double precio_producto;
        while (datos.good()) {
            getline(datos, linea);
            if (linea.substr(0, 1) == "#") {
                // comentarios
                continue;
            }
            if (linea != "") {

                // se llena la tabla de productos
                int pos_separador = linea.find("&");
                nombre_producto = trim(linea.substr(0, pos_separador));
                resto = linea.substr(pos_separador + 1, linea.length());

                int pos_separador_2 = resto.find("&");
                istringstream s(resto.substr(0, pos_separador_2));
                s >> inventario_producto;

                istringstream t(resto.substr(pos_separador_2 + 1, resto.length()));
                t >> precio_producto;

                tabla_producto_inventario[nombre_producto] = inventario_producto;
                tabla_producto_precio[nombre_producto] = precio_producto;
            }
        }
    }
    else {
        cerr << "ERROR: No se pudo abrir el archivo de proveedores." << endl;
        exit(1);
    }
    datos.close();
}

void imprimir_tabla_productos() {
    map<string, int>::const_iterator pos;
    cout << "{";
    for (pos = tabla_producto_inventario.begin();
         pos != tabla_producto_inventario.end(); ++pos) {
        cout << "\"" << pos->first << "\": <";
        cout << pos->second << ", ";
        cout << fixed << tabla_producto_precio[pos->first] << ">, ";
    }
    cout << "}" << endl;
}

int main(int argc, char** argv) {
    if (argc != 5) {
        return imprimir_uso();
    }
    cout.precision(2);

    string uso = string(argv[1]);
    string archivo_inventario = string(argv[2]);

    int puerto;
    istringstream s(argv[4]);
    s >> puerto;

    inicializar_tablas_productos(archivo_inventario);
    imprimir_tabla_productos();
}
