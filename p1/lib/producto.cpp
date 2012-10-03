#include <string>
#include <sstream>
#include <iostream>

#include "string_lib.cpp"

using namespace std;

class producto {
    public:
        string nombre;
        int cantidad;
        double precio;

        producto(string n, int c, double p) {
            nombre = n;
            cantidad = c;
            precio = p;
        }
};


producto* string_a_producto(string linea) {
    string resto;
    string nombre_producto;
    int inventario_producto;
    double precio_producto;
    int pos_separador = linea.find("&");
    nombre_producto = trim(linea.substr(0, pos_separador));
    resto = linea.substr(pos_separador + 1, linea.length());

    int pos_separador_2 = resto.find("&");
    istringstream s(resto.substr(0, pos_separador_2));
    s >> inventario_producto;

    istringstream t(resto.substr(pos_separador_2 + 1, resto.length()));
    t >> precio_producto;

    producto* p = new producto(nombre_producto, inventario_producto,
                  precio_producto);
    return p;
}
