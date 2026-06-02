#ifndef MQ_H
#define MQ_H

#include "../config/config.h"

int init_message_queue();
void send_traffic_message(int mq_id, int vehicle_id, int action, int intersection_id);
int receive_traffic_message(int mq_id, TrafficMessage* msg);
void destroy_message_queue(int mq_id);

#endif
