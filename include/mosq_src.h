#ifndef MOSQ_SRC_H
#define MOSQ_SRC_H 

#include "squeue.h"

typedef struct {
    char*   host;
    int     port;
    char*   username;
    char*   password;
    char*   topic;
    char*   client_id;

    Channel *c;
    pthread_t task_thread;

} mosq_source_config;

int mosq_source_init(mosq_source_config* cfg, Channel *c, const char* host, int port, const char* username, const char* password, const char* client_id, const char* topic);
int mosq_source_term(mosq_source_config* cfg);
int mosq_source_run(mosq_source_config* cfg);
int mosq_source_wait(mosq_source_config* cfg);

#endif // !MOSQ_SRC_H
