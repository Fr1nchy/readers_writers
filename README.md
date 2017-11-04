<div align="center"><h1>Lecteurs - Rédacteurs<h1><br/>
Différentes solutions d'implémentation du problème de synchronisation des lecteurs-rédacteurs</div>
  


<div align="center">BARROIS Florian - DUVERNEY Thomas</div>





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
  - Redacteurs prioritaires
  - Priorité selon l'ordre d'arrivée des lecteurs et rédacteurs (FIFO)

## 2. Spécification des fonctions

Dans chacune des solutions proposées, vous retrouverez quatre fonctions principales, dont le corps changera en fonction de 
la politique de gestion des priorités. Chacune d'elle prend en paramètre un pointeur sur la structure `lecteur_redacteur_t`
Ci-dessous les fonctions :

La fonction `void debut_lecture(lecteur_redacteur_t *lect_red)` est appelée par un thread lecteur lorsqu'il souhaite accéder à
une ressource en lecture. 

La fonction `void fin_lecture(lecteur_redacteur_t *lect_red)` est appelée par un thread lecteur lorsqu'il a terminé d'utiliser la ressource.  Après l'execution de cette fonction la ressource est disponible, elle peut être utilisée par un thread rédacteur ou plusieurs threads lecteurs.

La fonction `void debut_redaction(lecteur_redacteur_t *lect_red)` est appelée par un thread redacteur lorsqu'il souhaite accéder à une ressource en rédaction. De plus cette fonction garantie que seul le thread en cours de rédaction accèdera à la ressource. Dans le cas où un ou plusieurs threads utilisent déjà la ressource alors le thread redacteur appelant cette fonction est mis en attente jusqu'à que la ressource soit disponible.

La fonction `void fin_redaction(lecteur_redacteur_t *lect_red)` est appelée par thread redacteur lorsqu'il a terminé d'utiliser la ressource. Après l'execution de cette fonction la ressource est disponible, elle peut être utilisée par un thread rédacteur ou plusieurs threads lecteurs.

## 3. Détails d'implémentation 
  ### Lecteurs prioritaires
  
  Pour cette solution la structure `lecteur_redacteur_t` est composée de plusieurs éléments:
  
  - Le mutex `mutex_global` sert à protéger les variables globales manipulées dans les fonctions citées précédement. Il est utilisé à chaque début de fonction.
  - Le sémaphore `sem_fichier`  sert à protéger l'accès à la ressource utilisée par chacun des threads. Il est initialisé à 1, il ne peut donc avoir qu'un thread dans la section critique. Ce sémaphore permet au premier lecteur de réserver la ressource pour les prochains lecteurs, pour permettre plusieurs lectures en concurrence. On utilise une sémaphore plutôt qu'un thread pour que la ressource soit libéré par un thread différent de celui qu'il la vérouillé. En effet le premier lecteur réalise un `sem_wait` et le dernier lecteur un `sem_post`pour libérer la ressource. Si un mutex avait été utilisé le vérrou et la libération de la ressource aurait du être effectué par le même thread, ce qui est compliqué à mettre en place et pas très cohérent.
  - L'entier `nb_lecteurs` permet de compter le nombre de lecteurs en cours de lecture sur la ressource.
  
Pour comprendre la solution et les mécanismes mis en place, voici un scénario d'éxecution:

Lorsque la ressouce est disponible (le nombre de lecteurs est à zéro) et qu'un thread lecteur appelle `debut_lecture` il va prendre le jeton du sémaphore et réserver la ressource pour tous les lecteurs. Tous les threads lecteurs vont pouvoir accéder à la ressource, alors que les threads rédacteurs seront en attente pour prendre le jeton du sémaphore. 
La ressource est libérée par le dernier lecteur lorsque tous les threads ont terminé de lire. A ce moment là les threads rédacteurs en attente peuvent prendre le jeton du sémaphore.


   ### Rédacteurs prioritaires
  
   Pour cette solution la structure `lecteur_redacteur_t` est composée de plusieurs éléments:
   
   - Le mutex `mutex_global` sert à protéger les variables globales manipulées dans les fonctions citées précédemment. Il est utilisé à chaque début de fonction.
   - Une variable condition `file_lect` représentant une liste d'attente pour les lecteurs.
   - Une variable condition `file_rect` représentant une liste d'attente pour les redacteurs.  
   - Les entiers `nb_lecteurs` et `nb_redacteurs` pour compter le nombre de lecteurs ou de redacteurs utilisant la ressource.
   - Un booléen `bool_redacteur` pour savoir si une rédaction est en cours.
   
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
  programme plus détaillée avec l'affichage du contenu de la FIFO pour la solution avec priorité selon l'odre d'arrivée. Pour ça il faut compiler avec la commande `make debug`. L'affichage du contenu de la FIFO est un ensemble de 0 et de 1. Les 1, représente les rédacteurs et les 0, les lecteurs en attente dans la file.
  Pour chacune des solutions, une fois le programme compilé il peut être executé avec la commande et les arguments suivants `./test_lecteurs_redacteurs nb_lecteurs nb_redacteurs nb_iterations`.
  
 ## 5. Tests
 
 Pour une question de lisibilité, tous les tests décrits ici ont été réalisés en simulant 3 lecteurs et 3 rédacteurs, chacun effectuant une seule fois son action attitrée.
 
  ### Lecteurs prioritaires
       
<div align="center"><img src="prio_red.png" alt="lecteurs prioritaires" width="433" height="183"></div>
<p>&nbsp;</p>
Sur cet exemple, on observe que 4 acteurs arrivent au même moment de l'exécution : d'abord un lecteur, puis deux rédacteurs et à nouveau un lecteur. On peut remarquer que les deux rédacteurs passent après les deux lecteurs, même si le deuxième lecteur est arrivé en dernier. De plus, puisque les rédacteurs doivent de toute façon attendre que toutes les lectures soient terminées pour accéder à la ressource, le programme a été optimisé de façon à ce que les lectures soient faites en parallèle. Le deuxième rédacteur en revanche doit bel et bien attendre la fin de l'écriture précédente pour procéder à la sienne.
<p>&nbsp;</p>

### Rédacteurs prioritaires    
    
<div align="center"><img src="prio_red.png" alt="redacteurs prioritaires" width="444" height="184"></div>
<p>&nbsp;</p>
Sur cet exemple, un lecteur demande à accéder à la ressource, suivi par deux rédacteurs. Cette fois, ce sont les rédacteurs qui sont prioritaires et qui utilisent la ressource avant le lecteur bien que ce dernier en ait demandé l'accès en premier. Les rédactions ont lieu tour à tour en attendant bien que le rédacteur précédent ait terminé pour commencer leur travail. Tout comme pour le cas où les lecteurs sont prioritaires, plusieurs lecteurs peuvent utiliser la ressource parallèlement (tant qu'il n'y a pas de rédacteur en attente).
<p>&nbsp;</p>

### First In First Out (FIFO)

<div align="center"><img src="prio_fifo.png" alt="fifo" width="426" height="338"></div>
<p>&nbsp;</p>
Cet exemple illustre tous les cas pertinents concernant la logique FIFO : on remarque d'abord que l'ordre d'accès à la ressource est cohérent lorsqu'un rédacteur arrive avant un lecteur (le thread a07c9700 opère avant le thread a27cd700). Ensuite, on peut voir que c'est également le cas lorsque c'est un lecteur qui demande l'accès à la ressource avant un rédacteur (le thread a1fcc700 opère avant le thread a0fca700). De plus, puisque deux lecteurs sont arrivés consécutivement, le parallélisme des lectures a été permis (threads lecteurs a27cd700 et a1fcc700). Enfin, la priorité FIFO est également respectée lorsque la file contient deux rédacteurs et, comme attendu, le deuxième attend la fin de l'utilisation de la ressource par le premier pour commencer.
