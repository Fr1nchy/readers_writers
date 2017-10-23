#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>

typedef struct{
	sem_t sem_fichier;
	pthread_mutex_t mutex_l;
	int nb_lecteurs;
	int nb_redacteurs;
}lecteur_redacteur_t;

void debut_redaction(){

}



int main (int argc, char **argv){

	pthread_t* writers_tab;
	pthread_t* readers_tab;

if (argc != 3)
{
	fprintf(stderr, "usage : %s nb_readers nb_writers \n", argv[0]) ;
	exit (-1) ;
}
	nb_readers = atoi(argv[1]);
	nb_writers = atoi(argv[2]);
	int i;

	for (i = 0; i < nb_writers; i++)
	{
		pthread_create()

	}


	return EXIT_SUCCESS;
}