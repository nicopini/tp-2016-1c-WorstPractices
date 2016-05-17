/*
 ============================================================================
 Name        : Nucleo.c
 Author      : Jose Maria Suarez
 Version     : 0.1
 Description : Elestac - Nucleo
 ============================================================================
 */

#include <commons/config.h>
#include <commons/socketsIPCIRC.h>
#include <commons/ipctypes.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/elestaclibrary.h>
#include <commons/parser/metadata_program.h>
#include <commons/pcb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>


#include "nucleo.h"
#include "interprete.h"

/*
 ============================================================================
 Estructuras y definiciones
 ============================================================================
 */

/* Listas globales */
fd_set fds_master;			/* Lista de todos mis sockets.*/
fd_set read_fds;	  		/* Sublista de fds_master.*/

t_queue *colaReady;   	 	/*Cola de todos los PCB listos para ejecutar*/
t_queue *colaExit;		 	/*Cola de todos los PCB listos para liberar*/
t_queue *colaBlock;		 	/*Cola de todos los PCB listos para liberar*/


pthread_mutex_t mutexColaReady;

/*
 ============================================================================
 Funciones
 ============================================================================
 */
void loadInfo (stEstado* info){

	t_config* miConf = config_create (CFGFILE); /*Estructura de configuracion*/

	if (config_has_property(miConf,"IP")) {
		info->miIP  = config_get_string_value(miConf,"IP");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","IP");
		exit(-2);
	}

	if (config_has_property(miConf,"PUERTO")) {
		info->miPuerto = config_get_int_value(miConf,"PUERTO");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO");
		exit(-2);
	}

	if (config_has_property(miConf,"IP_UMC")) {
		info->ipUmc = config_get_string_value(miConf,"IP_UMC");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","IP_UMC");
		exit(-2);
	}

	if (config_has_property(miConf,"PUERTO_UMC")) {
		info->puertoUmc = config_get_int_value(miConf,"PUERTO_UMC");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_UMC");
		exit(-2);
	}


	if (config_has_property(miConf,"QUANTUM")) {
		info->quantum = config_get_int_value(miConf,"QUANTUM");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","QUANTUM");
		exit(-2);
	}

	if (config_has_property(miConf,"QUANTUM_SLEEP")) {
		info->quantumSleep = config_get_int_value(miConf,"QUANTUM_SLEEP");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","QUANTUM_SLEEP");
		exit(-2);
	}

	if (config_has_property(miConf,"SEM_IDS")) {
		info->semIds = config_get_array_value(miConf,"SEM_IDS");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","SEM_IDS");
		exit(-2);
	}

	if (config_has_property(miConf,"SEM_INIT")) {
		info->semInit = config_get_array_value(miConf,"SEM_INIT");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","SEM_INIT");
		exit(-2);
	}

	if (config_has_property(miConf,"IO_IDS")) {
		info->ioIds = config_get_array_value(miConf,"IO_IDS");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","IO_IDS");
		exit(-2);
	}

	if (config_has_property(miConf,"IO_SLEEP")) {
		info->semInit = config_get_array_value(miConf,"IO_SLEEP");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","IO_SLEEP");
		exit(-2);
	}

	if (config_has_property(miConf,"SHARED_VARS")) {
		info->sharedVars = config_get_array_value(miConf,"SHARED_VARS");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","SHARED_VARS");
		exit(-2);
	}
}

void monitoreoConfiguracion(stEstado* info){
	pthread_t id = pthread_self();
	char buffer[BUF_LEN];

	// Al inicializar inotify este nos devuelve un descriptor de archivo
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

	// Creamos un monitor sobre un path indicando que eventos queremos escuchar
	int watch_descriptor = inotify_add_watch(file_descriptor, CFGFILE, IN_MODIFY | IN_CREATE | IN_DELETE);
	int length = read(file_descriptor, buffer, BUF_LEN);
	if (length < 0) {
		perror("read");
	}
	loadInfo(info);
	printf("\nEl archivo de configuracion se ha modificado\n");
	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);
	monitoreoConfiguracion(info);
	pthread_exit(NULL);
}


void cerrarSockets(stEstado *elEstadoActual){

	int unSocket;
	for(unSocket=3; unSocket <= elEstadoActual->fdMax; unSocket++)
		if(FD_ISSET(unSocket,&(fds_master)))
			close(unSocket);

	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));
}

void finalizarSistema(stMensajeIPC *unMensaje,int unSocket, stEstado *unEstado){

	unEstado->salir = 1;
	unMensaje->header.tipo = -1;
}


void threadCPU(int unCpu){
	stHeaderIPC *stHeaderIPC;

	while(1){
		if(queue_size(colaReady)==0){
			continue;
		}
		pthread_mutex_lock(&mutexColaReady);
		stPCB *stPCB = queue_pop(colaReady);
		pthread_mutex_unlock(&mutexColaReady);

		stHeaderIPC = nuevoHeaderIPC(EXECANSISOP);

		if(!enviarHeaderIPC(unCpu, stHeaderIPC)){
			printf("Se perdio la conexion con el CPU conectado\n");
			close(unCpu);
			break;
		}

		/*TODO: para recibir desde el CPU: recv(unCliente, (char *) &stPCB, sizeof(stPCB), NULL);*/
		if(send(unCpu, (const char *) &stPCB, sizeof(stPCB),0)==-1){
			printf("No se pudo enviar el PCB porque se desconecto el CPU\n");
			/*Lo ponemos en la cola de Ready para que otro CPU lo vuelva a tomar*/
			pthread_mutex_lock(&mutexColaReady);
			queue_push(colaReady,stPCB);
			pthread_mutex_unlock(&mutexColaReady);
			printf("Se replanifica el PCB\n");
			break;
		}

		if(!recibirHeaderIPC(unCpu,stHeaderIPC)){
			printf("HandShake Error - No se pudo recibir mensaje de respuesta\n");
			pthread_mutex_lock(&mutexColaReady);
			queue_push(colaReady,stPCB);
			pthread_mutex_unlock(&mutexColaReady);
			close(unCpu);
			break;
		 }else{
			 switch (stHeaderIPC->tipo) {
				case IOANSISOP:
					/*Pide I/O*/
					break;

			}
		 }




	}
	liberarHeaderIPC(stHeaderIPC);
	pthread_exit(NULL);
}
/*
 ============================================================================
 Funcion principal
 ============================================================================
 */
int main(int argc, char *argv[]) {
	stHeaderIPC *stHeaderIPC;
	stEstado elEstadoActual;
	stMensajeIPC unMensaje;
	t_metadata_program *metadata_program;

	/*Inicializamos las listas todo:liberarlas luego*/
	colaReady = queue_create();
	colaExit  = queue_create();
	colaBlock = queue_create();


	int unCliente = 0, unSocket;
	int maximoAnterior = 0;
	struct sockaddr addressAceptado;

	int agregarSock;

	pthread_t  p_thread;

	printf("----------------------------------Elestac------------------------------------\n");
	printf("-----------------------------------Nucleo------------------------------------\n");
	printf("------------------------------------v0.1-------------------------------------\n\n");
	fflush(stdout);

	/*Carga del archivo de configuracion*/
	printf("Obteniendo configuracion...");
	loadInfo(&elEstadoActual);
	printf("OK\n");

	/*Se lanza el thread para identificar cambios en el archivo de configuracion*/
	pthread_create(&p_thread, NULL,(void*)&monitoreoConfiguracion,(void*)&elEstadoActual);

	/*Inicializacion de listas de socket*/
	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));

	/*Inicializacion de socket de escucha*/
	elEstadoActual.salir = 0;
	elEstadoActual.sockEscuchador= -1;

	/*Iniciando escucha en el socket escuchador de Consola*/
	printf("Estableciendo conexion con socket escuchador...");
	elEstadoActual.sockEscuchador = escuchar(elEstadoActual.miPuerto);
	FD_SET(elEstadoActual.sockEscuchador,&(fds_master));
	printf("OK\n\n");

	/*Seteamos el maximo socket*/
	elEstadoActual.fdMax = elEstadoActual.sockEscuchador;

	/*Conexion con el proceso UMC*/
	printf("Estableciendo conexion con la UMC...");
	elEstadoActual.sockUmc= conectar(elEstadoActual.ipUmc,elEstadoActual.puertoUmc);



	if (elEstadoActual.sockUmc != -1){
			FD_SET(elEstadoActual.sockUmc,&(fds_master));

			memset(unMensaje.contenido,'\0',LONGITUD_MAX_DE_CONTENIDO);

			recibirMensajeIPC(elEstadoActual.sockUmc,&unMensaje);

			if(unMensaje.header.tipo == QUIENSOS)
			{
				if(!enviarMensajeIPC(elEstadoActual.sockUmc,nuevoHeaderIPC(CONNECTNUCLEO),"")){
					printf("No se envio CONNECTNUCLEO a la UMC\n");
				}
			}

			memset(unMensaje.contenido,'\0',LONGITUD_MAX_DE_CONTENIDO);
			recibirMensajeIPC(elEstadoActual.sockUmc,&unMensaje);

			if(unMensaje.header.tipo == OK)
			{
				elEstadoActual.fdMax =	elEstadoActual.sockUmc;
				maximoAnterior = elEstadoActual.fdMax;
				printf("OK\n\n");
			}
			else
			{
				printf("No se pudo establecer la conexion con la UMC\n");
			}

		}
		else{
			printf("No se pudo establecer la conexion con la UMC\n");
		}

	/*Ciclo Principal del Nucleo*/
	printf(".............................................................................\n");
	fflush(stdout);
	printf("..............................Esperando Conexion.............................\n\n");
	fflush(stdout);

	while(elEstadoActual.salir == 0)
	{
		read_fds = fds_master;

		if(seleccionar(elEstadoActual.fdMax,&read_fds,1) == -1){
			printf("SELECT ERROR - Error Preparando el Select\n");
			return 1;
		}

		for(unSocket=0;unSocket<=elEstadoActual.fdMax;unSocket++){

			if(FD_ISSET(unSocket,&read_fds)){
			/*Nueva conexion*/
			if(unSocket == elEstadoActual.sockEscuchador){
				unCliente = aceptar(elEstadoActual.sockEscuchador,&addressAceptado);
				printf("Nuevo pedido de conexion...\n");

				stHeaderIPC = nuevoHeaderIPC(QUIENSOS);
				if(!enviarHeaderIPC(unCliente,stHeaderIPC)){
					printf("HandShake Error - No se pudo enviar mensaje QUIENSOS\n");
					liberarHeaderIPC(stHeaderIPC);
					close(unCliente);
					continue;
				}

				if(!recibirHeaderIPC(unCliente,stHeaderIPC)){
					printf("HandShake Error - No se pudo recibir mensaje de respuesta\n");
					liberarHeaderIPC(stHeaderIPC);
					close(unCliente);
					continue;
				 }

				/*Identifico quien se conecto y procedo*/
				switch (stHeaderIPC->tipo) {
					case CONNECTCONSOLA:

						stHeaderIPC = nuevoHeaderIPC(OK);
						if(!enviarHeaderIPC(unCliente, stHeaderIPC)){
							printf("No se pudo enviar un mensaje de confirmacion a la consola conectada\n");
							liberarHeaderIPC(stHeaderIPC);
							close(unCliente);
							continue;
						}

						printf("Nueva consola conectada\n");
						agregarSock=1;

						/*Agrego el socket conectado A la lista Master*/
						if(agregarSock==1){
							FD_SET(unCliente,&(fds_master));
							if (unCliente > elEstadoActual.fdMax){
								maximoAnterior = elEstadoActual.fdMax;
								elEstadoActual.fdMax = unCliente;
							}
							agregarSock=0;
						}

						/* Recibo Programa */
						if(!recibirMensajeIPC(unCliente,&unMensaje)){
							printf("Error:No se recibio el mensaje de la consola\n");
							break;
						}else{
							if (unMensaje.header.tipo == SENDANSISOP) {

								metadata_program = metadata_desde_literal(unMensaje.contenido);

							}

						 }

						break;

					case CONNECTCPU:

						if(!enviarMensajeIPC(unCliente,nuevoHeaderIPC(OK),"MSGOK")){
							printf("No se pudo enviar el MensajeIPC al CPU\n");
							return 0;
						}

						printf("Nuevo CPU conectado\n");
						agregarSock=1;

						/*Agrego el socket conectado A la lista Master*/
						if(agregarSock==1){
							FD_SET(unCliente,&(fds_master));
							if (unCliente > elEstadoActual.fdMax){
								maximoAnterior = elEstadoActual.fdMax;
								elEstadoActual.fdMax = unCliente;
							}
							agregarSock=0;
						}



						break;
					default:
						break;
				}
			}else
			{
				/*Conexion existente*/
				memset(unMensaje.contenido,'\0',LONGITUD_MAX_DE_CONTENIDO);
				if (!recibirMensajeIPC(unSocket,&unMensaje)){
					if(unSocket==elEstadoActual.sockEscuchador){
						printf("Se perdio conexion...\n ");
					}
					/*TODO: Sacar de la lista de cpu conectados si hay alguna desconexion.*/
					/*Saco el socket de la lista Master*/
					FD_CLR(unSocket,&fds_master);
					close (unSocket);

					if (unSocket > elEstadoActual.fdMax){
						maximoAnterior = elEstadoActual.fdMax;
						elEstadoActual.fdMax = unSocket;
					}

					fflush(stdout);
				}else{
					/*Recibo con mensaje de conexion existente*/


					}


				}



			}

		}
	}
			cerrarSockets(&elEstadoActual);
			finalizarSistema(&unMensaje,unSocket,&elEstadoActual);
			printf("\nNUCLEO: Fin del programa\n");
			return 0;
}
