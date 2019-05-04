#include "nodo.h"

int TESTIGO = 0; //0 si no lo tiene 1 si sí
int dentro = 0;
int buzon[NUM_NODOS]; //id del buzon
struct peticion peticiones[NUM_NODOS] = { { 0 } }; //Vector de peticiones recibidas
int atendidas[NUM_NODOS] = { 0 }; //Vector de peticiones atendidas (se actualizada la recibir el testigo)
int mi_peticion = 1;
int peticion_maxima = 0;
int hilo_escritor = 0;
int lectores = 0; //numero de lectores esperando
int leyendo = 0;  //numero de lectores en seccion critica
int identificador_ejecucion = 0;

FILE *fp;
static int SESSION_TRACKER; //Keeps track of session

struct info_cola procesos_cola;

/****************************************************************************************
 * 										TODO											*
 * 	Hilo de lectores añadido, falta:													*
 *	 - Que hago si soy le ultimo lector: Envío el testigo di fuese necesario			*
 *	 - Cambiar hilo de escritores para que se lleve bien con el nuevo hilo de lectores	*
 *	 - Cambiar el hilo receptor para que haga lo suyo									*
 *	 - Ver como implementamos las copias de los testigos								*
 *	hola a todos																					*
 ****************************************************************************************/

void log_print(int id_proceso, char *evento) {

	if (SESSION_TRACKER > 0) {
		switch (id_nodo) {
		case 1:
			fp = fopen("logNodo1.txt", "a+");
			break;
		case 2:
			fp = fopen("logNodo2.txt", "a+");
			break;
		case 3:
			fp = fopen("logNodo3.txt", "a+");
			break;
		case 4:
			fp = fopen("logNodo4.txt", "a+");
			break;
		case 0:
			fp = fopen("logNodo0.txt", "a+");
			break;
		default:
			break;
		}
	} else {
		switch (id_nodo) {
		case 1:
			fp = fopen("logNodo1.txt", "w");
			break;
		case 2:
			fp = fopen("logNodo2.txt", "w");
			break;
		case 3:
			fp = fopen("logNodo3.txt", "w");
			break;
		case 4:
			fp = fopen("logNodo4.txt", "w");
			break;
		case 0:
			fp = fopen("logNodo0.txt", "w");
			break;
		default:
			break;
		}
	}

	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	unsigned long time_in_micros = 1000000 * ts.tv_sec + ts.tv_nsec;
	fprintf(fp, "%ld", time_in_micros);
	fprintf(fp, ":%i", id_proceso);
	fprintf(fp, ":%s", evento);

	fputc('\n', fp);
	SESSION_TRACKER++;
	fclose(fp);
}

/**
 *	Parametros de entrada:
 *		Id_nodo: int de 0 a N-1 con el identificador del nodo
 */
int main(int argc, char *argv[]) {

	if (argc != 2) {
		printf("Numero de argumentos incorrecto.\n");
		exit(0);
	} else {
		id_nodo = atoi(argv[1]);
	}

	printf("Inicializando el nodo...\n");

	inicializarNodo();

	struct MensajeIntranodo proceso;

	peticiones[id_nodo].id_peticion = mi_peticion;
	peticiones[id_nodo].prioridad = 5;

	do {

		// SI EN msgrcv EL ARGUMENTO TYPE ES NEGATIVO, COGERA EL PRIMERO CON EL TIPO MAS BAJO (Y MENOR QUE EL VALOR ABSOLUTO DEL ARGUMENTO TYPE DE msgrcv)
		printf("\nNodo %i (Main): Esperando por mensajes intranodo...\n",
				id_nodo);
		if (msgrcv(idBuzonIntranodo, &proceso, sizeof(proceso) - sizeof(long),
				-4, 0) == -1) {
			printf("\nError en la recepción del mensaje\n");
			perror("msgrcv");
			exit(0);
		}
		printf(
				"\nNodo %i (Main): Se ha recogido un proceso de la cola de procesos\n",
				id_nodo);

		int tipoproceso = proceso.mtype;

		printf("\nNodo %i (Main): Proceso de tipo %i\n", id_nodo, tipoproceso);

		switch (tipoproceso) {
		case 1:
			printf("\nNodo %i (Main): Añadiendo proceso de tipo 1 a la cola\n",
					id_nodo);
			procesos_cola.anulaciones++;

			sem_wait(&acceso_hilo_escritor);
			if (hilo_escritor == 0) {
				sem_post(&acceso_hilo_escritor);
				initEscritor();
			} else
				sem_post(&acceso_hilo_escritor);
			break;

		case 2:
			printf("\nNodo %i (Main): Añadiendo proceso de tipo 2 a la cola\n",
					id_nodo);
			procesos_cola.pagos++;

			sem_wait(&acceso_hilo_escritor);
			if (hilo_escritor == 0) {
				sem_post(&acceso_hilo_escritor);
				initEscritor();
			} else
				sem_post(&acceso_hilo_escritor);
			break;

		case 3:
			printf("\nNodo %i (Main): Añadiendo proceso de tipo 3 a la cola\n",
					id_nodo);
			procesos_cola.prereservas++;

			sem_wait(&acceso_hilo_escritor);
			if (hilo_escritor == 0) {
				sem_post(&acceso_hilo_escritor);
				initEscritor();
			} else
				sem_post(&acceso_hilo_escritor);
			break;

		case 4:
			printf("\nNodo %i (Main): Añadiendo proceso de tipo 4 a la cola\n",
					id_nodo);
			initLector();
			procesos_cola.consultas++;
			break;

		default:
			printf(
					"\nNodo %i (Main): No se puede ejecutar un proceso de tipo %i\n",
					id_nodo, tipoproceso);
			continue;
		}

	} while (1);

	return 0;

}

/**
 * Iniciar hilo de escritor
 */
void initEscritor() {
	pthread_t hiloEscritor;
	//Iniciamos el hilo encargado de la gestion de los procesos intranodo/////////
	if (pthread_create(&hiloEscritor, NULL, (void *) escritor, NULL) != 0) {
		printf("No se ha podido iniciar el hilo de gestion de escritor.\n");
		perror("pthread_create");
		exit(0);
	} else {
		sem_wait(&acceso_hilo_escritor);
		hilo_escritor = 1;
		sem_post(&acceso_hilo_escritor);
		//printf("Nodo %i (Main): Hilo de gestión de escritores iniciado correctamente.\n", id_nodo);
	}

}

/**
 * Iniciar hilo de lector
 */
void initLector() {
	pthread_t hiloLector;
	if (pthread_create(&hiloLector, NULL, (void *) lector, NULL) != 0) {
		printf("No se ha podido iniciar el hilo de gestion de lector.\n");
		perror("pthread_create");
		exit(0);
	} else {
		//printf("Nodo %i (Main): Hilo de gestión de lector iniciado correctamente.\n", id_nodo);
	}
}

/*
 * HILO de cada lector
 * if(primero == 1) --> Es el primer lector
 * */
void *lector() {

	sem_wait(&acceso_id_ejecucion);
	int mi_identificador = identificador_ejecucion++;
	sem_post(&acceso_id_ejecucion);

	log_print(mi_identificador, "start");

	sem_wait(&acceso_lectores);
	sem_wait(&acceso_leyendo);

	if (lectores == 0 && leyendo == 0) { 		//Soy el primer lector

		sem_post(&acceso_leyendo);
		lectores++;
		sem_post(&acceso_lectores);

		primerLector();

		sem_wait(&acceso_lectores);
		if (lectores > 0) {
			sem_post(&acceso_lectores);
			sem_post(&paso_lectores); //aviso al siguieten lector, él deberá averiguar si puede o no ejecutarse
		} else {
			sem_post(&acceso_lectores);
		}

		/****************************
		 * 	Sección crítica			*
		 ****************************/
		log_print(mi_identificador, "entradaSC");
		printf("\nNodo %i (Lector): Estoy en mi Sección crítica\n", id_nodo);

		struct timespec tim, tim2;
		tim.tv_sec = 0;
		tim.tv_nsec = 20000;
		nanosleep(&tim, &tim2);

		log_print(mi_identificador, "salidaSC");
		printf("\nNodo %i (Lector): He salido de la seccion critica\n",
				id_nodo);
		/****************************
		 *  FIN	Sección crítica		*
		 ****************************/

		sem_wait(&acceso_leyendo);
		atendidas[id_nodo] = mi_peticion;
//		dentro = 0; esto lo dirá eñ ultimo
		leyendo--;
		//sem_post(&acceso_leyendo);
		peticiones[id_nodo].id_peticion = ++mi_peticion;
		if (leyendo == 0) {
			sem_post(&acceso_leyendo);
			sem_post(&papel);
			//ultimoLector(); //esta funcio debería mandar el testigo en caso de ser necesario a un proceso de escritura
		} else
			sem_post(&acceso_leyendo);

	} else if (lectores > 0) { //tengo que esperar a que el lector delante de mí me dé permiso

		sem_post(&acceso_leyendo);
		lectores++;
		sem_post(&acceso_lectores);

		sem_wait(&paso_lectores); //espero a que el lector que va delante mia me abra el semaforo
		sem_wait(&acceso_hilo_escritor);
		if (hilo_escritor == 1) {
			sem_post(&acceso_hilo_escritor);
			//en este caso hay un escritor (al menos) esperando por tanto debo convertirme en primer lector y competir por la exclusión mutua
			printf("\nMe convierto en el primer lector gg");
			primerLector();

		} else {
			sem_post(&acceso_hilo_escritor);

			sem_wait(&acceso_lectores);
			sem_wait(&acceso_leyendo);
			leyendo++;
			lectores--;
			sem_post(&acceso_lectores);
			sem_post(&acceso_leyendo);
		}

		sem_wait(&acceso_lectores);
		if (lectores > 0) {
			sem_post(&acceso_lectores);
			sem_post(&paso_lectores); //aviso al siguieten lector, él deberá averiguar si puede o no ejecutarse
		} else
			sem_post(&acceso_lectores);

		/****************************
		 * 	Sección crítica			*
		 ****************************/
		log_print(mi_identificador, "entradaSC");

		printf("\nNodo %i (Lector): Estoy en mi Sección crítica\n", id_nodo);
		struct timespec tim, tim2;
		tim.tv_sec = 0;
		tim.tv_nsec = 20000;
		nanosleep(&tim, &tim2);
		printf("\nNodo %i (Lector): He salido de la seccion critica\n",
				id_nodo);
		log_print(mi_identificador, "salidaSC");

		/****************************
		 *  FIN	Sección crítica		*
		 ****************************/

		sem_wait(&acceso_leyendo);
		atendidas[id_nodo] = mi_peticion;
//		dentro = 0; esto lo dirá el ultimo
		leyendo--;
		//sem_post(&acceso_leyendo); //no puedo hacer un signal porque aún voy a comprobar si leyendo es igual a 0
		peticiones[id_nodo].id_peticion = ++mi_peticion;

		if (leyendo == 0) {
			sem_post(&acceso_leyendo);
			sem_post(&papel);
			//ultimoLector(); //esta funcio debería mandar el testigo en caso de ser necesario a un proceso de escritura
		} else
			sem_post(&acceso_leyendo);

	} else if (leyendo > 0) { //puedo entrar SOLO si no hay escritores esperanod, en cuyo caso me transformaré en un lecotr primero

		printf("\nMe quedo en el tercer if");

		sem_post(&acceso_leyendo);
		lectores++;
		sem_post(&acceso_lectores);
		//se cierran aqui acceso_lectores y acceso_leyendo

		sem_wait(&acceso_hilo_escritor);
		if (hilo_escritor == 1) {
			sem_post(&acceso_hilo_escritor);
			//en este caso hay un escritor (al menos) esperando por tanto debo convertirme en primer lector y competir por la exclusión mutua
			primerLector();
		} else {
			sem_post(&acceso_hilo_escritor);
			sem_wait(&acceso_lectores);
			sem_wait(&acceso_leyendo);
			leyendo++;
			lectores--;
			sem_post(&acceso_lectores);
			sem_post(&acceso_leyendo);
		}

		sem_wait(&acceso_lectores);
		if (lectores > 0) {
			sem_post(&acceso_lectores);
			sem_post(&paso_lectores); //aviso al siguieten lector, él deberá averiguar si puede o no ejecutarse
		} else {
			sem_post(&acceso_lectores);
		}

		/****************************
		 * 	Sección crítica			*
		 ****************************/
		log_print(mi_identificador, "entradaSC");
		printf("\nNodo %i (Lector): Estoy en mi Sección crítica\n", id_nodo);
		struct timespec tim, tim2;
		tim.tv_sec = 0;
		tim.tv_nsec = 20000;
		nanosleep(&tim, &tim2);
		printf("Nodo %i (Lector): He salido de la seccion critica\n", id_nodo);
		log_print(mi_identificador, "salidaSC");

		/****************************
		 *  FIN	Sección crítica		*
		 ****************************/

		sem_wait(&acceso_leyendo);
		atendidas[id_nodo] = mi_peticion;
		//		dentro = 0; esto lo dirá el ultimo
		leyendo--;
		//sem_post(&acceso_leyendo);
		peticiones[id_nodo].id_peticion = ++mi_peticion;

		if (leyendo == 0) {
			sem_post(&acceso_leyendo);
			sem_post(&papel);
			//ultimoLector(); //esta funcio debería mandar el testigo en caso de ser necesario a un proceso de escritura
		} else
			sem_post(&acceso_leyendo);

	}

	log_print(mi_identificador, "stop");
	return NULL;
}

/**
 * Función de comportamiento del Lector designado como primer lector.
 * Aquel que deba luchar con los escritores por la exclusión mutua
 */
void primerLector() {

	sem_wait(&papel);	//compruebo que no haya ningún escritor

	//Tenemos que pedir el testigo (si no lo tneemos), a partir de aquí entendemos que el proceso lector está "en ejecución": aun que
	//no este en su seccion critica, ningun proceso escritor va a poder pararlo ahora, pero si pueden parar al resto de lectores que esperan

	//printf("\nNodo %i (Lector): Soy el primer lector. Pulse enter para intentar entrar en seccion critica\n", id_nodo);
	//while (getchar() != '\n');
	sleep(2);

	//Solo actualizamos esto si es el primer lector, pero independientemente de que tenga o no el testigo :p
	mi_peticion = ++peticion_maxima;
	peticiones[id_nodo].id_peticion = mi_peticion;
	peticiones[id_nodo].prioridad = 4;

	sem_wait(&acceso_TESTIGO);

	if (TESTIGO == 0) { // Soy el primer lector. No hay más escritores. Tengo que hacerme con el testigo.
		sem_post(&acceso_TESTIGO);

		printf("Nodo %i (Lector): No tengo el testigo\n", id_nodo);
		struct request_msgbuf mensaje;
		mensaje.mtype = 2;
		mensaje.mtext.id_peticion = mi_peticion;
		mensaje.mtext.origen = id_nodo;
		mensaje.mtext.prioridad = 4; //los receptores deberán ahora hacer algo distinto si la peticion es de tipo 4 (hacer una copia)

		for (int i = 0; i < NUM_NODOS; ++i) {
			if (i != id_nodo) {	//Este if es para que no se envíe una peticion a sí mismo
				printf("Nodo %i (Lector): Enviando solicitud al nodo: %i\n",
						id_nodo, i);
				msgsnd(buzon[i], &mensaje, sizeof(mensaje) - sizeof(long), 0);
			}
		}

		struct testigo_msgbuf testigomsg;
		msgrcv(buzon[id_nodo], &testigomsg, sizeof(testigomsg) - sizeof(long),
				1, 0);
		for (int i = 0; i < NUM_NODOS; i++) {
			if (testigomsg.mtext.atendidas[i] > atendidas[i])
				atendidas[i] = testigomsg.mtext.atendidas[i];
		}
		printf("\nNodo %i (Lector): Testigo recibido\n", id_nodo);
		//dentro = 1; 	//TODO diferenciar dentro de leyendo en receptor, podemos cargarnos uno de los dos seguro

		sem_wait(&acceso_TESTIGO);
		TESTIGO = 1;
		sem_post(&acceso_TESTIGO);

		sem_wait(&acceso_lectores);
		sem_wait(&acceso_leyendo);
		leyendo++;
		lectores--;
		sem_post(&acceso_lectores);
		sem_post(&acceso_leyendo);

	} else {
		sem_post(&acceso_TESTIGO);

		//parece que se ejecutaría sin problema, no??
		//dentro = 1; 	//TODO lo mismo que arriba

		sem_wait(&acceso_lectores);
		sem_wait(&acceso_leyendo);
		leyendo++;
		lectores--;
		sem_post(&acceso_lectores);
		sem_post(&acceso_leyendo);
	}
}

void ultimoLector() { //

}

/**
 * Gestion escritor
 */
void *escritor() {

	int tipoproceso;

	sem_wait(&papel);

	do {

		sem_wait(&acceso_id_ejecucion);
		int mi_identificador = identificador_ejecucion++;
		sem_post(&acceso_id_ejecucion);

		log_print(mi_identificador, "start");

		if (procesos_cola.anulaciones > 0) {
			tipoproceso = 1;
			procesos_cola.anulaciones--;
		} else if (procesos_cola.pagos > 0) {
			tipoproceso = 2;
			procesos_cola.pagos--;
		} else if (procesos_cola.prereservas > 0) {
			tipoproceso = 3;
			procesos_cola.prereservas--;
		}

		//printf("\nNodo %i (Escritor): Se ha detectado un proceso de tipo %i\n", id_nodo, tipoproceso);

		//printf("\nNodo %i (Escritor): TESTIGO: %i\n", id_nodo, TESTIGO);

		//printf("\nNodo %i (Escritor): Pulse enter para intentar entrar en seccion critica\n", id_nodo);
		//while (getchar() != '\n');

		mi_peticion = ++peticion_maxima;
		peticiones[id_nodo].id_peticion = mi_peticion;
		peticiones[id_nodo].prioridad = tipoproceso;


		sem_wait(&acceso_TESTIGO);
		if (TESTIGO == 0) {
			sem_post(&acceso_TESTIGO);

			printf("\nNodo %i (Escritor): No tengo el testigo\n", id_nodo);
			struct request_msgbuf mensaje;
			mensaje.mtype = 2;
			mensaje.mtext.id_peticion = mi_peticion;
			mensaje.mtext.origen = id_nodo;
			mensaje.mtext.prioridad = tipoproceso;

			for (int i = 0; i < NUM_NODOS; ++i) {
				if (i != id_nodo) {	//Este if es para que no se envíe una peticion a sí mismo
					printf(
							"\nNodo %i (Escritor): Enviando solicitud al nodo: %i\n",
							id_nodo, i);
					msgsnd(buzon[i], &mensaje, sizeof(mensaje) - sizeof(long),
							0);
				}
			}
			struct testigo_msgbuf testigomsg;
			msgrcv(buzon[id_nodo], &testigomsg,
					sizeof(testigomsg) - sizeof(long), 1, 0);
			for (int i = 0; i < NUM_NODOS; i++) {
				if (testigomsg.mtext.atendidas[i] > atendidas[i])
					atendidas[i] = testigomsg.mtext.atendidas[i];
			}
			printf("\nNodo %i (Escritor): Testigo recibido\n", id_nodo);
			//dentro = 1;

			sem_wait(&acceso_TESTIGO);
			TESTIGO = 1;
			sem_post(&acceso_TESTIGO);

		} else {
			sem_post(&acceso_TESTIGO);
			//dentro = 1;
		}

		if (procesos_cola.anulaciones > 0) {
			peticiones[id_nodo].prioridad = 1;
		} else if (procesos_cola.pagos > 0) {
			peticiones[id_nodo].prioridad = 2;
		} else if (procesos_cola.prereservas > 0) {
			peticiones[id_nodo].prioridad = 3;
		} else {
			peticiones[id_nodo].prioridad = 5;
		}

		/****************************
		 * 	Sección crítica			*
		 ****************************/
		log_print(mi_identificador, "entradaSC");
		printf(
				"\nNodo %i (Escritor): Estoy en mi Sección crítica con un proceso de tipo %i\n",
				id_nodo, tipoproceso);
		struct timespec tim, tim2;
		tim.tv_sec = 0;
		tim.tv_nsec = 20000;
		nanosleep(&tim, &tim2);
		printf("\nNodo %i (Escritor): He salido de la seccion critica\n",
				id_nodo);
		log_print(mi_identificador, "salidaSC");
		/****************************
		 * 	Sección crítica			*
		 ****************************/

		atendidas[id_nodo] = mi_peticion;
		//dentro = 0;

		peticiones[id_nodo].id_peticion = ++mi_peticion;

		int id_nodo_sig = nodo_Prioritario();
		if (id_nodo_sig != id_nodo) {
			printf(
					"\nNodo %i: Enviando el testigo al nodo %i mediante el gestor intranodo dado que la peticion del nodo es %i y la ultima atendida es %i\n",
					id_nodo, id_nodo_sig, peticiones[id_nodo_sig].id_peticion,
					atendidas[id_nodo_sig]);
			send_token(id_nodo_sig);
		}

		log_print(mi_identificador, "stop");

	} while (procesos_cola.anulaciones != 0 || procesos_cola.pagos != 0
			|| procesos_cola.prereservas != 0);

	sem_wait(&acceso_hilo_escritor);
	hilo_escritor = 0;
	sem_post(&acceso_hilo_escritor);

	sem_post(&papel);

	return NULL; // Se podría evitar la muerte del hilo con un semaforo???
}

void *gestionReceptor() {
	struct request_msgbuf mensaje;
	int origen, id_peticion_origen, prio_peticion_origen;
	while (1) {
		msgrcv(buzon[id_nodo], &mensaje, sizeof(mensaje) - sizeof(long), 2, 0);
		origen = mensaje.mtext.origen;
		printf("ORIGEN: %i\n", origen);
		id_peticion_origen = mensaje.mtext.id_peticion; //TODO hay que poner que el id_peticion sea > los id recibidos
		prio_peticion_origen = mensaje.mtext.prioridad;
		printf("\nNodo %i (Receptor): Peticion recibida del nodo %i\n", id_nodo,
				origen);
		//TODO es muy tarde así que no voy a tocar el código no la líe, pero esto np deberia de copmprobar la prioridad?? y si es más prioritario guardarlo
		//NO el id de peticion que es solo para desempatar en caso de igual prioridad :)
		//if (id_peticion_origen > peticiones[origen].id_peticion) {

		// Compruebo el id de la petición por si es una que está desactualizada y ya ha sido atendida. Las atendidas lo miro en el testigo.
		if (id_peticion_origen > atendidas[origen]) {
			if (prio_peticion_origen < peticiones[origen].prioridad
					|| peticiones[origen].id_peticion <= atendidas[origen]) {
				printf(
						"\nNodo %i (Receptor): Peticion actualizada para nodo %i al valor de peticion %i\n",
						id_nodo, origen, id_peticion_origen);
				peticiones[origen].id_peticion = id_peticion_origen;
				peticiones[origen].prioridad = prio_peticion_origen;
				printf("ID PETICION ORIGEN: %i\n", id_peticion_origen);
				printf("PRIORIDAD ORIGEN: %i\n", prio_peticion_origen);
			}
		} else { // La petición ya ha sido atendida.
			return NULL;
		}

		//Esto envia el token sólo cuando el escritor se ha dormido y no ha pasado el testigo TODO
		sem_wait(&acceso_hilo_escritor);
		sem_wait(&acceso_TESTIGO);
		// Voy a probar con la variable leyendo en lugar de dentro.
		sem_wait(&acceso_leyendo);
		if ((TESTIGO == 1) && (leyendo == 0) && (hilo_escritor == 0)) {
			sem_post(&acceso_leyendo);
			sem_post(&acceso_TESTIGO);
			sem_post(&acceso_hilo_escritor);

			int nodo_prio = nodo_Prioritario();
			if (nodo_prio != id_nodo) { //TODO esta linea es innecesaria, pero por si
				send_token(nodo_prio);
			}
		} else {
			sem_post(&acceso_leyendo);
			sem_post(&acceso_TESTIGO);
			sem_post(&acceso_hilo_escritor);
		}

		if (mi_peticion < id_peticion_origen)
			peticion_maxima = id_peticion_origen;

	}
}

/**
 * Función que devuelve el nodo más prioritario para la siguiente ejecución
 */
int nodo_Prioritario() {
	int nodo_prio = id_nodo;
	for (int i = 0; i < NUM_NODOS; ++i) {
		if (peticiones[i].id_peticion > atendidas[i]) {
			if (peticiones[i].prioridad == peticiones[nodo_prio].prioridad) {

				if (peticiones[i].id_peticion
						< peticiones[nodo_prio].id_peticion) {
					nodo_prio = i;
				} else if (peticiones[i].id_peticion
						== peticiones[nodo_prio].id_peticion) {
					if (i <= nodo_prio) { //Es mejor que cause inanicon en un nodo o pasandose el testigo ?? TODO
						nodo_prio = i;
					}
				}
			} else if (peticiones[i].prioridad
					< peticiones[nodo_prio].prioridad) {
				nodo_prio = i;
			}
		}
	}
	return nodo_prio;
}

void send_token(int id_destino) {
	sem_wait(&acceso_TESTIGO);
	TESTIGO = 0;
	sem_post(&acceso_TESTIGO);

	struct testigo_msgbuf testigomsg;
	testigomsg.mtype = 1;
	for (int i = 0; i < NUM_NODOS; i++) {
		testigomsg.mtext.atendidas[i] = atendidas[i];
	}

	printf("Nodo %i (Token senderator): Enviando el testigo al nodo %i\n",
			id_nodo, id_destino);

	msgsnd(buzon[id_destino], &testigomsg, sizeof(testigomsg) - sizeof(long),
			0);

}

/**
 * Inicializa las variables generales del nodo: ID, TESTIGO Y BUZON.
 * Crea y ejecuta los diferentes hilos de gestion del nodo.
 */
void inicializarNodo() {

	sem_init(&papel, 0, 1);
	sem_init(&paso_lectores, 0, 0);
	sem_init(&acceso_lectores, 0, 1);
	sem_init(&acceso_leyendo, 0, 1);
	sem_init(&acceso_hilo_escritor, 0, 1);
	sem_init(&acceso_TESTIGO, 0, 1);
	sem_init(&acceso_id_ejecucion, 0, 1);

	//TODO Automatizar la creacion de los buzones
	//Acordarmos darle el testigo al primer nodo. Podría haber sido otro
	sem_wait(&acceso_TESTIGO);
	if (id_nodo == 0)
		TESTIGO = 1;
	else
		TESTIGO = 0;
	sem_post(&acceso_TESTIGO);

	////////////////////////////////////////////////////////////////////

	//Creamos el BUZON INTERNODO//////////////////////////////////////
	buzon[id_nodo] = msgget(id_nodo, (0777 | IPC_CREAT));
	if (buzon[id_nodo] < 0) {
		printf("Error al crear el buzón internodo\n");
		perror("msgget");
		exit(0);
	} else {
		printf("Se ha creado el buzón internodo con el ID: %i.\n",
				buzon[id_nodo]);
	}
	////////////////////////////////////////////////////////////////////

	//Inicializamos el HILO encargado de recepción de peticiones
	pthread_t hiloReceptor;
	int estado_hilo_inter = pthread_create(&hiloReceptor, NULL,
			(void *) gestionReceptor, NULL);
	if (estado_hilo_inter != 0) {
		printf("No se ha podido crear el hilo de recepción de peticiones.\n");
		perror("pthread_create");
		exit(0);
	} else {
		printf("Hilo de recepción de peticiones creado correctamente.\n");
	}
	//////////////////////////////////////////////////////////////////////////////

	//Pedimos todos los ids de los buzones de los demás nodos.
	printf("Las colas de mensajes del sistema son: \n");
	system("ipcs -q");		//Para borrar sería ipcrm --all=msg :)
	for (int i = 0; i < NUM_NODOS; ++i) {
		if (i != id_nodo) {
			printf("\nIntroduzca el id del buzon del nodo %i: ", i);
			scanf("%i", &buzon[i]);
			while (getchar() != '\n')
				; //Este detector de salto de línea está para que no loquée y entre a su sec, criti
		}
	}
	///////////////////////////////////////////////////////////

	//Creamos el buzón intranodo///////////////////////////////////////
	int clave_intranodo = ftok("./README.md", id_nodo);
	idBuzonIntranodo = msgget(clave_intranodo, (0777 | IPC_CREAT));
	if (idBuzonIntranodo < 0) {
		printf("\nError al crear el buzón intranodo\n");
		perror("msgget");
		exit(0);
	} else {
		printf("\nSe ha creado el buzón intranodo con el ID: %i.\n",
				idBuzonIntranodo);
	}
	////////////////////////////////////////////////////////////////////

	/* semaforos de acceso a variables: (igual no necesitamos todos?? //TODO
	 sem_t acceso_TESTIGO;
	 sem_t acceso_dentro;
	 sem_t acceso_peticiones;
	 sem_t acceso_atendidas;
	 sem_t acceso_mi_peticion;
	 sem_t acceso_peticion_maxima;
	 sem_t acceso_hilo_escritor;
	 sem_t acceso_lectores;
	 sem_t acceso_leyendo;
	 */

}
