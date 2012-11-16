#include <string>
#include <sstream>
#include <iostream>

#include "string_lib.h"

using namespace std;

class vendedor {
    public:
        string nombre;
        string direccion;
        int puerto;

        vendedor(string n, string d, int p) {
            nombre = n;
            direccion = d;
            puerto = p;
        }
};

vendedor* string_a_vendedor(string linea);
