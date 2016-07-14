/*
 * TablaMarcos.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "TablaMarcos.h"

/* puntero a la tabla de Marcos */
static t_list_mutex *TablaMarcos;

stRegistroTP *buscarRegistroEnTabla(uint16_t pid, uint16_t paginaBuscada){

	stNodoListaTP *nodo;
	stRegistroTP *registro;

	nodo = buscarPID(pid);
	registro = nodo->tabla+(sizeof(stRegistroTP)*paginaBuscada);

	return registro;
}

int buscarEnTabla(uint16_t pid, uint16_t paginaBuscada, uint16_t *frame){

	stRegistroTP *registro = NULL;

	registro = buscarRegistroEnTabla(pid, paginaBuscada);
	if(registro){

		if(registro->bitPresencia==0)
			return 0;
		registro->bit2ndChance=1;
		*frame = registro->marco;
		return registro->marco;
	}
	return 0;
}

/*
	 * Clock o second chance
	 * Si el frame tiene el bit de acceso en 1, lo actualiza a 0 y avanza el puntero.
	 * Cuando encuentra un frame en 0, reemplaza la pagiina que contiene por la que produjo el fallo de pagina.
	 */

stRegistroTP *EjecutarClock(stNodoListaTP *tablaPaginas, uint16_t pagina){
	stRegistroTP *victima = NULL;
	stRegistroTP *regTP = NULL;
	uint16_t i = tablaPaginas->punteroClock;
	uint16_t puntero_siguiente = 0;

	// Me posiciono donde apunta el puntero
	while(!victima){

		if(i < tablaPaginas->size)
			i++;
		else
			i=0;  // todas las paginas empiezan en 0

		regTP = obtenerRegistroTabladePaginas(tablaPaginas, i);

		// Si es valido el registro
		if(regTP->bitPresencia==1){
			// Si bit2ndChance es 1 lo pongo en 0 y avanzo.
			if(regTP->bit2ndChance==1){
				regTP->bit2ndChance=0;
			// Si bit2ndChance es 0, reemplazo
			}else{
				victima=regTP;
			}
		}
	}
	// Posiciono el puntero en la pagina siguiente con presencia
	// con el ultimo i del while anterior
	while(0 == puntero_siguiente){

		if(i < tablaPaginas->size)
			i++;
		else
			i=0;  // todas las paginas empiezan en 0

		if(regTP->bitPresencia==1)
			puntero_siguiente=i;
	}
	tablaPaginas->punteroClock=puntero_siguiente;

	return victima;
}

stRegistroTP *EjecutarClockModificado(stNodoListaTP *tablaPaginas, uint16_t pagina){
//stRegistroTP *EjecutarClockModificado(stNodoListaTP *nodo, uint16_t pagina, stRegistroTP registro, uint8_t flag){
	stRegistroTP *regTP = NULL;
//	int i, estado;
//
//	/*
//	 * Clock Modificado o Enhanced second chance
//	 * Si el frame tiene el bit de acceso en 1, lo actualiza a 0 y avanza el puntero.
//	 * Cuando encuentra un frame en 0, reemplaza la pagina que contiene por la que produjo el fallo de pagina.
//	 *
//	 * Las pagina modificadas no pueden reemplazarse hasta que se escriban en memoria secundaria (Disco).
//	 * El clock mejorado genera 4 clases de frames:
//	 * 		1) No accedida, No Modificada (bit2ndChance=0;bitModificado=0)
//	 * 		2) Accedida, No modificada (bit2ndChance=1;bitModificado=0)
//	 * 		3) No accedida, Modificada (bit2ndChance=0;bitModificado=1)
//	 * 		4) Accedida, Modificada (bit2ndChance=1;bitModificado=1)
//	 * 	Paso 1: Recorro la lista de frames buscando uno con los bits bit2ndChance=0; bitModificado=0 (No cambiar los bits de uso)
//	 * 	Paso 2: Si en el paso anterior no encuentro ningun frame, recorro la lista de frames buscando los bits bit2ndChance=0; bitModificado=1.
//	 * 	En el recorrido pongo el bit bit2ndChance en 0 a medida que voy pasando.
//	 *	Paso 3: Si el paso anterior no encontro ningun frame, habre regresado a la posicion de comienzo y todos los frames tienen el bit2ndChance=0.
//	 *	Proseguir con el paso 1 y de ser necesario el paso 2.
//	 */
//
//	do{
//		// recorro tabla Paso 1
//		for(i=0;i<nodo->size;i++){
//			regTP = nodo->tabla+(sizeof(stRegistroTP)*i);
//
//			// Si es valido el registro
//			if(regTP->bitPresencia==1){
//				// Si bit2ndChance=0; bitModificado=0 o avanzo.
//				if(regTP->bit2ndChance==0 && regTP->bitModificado==0){
//					regTP->bit2ndChance=registro.bit2ndChance;
//					regTP->bitModificado=registro.bitModificado;
//					regTP->bitPresencia=registro.bitPresencia;
//					// Si no tengo que reusar la memoria, asigno el nuevo marco a la pagina
//					if (flag!=REEMPLAZAR_MARCO)
//						regTP->marco=registro.marco;
//					// termino de recorrer
//					estado=ENCONTRADO;
//					return regTP;
//				}
//			}
//			// avanzo
//		}
//		// recorro tabla Paso 2
//		for(i=0;i<nodo->size && estado==NO_ENCONTRADO;i++){
//			regTP = nodo->tabla+(sizeof(stRegistroTP)*i);
//
//			// Si es valido el registro
//			if(regTP->bitPresencia==1){
//				// Si bit2ndChance=0; bitModificado=1 o avanzo.
//				if(regTP->bit2ndChance==0 && regTP->bitModificado==1){
//					regTP->bit2ndChance=registro.bit2ndChance;
//					regTP->bitModificado=registro.bitModificado;
//					regTP->bitPresencia=registro.bitPresencia;
//					// Si no tengo que reusar la memoria, asigno el nuevo marco a la pagina
//					if (flag!=REEMPLAZAR_MARCO)
//						regTP->marco=registro.marco;
//					// termino de recorrer
//					estado=ENCONTRADO;
//					break;
//				}else if(regTP->bit2ndChance==1){
//					// Si bit2ndChance es 1 lo pongo en 0 y avanzo.
//					regTP->bit2ndChance=0;
//				}
//			}
//			// avanzo
//
//		}
//	}while(estado==ENCONTRADO);
//
	return regTP;
}


int agregarFrameATablaMarcos(uint16_t frameNuevo, stNodoListaTP *tablaPaginas, uint16_t pagina){

	stRegistroTP *registro = NULL;
	registro = obtenerRegistroTabladePaginas(tablaPaginas, pagina);

	if(registro==NULL)
		return EXIT_FAILURE;

	registro->marco = frameNuevo;
	registro->bitPresencia = 1;
	registro->bitModificado = 0;
	registro->bit2ndChance = 1;   // Empieza inicializado en 1 (ver ppt)

	// Muevo el puntero de marcos de esta tabladePaginas
	tablaPaginas->punteroClock = pagina;

	return EXIT_SUCCESS;
}



/* Devuelve null si no existe la tabla de paginas del PID o es un proceso nuevo y no hay marcos libres */
int reemplazarValorTabla(uint16_t *frameNuevo, stNodoListaTP *tablaPaginas, uint16_t pagina){

	stRegistroTP *victima = NULL;
	victima = obtenerRegistroTabladePaginas(tablaPaginas, pagina);

	// El registro de la pagina solicitada se va a guardar en frameNuevo
	// Busco victima en la tabla de paginas del proceso.
	if(string_equals_ignore_case(losParametros.algoritmo,"CLOCK_MODIFICADO"))
		victima = EjecutarClockModificado(tablaPaginas, pagina);
	else
		victima = EjecutarClock(tablaPaginas, pagina);

	if(victima==NULL)
		return EXIT_FAILURE;

	if(victima->bitModificado==1){
		grabarEnSwap(tablaPaginas->pid, victima->marco, pagina);
	}

	return EXIT_SUCCESS;
}

void creatListaDeTablas(){
	//sleep(losParametros.delay);
	TablaMarcos = list_mutex_create();
}

int crearTabla(uint16_t processId, uint16_t longitud_tabla){

	stNodoListaTP *nodo = NULL;
	void *tabla = NULL;
	int i;
	int posicionEnTablaMarcos;

	tabla = calloc(longitud_tabla,sizeof(stRegistroTP));

	//recorro la tabla para inicializarla
	for(i=0;i<longitud_tabla;i++){
		((stRegistroTP *)(tabla+(sizeof(stRegistroTP)*i)))->bit2ndChance=0;
		((stRegistroTP *)(tabla+(sizeof(stRegistroTP)*i)))->bitModificado=0;
		((stRegistroTP *)(tabla+(sizeof(stRegistroTP)*i)))->bitPresencia=0;
	}

	nodo = calloc(1,sizeof(stNodoListaTP));

	nodo->pid=processId;
	nodo->size=longitud_tabla;
	nodo->tabla=tabla;
	nodo->punteroClock = 0; // Apunta al ultimo elemento de la tabla

	// agrego retardo
	// TODO Sacar // sleep(losParametros.delay);
	//enlazo en la lista
	posicionEnTablaMarcos = list_mutex_add(TablaMarcos, nodo);

	return posicionEnTablaMarcos;
}

stNodoListaTP *buscarPID(uint16_t pid){
	stNodoListaTP* nodoListaTP = NULL;

	void _comparo_con_pid(stNodoListaTP *list_nodo){
		if(list_nodo->pid == pid){
			nodoListaTP = list_nodo;
		}
	}
	sleep(losParametros.delay);
	list_mutex_iterate(TablaMarcos, (void*)_comparo_con_pid);

	// NULL: si no lo encontro, sino puntero a nodo
	return nodoListaTP;
}

int obtenerPresenciasTabladePaginas(stNodoListaTP* nodo){
	int i=0, presencias=0;
	stRegistroTP *retorno = NULL;

	if(nodo==NULL)
		return 0;

	for(i=0;i<nodo->size;i++){
		retorno = nodo->tabla+(sizeof(stRegistroTP)*i);
		if(retorno->bitPresencia==1)
			presencias++;
	}

	return presencias;
}

stRegistroTP* obtenerRegistroTabladePaginas(stNodoListaTP* nodo, int pagina){
	stRegistroTP *registro = NULL;

	if(nodo==NULL)
		return registro;

	// es mayor igual porque empieza en la pagina 0
	if(pagina >= nodo->size)
		return registro;

	registro = nodo->tabla+(sizeof(stRegistroTP)*pagina);
	return registro;
}

int grabarEnSwap(uint16_t pid, uint16_t marco, uint16_t pagina){
	stPosicion posLogica;

	void *buf=NULL;
	buf=malloc(losParametros.frameSize);

	// Posicion a grabar
	posLogica.offset=0;
	posLogica.pagina = pagina;
	posLogica.size = losParametros.frameSize;

	if(leerMemoria(buf, marco, posLogica))
		return EXIT_FAILURE;

	if(enviarPagina(pid, pagina, buf))
		return EXIT_FAILURE;

	free(buf);

	return EXIT_SUCCESS;;
}


void _mostrarContenidoTP(stNodoListaTP *list_nodo){
	int i;
	stRegistroTP *nodo;

	printf("pid: %d\n", list_nodo->pid);
	nodo= ((stRegistroTP*)list_nodo->tabla);
	for(i=0;i<list_nodo->size;i++){
		printf("Pagina: %d ", i);
		printf("Marco: %d ", (nodo+(i*sizeof(stRegistroTP)))->marco);
		printf("bit2ndChance: %d ", (nodo+(i*sizeof(stRegistroTP)))->bit2ndChance);
		printf("bitModificado: %d ", (nodo+(i*sizeof(stRegistroTP)))->bitModificado);
		printf("bitPresencia: %d ", (nodo+(i*sizeof(stRegistroTP)))->bitPresencia);
		printf("\n");
	}
}
void _mostrarContenidoMemoria(stNodoListaTP* nodoPid){
	stRegistroTP *nodo;
	stPosicion posicion;
	int i;
	uint16_t marco;
	void* buffer;

	printf("\npid: %d\n", nodoPid->pid);
	nodo= ((stRegistroTP*)nodoPid->tabla);
	for(i=0;i<nodoPid->size;i++){
		if(((stRegistroTP*)nodo+(i*sizeof(stRegistroTP)))->bitPresencia==1){
			printf("Pagina %d:\n", i);
			posicion.offset=0;
			posicion.pagina=i;
			posicion.size=losParametros.frameSize;

			marco = ((stRegistroTP*)nodo+(i*sizeof(stRegistroTP)))->marco;
			buffer = calloc(1, losParametros.frameSize+1);
			if(leerMemoria(&buffer, marco, posicion)!=0){
				log_error("no se pudo leer memoria - marco: %d", marco);
			}
			printf("%s", (char*)buffer);
			free(buffer);
		}
	}

}
void marcarMemoriaModificada(uint16_t pid){
	int i;
	stRegistroTP *nodo;
	stNodoListaTP *list_nodo;

	list_nodo = buscarPID(pid);
	printf("pid: %d\n", list_nodo->pid);
	nodo= ((stRegistroTP*)list_nodo->tabla);
	for(i=0;i<list_nodo->size;i++){
		printf("Pagina: %d ", i);
		printf("Marco: %d ", (nodo+(i*sizeof(stRegistroTP)))->marco);
		(nodo+(i*sizeof(stRegistroTP)))->bitModificado=1;
		printf("Marcado como modificado");
		printf("\n");
	}
}
void listarMemoria(){

	list_mutex_iterate(TablaMarcos, (void*)_mostrarContenidoMemoria);

}

void listarMemoriaPid(uint16_t pid){
	stNodoListaTP* nodoPid;

	//sleep(losParametros.delay);
	nodoPid = buscarPID(pid);
	_mostrarContenidoMemoria(nodoPid);
}
void mostrarTabla(){

	//sleep(losParametros.delay);
	list_mutex_iterate(TablaMarcos, (void*)_mostrarContenidoTP);
	return;
}
void mostrarTablaPid(uint16_t pid){
	//sleep(losParametros.delay);
	_mostrarContenidoTP(buscarPID(pid));
}

void liberarTablaPid(uint16_t pid){
	stNodoListaTP *nodo = NULL;
	stRegistroTP *registro = NULL;
	int i;
	int index = 0;


	nodo = buscarPID(pid);
	if(nodo){

		for(i=0;i<nodo->size;i++){
			registro = nodo->tabla+(sizeof(stRegistroTP)*i);
			liberarMarco(registro->marco);
		}
		free(registro);

		// Busco indice de TablaMarcos ( se elimina por index :( )
		nodo = NULL;
		i = 0;
		void _index(stNodoListaTP *list_nodo){
			if(list_nodo->pid == pid){
				nodo = list_nodo;
				index = i;
			}
			i++;
		}
		if(nodo)
			sleep(losParametros.delay);
			list_mutex_remove(TablaMarcos,index);
	}
}
