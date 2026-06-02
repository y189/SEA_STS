#include "barber_light.h"

void init_barber_light(BarberLightSync* sync) {
    sem_init(&sync->mutex, 1, 1);
    sem_init(&sync->light_changer, 1, 0);
    sync->waiting_updates = 0;
}

void request_light_change(BarberLightSync* sync) {
    sem_wait(&sync->mutex);
    sync->waiting_updates++;
    if (sync->waiting_updates == 1) {
        sem_post(&sync->light_changer);
    }
    sem_post(&sync->mutex);
}

void wait_for_light_request(BarberLightSync* sync) {
    sem_wait(&sync->light_changer);
    sem_wait(&sync->mutex);
    sync->waiting_updates--;
    sem_post(&sync->mutex);
}

void destroy_barber_light(BarberLightSync* sync) {
    sem_destroy(&sync->mutex);
    sem_destroy(&sync->light_changer);
}
