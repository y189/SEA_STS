#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include "sync/barber_light.h"

void run_traffic_light_process(int id, int shm_id, BarberLightSync* sync_light);

#endif
