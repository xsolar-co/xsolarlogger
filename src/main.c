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

#include <unistd.h> // for sleep
#include <getopt.h>

static void daemonize();
void* mqtt_sink_task(void* arg);
void* mqtt_source_reader_task(void* arg);

// 
Queue   influx_queue;
Queue   mqtt_sink_queue;
Channel channel;

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
    pthread_t mqtt_writer_thread;
    pthread_create(&mqtt_writer_thread, NULL, mqtt_sink_task, &mqtt_sink_queue);

    // mqtt source task
    logMessage(LOG_INFO, "Init MQTT source reader task\n");
    initChannel(&channel, 2);
    channel.queue[0] = &influx_queue;
    channel.queue[1] = &mqtt_sink_queue;
    pthread_t mqtt_source_reader_thread;
    pthread_create(&mqtt_source_reader_thread, NULL, mqtt_source_reader_task, &channel);

    while (1) {
        sleep(1);
    }

    cleanupLogger();
    pthread_join(influx_writer_thread, NULL);

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
