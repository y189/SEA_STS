#ifndef VEHICLE_H
#define VEHICLE_H

#include "sync/philosophers.h"

void run_vehicle_process(int id, int shm_id, int mq_id, int fd_pipe_write, IntersectionPhilo* sync_philo);

#endif
