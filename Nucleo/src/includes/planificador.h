/*
 * ready.h
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include 	<commons/pcb.h>
#include 	<commons/collections/queue.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<pthread.h>

void *ready_productor(void *arg);
stPCB *ready_consumidor();
void eliminar_pcb_ready(int pid);
void destruir_cola_ready();

#endif /* PLANIFICADOR_H_ */
