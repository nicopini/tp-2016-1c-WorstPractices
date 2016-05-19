/*
 * elestaclibrary.h
 *
 *  Created on: 12/5/2016
 *      Author: utnso
 */

#ifndef COMMONS_ELESTACLIBRARY_H_
#define COMMONS_ELESTACLIBRARY_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "sockets.h"

typedef struct{
	uint16_t processId;		/* identificador del proceso del PCB. */
	uint16_t cantidadPaginas;/* cantidad de paginas que necesita el programa. */
} __attribute__((packed)) stPageIni;

typedef struct {
	uint16_t  offset;
	uint16_t  size;
	uint16_t  pagina;

}stPosicion;						/*Representa a el Indice de Codigo propuesto por el TP*/

typedef struct{
	uint16_t nroPagina;	/* numero de pagina a escribir. */
	uint16_t offset;		/* desplazamiento de la posicion inicial del pagina a escribir. */
	uint16_t tamanio;	/* cantidad de bytes a escribir. */
	void* buffer;	/* datos a escribir. */
} stEscrituraPagina;

typedef struct{
	uint16_t processId;		/* identificador del proceso del PCB. */
}  __attribute__((packed))stPageEnd;

typedef struct{
	uint16_t paginasXProceso;
	uint16_t tamanioPagina;
} __attribute__((packed))stUMCConfig;

typedef struct {
	uint16_t length;
	char *data;
}t_stream;

t_stream* serializarConfigUMC(stUMCConfig *self);
stUMCConfig* deserializarConfigUMC(t_stream *stream);

#endif /* COMMONS_ELESTACLIBRARY_H_ */
