// main
#include "logger.h"
#include "squeue.h"
#include "datalog.h"
#include "influxdb.h"

#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "MQTTClient.h"
#include "cjson/cJSON.h"
#include "mqtt_sink.h"
#include "mqtt_src.h"

#include <unistd.h> // for sleep
#include <getopt.h>

static void daemonize();
 
Queue   influx_queue;
Queue   mqtt_sink_queue;
Channel channel;

mqtt_sync_config mqtt_sink_conf;

int mqtt_sink_task_init()
{
    char* host = "103.161.39.186";
    int port = 1883;
    char* username = "lxdvinhnguyen01";
    char* password = "lxd@123";
    char* clientid = "sinktaskcli-01";
    char* topic = "lxd/BA31605780";
    
    mqtt_sink_init(&mqtt_sink_conf, &mqtt_sink_queue, host, port, username, password, NULL, topic);
    mqtt_sink_run(&mqtt_sink_conf);

    return 0;
}

int mqtt_sink_task_cleanup()
{
    mqtt_sink_wait(&mqtt_sink_conf);
    mqtt_sink_term(&mqtt_sink_conf);

    return 0;
}

mqtt_source_config mqtt_source_conf;
int mqtt_source_task_init()
{
    char* host = "192.168.31.166";
    int port = 1883;
    char* username = NULL;
    char* password = NULL;
    char* clientid = "sourcetaskcli-01";
    char* topic = "lxp/BA31605780";
    
    mqtt_source_init(&mqtt_source_conf, &channel, host, port, username, password, NULL, topic);
    mqtt_source_run(&mqtt_source_conf);

    return 0;
}

int mqtt_source_task_cleanup()
{
    mqtt_source_wait(&mqtt_source_conf);
    mqtt_source_term(&mqtt_source_conf);

    return 0;
}


int main(int argc, char* argv[]) {
    int daemonize_flag = 0;
    char *config_file = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "dc:")) != -1) {
        switch (opt) {
            case 'd':
                daemonize_flag = 1;
                break;
            case 'c':
                config_file = optarg;
                break;
            default:
                printf("Usage: %s [-d] -c config_file\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // if (config_file == NULL) {
    //     printf("Error: Missing config file. Use -c option to specify a config file.\n");
    //     exit(EXIT_FAILURE);
    // }

    if (daemonize_flag) {
        daemonize();
    }

    // loadConfig(config_file);
    initLogger("logfile.txt");

    // influx task
    logMessage(LOG_INFO, "Init Influx send task\n");
    initQueue(&influx_queue);
    pthread_t influx_writer_thread;
    pthread_create(&influx_writer_thread, NULL, influxdb_write_task, &influx_queue);

    // mqtt target task
    logMessage(LOG_INFO, "Init MQTT send task\n");
    initQueue(&mqtt_sink_queue);
    mqtt_sink_task_init();
  
    // mqtt source task
    logMessage(LOG_INFO, "Init MQTT source reader task\n");
    initChannel(&channel, 2);
    channel.queue[0] = &influx_queue;
    channel.queue[1] = &mqtt_sink_queue;
    mqtt_source_task_init();

    // pthread_t mqtt_source_reader_thread;
    // pthread_create(&mqtt_source_reader_thread, NULL, mqtt_source_reader_task, &channel);

    while (1) {
        sleep(1);
    }

    cleanupLogger();
    pthread_join(influx_writer_thread, NULL);

    mqtt_source_task_cleanup();
    mqtt_sink_task_cleanup();



    return 0;
}

// go background
void daemonize() {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
