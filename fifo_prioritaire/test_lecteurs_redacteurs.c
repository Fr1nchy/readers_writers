#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
//#include "lecteur_redacteur.h"

pthread_cond_t file_lect = PTHREAD_COND_INITIALIZER;
pthread_cond_t file_rect = PTHREAD_COND_INITIALIZER;

typedef struct{
    pthread_mutex_t mutex_global;
    int nb_lecteurs;
    int nb_redacteurs;
    int bool_redacteur;
}lecteur_redacteur_t;

typedef struct {
    lecteur_redacteur_t lecteur_redacteur;
    int iterations;
    int donnee;
} donnees_thread_t;


void debut_redaction(lecteur_redacteur_t *lect_red){
    pthread_mutex_lock(&lect_red->mutex_global);
    lect_red->nb_redacteurs++;

    while(lect_red->bool_redacteur || lect_red->nb_lecteurs> 0)
        pthread_cond_wait(&file_rect,&lect_red->mutex_global);

    lect_red->bool_redacteur = 1;
    pthread_mutex_unlock(&lect_red->mutex_global);
}
void fin_redaction(lecteur_redacteur_t *lect_red){
    pthread_mutex_lock(&lect_red->mutex_global);
    lect_red->nb_redacteurs--;
    lect_red->bool_redacteur = 0;

    if(lect_red->nb_redacteurs > 0)
        pthread_cond_signal(&file_rect);
    else
        pthread_cond_broadcast(&file_lect);

    pthread_mutex_unlock(&lect_red->mutex_global);

}
void debut_lecture(lecteur_redacteur_t *lect_red){
    pthread_mutex_lock(&lect_red->mutex_global);

    while(lect_red->nb_redacteurs > 0)
        pthread_cond_wait(&file_lect,&lect_red->mutex_global);

    lect_red->nb_lecteurs++;
    pthread_mutex_unlock(&lect_red->mutex_global);
}

void fin_lecture(lecteur_redacteur_t *lect_red){
    pthread_mutex_lock(&lect_red->mutex_global);
    lect_red->nb_lecteurs--;
    if(lect_red->nb_lecteurs == 0)
        pthread_cond_signal(&file_rect);
    pthread_mutex_unlock(&lect_red->mutex_global);
}

void initialiser_lecteur_redacteur(lecteur_redacteur_t *lect_red){
    lect_red->nb_lecteurs = 0;
    lect_red->nb_redacteurs = 0;
    lect_red->bool_redacteur =0;
    pthread_mutex_init(&lect_red->mutex_global,NULL);
    pthread_cond_init(&file_lect,NULL);
    pthread_cond_init(&file_rect,NULL);
}

void detruire_lecteur_redacteur(lecteur_redacteur_t *lect_red){
    pthread_mutex_destroy(&lect_red->mutex_global);
    pthread_cond_destroy(&file_lect);
    pthread_cond_destroy(&file_rect);
}

void dodo(int scale) {
    usleep((random()%1000000)*scale);
}

void *lecteur(void *args) {
    donnees_thread_t *d = args;
    int i, valeur;
    srandom((int) pthread_self());

    for (i=0; i < d->iterations; i++) {
        dodo(2);
	       debut_lecture(&d->lecteur_redacteur);
        	printf("Thread %x : debut lecture\n", (int) pthread_self());
        	valeur = d->donnee;
        	dodo(1);
        	printf("Thread %x : ", (int) pthread_self());
        	if (valeur != d->donnee)
            		printf("LECTURE INCOHERENTE !!!\n");
        	else
            		printf("lecture coherente\n");
        fin_lecture(&d->lecteur_redacteur);
    }
    pthread_exit(0);
}

void *redacteur(void *args) {
    donnees_thread_t *d = args;
    int i, valeur;
    srandom((int) pthread_self());

    for (i=0; i < d->iterations; i++) {
        dodo(2);
        debut_redaction(&d->lecteur_redacteur);
        	printf("Thread %x : debut redaction......\n", (int) pthread_self());
        	valeur = random();
        	d->donnee = valeur;
        	dodo(1);
        	printf("Thread %x : ", (int) pthread_self());
        	if (valeur != d->donnee)
            		printf("REDACTION INCOHERENTE !!!\n");
        	else
            		printf("redaction coherente......\n");
        fin_redaction(&d->lecteur_redacteur);
    }
    pthread_exit(0);
}



int main(int argc, char *argv[]) {
    pthread_t *threads, *thread_courant;
    donnees_thread_t donnees_thread;
    int i, nb_lecteurs, nb_redacteurs;
    void *resultat;

    if (argc < 4) {
        fprintf(stderr, "Utilisation: %s nb_lecteurs nb_redacteurs "
                        "nb_iterations\n", argv[0]);
        exit(1);
    }

    nb_lecteurs = atoi(argv[1]);
    nb_redacteurs = atoi(argv[2]);
    donnees_thread.iterations = atoi(argv[3]);

    threads = malloc((nb_lecteurs+nb_redacteurs)*sizeof(pthread_t));
    thread_courant = threads;
    initialiser_lecteur_redacteur(&donnees_thread.lecteur_redacteur);

    for (i=0; i<nb_lecteurs; i++)
        pthread_create(thread_courant++, NULL, lecteur, &donnees_thread);
    for (i=0; i<nb_redacteurs; i++)
        pthread_create(thread_courant++, NULL, redacteur, &donnees_thread);

    for (i=0; i<nb_lecteurs+nb_redacteurs; i++)
        pthread_join(threads[i], &resultat);
    detruire_lecteur_redacteur(&donnees_thread.lecteur_redacteur);
    free(threads);
    return 0;
}
