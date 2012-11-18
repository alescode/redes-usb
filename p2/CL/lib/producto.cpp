#include "producto.h"

using namespace std;

producto::producto() {
            nombre = "";
            cantidad = 0;
            precio = 0;
            nombre_vendedor = "";
}

producto::producto(string n, int c, double p, string v = "") {
            nombre = n;
            cantidad = c;
            precio = p;
            nombre_vendedor = v;
}

producto* cargar_producto(string linea) {
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

producto* mensaje_a_producto(string linea, string nombre_producto, 
                             string nombre_vendedor) {
    string resto;
    int cantidad_producto;
    double precio_producto;
    int pos_separador = linea.find("&");

    istringstream s((linea.substr(0, pos_separador)));
    s >> cantidad_producto;

    resto = linea.substr(pos_separador + 1, linea.length());
    istringstream t(resto);
    t >> precio_producto;

    producto* p = new producto(nombre_producto, cantidad_producto,
                  precio_producto, nombre_vendedor);
    return p;
}
