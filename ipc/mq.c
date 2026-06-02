#include "mq.h"
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>

int init_message_queue() {
    int mq_id = msgget(COMPILATION_KEY + 1, IPC_CREAT | 0666);
    if (mq_id < 0) {
        perror("msgget failed");
        exit(1);
    }
    return mq_id;
}

void send_traffic_message(int mq_id, int vehicle_id, int action, int intersection_id) {
    TrafficMessage msg;
    msg.msg_type = 1;
    msg.vehicle_id = vehicle_id;
    msg.action = action;
    msg.intersection_id = intersection_id;
    msgsnd(mq_id, &msg, sizeof(TrafficMessage) - sizeof(long), 0);
}

int receive_traffic_message(int mq_id, TrafficMessage* msg) {
    return msgrcv(mq_id, msg, sizeof(TrafficMessage) - sizeof(long), 1, IPC_NOWAIT);
}

void destroy_message_queue(int mq_id) {
    msgctl(mq_id, IPC_RMID, NULL);
}
