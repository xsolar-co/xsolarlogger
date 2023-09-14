#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "MQTTClient.h"
#include "squeue.h"

#define MQTT_ADDRESS "tcp://103.161.39.186:1883"  // Change to your MQTT broker address
#define MQTT_TOPIC "lxd/BA31605780"
#define MQTT_USERNAME "lxdvinhnguyen01"
#define MQTT_PASSWORD "lxd@123"
#define CLIENT_ID "ClientID-vn" // Change to a unique identifier for your client

// connection lost callback
volatile int _connected = 0;
static void connectionLost(void* context, char* cause) {
    printf("Connection lost, cause: %s\n", cause);
    _connected = 0;
}

// delivery callback
volatile MQTTClient_deliveryToken _deliveredtoken;
static void delivered(void* context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);

    _deliveredtoken = dt;
}

// sync thread
void* mqtt_sink_task(void* arg) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    Queue* q = (Queue*)arg;
    char data[MAX_QUEUE_DATA_SIZE];

    if (q == NULL)
    {
        printf("Error queue...\n");
        exit(-1);
    }

    while(1)
    {
        MQTTClient_create(&client, MQTT_ADDRESS, CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
        conn_opts.keepAliveInterval = 20;
        conn_opts.cleansession = 1;
        conn_opts.username = MQTT_USERNAME;
        conn_opts.password = MQTT_PASSWORD;

        MQTTClient_setCallbacks(client, NULL, connectionLost, NULL, delivered);


        if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
            printf("Failed to connect, return code %d, retrying ...\n", rc);
            sleep(5);

            continue;
        }

        _connected = 1;

    
        while (_connected) {
            if (wait_dequeue(q, data))
            {
                printf("%s\n", data);


                MQTTClient_message pubmsg = MQTTClient_message_initializer;
                MQTTClient_deliveryToken token;

                pubmsg.payload = data;
                pubmsg.payloadlen = strlen(data);
                pubmsg.qos = 1;
                pubmsg.retained = 0;

                MQTTClient_publishMessage(client, MQTT_TOPIC, &pubmsg, &token);
                MQTTClient_waitForCompletion(client, token, 1000);
            }
        }

        // connect fail, reconnect
        MQTTClient_disconnect(client, 10000);
        MQTTClient_destroy(&client);
    }


    return NULL;
}