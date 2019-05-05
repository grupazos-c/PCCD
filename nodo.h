#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <semaphore.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h> //Para crear el directorio

//PRIORIDADES///////////////
#define ANULACION 1
#define PAGO 2
#define PRERESERVA 3
#define CONSULTA 4
///////////////////////////

#define TIPO_PETICION_TESTIGO 1
#define TIPO_ENTREGA_TESTIGO 2
#define NUM_NODOS 256

#define S_SLEEP 0
#define NS_SLEEP 100000 //1000000000ns = 1s


void inicializarNodo();
void *escritor();
void *gestionReceptor();
void *lectorPrimero();
void *lector();
void send_token(int id_destino);
void initEscritor();
void initLector();
int nodo_Prioritario();
void primerLector();
void ultimoLector();
void log_print(int id_proceso,char *evento, int prioridad);

sem_t acceso_log;

sem_t papel; //semaforo que comparten tanto lectores como escritores y en el se suspenden para esperar a que acabe el otro
sem_t paso_lectores; //semaforo de paso de lectores

//semaforos de acceso a variables
sem_t acceso_TESTIGO;
sem_t acceso_dentro;
sem_t acceso_peticiones;
sem_t acceso_atendidas;
sem_t acceso_mi_peticion;
sem_t acceso_peticion_maxima;
sem_t acceso_hilo_escritor;
sem_t acceso_lectores;
sem_t acceso_leyendo;
sem_t acceso_hilo_escritor;
sem_t acceso_TESTIGO;
sem_t acceso_id_ejecucion;

struct MensajeIntranodo {
	long mtype;
	char *text;
};

struct testigo_struct {
	int orden;				//nº de testigos clonados
	int atendidas[NUM_NODOS];//Vector que acoje cual ha sido la última petición atendida de cada nodo
};

struct request_struct {
	int prioridad;			//Prioridad del proceso que solicita ejecutarse
	int origen;				//Nodo que envía la solicitud
	int id_peticion;		//Id de petición
};

/*Msgbuf para envío de solicitudes*/
struct request_msgbuf {
	long mtype;				//reservamos el mtye = 2 para requests
	struct request_struct mtext;
};

/*Msgbuf para envío de testigo*/
struct testigo_msgbuf {
	long mtype;				//reservamos el mtype = 1 para testigos
	struct testigo_struct mtext;
};

struct peticion {
	int prioridad;
	int id_peticion;
};

struct info_cola {
	int anulaciones;
	int pagos;
	int prereservas;
	int consultas;
};

//VARIABLES FIJAS. Se inicializan una vez y no se vuelven a editar. No necesitan Semaforos,
int idBuzonIntranodo; //ID del buzon del nodo especifico para mensajes internos (se crea en base a su id)
int id_nodo; //id del nodo
int num_nodos;
//////////////////////////////////////////////////////////////////////////////////////////
