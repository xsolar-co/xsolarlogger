/**
 * @file influxdb.c
 * @author longdh (longdh@xsolar.vn)
 * @brief 
 * @version 0.1
 * @date 2023-09-21
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "influxdb.h"
#include "cjson/cJSON.h"
#include "squeue.h"
#include "datalog.h"
#include <stdio.h>

//FIXME
extern char* strdup(const char*);

#define INFLUXDB_ADDRESS    "http://192.168.31.166:8086/write?db=lxp"
#define INFLUXDB2_ADDRESS   "http://103.161.39.186:8086/api/v2/write?org=5b2b5d425dabd4e0&bucket=lxpb"
#define INFLUXDB2_USERNAME  ""
#define INFLUXDB2_PASSWORD  ""
#define INFLUXDB2_ORG       "5b2b5d425dabd4e0"
#define INFLUXDB2_TOKEN     "1Ir9UfVc6xq2Tl8b2G_kn-79N13CeS6Vzr1XSR9SLIt-nktNUlticVYkMSn90aHWVacVy1xEtob8QlzxnJjrkQ=="

/**
 * @brief V1
 * 
 * @param data 
 */
static void sendDataToInfluxDB(influx_sink_config* cfg, const char* data) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, INFLUXDB_ADDRESS);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

/**
 * @brief send data to InfluxDB v2 using libcurl
 * 
 * @param data 
 */
static void sendDataToInfluxDBv2(influx_sink_config* cfg, const char* data) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: text/plain; charset=utf-8");

        char auth[512];
        snprintf(auth, sizeof(auth), "Authorization: Token %s", cfg->token);
        headers = curl_slist_append(headers, auth);

        // char url[256]; 
        // sprintf(url, INFLUXDB2_ADDRESS); // Modify URL parameters accordingly

        // curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_URL, cfg->url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

/**
 * @brief Influx Write function
 * 
 * @param arg 
 * @return void* 
 */
static void* influxdb_write_task(void* arg) {
    influx_sink_config* cfg = (influx_sink_config*) arg;
    Queue* q = (Queue*)cfg->q;

    char data[2048];
    while (1) {        
        int ret = wait_dequeue(q, data);

        if (ret == 0) {
            continue;
        }

        printf("%s\n", data);
        
        // Parse JSON with cJSON
        cJSON* json = cJSON_Parse(data);
        if (json) {

            // parse to object
            struct DataLog * logData = cJSON_GetDataLogValue(json);
            if (logData)
            {
                char* jsonp = cJSON_PrintDataLog(logData);

                // DEBUG: 
                printf("%s\n", jsonp);

                // Convert data to InfluxDB format and send
                char influxData[1024]; // Adjust size as needed
                snprintf(influxData, sizeof(influxData), 
                    "lxpvt v_pv_1=%f,p_pv_1=%ld,p_inv=%ld,p_to_user=%ld,e_to_user_day=%f,e_pv_all_1=%f,e_to_user_all=%f\n", 
                    *logData->v_pv_1,
                    *logData->p_pv_1,
                    *logData->p_inv,
                    *logData->p_to_user,
                    *logData->e_to_user_day,
                    *logData->e_pv_all_1,
                    *logData->e_to_user_all
                    // *logData->time * 1000000000ll
                    );

                // sendDataToInfluxDB(influxData);
                sendDataToInfluxDBv2(cfg, influxData);
               
                // free
                cJSON_free(jsonp);
                cJSON_DeleteDataLog(logData);
            }

        } else {
            // LOG
            printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        }
    }
    
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
int influx_sink_init(influx_sink_config* cfg, Queue* q, const char* host, int port, const char* username, const char* password, const char* client_id, const char* topic)
{
    memset(cfg, 0, sizeof (influx_sink_config));

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
        sprintf(id, "client-src-%lu\n", (unsigned long)time(NULL));

        cfg->client_id = strdup(id);
    } 

    return 0;
}

/**
 * @brief init for v2
 * 
 * @param cfg 
 * @param q 
 * @param url 
 * @param orgid 
 * @param token 
 * @return int 
 */
int influx_sink_init2(influx_sink_config* cfg, Queue* q, const char* url, const char* orgid, const char* token)
{
    memset(cfg, 0, sizeof (influx_sink_config));

    cfg->q = q;

    if (url != NULL)
        cfg->url = strdup(url);

    if (orgid != NULL)
        cfg->orgid = strdup(orgid);

    if (token != NULL)
        cfg->token = strdup(token);

    return 0;
}

/**
 * @brief Free task's data
 * 
 * @param cfg 
 * @return int 
 */
int influx_sink_term(influx_sink_config* cfg)
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
int influx_sink_run(influx_sink_config* cfg)
{   
    return 
        pthread_create(&cfg->task_thread, NULL, influxdb_write_task, cfg);
    
}

/**
 * @brief Wait until end
 * 
 * @param cfg 
 * @return int 
 */
int influx_sink_wait(influx_sink_config* cfg)
{
    return 
        pthread_join(&cfg->task_thread, NULL);    
}