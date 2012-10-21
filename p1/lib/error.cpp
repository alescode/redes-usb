#include "error.h"

void imprimir_error_socket(int puerto) {
    cerr << "Error de comunicación." << endl;
    cerr << "La razón más común de error es que el puerto " 
         << puerto << " se encuentre todavía en uso. "
         << "Por favor espere unos segundos y vuelva a intentarlo." << endl;
    exit(1);
}
