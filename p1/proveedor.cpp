#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "lib/producto.cpp"

using namespace std;

map<string, producto*> tabla_productos;

int imprimir_uso() {
    cerr << "Uso: proveedor -f [archivo de inventario] -p [puerto]" << endl;
    return 1; // reportar error a la consola
}

void imprimir_tabla_productos() {
    map<string, producto*>::const_iterator pos;
    cout << "{";
    for (pos = tabla_productos.begin();
         pos != tabla_productos.end(); ++pos) {
        cout << "\"" << pos->first << "\": <";
        cout << pos->second->cantidad << ", ";
        cout << fixed << pos->second->precio << ">, ";
    }
    cout << "}" << endl;
}

void inicializar_tabla_productos(string archivo_inventario) {
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
                producto* p = string_a_producto(linea);
                tabla_productos[p->nombre] = p;
            }
        }
    }
    else {
        cerr << "ERROR: No se pudo abrir el archivo de productos." << endl;
        exit(1);
    }
    datos.close();
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

    inicializar_tabla_productos(archivo_inventario);
    imprimir_tabla_productos();
}
