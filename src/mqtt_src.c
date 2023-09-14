#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "MQTTClient.h"
#include "mqtt_src.h"
#include "squeue.h"

//  
#define MQTT_ADDRESS "tcp://192.168.31.166:1883"  // Change to your MQTT broker address
#define MQTT_TOPIC "lxp/BA31605780"
#define MQTT_USERNAME "lxdvinhnguyen01"
#define MQTT_PASSWORD "lxd@123"
#define CLIENT_ID "ClientID-vn-influx-2" // Change to a unique identifier for your client
#define KEEP_ALIVE_INTERVAL 20

volatile MQTTClient_deliveryToken deliveredtoken;
volatile int connected = 0;


// deliver callback
static void delivered(void* context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);

    deliveredtoken = dt;
}

// msg arrive
static int msgarrvd(void* context, char* topicName, int topicLen, MQTTClient_message* message) {
    char* payloadptr = message->payload;
    int len = message->payloadlen;
    int i = 0;
    Channel *c = (Channel*) context;
    
    // Assuming message is in JSON format
    printf("Received message %d: %s: %s\n",len, topicName, payloadptr);

    // send to queue
    for (i=0; i< c->total; i++) {
        enqueue( c->queue[i], payloadptr);
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return 1;
}

// lost function callback
static void connectionLost(void* context, char* cause) {
    printf("Connection lost, cause: %s\n", cause);
    connected = 0;
}

// main task
void* mqtt_source_reader_task(void* arg) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    Channel *c = (Channel*) arg;

 
    while (1) {
        MQTTClient_create(&client, MQTT_ADDRESS, CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
        conn_opts.keepAliveInterval = KEEP_ALIVE_INTERVAL;
        conn_opts.cleansession = 1;
        // conn_opts.username = MQTT_USERNAME;
        // conn_opts.password = MQTT_PASSWORD;

        MQTTClient_setCallbacks(client, (void*) c, connectionLost, msgarrvd, delivered);

        if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
            printf("Failed to connect, return code %d. Retrying...\n", rc);
            sleep(5); // Wait for a while before retrying

            continue;
        }

        MQTTClient_subscribe(client, MQTT_TOPIC, 0);
        connected = 1;

        while (connected) {
            // Your main loop logic goes here

            sleep(1); // Sleep for a short period to avoid busy-waiting
        }

        MQTTClient_disconnect(client, 10000);
        MQTTClient_destroy(&client);
    }

    return NULL;
}
