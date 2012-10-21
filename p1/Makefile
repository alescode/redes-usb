.PHONY: all clean

CPP = g++

all: ordenes proveedor

clean:
	rm -f ordenes proveedor lib/*.o

%.o: %.cpp
	$(CPP) -c $< -o $@

ordenes: lib/string_lib.o lib/producto.o lib/vendedor.o lib/error.o
proveedor: lib/string_lib.o lib/producto.o lib/vendedor.o lib/error.o
