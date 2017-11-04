#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct{
    pthread_mutex_t mutex_global;
    sem_t sem_fichier;
    int nb_lecteurs;
}lecteur_redacteur_t;

typedef struct {
    lecteur_redacteur_t lecteur_redacteur;
    int iterations;
    int donnee;
} donnees_thread_t;

void debut_redaction(lecteur_redacteur_t *lect_red){
	printf("Arrivée du thread redacteur %x\n", (int) pthread_self());
    // Prend la ressource si le jeton est disponible sinon le rédacteur est en attente
    sem_wait(&lect_red->sem_fichier);
}

void fin_redaction(lecteur_redacteur_t *lect_red){
    // Rédacteur rend le jeton sur la sémaphore pour qu'il puisse être utilisé par un autre thread
    sem_post(&lect_red->sem_fichier);
}
void debut_lecture(lecteur_redacteur_t *lect_red){
	printf("Arrivée du thread lecteur %x\n", (int) pthread_self());
    pthread_mutex_lock(&lect_red->mutex_global);
    /* Si le nombre de lecteurs est à zéro alors le thread prend
    le jeton de la sémaphore qui va réserver la ressource aux lecteurs */
    if(lect_red->nb_lecteurs == 0)
        sem_wait(&lect_red->sem_fichier);

    lect_red->nb_lecteurs++;
    pthread_mutex_unlock(&lect_red->mutex_global);
}

void fin_lecture(lecteur_redacteur_t *lect_red){
    pthread_mutex_lock(&lect_red->mutex_global);
    lect_red->nb_lecteurs--;

    /*le dernier lecteur à lire va rendre le jeton de la sémaphore
    pour rendre la ressource disponible. Un thread rédacteur ou lecteur pourra réservé
    à nouveau la ressource */
    if(lect_red->nb_lecteurs == 0)
        sem_post(&lect_red->sem_fichier);

    pthread_mutex_unlock(&lect_red->mutex_global);
}

void initialiser_lecteur_redacteur(lecteur_redacteur_t *lect_red){
    lect_red->nb_lecteurs = 0; // zéro lecteur au début du programme
    pthread_mutex_init(&lect_red->mutex_global,NULL);
    sem_init(&lect_red->sem_fichier,0,1); // Initialisation de la sémaphore à un, un seul thread peut prendre le vérrou
}

void detruire_lecteur_redacteur(lecteur_redacteur_t *lect_red){
    pthread_mutex_destroy(&lect_red->mutex_global);
    sem_destroy(&lect_red->sem_fichier);
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
