# Lecteurs - Rédacteurs

Différentes solutions d'implémentation du problème de synchronisation des lecteurs-rédacteurs 

## 1. Introduction

Il s’agit d’accès concurrents à une ressource partagée par deux types d’entités : les lecteurs et
les rédacteurs. Les lecteurs accèdent à la ressource sans la modifier. Les rédacteurs, eux, modifient
la ressource. Pour garantir un état cohérent de la ressource, plusieurs lecteurs peuvent y accéder en
même temps mais l’accès pour les rédacteurs est en accès exclusif. En d’autres termes, si un rédacteur
travaille avec la ressource, aucune autre entité (lecteur ou rédacteur) ne doit pouvoir accéder à celle-ci.
Le problème des lecteurs-rédacteurs est un problème classique de synchronisation lors de l’utilisation
d’une ressource partagée. Ce schéma est typiquement utilisé pour la manipulation de fichiers ou de
zones mémoire.

Vous trouverez dans notre archive trois répertoires, détaillant une solution pour chacune des différentes politiques 
de gestion des priorités entre les lecteurs et les rédacteurs :
  - Lecteurs prioritaires 
  - Redacteurs prioritaire
  - Priorité selon l'ordre d'arrivée des lecteurs et rédacteurs (FIFO)

## 2. Spécification des fonctions

Dans chacune des solutions proposées, vous retrouverez quatres fonctions principales, dont le corps changera en fonction de 
la politique de gestion des priorités. Chacune d'elles prennent en paramètre un pointeur sur la structure `lecteur_redacteur_t`
Ci-dessous les fonctions :

La fonction `void debut_lecture(lecteur_redacteur_t *lect_red)` est appellée par un thread lecteur lorsqu'il souhaite accèder à
une ressource en lecture. 

La fonction `void fin_lecture(lecteur_redacteur_t *lect_red)` est appellée par un thread lecteur lorsqu'il a terminé d'utiliser la ressource.  Après l'execution de cette fonction la ressource est disponible, elle peut être utilisée par un thread rédacteur ou plusieurs threads lecteurs

La fonction `void debut_redaction(lecteur_redacteur_t *lect_red)` est appellée par un thread redacteur lorsqu'il souhaite accèder à une ressource en rédaction. De plus cette fonction garantie que seul le thread en cours de rédaction accèdera à la ressource. Dans le cas où un ou plusieurs threads utilisent déjà la ressource alors le thread redacteur appellant cette fonction est mis en attente jusqu'à que la ressource soit disponible.

La fonction `void fin_redaction(lecteur_redacteur_t *lect_red)` est appellée par thread redacteur lorsqu'il a terminé d'utiliser la ressource. Après l'execution de cette fonction la ressource est disponible, elle peut être utilisée par un thread rédacteur ou plusieurs threads lecteurs.

## 3. Détails d'implémentation 
  ### Lecteurs prioritaires
  
  Pour cette solution la structure `lecteur_redacteur_t` est composée de plusieurs éléments:
  
  - Le mutex `mutex_global` sert à protéger les variables globales manipulées dans les fonctions citées précédement. Il est utilisé à chaque début de fonction.
  - La sémaphore `sem_fichier`  sert à protéger l'accès à la ressource utiliser par chacun des threads. Elle est initialisé à un, il ne peut donc avoir qu'un thread dans la section critique. Cette sémaphore permet au premier lecteur de réservé la ressource pour les prochains lecteurs, pour permettre plusieurs lectures en concurence.
  - L'entier `nb_lecteurs` permet de compter le nombre de lecteur en cours de lecture sur la ressource.
  
Pour comprendre la solution et les mécanismes mis en place, voici un sénario d'éxecution:

Lorsque la ressouce est disponible, le nombre de lecteur est à zéro et qu'un thread lecteur appelle `debut_lecture` il va prendre le jeton de la sémaphore et réservé la ressources pour tout les lecteurs. Tout les threads lecteur vont pouvoir accèder à la ressource, alors que les threads rédacteur seront en attente pour prendre le jeton de la sémpharore. 
La ressource est libéré par le dernier lecteur lorsque tout les threads ont terminés de lire. A ce moment la les threads rédacteurs en attente peuvent prendre le jeton sur la sémaphore.


   ### Rédacteurs prioritaires
   
   Pour cette solution la structure `lecteur_redacteur_t` est composée de plusieurs éléments:
   
   - Le mutex `mutex_global` sert à protéger les variables globales manipulées dans les fonctions citées précédement. Il est utilisé à chaque début de fonction.
   - Une variable condition `file_lect` représentant une liste d'attente pour les lecteurs 
   - Une variable condition `file_rect` représentant une liste d'attente pour les redacteurs  
   - L'entier `nb_lecteurs` et `nb_redacteurs` pour compter le nombre de lecteur ou de redacteur utilisant la ressource.
   - Un boolléen `bool_redacteur` pour savoir si une rédaction est en court.
   
   ### Priorité FIFO
   
   Dans cette implémentation, une structure de donnée FIFO a été rajoutée, elle va permettre de modéliser une file d'attente pour les threads. Les threads passent selon leur ordre d’arrivée en permettant tout de même l’accès en parallèle à plusieurs lecteurs. Par exemple,l’ordre d’arrivée suivant : L1, L2, L3, R1, R2, L4, L5, R3 donnera l’ordre de passage suivant :
   
   - L1,L2,L3 en parallèle
   - R1 seul
   - R2 seul
   - L4,L5 en parallèle
   - R3 seul
     
   Pour cette solution la structure `lecteur_redacteur_t` est composée de plusieurs éléments:
   
- Le mutex `mutex_global` sert à protéger les variables globales manipulées dans les fonctions citées précédement. Il est utilisé à chaque début de fonction.
- L'entier `nb_lecteurs` permet de compter le nombre de lecteur en cours de lecture sur la ressource.
- Un boolléen `bool_redacteur` pour savoir si une rédaction est en court.
- Deux pointeurs `tete` et `queue` sur une structure `maillon_t`qui permettent de modéliser une liste FIFO. Chaque maillon de la file possède une variable condition `cond_thread`, un état indiquant le type lecteur ou redacteur, ainsi qu'un pointeur sur le maillon suivant pour chainer les maillons.

   
   
## 4. Compilation et éxecution du code 

  Pour compiler le code en version finale il suffit d'executer la commande `make.` Il est possible d'obtenir une execution du
  programme plus détaillée avec l'affichage du contenu de la FIFO pour la solution avec priorité selon l'odre d'arrivée. Pour ça il faut compiler avec la commande `make debug`.
  Pour chacune des solutions, une fois le programme compilé il peut être executé avec la commande et les arguments suivants `./test_lecteurs_redacteurs nb_lecteurs nb_redacteurs nb_iterations`
  
## 5. Tests
  
  
