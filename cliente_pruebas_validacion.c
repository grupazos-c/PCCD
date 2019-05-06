#include "nodo.h"

/**
 * Envia el numero de pagos que reciba por argumentos a cada nodo existente.
 * Envia un mensaje a cada nodo tantas veces como el numero que nos digan.
 */
 int num_pagos;
 int buzones[NUM_NODOS];
 struct MensajeIntranodo pagos;
 struct MensajeIntranodo consultas;
 struct MensajeIntranodo anulaciones;
 struct MensajeIntranodo prereservas;

 int clave;

 void enviar_pagos();
 void consultas_un_nodo();
 void prioridades();
 int input();
int main(int argc, char *argv[]) {



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

  int choice;
  while (1) {
    printf("\n");
    printf("\n");
    printf("\n");
   printf("1) Ejecutar la prueba 1 (Mandar x procesos de pagos)\n");
   printf("2) Ejecutar la prueba 2 (Consultas en un nodo, llegan pagos a otro)\n");
   printf("3) Ejecutar la prueba 3 (Prioridades)\n");
   printf("\n0) Pulsa 0 para salir\n\n");
   choice = input();

     switch (choice) {
     case 1: {
         enviar_pagos();
         break;
     }
     case 2: {
         consultas_un_nodo();
         break;
     }
     case 3: {
         prioridades();
         break;
     }
     case 0: {
         exit(0);
     }
     default:
         printf("Numero no valido\n");
     }
   }

   return 0;
}



int input(){
   int number;
   scanf("%d", &number);
   return (number);
}

void enviar_pagos(){
  pagos.mtype = PAGO;
  printf("Introduce el numero de pagos que quieres enviar:\n");
  num_pagos = input();
  for (int i=0; i<num_pagos; i++){
		for (int j = 0; j<NUM_NODOS; j++) {
			msgsnd(buzones[j], &pagos, sizeof(pagos) - sizeof(long), 0);
		}
	}
  return;
}

void consultas_un_nodo(){
  pagos.mtype = PAGO;
  consultas.mtype = CONSULTA;


  //envia 10 consultas al nodo 0
  for (int i=0; i<10; i++){
			msgsnd(buzones[0], &consultas, sizeof(consultas) - sizeof(long), 0);
	}
  sleep(1);
  //envia 1 pago al nodo 1
  msgsnd(buzones[1], &pagos, sizeof(pagos) - sizeof(long), 0);
  sleep(1);
  //envia consultas al nodo 0
  for (int i=0; i<10; i++){
			msgsnd(buzones[0], &consultas, sizeof(consultas) - sizeof(long), 0);
	}
}

void prioridades(){
  pagos.mtype = PAGO;
  consultas.mtype = CONSULTA;
  anulaciones.mtype = ANULACION;
  prereservas.mtype = PRERESERVA;
  msgsnd(buzones[0], &pagos, sizeof(pagos) - sizeof(long), 0);
  sleep(1);
  msgsnd(buzones[1], &pagos, sizeof(consultas) - sizeof(long), 0);
  msgsnd(buzones[1], &consultas, sizeof(consultas) - sizeof(long), 0);
  msgsnd(buzones[1], &anulaciones, sizeof(consultas) - sizeof(long), 0);
  msgsnd(buzones[2], &prereservas, sizeof(consultas) - sizeof(long), 0);
  msgsnd(buzones[2], &pagos, sizeof(consultas) - sizeof(long), 0);
  msgsnd(buzones[2], &anulaciones, sizeof(consultas) - sizeof(long), 0);

}
