// main
#include <unistd.h>
#include <stdlib.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h> // for sleep
#include <getopt.h>

#include "MQTTClient.h"
#include "cjson/cJSON.h"
#include "mqtt_sink.h"
#include "mqtt_src.h"
#include "logger.h"
#include "squeue.h"
#include "datalog.h"
#include "influxdb.h"
#include "configuration.h"


static void daemonize();

// libconfig
config_t cfg;
 
Queue   influx_queue;
Queue   mqtt_sink_queue;
Channel channel;

influx_sink_config influx_sink_conf;
int influx_sink_task_init()
{
        // read config
    // Access the 'mqtt-sink' subsetting
    config_setting_t* influx_sink = config_lookup(&cfg, "influx-sink");

    if (influx_sink != NULL) {
        char* url = (char*)read_string_setting(influx_sink, "url", "");
        char* orgid = (char*)read_string_setting(influx_sink, "orgid", "lxdvinhnguyen01");
        char* token = (char*)read_string_setting(influx_sink, "token", "lxd@123");
        
        printf("url: %s\n", url);
        printf("orgid: %s\n", orgid);
        printf("token: %s\n", token);
    
        influx_sink_init2(&influx_sink_conf, &mqtt_sink_queue, url, orgid, token);
        influx_sink_run(&influx_sink_conf);

    } else {
        fprintf(stderr, "The 'influx-sink' subsetting is missing.\n");
    }   

    return 0;
}

int influx_sink_task_cleanup()
{
    influx_sink_wait(&influx_sink_conf);
    influx_sink_term(&influx_sink_conf);

    return 0;
}

mqtt_sync_config mqtt_sink_conf;
int mqtt_sink_task_init()
{
    // read config
    // Access the 'mqtt-sink' subsetting
    config_setting_t* mqtt_src = config_lookup(&cfg, "mqtt-sink");

    if (mqtt_src != NULL) {
        char* host = (char*)read_string_setting(mqtt_src, "host", "103.161.39.186");
        int port = read_int_setting(mqtt_src, "port", 1883);
        char* username = (char*)read_string_setting(mqtt_src, "username", "lxdvinhnguyen01");
        char* password = (char*)read_string_setting(mqtt_src, "password", "lxd@123");
        char* clientid = (char*)read_string_setting(mqtt_src, "clientid", "sinktaskcli-01");
        char* topic = (char*)read_string_setting(mqtt_src, "topic", "lxd/BA31605780");

        printf("Host: %s\n", host);
        printf("Port: %d\n", port);
        printf("Username: %s\n", username);
        printf("Password: %s\n", password);
        printf("Client ID: %s\n", clientid);
        printf("Topic: %s\n", topic);

        mqtt_sink_init(&mqtt_sink_conf, &mqtt_sink_queue, host, port, username, password, NULL, topic);
        mqtt_sink_run(&mqtt_sink_conf);

    } else {
        fprintf(stderr, "The 'mqtt-sink' subsetting is missing.\n");
    }   

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
    config_setting_t* mqtt_src = config_lookup(&cfg, "mqtt-src");

    if (mqtt_src != NULL) {
        char* host = (char*)read_string_setting(mqtt_src, "host", "192.168.31.166");
        int port = read_int_setting(mqtt_src, "port", 1883);
        char* username = (char*)read_string_setting(mqtt_src, "username", "lxdvinhnguyen01");
        char* password = (char*)read_string_setting(mqtt_src, "password", "lxd@123");
        char* clientid = (char*)read_string_setting(mqtt_src, "clientid", "sourcetaskcli-01");
        char* topic = (char*)read_string_setting(mqtt_src, "topic", "lxp/BA31605780");

        printf("Host: %s\n", host);
        printf("Port: %d\n", port);
        printf("Username: %s\n", username);
        printf("Password: %s\n", password);
        printf("Client ID: %s\n", clientid);
        printf("Topic: %s\n", topic);

        mqtt_source_init(&mqtt_source_conf, &channel, host, port, username, password, NULL, topic);
        mqtt_source_run(&mqtt_source_conf);


        // free(host);
        // free(username);
        // free(password);
        // free(clientid);
        // free(topic);

    } else {
        fprintf(stderr, "The 'mqtt-src' subsetting is missing.\n");
    }   

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

    if (config_file == NULL) {
        printf("Error: Missing config file. Use -c option to specify a config file.\n");
        config_file = "../etc/config.cfg";
    }

    config_init(&cfg);

    if (!config_read_file(&cfg, config_file)) {
        fprintf(stderr, "Error reading configuration file: %s\n", config_error_text(&cfg));
        config_destroy(&cfg);

        return 1;
    }

    if (daemonize_flag) {
        daemonize();
    }

    // loadConfig(config_file);
    initLogger("logfile.log");

    // influx task
    logMessage(LOG_INFO, "Init Influx send task\n");
    initQueue(&influx_queue);
    influx_sink_task_init();

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

    while (1) {
        sleep(1);
    }

    cleanupLogger();
    mqtt_source_task_cleanup();
    influx_sink_task_cleanup();
    mqtt_sink_task_cleanup();

    config_destroy(&cfg);

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
