#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct{
    pthread_mutex_t mutex_global;
    int nb_lecteurs;
    int nb_redacteurs;
    int bool_redacteur;
    pthread_cond_t file_lect;
    pthread_cond_t file_rect;
}lecteur_redacteur_t;

typedef struct {
    lecteur_redacteur_t lecteur_redacteur;
    int iterations;
    int donnee;
} donnees_thread_t;

void debut_redaction(lecteur_redacteur_t *lect_red){
    printf("Arrivee du thread redacteur %x\n", (int) pthread_self());
    pthread_mutex_lock(&lect_red->mutex_global);
    lect_red->nb_redacteurs++;

    /*Les threads rédacteurs sont en attente sur la file_rect si il y'a des lecteurs
    en court ou si il y'a un rédacteur utilisant la ressource*/
    while(lect_red->bool_redacteur || lect_red->nb_lecteurs> 0)
        pthread_cond_wait(&lect_red->file_rect,&lect_red->mutex_global);

    /* Lorsque l'on prend la ressource on affecte un booléen à un pour que le autres rédacteur
    ne puisse accèder à la ressource */
    lect_red->bool_redacteur = 1;
    pthread_mutex_unlock(&lect_red->mutex_global);
}
void fin_redaction(lecteur_redacteur_t *lect_red){
    pthread_mutex_lock(&lect_red->mutex_global);
    lect_red->nb_redacteurs--;
    lect_red->bool_redacteur = 0;

    /*Si le nombre de redacteurs souhaitant accèder à la ressource est supérieur à zéro
    alors on notifie les threads en attente sur la variable condition file_rect*/
    if(lect_red->nb_redacteurs > 0)
        pthread_cond_signal(&lect_red->file_rect);
    /* Sinon on notifie tout les threads lecteurs en attente sur la file_lect*/
    else
        pthread_cond_broadcast(&lect_red->file_lect);

    pthread_mutex_unlock(&lect_red->mutex_global);

}
void debut_lecture(lecteur_redacteur_t *lect_red){
    printf("Arrivee du thread lecteur %x\n", (int) pthread_self());
    pthread_mutex_lock(&lect_red->mutex_global);
    /* Tant qu'il a des rédacteurs, les lecteurs attendent sur la variable condition file_lect
    si aucun rédacteur alors on incrémente sur le nombre de lecteur et la ressource est prise pour
    tout les lecteurs*/
    while(lect_red->nb_redacteurs > 0)
        pthread_cond_wait(&lect_red->file_lect,&lect_red->mutex_global);

    lect_red->nb_lecteurs++;
    pthread_mutex_unlock(&lect_red->mutex_global);
}

void fin_lecture(lecteur_redacteur_t *lect_red){
    pthread_mutex_lock(&lect_red->mutex_global);
    lect_red->nb_lecteurs--;
    /* si le nombre de lecteur est à zéro, lorsque le dernier thread a fini de lire
    alors on signal les threads rédacteurs en attente sur la variable condition file_rect*/
    if(lect_red->nb_lecteurs == 0)
        pthread_cond_signal(&lect_red->file_rect);
    pthread_mutex_unlock(&lect_red->mutex_global);
}

void initialiser_lecteur_redacteur(lecteur_redacteur_t *lect_red){
    lect_red->nb_lecteurs = 0;
    lect_red->nb_redacteurs = 0;
    lect_red->bool_redacteur =0;
    pthread_mutex_init(&lect_red->mutex_global,NULL);
    pthread_cond_init(&lect_red->file_lect,NULL);
    pthread_cond_init(&lect_red->file_rect,NULL);
}

void detruire_lecteur_redacteur(lecteur_redacteur_t *lect_red){
    pthread_mutex_destroy(&lect_red->mutex_global);
    pthread_cond_destroy(&lect_red->file_lect);
    pthread_cond_destroy(&lect_red->file_rect);
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
