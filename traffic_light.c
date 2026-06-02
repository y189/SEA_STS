#include "traffic_light.h"
#include "ipc/shm_city.h"
#include <unistd.h>
#include <stdio.h>

void run_traffic_light_process(int id, int shm_id, BarberLightSync* sync_light) {
    CityMapSHM* city = attach_city_shm(shm_id);
    
    while(1) {
        request_light_change(sync_light);
        wait_for_light_request(sync_light);
        
        // Alternance de l'état du feu (Rouge / Vert)
        city->lights[id].state = !city->lights[id].state;
        
        printf("[Feu %d] Changement d'état : %s\n", id, city->lights[id].state ? "VERT" : "ROUGE");
        sleep(3); // Cycle du feu
    }
    
    detach_city_shm(city);
}
