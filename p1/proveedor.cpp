#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "lib/string_lib.cpp"

using namespace std;

typedef struct {
    string nombre;
    int cantidad;
    double precio;
} producto;

map<string, producto> tabla_productos;

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

                producto p = {nombre_producto, inventario_producto,
                              precio_producto};
                tabla_productos[nombre_producto] = p;
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
    map<string, producto>::const_iterator pos;
    cout << "{";
    for (pos = tabla_productos.begin();
         pos != tabla_productos.end(); ++pos) {
        cout << "\"" << pos->first << "\": <";
        cout << pos->second.cantidad << ", ";
        cout << fixed << pos->second.precio << ">, ";
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
