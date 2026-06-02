#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include "config/config.h"
#include "ipc/shm_city.h"
#include "ipc/mq.h"
#include "ipc/pipe_report.h"
#include "sync/barber_light.h"
#include "sync/philosophers.h"
#include "traffic_light.h"
#include "vehicle.h"

// Déclaration de la fonction externe du moniteur
void run_traffic_monitor(int mq_id, int fd_pipe_read);
void display_intersection_states();

int main() {
    printf("=== DÉMARRAGE DU SMART TRAFFIC SIMULATOR ===\n");
    display_intersection_states();

    // 1. Initialisation des mécanismes IPC
    int shm_id = init_city_shm();
    CityMapSHM* city = attach_city_shm(shm_id);
    city->active_vehicles = 0;
    
    for(int i = 0; i < MAX_INTERSECTIONS; i++) {
        city->lights[i].id = i;
        city->lights[i].state = 0; // Commencent tous au rouge
        city->lights[i].car_count = 0;
    }

    int mq_id = init_message_queue();
    
    int pipe_fd[2];
    init_report_pipe(pipe_fd);

    // 2. Allocation de la mémoire partagée anonyme (mmap) pour les structures de synchronisation Posix
    BarberLightSync* sync_light = mmap(NULL, sizeof(BarberLightSync), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    IntersectionPhilo* sync_philo = mmap(NULL, sizeof(IntersectionPhilo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    init_barber_light(sync_light);
    init_philosophers(sync_philo);

    // 3. Création du processus Moniteur
    if (fork() == 0) {
        close(pipe_fd[1]); // Ferme l'écriture pour le lecteur
        run_traffic_monitor(mq_id, pipe_fd[0]);
        exit(0);
    }

    // 4. Création des processus Feux de signalisation
    for (int i = 0; i < MAX_INTERSECTIONS; i++) {
        if (fork() == 0) {
            run_traffic_light_process(i, shm_id, sync_light);
            exit(0);
        }
    }

    // 5. Création des processus Véhicules (Traitement Parallèle Intensif)
    for (int i = 0; i < 8; i++) { // Génère 8 véhicules au démarrage
        if (fork() == 0) {
            close(pipe_fd[0]); // Ferme la lecture pour l'écrivain
            run_vehicle_process(i + 1, shm_id, mq_id, pipe_fd[1], sync_philo);
            exit(0);
        }
        usleep(200000); // Espacement des lancements
    }

    // Attente de la fin des processus enfants (ici les véhicules)
    for(int i = 0; i < 8; i++) {
        wait(NULL);
    }
    
    
    printf("\n[MAIN] Tous les véhicules ont terminé leurs trajets.\n");
    printf("=== FIN DE LA SIMULATION (Nettoyage des processus restants) ===\n");
    
    // Envoyer un signal de terminaison à tous les processus enfants encore en vie (Feux et Moniteur)
    // kill(0, SIGTERM) envoie le signal à tout le groupe de processus
    // Pour éviter que le main se tue lui-même prématurément, on ignore temporairement le signal
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);
    
    // Nettoyage final des IPC et des sémaphores
    destroy_barber_light(sync_light);
    destroy_philosophers(sync_philo);
    munmap(sync_light, sizeof(BarberLightSync));
    munmap(sync_philo, sizeof(IntersectionPhilo));
    
    detach_city_shm(city);
    destroy_city_shm(shm_id);
    destroy_message_queue(mq_id);

    return 0;
}
