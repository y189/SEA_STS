# Smart Traffic Simulator

## 1. Introduction & Objectifs Système

### 1.1 Contexte
Le projet **Smart Traffic Simulator** est un simulateur décentralisé de trafic urbain appliqué à une topologie de 4 intersections interconnectées. L'objectif est de modéliser le comportement autonome de feux de signalisation et de véhicules en situation de concurrence stricte pour l'accès aux ressources physiques (les voies).

### 1.2 Contraintes Architecturales
* **Isolation stricte :** Refus de l'architecture monolithique ou du modèle multi-threadé applicatif global. Chaque entité (véhicule, feu) est encapsulée dans un **processus lourd autonome** (copie d'espace mémoire via `fork()`).
* **Communication de bas niveau :** Utilisation exclusive des mécanismes de communication inter-processus (**IPC System V / POSIX**).
* **Sécurité algorithmique :** Exclusion mutuelle parfaite lors du franchissement des carrefours et absence totale d'interblocage (*deadlock*).

---

## 2. Architecture des Processus (Arbre Système)

Au démarrage du binaire `./traffic_simulator`, le noyau Linux engendre une topologie de processus parents/enfants répartie comme suit :

```text
                 [Processus Parent : MAIN (PID X)]
                                 |
      +--------------------------+--------------------------+
      |                          |                          |
[MONITEUR (PID Y)]    [4 x FEUX (PID F0..F3)]    [8 x VÉHICULES (PID V1..V8)]

```

### 2.2 Rôles des Composants Système
* **Processus MAIN :** Gère le cycle de vie initial, l'instanciation des IPC, la génération des nœuds enfants, la synchronisation de fin via `wait()` et le nettoyage de la table d'espace noyau via les signaux POSIX.
* **Processus TRAFFIC_LIGHT (x4) :** Agents autonomes simulant le contrôleur électronique de chaque carrefour. Le rythme d'inversion d'état est cadencé par une horloge asynchrone (`sleep(3)`).
* **Processus VEHICLE (xN) :** Agents mobiles exécutant un thread d'action dynamique, requérant l'accès aux ressources en fonction de l'état environnemental partagé.
* **Processus TRAFFIC_MONITOR (x1) :** Collecteur centralisé des états et des logs. Il s'agit d'un processus d'affichage pur (Lecture seule).

---

## 3. Spécifications des IPC (Inter-Process Communication)

Pour assurer la convergence des données entre les espaces mémoires virtuels isolés, trois abstractions d'IPC sont implémentées :

### 3.1 Mémoire Partagée (Shared Memory - System V)
* **Clé d'accès :** `0x000004d2` (Définie par macro dans `shm_city.h`).
* **Structure de données :** `CityMapSHM`
* **Mode d'accès :** * *Processus Feux :* Écriture synchrone par écrasement de l'état binaire ($0 = \text{Rouge}$, $1 = \text{Vert}$).
  * *Processus Véhicules :* Lecture synchrone pour conditionner le franchissement.

### 3.2 File de Messages (Message Queue - System V)
* **Clé d'accès :** `0x000004d3` (Définie dans `mq.h`).
* **Structure de données :** `TrafficMessage` (`Type`, `ID_Vehicule`, `Action_ID`, `Timestamp`).
* **Mode d'accès :** Asynchrone (Écrivains multiples : Véhicules via `msgsnd()` / Lecteur unique : Moniteur via `msgrcv()`).
* **Propriété :** Persistance noyau assurant l'absence de perte d'événements en cas de surcharge de l'affichage.

### 3.3 Tube Anonyme (Pipe POSIX)
* **Nature :** Unidirectionnel, hérité par `fork()`.
* **Flux :** Flux d'octets textuels standardisés (`char[]`).
* **Propriété :** Garantie d'atomicité des écritures de logs grâce au respect de la taille critique de buffer inférieur à `PIPE_BUF`.

---

## 4. Modélisation de la Synchronisation & Primitives

### 4.1 Implémentation du Problème des Philosophes de Dijkstra
Pour l'accès aux intersections (zones critiques), chaque carrefour est modélisé comme une table de philosophes où les voies adjacentes représentent les fourchettes (`sem_t forks[4]`).

* **Primitive de synchronisation :** Sémaphores POSIX nommés insérés au sein d'une zone mémoire anonyme allouée par l'appel système `mmap()` avec les drapeaux `MAP_SHARED | MAP_ANONYMOUS`.
* **Algorithme Anti-Deadlock :** Brisure de la condition d'attente circulaire par l'établissement d'une hiérarchie stricte des ressources. Le processus véhicule évalue les index des deux verrous nécessaires et exécute l'appel bloquant `sem_wait()` impérativement sur **l'index le plus petit en premier**, indépendamment de son vecteur cardinal de déplacement.

### 4.2 Optimisation Énergétique (Barbier Dormeur)
* Élimination de l'attente active (*busy waiting*) au niveau des processus de gestion des feux.
* Utilisation du mécanisme de réveil asynchrone par sémaphores (`sem_wait(&sync_light->light_changer)`), basculant le processus à l'état *Blocked* par le scheduler Linux lorsque aucune transition d'état n'est requise.

---

## 5. Cycle de Vie, Signaux et Sécurité Système

* **Phase d'Initialisation :** Le `MAIN` instancie les ressources physiques (SHM, MQ, Pipe, mmap Sémaphores) avant l'exécution de tout `fork()`.
* **Phase d'Exécution :** Distribution des processus par l'ordonnanceur Linux sur les cœurs physiques disponibles (exécution concurrente et parallèle).
* **Phase de Terminaison (Anti-Zombie) :**
  * Le `MAIN` exécute une barrière de synchronisation via `wait(NULL)` indexée sur le nombre exact de véhicules initiés.
  * À la complétion des trajectoires des véhicules, le `MAIN` capture le signal de fermeture globale et propage l'appel `kill(0, SIGTERM)` pour purger les boucles infinies des feux et du moniteur.
  * Le `MAIN` intercepte et ignore son
 
  ## 6. Opération Dynamique des Véhicules

Le cycle algorithmique d'un processus `VEHICLE` suit rigoureusement les étapes suivantes :

```text
[Arrivée] -> Log Pipe -> [Vérification SHM] -> Si Rouge (0): Bloqué
                                            -> Si Vert (1): Continuer
                                                   |
[Libération] <- Sortie <- [Traversée & MQ] <- [Verrouillage Philosophes]
