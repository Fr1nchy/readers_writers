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
  
  Pour cette solution la structure `lecteur_redacteur_t` est composée d'un mutex `mutex_global`, d'une sémaphore `sem_fichier`  et d'un entier `nb_lecteurs`.
  - Le mutex sert à protéger les variables globales manipulées dans les fonctions citées précédement. Il est utilisé à chaque début de fonction.
  - La sémaphore sert à protéger l'accès à la ressource utiliser par chacun des threads. Elle est initialisé à un, il ne peut donc avoir qu'un thread dans la section critique. Cette sémaphore permet au premier lecteur de réservé la ressource pour les prochains lecteurs, pour permettre plusieurs lectures en concurence.
  - L'entier permet de compter le nombre de lecteur en cours de lecture sur la ressource.
  
Lorsque la ressouce est disponible, le nombre de lecteur est à zéro et qu'un thread lecteur appelle `debut_lecture` il va prendre le jeton de la sémaphore et réservé la ressources pour tout les lecteurs. Tout les threads lecteur vont pouvoir accèder à la ressource, alors que les threads rédacteur seront en attente pour prendre le jeton de la sémpharore. 
La ressource est libéré par le dernier lecteur lorsque tout les threads ont terminés de lire. A ce moment la les threads rédacteurs en attente peuvent prendre le jeton sur la sémaphore.
  
  
  
  
