#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x)
#endif

typedef enum{thread_lecteur,thread_redacteur} T_state;

typedef struct maillon_t{
    pthread_cond_t cond_thread;
    T_state etat;
    struct maillon_t * next_maillon;
}maillon_t;

typedef struct{
    pthread_mutex_t mutex_global;
    int nb_lecteurs;
    int bool_redacteur;
    maillon_t * tete;
    maillon_t * queue;

}lecteur_redacteur_t;

typedef struct {
    lecteur_redacteur_t lecteur_redacteur;
    int iterations;
    int donnee;
} donnees_thread_t;

void ajouter_queue(lecteur_redacteur_t * lect_red, T_state etat){
        maillon_t * new_maillon = malloc(sizeof(maillon_t));
        new_maillon->next_maillon = NULL;
        new_maillon->etat = etat;
        pthread_cond_init(&new_maillon->cond_thread,NULL);

        if(lect_red->tete == NULL){
            lect_red->tete = new_maillon;
            lect_red->queue = new_maillon;
        }else{
            lect_red->queue->next_maillon = new_maillon;
            lect_red->queue = new_maillon;
        }
}

void enlever_tete(lecteur_redacteur_t * lect_red){
    if(lect_red->tete != NULL){
        maillon_t * temp = lect_red->tete->next_maillon;
        free(lect_red->tete);
        lect_red->tete = temp;
    }
}

void contenu_fifo(lecteur_redacteur_t * lect_red){

    maillon_t * maillon_courant = lect_red->tete;
    DEBUG_PRINT(("\x1b[32m Contenu Fifo:\x1b[32m"));
    while(maillon_courant != NULL){
            DEBUG_PRINT(("%i",maillon_courant->etat));
            maillon_courant = maillon_courant->next_maillon;
    }
    DEBUG_PRINT(("\x1b[0m\n"));
}

void debut_redaction(lecteur_redacteur_t *lect_red){

    pthread_mutex_lock(&lect_red->mutex_global);
    // Affiche le contenu de la fifo si compiler avec la macro Debug
    contenu_fifo(lect_red);
    printf("Thread %x : Veut Ecrire \n", (int) pthread_self());
    // ajoute le redacteur en queue de fifo
    ajouter_queue(lect_red,thread_redacteur);

    /* Tant qu'un redacteur ou des lecteurs utilisent la ressource
    le thread est en attente. Il revérifie cette condition au signal d'un
    autre thread */
    while(lect_red->bool_redacteur || lect_red->nb_lecteurs > 0){
        printf("Thread %x : Est en attente \n", (int) pthread_self());
        pthread_cond_wait(&lect_red->queue->cond_thread,&lect_red->mutex_global);
    }
    // Retire le thread de la fifo
    enlever_tete(lect_red);
    // Le redacteur prend la ressource, booléen à 1
    lect_red->bool_redacteur = 1;
    pthread_mutex_unlock(&lect_red->mutex_global);
}
void fin_redaction(lecteur_redacteur_t *lect_red){

    pthread_mutex_lock(&lect_red->mutex_global);
    // Affiche le contenu de la fifo si compiler avec la macro Debug
    contenu_fifo(lect_red);
    // Le redacteur relache la ressource, booléen à
    lect_red->bool_redacteur = 0;
    /* Si le thread suivant est un redacteur on le réveille
    sinon on réveille tout les threads lecteurs suivants*/
    if(lect_red->tete != NULL && lect_red->tete->etat == thread_redacteur){
        pthread_cond_signal(&lect_red->tete->cond_thread);
    }else{
        maillon_t * maillon_courant = lect_red->tete;
        while(maillon_courant != NULL && maillon_courant->etat == thread_lecteur){
                pthread_cond_signal(&maillon_courant->cond_thread);
                maillon_courant = maillon_courant->next_maillon;
        }
    }

    pthread_mutex_unlock(&lect_red->mutex_global);
}
void debut_lecture(lecteur_redacteur_t *lect_red){

    pthread_mutex_lock(&lect_red->mutex_global);
    contenu_fifo(lect_red);
    printf("Thread %x : Veut lire \n", (int) pthread_self());
    ajouter_queue(lect_red,thread_lecteur);

    /* Tant qu'un redacteur utilise la ressource le thread est en attente.
    Et si le premier element de la fifo est un redacteur alors le lecteur est en attente
    Il revérifie cette condition de boucle au signal d'un autre thread, lorsqu'il a relaché la ressource*/
    while(lect_red->bool_redacteur || (lect_red->tete != NULL && lect_red->tete->etat == thread_redacteur)){
            printf("Thread %x : Est en attente \n", (int) pthread_self());
            pthread_cond_wait(&lect_red->queue->cond_thread,&lect_red->mutex_global);
    }

    enlever_tete(lect_red);
    lect_red->nb_lecteurs++;
    pthread_mutex_unlock(&lect_red->mutex_global);
}

void fin_lecture(lecteur_redacteur_t *lect_red){
    pthread_mutex_lock(&lect_red->mutex_global);
    contenu_fifo(lect_red);

    lect_red->nb_lecteurs--;
    /*Si la tête de la fifo n'est pas nulle et que le nombres de lecteurs est à zéro
    alors on notifie le thread en tête de fifo*/
    if(lect_red->tete != NULL && lect_red->nb_lecteurs == 0 ){
            pthread_cond_signal(&lect_red->tete->cond_thread);
    }

    pthread_mutex_unlock(&lect_red->mutex_global);
}

void initialiser_lecteur_redacteur(lecteur_redacteur_t *lect_red){
    lect_red->tete = NULL;
    lect_red->queue = NULL;
    lect_red->nb_lecteurs = 0;
    lect_red->bool_redacteur = 0;
    pthread_mutex_init(&lect_red->mutex_global,NULL);
}

void detruire_lecteur_redacteur(lecteur_redacteur_t *lect_red){
    pthread_mutex_destroy(&lect_red->mutex_global);

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
