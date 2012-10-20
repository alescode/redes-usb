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

        producto(string n, int c, double p, string v);
};

producto* cargar_producto(string linea);
producto* mensaje_a_producto(string linea, string nombre_producto, 
                             string nombre_vendedor);
