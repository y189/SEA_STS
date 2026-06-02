#ifndef BARBER_LIGHT_H
#define BARBER_LIGHT_H

#include <semaphore.h>

// Modèle Barbier Dormeur pour la synchronisation des feux (éviter les conflits d'accès)
typedef struct {
    sem_t mutex;
    sem_t light_changer;
    int waiting_updates;
} BarberLightSync;

void init_barber_light(BarberLightSync* sync);
void request_light_change(BarberLightSync* sync);
void wait_for_light_request(BarberLightSync* sync);
void destroy_barber_light(BarberLightSync* sync);

#endif
