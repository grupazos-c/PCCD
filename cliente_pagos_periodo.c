#include "nodo.h"

/**
 * Enviamos el numero de mensajes recibido por parametros a cada nodo. Esperamos un periodo recibido entre cada rafaga de mensajes
 * Envia un mensaje a cada nodo tantas veces como el numero que nos digan.
 */
int main(int argc, char *argv[]) {

	int num_pagos;
	long periodo;
	int buzones[NUM_NODOS];
	struct MensajeIntranodo pagos;
	pagos.mtype = PAGO;
	int clave;
	struct timespec tim, tim2;

	if (argc != 3) {
		printf("Numero de argumentos incorrecto. (NUM_MENSAJES PERIODO)\n");
		exit(0);
	} else {
		num_pagos = atoi(argv[1]);
		periodo = atol(argv[2]);
	}

	if (periodo > 999999999) {
		tim.tv_sec = 1;
		tim.tv_nsec = 0;
	} else {
		tim.tv_sec = 0;
		tim.tv_nsec = periodo;
	}

	nanosleep(&tim, &tim2);
	printf("Fin del primer nanosleep");

	printf("NUMERO DE NODO = %i\n", NUM_NODOS);

	for (int i = 0; i<NUM_NODOS; i++) {
		printf("Creando buzon para el nodo %i\n", i);
		clave= ftok("./README.md", i);
		buzones[i] = msgget(clave, 0777);
			if (buzones[i] < 0) {
				printf("\nNo existe el buzon\n");
				perror("msgget");
				exit(0);
			}
	}

	for (int i=0; i<num_pagos; i++){
		//printf("Iteraccion %i de envio de mensajes a todos los nodos\n", i);
		for (int j = 0; j<NUM_NODOS; j++) {
			msgsnd(buzones[j], &pagos, sizeof(pagos) - sizeof(long), 0);
			//printf("Mensaje de pagos enviado a nodo %i\n" , j);
		}
		nanosleep(&tim, &tim2);
	}

}
