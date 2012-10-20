%.o: %.cpp
	$(CPP) -c $< -o $@

all:
	make ordenes proveedor

ordenes: lib/string_lib.o lib/producto.o
proveedor: lib/string_lib.o lib/producto.o lib/vendedor.o
