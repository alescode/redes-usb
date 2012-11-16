/*
 * proveedor.x  especificacion del proveedor remoto
 */
 
/*
 * Define 5 procedimientos
 *	
 *     
 */
 
program PROVEEDOR_PROG {
    version PROVEEDOR_VERS {
		int IMPRIMIR_INVENTARIO(void) = 1;				/* procedimiento numero = 1*/
		int INICIALIZAR_TABLA_PRODUCTOS(void) = 2;     /* procedimiento numero = 2*/
		int ACTUALIZAR_INVENTARIO(void) = 3;			/* procedimiento numero = 3*/
		string CONSULTAR_INVENTARIO(string) = 4;		/* procedimiento numero = 4*/
		string REALIZAR_PEDIDO(string) = 5; 			/* procedimiento numero = 5*/
    } = 1;                          /* version numero = 1 */
} = 0x20000001; 
