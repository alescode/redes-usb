#include "vendedor.h"

using namespace std;

vendedor* string_a_vendedor(string linea) {
    string resto;
    string nombre_vendedor;
    string direccion_vendedor;
    int puerto_vendedor;
    
    int pos_separador = linea.find("&");
    nombre_vendedor = trim(linea.substr(0, pos_separador));
    resto = linea.substr(pos_separador + 1, linea.length());

    int pos_separador_2 = resto.find("&");
    direccion_vendedor = trim(resto.substr(0, pos_separador_2));

    istringstream s(resto.substr(pos_separador_2 + 1, resto.length()));
    s >> puerto_vendedor;

    return new vendedor(nombre_vendedor, direccion_vendedor, puerto_vendedor);
}
