#include "nodo.h"

int TESTIGO = 0; //0 si no lo tiene 1 si sí
int buzon[NUM_NODOS]; //id del buzon
struct peticion peticiones[NUM_NODOS] = { { 0 } }; //Vector de peticiones recibidas
int atendidas[NUM_NODOS] = { 0 }; //Vector de peticiones atendidas (se actualizada la recibir el testigo)
int mi_peticion = 1;
int peticion_maxima = 0;
int hilo_escritor = 0;
int lectores = 0; //numero de lectores esperando
int leyendo = 0;  //numero de lectores en seccion critica
int identificador_ejecucion = 0;
char* log_dir;

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/*
 * SEMAFOROS EN:
peticion peticiones[NUM_NODOS]
atendidas[NUM_NODOS] = { 0 };
mi_peticion = 1;
peticion_maxima = 0;
*/

FILE *fp;
static int SESSION_TRACKER = 0;
char stlog[50];

struct info_cola procesos_cola;

void log_print(int id_proceso, char *evento, int prioridad) {

	printf("Accediendo a %s", stlog);

	sem_wait(&acceso_log);
	if (SESSION_TRACKER > 0) {
		fp = fopen(stlog, "a+");
	} else {
		printf("Entrando en el archivo por primera vez");
		struct stat st = {0};
		if (stat(log_dir, &st) == -1) {
		    mkdir(log_dir, 0700);
		}
		fp = fopen(stlog, "w");
	}


	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	unsigned long time_in_nanos = 1000000000 * ts.tv_sec + ts.tv_nsec; //Un segundo son 1e9 ns TODO, mejor revisar que no me estoy colando con el valor
	fprintf(fp, "%ld", time_in_nanos);
	fprintf(fp, ":%i", id_proceso);
	fprintf(fp, ":%s", evento);
	fprintf(fp, ":%i", prioridad);

	fputc('\n', fp);
	SESSION_TRACKER++;
	fclose(fp);
	sem_post(&acceso_log);
}

/**
 *	Parametros de entrada:
 *		Id_nodo: int de 0 a N-1 con el identificador del nodo
 */
int main(int argc, char *argv[]) {

	if (argc != 3) {
		printf("Numero de argumentos incorrecto.\n Uso: ./nodo [id_nodo] [dir_log]");
		exit(0);
	} else {
		id_nodo = atoi(argv[1]);
		log_dir = argv[2];
	}

	printf("Inicializando el nodo...\n");

	inicializarNodo(log_dir);

	struct MensajeIntranodo proceso;

	sem_wait(&acceso_peticiones);
	peticiones[id_nodo].id_peticion = mi_peticion;
	peticiones[id_nodo].prioridad = 5;
	sem_post(&acceso_log);

	do {

		// SI EN msgrcv EL ARGUMENTO TYPE ES NEGATIVO, COGERA EL PRIMERO CON EL TIPO MAS BAJO (Y MENOR QUE EL VALOR ABSOLUTO DEL ARGUMENTO TYPE DE msgrcv)
//		printf("\nNodo %i (Main): Esperando por mensajes intranodo...\n", id_nodo);
		if (msgrcv(idBuzonIntranodo, &proceso, sizeof(proceso) - sizeof(long),
				-4, 0) == -1) {
			printf("\nError en la recepción del mensaje\n");
			perror("msgrcv");
			exit(0);
		}
//		printf("\nNodo %i (Main): Se ha recogido un proceso de la cola de procesos\n", id_nodo);

		int tipoproceso = proceso.mtype;

//		printf("\nNodo %i (Main): Proceso de tipo %i\n", id_nodo, tipoproceso);

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
//		printf("Nodo %i (Main): Hilo de gestión de lector iniciado correctamente.\n", id_nodo);
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

	log_print(mi_identificador, "start", 4);

	sem_wait(&acceso_lectores);
	sem_wait(&acceso_leyendo);

	/*PRIMER LECTOR*/
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
		log_print(mi_identificador, "entradaSC", 4);
		printf("\nNodo %i (Lector): Estoy en mi Sección crítica @como primer lector@\n", id_nodo);

		struct timespec tim, tim2;
		tim.tv_sec = S_SLEEP;
		tim.tv_nsec = NS_SLEEP;
		nanosleep(&tim, &tim2);

		log_print(mi_identificador, "salidaSC", 4);
		printf("\nNodo %i (Lector): He salido de la seccion critica\n", id_nodo);
		/****************************
		 *  FIN	Sección crítica		*
		 ****************************/

		sem_wait(&acceso_leyendo);
//		atendidas[id_nodo] = mi_peticion; //TODO: y está, do deberíamos decir que está atendida, cuando se haya acabado: último lector??
		leyendo--;

//		peticiones[id_nodo].id_peticion = ++mi_peticion; //TODO: por que existe esta línea??

		if (leyendo == 0) {
			sem_post(&acceso_leyendo);
			ultimoLector(); //esta funcio debería mandar el testigo en caso de ser necesario a un proceso de escritura
		} else
			sem_post(&acceso_leyendo);

	/*LECTOR EN COLA*/
	} else if (lectores > 0) { //tengo que esperar a que el lector delante de mí me dé permiso

		sem_post(&acceso_leyendo);
		lectores++;
		sem_post(&acceso_lectores);

		sem_wait(&paso_lectores); //espero a que el lector que va delante mia me abra el semaforo
		sem_wait(&acceso_hilo_escritor);
		if (hilo_escritor == 1 || nodo_Prioritario() != id_nodo) {
			sem_post(&acceso_hilo_escritor);
			//en este caso hay un escritor (al menos) esperando por tanto debo convertirme en primer lector y competir por la exclusión mutua
			printf("\nNodo %i (Lector): Me convierto en primer lector\n", id_nodo);
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
		log_print(mi_identificador, "entradaSC", 4);

		printf("\nNodo %i (Lector): Estoy en mi Sección crítica @despues de esperar en cola@\n", id_nodo);
		struct timespec tim, tim2;
		tim.tv_sec = S_SLEEP;
		tim.tv_nsec = NS_SLEEP;
		nanosleep(&tim, &tim2);
		printf("\nNodo %i (Lector): He salido de la seccion critica\n",
				id_nodo);
		log_print(mi_identificador, "salidaSC", 4);

		/****************************
		 *  FIN	Sección crítica		*
		 ****************************/

		sem_wait(&acceso_leyendo);

//		atendidas[id_nodo] = mi_peticion;
		leyendo--;
//		peticiones[id_nodo].id_peticion = ++mi_peticion;

		if (leyendo == 0) {
			sem_post(&acceso_leyendo);
			ultimoLector(); //esta funcio debería mandar el testigo en caso de ser necesario a un proceso de escritura
		} else
			sem_post(&acceso_leyendo);

	/*LECTOR QUE "PUEDE" ENTRAR*/
	} else if (leyendo > 0) { //puedo entrar SOLO si no hay escritores esperanod, en cuyo caso me transformaré en un lecotr primero

		sem_post(&acceso_leyendo);
		lectores++;
		sem_post(&acceso_lectores);
		//se cierran aqui acceso_lectores y acceso_leyendo

		sem_wait(&acceso_hilo_escritor);
		if (hilo_escritor == 1 || nodo_Prioritario() != id_nodo) { //TODO Tienen que comprobar tambien en las peticiones recibidas
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
		log_print(mi_identificador, "entradaSC", 4);
		printf("\nNodo %i (Lector): Estoy en mi Sección crítica @Porque ya habia lectores leyendo@\n", id_nodo);
		struct timespec tim, tim2;
		tim.tv_sec = S_SLEEP;
		tim.tv_nsec = NS_SLEEP;
		nanosleep(&tim, &tim2);
		printf("Nodo %i (Lector): He salido de la seccion critica\n", id_nodo);
		log_print(mi_identificador, "salidaSC", 4);

		/****************************
		 *  FIN	Sección crítica		*
		 ****************************/

		sem_wait(&acceso_leyendo);
//		atendidas[id_nodo] = mi_peticion;
		leyendo--;
//		peticiones[id_nodo].id_peticion = ++mi_peticion;

		if (leyendo == 0) {
			sem_post(&acceso_leyendo);
			ultimoLector(); //esta funcio debería mandar el testigo en caso de ser necesario a un proceso de escritura
		} else
			sem_post(&acceso_leyendo);

	}

	log_print(mi_identificador, "stop", 4);

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

	//Solo actualizamos esto si es el primer lector, pero independientemente de que tenga o no el testigo :p
	mi_peticion = ++peticion_maxima;

	sem_wait(&acceso_peticiones);
	peticiones[id_nodo].id_peticion = mi_peticion;
	peticiones[id_nodo].prioridad = 4;
	sem_post(&acceso_peticiones);

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
			sem_wait(&acceso_atendidas);
			if (testigomsg.mtext.atendidas[i] > atendidas[i]){
				sem_post(&acceso_atendidas);
				atendidas[i] = testigomsg.mtext.atendidas[i];
			} else {
				sem_post(&acceso_atendidas);
			}
		}
		printf("\nNodo %i (Lector): Testigo recibido\n", id_nodo);

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

		sem_wait(&acceso_lectores);
		sem_wait(&acceso_leyendo);
		leyendo++;
		lectores--;
		sem_post(&acceso_lectores);
		sem_post(&acceso_leyendo);
	}
}

void ultimoLector() {

//	sem_post(&papel);//SAMEDIT: No se deberia comprobar al final de la funcion? Otro proceso de este nodo podria ejecutarse en cuanto este semaforo se abra

	printf("\n\tNodo %i (UltimoLector): Soy el ultimo lector\n", id_nodo);

	sem_wait(&acceso_atendidas);
	atendidas[id_nodo] = mi_peticion;
	sem_post(&acceso_atendidas);
//	peticiones[id_nodo].id_peticion = ++mi_peticion;

	int id_nodo_sig = nodo_Prioritario();
	printf("\n\tNodo %i (UltimoLector): El nodo mas prioritario a mi salida es: %i\n", id_nodo, id_nodo_sig);
	if (id_nodo_sig != id_nodo) {
		printf("\nNodo %i: Enviando el testigo al nodo %i mediante el gestor intranodo dado que la peticion del nodo es %i y la ultima atendida es %i\n",
				id_nodo, id_nodo_sig, peticiones[id_nodo_sig].id_peticion, atendidas[id_nodo_sig]);
		send_token(id_nodo_sig);
	}

	sem_post(&papel);

	return;

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

		log_print(mi_identificador, "start", tipoproceso);

		//printf("\nNodo %i (Escritor): Se ha detectado un proceso de tipo %i\n", id_nodo, tipoproceso);

		//printf("\nNodo %i (Escritor): TESTIGO: %i\n", id_nodo, TESTIGO);

		//printf("\nNodo %i (Escritor): Pulse enter para intentar entrar en seccion critica\n", id_nodo);
		//while (getchar() != '\n');

		mi_peticion = ++peticion_maxima;

		sem_wait(&acceso_peticiones);
		peticiones[id_nodo].id_peticion = mi_peticion;
		peticiones[id_nodo].prioridad = tipoproceso;
		sem_post(&acceso_peticiones);

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
				sem_wait(&acceso_atendidas);
				if (testigomsg.mtext.atendidas[i] > atendidas[i]){
					atendidas[i] = testigomsg.mtext.atendidas[i];
				}
				sem_post(&acceso_atendidas);
			}
			printf("\nNodo %i (Escritor): Testigo recibido\n", id_nodo);

			sem_wait(&acceso_TESTIGO);
			TESTIGO = 1;
			sem_post(&acceso_TESTIGO);

		} else {
			sem_post(&acceso_TESTIGO);
		}


		sem_wait(&acceso_peticiones);
		if (procesos_cola.anulaciones > 0) { //TODO No capto la idea...?¿?¿
			peticiones[id_nodo].prioridad = 1;
		} else if (procesos_cola.pagos > 0) {
			peticiones[id_nodo].prioridad = 2;
		} else if (procesos_cola.prereservas > 0) {
			peticiones[id_nodo].prioridad = 3;
		} else {
			peticiones[id_nodo].prioridad = 5;
		}
		sem_post(&acceso_peticiones);

		/****************************
		 * 	Sección crítica			*
		 ****************************/
		log_print(mi_identificador, "entradaSC", tipoproceso);
		printf("\nNodo %i (Escritor): Estoy en mi Sección crítica con un proceso de tipo %i\n", id_nodo, tipoproceso);
		struct timespec tim, tim2;
		tim.tv_sec = S_SLEEP;
		tim.tv_nsec = NS_SLEEP;
		nanosleep(&tim, &tim2);
		printf("\nNodo %i (Escritor): He salido de la seccion critica\n", id_nodo);
		log_print(mi_identificador, "salidaSC", tipoproceso);
		/****************************
		 * 	Sección crítica			*
		 ****************************/

		sem_wait(&acceso_atendidas);
		atendidas[id_nodo] = mi_peticion;
		sem_post(&acceso_atendidas);

		sem_wait(&acceso_peticiones);
		peticiones[id_nodo].id_peticion = ++mi_peticion;
		sem_post(&acceso_peticiones);

		int id_nodo_sig = nodo_Prioritario();
		if (id_nodo_sig != id_nodo) {
			printf(
					"\nNodo %i: Enviando el testigo al nodo %i mediante el gestor intranodo dado que la peticion del nodo es %i y la ultima atendida es %i\n",
					id_nodo, id_nodo_sig, peticiones[id_nodo_sig].id_peticion,
					atendidas[id_nodo_sig]);
			send_token(id_nodo_sig);
		}

		log_print(mi_identificador, "stop", tipoproceso);

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
		printf("\nNodo %i (Receptor): Peticion recibida del nodo %i\n", id_nodo, origen);
		//TODO es muy tarde así que no voy a tocar el código no la líe, pero esto np deberia de copmprobar la prioridad?? y si es más prioritario guardarlo
		//NO el id de peticion que es solo para desempatar en caso de igual prioridad :)
		//if (id_peticion_origen > peticiones[origen].id_peticion) {

		// Compruebo el id de la petición por si es una que está desactualizada y ya ha sido atendida. Las atendidas lo miro en el testigo.
		printf("\nNodo %i (Receptor): id_peticion_origen = %i, atendidas_origen %i\n", id_nodo, id_peticion_origen, atendidas[origen]);
		sem_wait(&acceso_atendidas);
		if (id_peticion_origen > atendidas[origen]) {
			sem_wait(&acceso_peticiones);
			if (prio_peticion_origen < peticiones[origen].prioridad	|| peticiones[origen].id_peticion <= atendidas[origen]) {
				sem_post(&acceso_atendidas);
				printf("\nNodo %i (Receptor): Peticion actualizada para nodo %i al valor de peticion %i\n",
						id_nodo, origen, id_peticion_origen);
				peticiones[origen].id_peticion = id_peticion_origen;
				peticiones[origen].prioridad = prio_peticion_origen;
				printf("ID PETICION ORIGEN: %i\n", id_peticion_origen);
				printf("PRIORIDAD ORIGEN: %i\n", prio_peticion_origen);
			} else if(prio_peticion_origen == peticiones[origen].prioridad && id_peticion_origen > peticiones[origen].id_peticion){
				sem_post(&acceso_atendidas);
				printf("\nNodo %i (Receptor): Peticion actualizada para nodo %i al valor de peticion %i\n",
						id_nodo, origen, id_peticion_origen);
				peticiones[origen].id_peticion = id_peticion_origen;
				peticiones[origen].prioridad = prio_peticion_origen;
				printf("ID PETICION ORIGEN: %i\n", id_peticion_origen);
				printf("PRIORIDAD ORIGEN: %i\n", prio_peticion_origen);
			}else {
				sem_post(&acceso_atendidas);
			}
			sem_post(&acceso_peticiones);
		} else { // La petición ya ha sido atendida.
			sem_post(&acceso_atendidas);
			return NULL;
		}

		//Esto envia el token sólo cuando el escritor se ha dormido y no ha pasado el testigo TODO
		sem_wait(&acceso_hilo_escritor);
		sem_wait(&acceso_TESTIGO);
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
 *
int nodo_Prioritario() {
	int nodo_prio = id_nodo;
	printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): NODO 0 - UltimaPeticionEnCursoGuardada: I%i-P%i , UltimaPeticionAtendida: %i"ANSI_COLOR_RESET"\n", id_nodo, peticiones[0].id_peticion, peticiones[0].prioridad, atendidas[0]);
	printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): NODO 1 - UltimaPeticionEnCursoGuardada: I%i-P%i , UltimaPeticionAtendida: %i"ANSI_COLOR_RESET"\n", id_nodo, peticiones[1].id_peticion, peticiones[1].prioridad, atendidas[1]);
	for (int i = 0; i < NUM_NODOS; ++i) {
		//printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): iter-%i, entramos en comparacion de peticion y atendidas"ANSI_COLOR_RESET, id_nodo, i);
		if (peticiones[i].id_peticion > atendidas[i]) {
			//printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): iter-%i, id_peticion > atendidas"ANSI_COLOR_RESET, id_nodo, i);
			if (peticiones[i].prioridad == peticiones[nodo_prio].prioridad) {
				//printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): iter-%i, peticion_prioridad = nodo%i.prioridad"ANSI_COLOR_RESET, id_nodo, i, nodo_prio);
				if (peticiones[i].id_peticion < peticiones[nodo_prio].id_peticion) {
					//printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): iter-%i, id_peticion < nodo%i.id_peticion"ANSI_COLOR_RESET, id_nodo, i, nodo_prio);
					nodo_prio = i;
				} else if (peticiones[i].id_peticion == peticiones[nodo_prio].id_peticion) {
					//printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): iter-%i, id_peticion = nodo%i.id_peticion"ANSI_COLOR_RESET, id_nodo, i, nodo_prio);
					if (i <= nodo_prio) { //Es mejor que cause inanicon en un nodo o pasandose el testigo ?? TODO
						//printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): iter-%i, id_nodo <= nodo%i.id_nodo"ANSI_COLOR_RESET, id_nodo, i, nodo_prio);
						nodo_prio = i;
					}
				}
			} else if (peticiones[i].prioridad < peticiones[nodo_prio].prioridad) {
				printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): iter-%i, peticion_prioridad < nodo%i.prioridad"ANSI_COLOR_RESET, i, id_nodo, nodo_prio);
				nodo_prio = i;
			}
		}
	}
	printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): Se ha decidido el nodo %i como prioritario"ANSI_COLOR_RESET"\n", id_nodo, nodo_prio);
	return nodo_prio;
}
*/

int nodo_Prioritario() {

	int nodo_prio = id_nodo;
	int factibles[NUM_NODOS];

	printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): NODO 0 - UltimaPeticionEnCursoGuardada: I%i-P%i , UltimaPeticionAtendida: %i"ANSI_COLOR_RESET"\n", id_nodo, peticiones[0].id_peticion, peticiones[0].prioridad, atendidas[0]);
	printf(ANSI_COLOR_RED "\nNodo %i (nodo_Prioritario): NODO 1 - UltimaPeticionEnCursoGuardada: I%i-P%i , UltimaPeticionAtendida: %i"ANSI_COLOR_RESET"\n", id_nodo, peticiones[1].id_peticion, peticiones[1].prioridad, atendidas[1]);

	for (int i = 0; i < NUM_NODOS; ++i) {
		sem_wait(&acceso_peticiones);
		sem_wait(&acceso_atendidas);
		if (peticiones[i].id_peticion > atendidas[i]) {
			sem_post(&acceso_peticiones);
			sem_post(&acceso_atendidas);
			factibles[i] = 1;
		} else {
			sem_post(&acceso_peticiones);
			sem_post(&acceso_atendidas);
			factibles[i] = 0;
		}
	}

	if (factibles[id_nodo] == 0) {
		for (int i=0; i<NUM_NODOS; i++) {
			if (factibles[i] == 1) {
				nodo_prio = i;
				break;
			}
		}
	}

	for (int i=0; i<NUM_NODOS; i++) {
		if (factibles[i] == 1) {

			sem_wait(&acceso_peticiones);
			if (peticiones[i].prioridad == peticiones[nodo_prio].prioridad) {
				if (peticiones[i].id_peticion < peticiones[nodo_prio].id_peticion) {
					sem_post(&acceso_peticiones);
					nodo_prio = i;
				} else if (peticiones[i].id_peticion == peticiones[nodo_prio].id_peticion) {
					sem_post(&acceso_peticiones);
					if (i <= nodo_prio) {
						nodo_prio = i;
					}
				} else {
					sem_post(&acceso_peticiones);
				}
			} else if (peticiones[i].prioridad < peticiones[nodo_prio].prioridad) {
				sem_post(&acceso_peticiones);
				nodo_prio = i;
			} else {
				sem_post(&acceso_peticiones);
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
		sem_wait(&acceso_atendidas);
		testigomsg.mtext.atendidas[i] = atendidas[i];
		sem_post(&acceso_atendidas);
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
void inicializarNodo(char* log_dir) {

	strcat(stlog, log_dir);
//	strcat(stlog, "/");
	strcat(stlog, "logNodo");
	char aux[9];
	sprintf(aux, "%d", id_nodo);
	strcat(stlog, aux);
	strcat(stlog, ".txt");
	//Ahora log será: "logNodoXX.txt TODO: que tal añadirle el dir pasado por parametros?

	sem_init(&papel, 0, 1);
		sem_init(&paso_lectores, 0, 0);
		sem_init(&acceso_lectores, 0, 1);
		sem_init(&acceso_leyendo, 0, 1);
		sem_init(&acceso_hilo_escritor, 0, 1);
		sem_init(&acceso_TESTIGO, 0, 1);
		sem_init(&acceso_id_ejecucion, 0, 1);
		sem_init(&acceso_log, 0, 1);
		sem_init(&acceso_peticiones, 0, 1);
		sem_init(&acceso_mi_peticion, 0, 1);
		sem_init(&acceso_peticion_maxima, 0, 1);
		sem_init(&acceso_atendidas, 0, 1);

		//Acordarmos darle el testigo al primer nodo
		sem_wait(&acceso_TESTIGO);
		if (id_nodo == 0)
			TESTIGO = 1;
		else
			TESTIGO = 0;
		sem_post(&acceso_TESTIGO);

		//Obtenemos todos los buzones internodo de todos los nodos y creamos el nuestro//////////////////////////////////////
		for (int i = 0; i < NUM_NODOS; i++) {
			buzon[i] = msgget(i + 1, (0777 | IPC_CREAT));
			if (buzon[i] < 0) {
				printf("Error al crear el buzón internodo\n");
				perror("msgget");
				exit(0);
			} else {
				printf("Se ha creado el buzón internodo para el nodo %i con el ID %i.\n", i, buzon[i]);
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

}
