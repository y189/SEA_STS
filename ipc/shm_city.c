#include "shm_city.h"
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>

int init_city_shm() {
    int shm_id = shmget(COMPILATION_KEY, sizeof(CityMapSHM), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        exit(1);
    }
    return shm_id;
}

CityMapSHM* attach_city_shm(int shm_id) {
    CityMapSHM* ptr = (CityMapSHM*)shmat(shm_id, NULL, 0);
    if (ptr == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    return ptr;
}

void detach_city_shm(CityMapSHM* shm_ptr) {
    shmdt(shm_ptr);
}

void destroy_city_shm(int shm_id) {
    shmctl(shm_id, IPC_RMID, NULL);
}
