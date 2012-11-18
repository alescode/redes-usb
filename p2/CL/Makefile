.PHONY: all clean

CPP = g++

#falta RPC-proveedor
all: RPC-ordenes RPC-inter 

clean:
	rm -f RPC-ordenes RPC-inter RPC-proveedor lib/*.o *.o proveedor_clnt.c proveedor_svc.c proveedor.h

%.o: %.cpp
	$(CPP) -c $< -o $@ -w

RPC-ordenes: lib/error.o lib/string_lib.o lib/producto.o lib/vendedor.o
RPC-inter: lib/error.o lib/string_lib.o lib/producto.o lib/vendedor.o proveedor_clnt.o 
RPC-proveedor: proveedor_svc.o lib/string_lib.o lib/producto.o lib/vendedor.o
