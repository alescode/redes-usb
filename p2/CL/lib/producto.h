#include <string>
#include <sstream>
#include <iostream>

#include "string_lib.h"

using namespace std;

class producto {
    public:
        string nombre;
        int cantidad;
        double precio;
        string nombre_vendedor;

        producto();
        producto(string n, int c, double p, string v);
};

class comparacionProductos {
    bool reverse;
    public:
        comparacionProductos(const bool& revparam=false) {
            reverse=revparam;
        }

        bool operator() (const producto& lhs, const producto& rhs) const {
            if (reverse) return (lhs.precio < rhs.precio);
            else return (lhs.precio > rhs.precio);
        }
};

producto* cargar_producto(string linea);
producto* mensaje_a_producto(string linea, string nombre_producto, 
                             string nombre_vendedor);
