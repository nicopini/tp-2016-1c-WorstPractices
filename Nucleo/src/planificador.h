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

t_list *listaBlock; /*Lista de todos los PCB bloqueados*/
t_queue *colaReady; /*Cola de todos los PCB listos para ejecutar*/
t_queue *colaExit; /*Cola de todos los PCB listos para liberar*/

void *ready_productor(void *arg);
stPCB *ready_consumidor();

#endif /* PLANIFICADOR_H_ */