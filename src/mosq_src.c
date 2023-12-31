/**
 * @file mosq_src.c
 * @author longdh (longdh@xsolar.vn)
 * @brief 
 * @version 0.1
 * @date 2027-09-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mosquitto.h>
#include "mosq_src.h"
#include "error.h"
#include "logger.h"

//FIXME
extern char* strdup(const char*);

static volatile int connected = 0;

#define DEBUG

/**
 * @brief message arrive callback
 * 
 * @param mosq 
 * @param obj 
 * @param message 
 */
static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) 
{
    mosq_source_config* cfg = (mosq_source_config*) obj;

    int i = 0;
    Channel *c = (Channel*) cfg->c;
    
    #ifdef DEBUG
    // Assuming message is in JSON format
    // printf("Received message %d: %s: %s\n",len, topicName, payloadptr);
    #endif // DEBUG    

    // send to queue
    for (i=0; i< c->total; i++) 
    {
        // text data
        enqueue( c->queue[i], (char*) message->payload);
    }    
    
}

/**
 * @brief connected callback
 * 
 * @param mosq 
 * @param obj 
 * @param rc 
 */
static void on_connect(struct mosquitto *mosq, void *obj, int rc) 
{
    mosq_source_config* cfg = (mosq_source_config*) obj;

    if (rc == 0) 
    {
        log_message(LOG_INFO, "Connected successfully\n");        

        // Subscribe to the desired topic after successful connection
        mosquitto_subscribe(mosq, NULL, cfg->topic, 0);

    } 
    else 
    {
        log_message(LOG_ERROR, "Connect failed: %s\n", mosquitto_strerror(rc));

        exit(EQUERR);
    }
}

/**
 * @brief dis-connect callback
 * 
 * @param mosq 
 * @param obj 
 * @param rc 
 */
static void on_disconnect(struct mosquitto *mosq, void *obj, int rc) 
{
    if (rc == MOSQ_ERR_SUCCESS) 
    {
        log_message(LOG_INFO, "Source Disconnecting gracefully...\n");
    } 
    else 
    {
        log_message(LOG_INFO, "Source Disconnected unexpectedly, will try to reconnect...\n");
        mosquitto_reconnect(mosq);
    }
}

/**
 * @brief Main thread
 * 
 * @param arg 
 * @return void* 
 */
static void* mosq_source_reader_task(void* arg) 
{
    mosq_source_config* cfg = (mosq_source_config*) arg;
    
    // Initialize mosquitto library
    mosquitto_lib_init();

    struct mosquitto *mosq = NULL;
    int rc;
 
    mosq = mosquitto_new(NULL, true, (void*) cfg);

    if (!mosq) 
    {
        log_message(LOG_ERROR, "Error: Out of memory.\n");
        exit(ESYSERR);
    }

    // Set callback functions
    mosquitto_message_callback_set(mosq, on_message);
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);

    // Connect to MQTT broker
    rc = mosquitto_connect(mosq, cfg->host, cfg->port, 60);
    if (rc != MOSQ_ERR_SUCCESS) 
    {
        log_message(LOG_ERROR, "Unable to connect (%d): %s\n", rc, mosquitto_strerror(rc));
        exit(ESVRERR);
    }

    // Main loop
    while (1) 
    {
        rc = mosquitto_loop(mosq, -1, 1);
        if (rc != MOSQ_ERR_SUCCESS) 
        {
            log_message(LOG_INFO, "Mosquitto loop error (%d): %s\n", rc, mosquitto_strerror(rc));
            sleep(2); // Wait for a while before reconnecting
            
            mosquitto_reconnect(mosq);
        }
    }

    // Clean up
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return NULL;
}

/**
 * @brief Init Source task
 * 
 * @param cfg 
 * @param host 
 * @param port 
 * @param username 
 * @param password 
 * @return int 
 */
int mosq_source_init(mosq_source_config* cfg, Channel *c, const char* host, int port, const char* username, const char* password, const char* client_id, const char* topic)
{
    memset(cfg, 0, sizeof (mosq_source_config));

    cfg->c = c;

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
        memset(id, 0, sizeof(id));
        sprintf(id, "src_%lu\n", (unsigned long)time(NULL));

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
int mosq_source_term(mosq_source_config* cfg)
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
int mosq_source_run(mosq_source_config* cfg)
{   
    return 
        pthread_create(&cfg->task_thread, NULL, mosq_source_reader_task, cfg);
    
}

/**
 * @brief Wait until end
 * 
 * @param cfg 
 * @return int 
 */
int mosq_source_wait(mosq_source_config* cfg)
{
    return 
        pthread_join(cfg->task_thread, NULL);    
}