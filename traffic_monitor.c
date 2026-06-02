#include "ipc/mq.h"
#include "ipc/pipe_report.h"
#include <stdio.h>
#include <unistd.h>

void run_traffic_monitor(int mq_id, int fd_pipe_read) {
    TrafficMessage msg;
    char report_buffer[128];
    
    printf("[MONITEUR] Lancement du superviseur de trafic.\n");
    
    while(1) {
        // 1. Lire les logs provenant du Pipe
        read_report(fd_pipe_read, report_buffer, sizeof(report_buffer));
        if (report_buffer[0] != '\0') {
            printf("[RAPPORT PIPE] %s\n", report_buffer);
        }
        
        // 2. Consommer les événements asynchrones de la File de messages
        if (receive_traffic_message(mq_id, &msg) == 0) {
            printf("[EVENT MQ] Véhicule %d -> Action Code: %d à l'intersection %d\n", 
                   msg.vehicle_id, msg.action, msg.intersection_id);
        }
        
        usleep(500000); // Monitoring toutes les 500ms
    }
}
