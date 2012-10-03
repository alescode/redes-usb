#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "lib/string_lib.cpp"
#include "lib/map_lib.cpp"

using namespace std;

map<string, int> tabla_pedidos;
// relaciona nombres de productos a la cantidad que se desea ordenar

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Uso: ./cargar_pedidos <nombre de archivo>" << endl;
        return 1;
    }

    ifstream datos; // archivo de entrada
    datos.open(argv[1]);

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
        cerr << "ERROR: No se pudo abrir el archivo." << endl;
        return 1;
    }
    datos.close();

    print_map(tabla_pedidos);

    return 0;
}
