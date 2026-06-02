#include "vehicle.h"
#include "ipc/shm_city.h"
#include "ipc/mq.h"
#include "ipc/pipe_report.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void run_vehicle_process(int id, int shm_id, int mq_id, int fd_pipe_write, IntersectionPhilo* sync_philo) {
    CityMapSHM* city = attach_city_shm(shm_id);
    char log_buf[100];
    
    // Détermination de l'intersection cible (Simulation simplifiée)
    int target_intersection = id % MAX_INTERSECTIONS;
    int left_res = target_intersection;
    int right_res = (target_intersection + 1) % 4;

    // 1. Notification via la File de Messages (MQ)
    send_traffic_message(mq_id, id, 1, target_intersection);
    
    // 2. Trajet vers l'intersection
    sprintf(log_buf, "Véhicule %d en route vers l'intersection %d.", id, target_intersection);
    write_report(fd_pipe_write, log_buf);
    sleep(1 + rand() % 3);

    // 3. Phase critique de franchissement d'intersection (Modèle Philosophes)
    acquire_intersection_resources(sync_philo, left_res, right_res);
    
    // Vérification du feu de signalisation dans la SHM
    while (city->lights[target_intersection].state == 0) {
        // Le feu est rouge, le véhicule patiente
        send_traffic_message(mq_id, id, 3, target_intersection);
        sleep(1);
    }

    // Franchissement
    city->lights[target_intersection].car_count++;
    sprintf(log_buf, "Véhicule %d traverse l'intersection %d.", id, target_intersection);
    write_report(fd_pipe_write, log_buf);
    sleep(1); // Temps de passage
    
    release_intersection_resources(sync_philo, left_res, right_res);

    // 4. Sortie de la zone
    send_traffic_message(mq_id, id, 2, target_intersection);
    
    detach_city_shm(city);
    exit(0);
}
