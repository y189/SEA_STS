#ifndef PHILOSOPHERS_H
#define PHILOSOPHERS_H

#include <semaphore.h>

// Modèle des Philosophes pour gérer l'interblocage (Deadlock) des intersections par les véhicules
typedef struct {
    sem_t forks[4]; // 4 ressources de croisement
} IntersectionPhilo;

void init_philosophers(IntersectionPhilo* philo);
void acquire_intersection_resources(IntersectionPhilo* philo, int left_resource, int right_resource);
void release_intersection_resources(IntersectionPhilo* philo, int left_resource, int right_resource);
void destroy_philosophers(IntersectionPhilo* philo);

#endif
