#include "philosophers.h"

void init_philosophers(IntersectionPhilo* philo) {
    for (int i = 0; i < 4; i++) {
        sem_init(&philo->forks[i], 1, 1); // Partagé entre processus
    }
}

void acquire_intersection_resources(IntersectionPhilo* philo, int left_resource, int right_resource) {
    // Stratégie anti-deadlock : toujours acquérir la ressource de plus petit index en premier
    if (left_resource < right_resource) {
        sem_wait(&philo->forks[left_resource]);
        sem_wait(&philo->forks[right_resource]);
    } else {
        sem_wait(&philo->forks[right_resource]);
        sem_wait(&philo->forks[left_resource]);
    }
}

void release_intersection_resources(IntersectionPhilo* philo, int left_resource, int right_resource) {
    sem_post(&philo->forks[left_resource]);
    sem_post(&philo->forks[right_resource]);
}

void destroy_philosophers(IntersectionPhilo* philo) {
    for (int i = 0; i < 4; i++) {
        sem_destroy(&philo->forks[i]);
    }
}
