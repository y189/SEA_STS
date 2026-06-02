#ifndef CONFIG_H
#define CONFIG_H

#define MAX_VEHICLES 20
#define MAX_INTERSECTIONS 4
#define SHM_PATH "/smart_traffic_shm"
#define MQ_PATH "/smart_traffic_mq"
#define COMPILATION_KEY 1234

typedef struct {
    int id;
    int state; // 0: Rouge, 1: Vert
    int car_count;
} TrafficLightSHM;

typedef struct {
    TrafficLightSHM lights[MAX_INTERSECTIONS];
    int active_vehicles;
} CityMapSHM;

typedef struct {
    long msg_type;
    int vehicle_id;
    int action; // 1: Entrée, 2: Sortie, 3: Attente
    int intersection_id;
} TrafficMessage;

#endif
