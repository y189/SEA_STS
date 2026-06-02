#ifndef SHM_CITY_H
#define SHM_CITY_H

#include "../config/config.h"

int init_city_shm();
CityMapSHM* attach_city_shm(int shm_id);
void detach_city_shm(CityMapSHM* shm_ptr);
void destroy_city_shm(int shm_id);

#endif
