.PHONY: all clean

CPP = g++

all: RPC-ordenes RPC-proveedor

clean:
	rm -f RPC-ordenes RPC-proveedor lib/*.o *.o proveedor_clnt.c proveedor_svc.c proveedor.h

%.o: %.cpp
	$(CPP) -c $< -o $@ -w

RPC-ordenes: proveedor_clnt.o lib/string_lib.o lib/producto.o lib/vendedor.o 
RPC-proveedor: proveedor_svc.o lib/string_lib.o lib/producto.o lib/vendedor.o



