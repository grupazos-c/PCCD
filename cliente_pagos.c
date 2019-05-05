#include "nodo.h"

/**
 * Envia el numero de pagos que reciba por argumentos a cada nodo existente.
 * Envia un mensaje a cada nodo tantas veces como el numero que nos digan.
 */

int main(int argc, char *argv[]) {

	int num_pagos;
	int buzones[NUM_NODOS];
	struct MensajeIntranodo pagos;
	pagos.mtype = PAGO;
	int clave;

	if (argc != 2) {
		printf("Numero de argumentos incorrecto. (NUM_MENSAJES)\n");
		exit(0);
	} else {
		num_pagos = atoi(argv[1]);
	}

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
		for (int j = 0; j<NUM_NODOS; j++) {
			msgsnd(buzones[j], &pagos, sizeof(pagos) - sizeof(long), 0);
		}
	}

}
