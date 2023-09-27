/**
 * @file mosq_sink.c
 * @author longdh (longdh@xsolar.vn)
 * @brief 
 * @version 0.1
 * @date 2027-09-20
 * 
 * @copyright Copyright (c) 2023
 * 
 * Note: chu y cho ID-client
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "error.h"
#include "mosq_sink.h"

//FIXME
extern char* strdup(const char*);

#define DEBUG

/**
 * @brief connect callback
 * 
 * @param mosq 
 * @param obj 
 * @param rc 
 */
static void on_connect(struct mosquitto *mosq, void *obj, int rc) 
{
    if (rc == 0) {
        printf("Connected successfully\n");
    } else {
        fprintf(stderr, "Connect failed: %s\n", mosquitto_strerror(rc));
    }
}

/**
 * @brief 
 * 
 * @param mosq 
 * @param obj 
 * @param rc 
 */
static void on_disconnect(struct mosquitto *mosq, void *obj, int rc) 
{
    if (rc == MOSQ_ERR_SUCCESS) 
    {
        printf("Disconnecting gracefully...\n");
    } 
    else 
    {
        printf("Disconnected unexpectedly, will try to reconnect...\n");
        mosquitto_reconnect(mosq);
    }
}

/**
 * @brief 
 * 
 * @param mosq 
 * @param topic 
 * @param message 
 */
static void send_to_mqtt(struct mosquitto *mosq, const char* topic, const char* message) 
{
    int rc = mosquitto_publish(mosq, NULL, topic, strlen(message), message, 0, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to publish (%d): %s\n", rc, mosquitto_strerror(rc));
    }
}

/**
 * @brief sink thread (forward thread)
 * 
 * @param arg 
 * @return void* 
 */
void* mosq_sink_task(void* arg) 
{
    mosq_sync_config* cfg= (mosq_sync_config*) arg;
    if (cfg == NULL)
    {
        #ifdef DEBUG
        printf("Error queue...\n");
        #endif // DEBUG
        
        exit(-1);
    }

    Queue* q = (Queue*) cfg->q;
    char data[MAX_QUEUE_DATA_SIZE];

    if (q == NULL)
    {
        #ifdef DEBUG
        printf("Error queue...\n");
        #endif // DEBUG
        
        exit(EQUERR);
    }

    // Initialize mosquitto library
    mosquitto_lib_init();

    struct mosquitto *mosq = NULL;
    int rc;

    // Create a mosquitto client instance
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) 
    {
        fprintf(stderr, "Error: Out of memory.\n");
        exit(ESYSERR);
    }

    // Set callback functions
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);

    // Connect to MQTT broker
    rc = mosquitto_connect(mosq, cfg->host, cfg->port, 60);
    if (rc != MOSQ_ERR_SUCCESS) 
    {
        fprintf(stderr, "Unable to connect (%d): %s\n", rc, mosquitto_strerror(rc));
        exit(ESVRERR);
    }

    while (1) 
    {
        if (wait_dequeue(q, data))
        {
            #ifdef DEBUG
            printf("%s\n", data);
            #endif // DEBUG

            // Publish to MQTT broker and topic
            send_to_mqtt(mosq, cfg->topic, data);

        }
    }

    // Clean up
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();


    return NULL;
}

/**
 * @brief Init sync task
 * 
 * @param cfg 
 * @param host 
 * @param port 
 * @param username 
 * @param password 
 * @return int 
 */
int mosq_sink_init(mosq_sync_config* cfg, Queue* q, const char* host, int port, const char* username, const char* password, const char* client_id, const char* topic)
{
    memset(cfg, 0, sizeof (mosq_sync_config));

    cfg->q = q;

    if (host != NULL)
        cfg->host = strdup(host);

    cfg->port = port;
    if (username != NULL)
        cfg->username = strdup(username);

    if (password != NULL)
        cfg->password = strdup(password);

    if (topic != NULL)
        cfg->topic = strdup(topic);

    if (client_id != NULL)
        cfg->client_id = strdup(client_id);
    else {
        char id[128];
        sprintf(id, "cli_%lu\n", (unsigned long)time(NULL));

        cfg->client_id = strdup(id);
    } 

    return 0;
}

/**
 * @brief Free task's data
 * 
 * @param cfg 
 * @return int 
 */
int mosq_sink_term(mosq_sync_config* cfg)
{
    if (cfg->host != NULL)
        free(cfg->host);

    if (cfg->username != NULL)
        free(cfg->username);

    if (cfg->password != NULL)
        free(cfg->password);
    
    if (cfg->client_id != NULL)
        free(cfg->client_id);

    if (cfg->topic != NULL)
        free(cfg->topic);

    return 0;
}

/**
 * @brief Do the task
 * 
 * @param cfg 
 * @return int 
 */
int mosq_sink_run(mosq_sync_config* cfg)
{   
    return 
        pthread_create(&cfg->task_thread, NULL, mosq_sink_task, cfg);
    
}

/**
 * @brief Wait until end
 * 
 * @param cfg 
 * @return int 
 */
int mosq_sink_wait(mosq_sync_config* cfg)
{
    return 
        pthread_join(cfg->task_thread, NULL);    
}