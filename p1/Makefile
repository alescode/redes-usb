.PHONY: all clean

CPP = g++

all: ordenes proveedor

clean:
	rm -f ordenes lib/*.o

%.o: %.cpp
	$(CPP) -c $< -o $@

ordenes: lib/string_lib.o lib/producto.o
proveedor: lib/string_lib.o lib/producto.o lib/vendedor.o
