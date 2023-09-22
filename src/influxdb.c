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

static void convertToInfluxDBLine(struct DataLog* data, char* influxDBLine) {
    sprintf(influxDBLine,
        "lxpvtf status=%ld,v_pv_1=%lf,v_pv_2=%ld,v_pv_3=%lf,v_bat=%ld,soc=%ld,soh=%ld,"
        "p_pv=%ld,p_pv_1=%ld,p_pv_2=%ld,p_pv_3=%ld,p_charge=%ld,p_discharge=%ld,"
        "v_ac_r=%lf,v_ac_s=%ld,v_ac_t=%ld,f_ac=%lf,p_inv=%ld,p_rec=%ld,pf=%ld,"
        "v_eps_r=%lf,v_eps_s=%lf,v_eps_t=%ld,f_eps=%lf,p_eps=%ld,s_eps=%ld,"
        "p_to_grid=%ld,p_to_user=%ld,e_pv_day=%lf,e_pv_day_1=%lf,e_pv_day_2=%ld,e_pv_day_3=%ld,"
        "e_inv_day=%lf,e_rec_day=%ld,e_chg_day=%ld,e_dischg_day=%ld,e_eps_day=%ld,e_to_grid_day=%ld,"
        "e_to_user_day=%lf,v_bus_1=%lf,v_bus_2=%lf,e_pv_all=%lf,e_pv_all_1=%lf,e_pv_all_2=%ld,e_pv_all_3=%ld,"
        "e_inv_all=%lf,e_rec_all=%ld,e_chg_all=%ld,e_dischg_all=%ld,e_eps_all=%ld,e_to_grid_all=%ld,"
        "e_to_user_all=%lf,t_inner=%ld,t_rad_1=%ld,t_rad_2=%ld,t_bat=%ld,runtime=%ld,"
        "max_chg_curr=%ld,max_dischg_curr=%ld,charge_volt_ref=%ld,dischg_cut_volt=%ld,"
        "bat_status_0=%ld,bat_status_1=%ld,bat_status_2=%ld,bat_status_3=%ld,bat_status_4=%ld,"
        "bat_status_5=%ld,bat_status_6=%ld,bat_status_7=%ld,bat_status_8=%ld,bat_status_9=%ld,"
        "bat_status_inv=%ld,bat_count=%ld,bat_capacity=%ld,bat_current=%ld,bms_event_1=%ld,"
        "bms_event_2=%ld,max_cell_voltage=%ld,min_cell_voltage=%ld,max_cell_temp=%ld,"
        "min_cell_temp=%ld,bms_fw_update_state=%ld,cycle_count=%ld,vbat_inv=%lf,time=%ld,datalog=%s",
        *data->status, *data->v_pv_1, *data->v_pv_2, *data->v_pv_3, *data->v_bat, *data->soc, *data->soh,
        *data->p_pv, *data->p_pv_1, *data->p_pv_2, *data->p_pv_3, *data->p_charge, *data->p_discharge,
        *data->v_ac_r, *data->v_ac_s, *data->v_ac_t, *data->f_ac, *data->p_inv, *data->p_rec, *data->pf,
        *data->v_eps_r, *data->v_eps_s, *data->v_eps_t, *data->f_eps, *data->p_eps, *data->s_eps,
        *data->p_to_grid, *data->p_to_user, *data->e_pv_day, *data->e_pv_day_1, *data->e_pv_day_2, *data->e_pv_day_3,
        *data->e_inv_day, *data->e_rec_day, *data->e_chg_day, *data->e_dischg_day, *data->e_eps_day, *data->e_to_grid_day,
        *data->e_to_user_day, *data->v_bus_1, *data->v_bus_2, *data->e_pv_all, *data->e_pv_all_1, *data->e_pv_all_2,
        *data->e_pv_all_3, *data->e_inv_all, *data->e_rec_all, *data->e_chg_all, *data->e_dischg_all, *data->e_eps_all,
        *data->e_to_grid_all, *data->e_to_user_all, *data->t_inner, *data->t_rad_1, *data->t_rad_2, *data->t_bat,
        *data->runtime, *data->max_chg_curr, *data->max_dischg_curr, *data->charge_volt_ref, *data->dischg_cut_volt,
        *data->bat_status_0, *data->bat_status_1, *data->bat_status_2, *data->bat_status_3, *data->bat_status_4,
        *data->bat_status_5, *data->bat_status_6, *data->bat_status_7, *data->bat_status_8, *data->bat_status_9,
        *data->bat_status_inv, *data->bat_count, *data->bat_capacity, *data->bat_current, *data->bms_event_1,
        *data->bms_event_2, *data->max_cell_voltage, *data->min_cell_voltage, *data->max_cell_temp, *data->min_cell_temp,
        *data->bms_fw_update_state, *data->cycle_count, *data->vbat_inv, *data->time, data->datalog);
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

                char influxDataFull[1024*2];
                convertToInfluxDBLine(logData, influxDataFull);
                sendDataToInfluxDBv2(cfg, influxDataFull);

               
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