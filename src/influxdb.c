#include "influxdb.h"
#include "cjson/cJSON.h"
#include "squeue.h"
#include "datalog.h"

#define INFLUXDB_ADDRESS    "http://192.168.31.166:8086/write?db=lxp"
#define INFLUXDB2_ADDRESS   "http://103.161.39.186:8086/api/v2/write?org=5b2b5d425dabd4e0&bucket=lxp"
#define INFLUXDB2_USERNAME  ""
#define INFLUXDB2_PASSWORD  ""
#define INFLUXDB2_ORG       "5b2b5d425dabd4e0"
#define INFLUXDB2_TOKEN     "ygqFkEUqpWUMyyAu-gA_na_IIqfi9oey7oHu-XmgV9RCxpnNvSgUjG2KB1m9vzFyP9FXd2aSkt1LcPjs_mwjow=="


void sendDataToInfluxDB(const char* data) {
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

// Function to send data to InfluxDB v2 using libcurl
// Function to send data to InfluxDB v2
void sendDataToInfluxDBv2(const char* data) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: text/plain; charset=utf-8");
        headers = curl_slist_append(headers, "Authorization: Token "INFLUXDB2_TOKEN); // Replace with your actual token

        char url[256]; // Assuming your InfluxDB URL is within 256 characters
        sprintf(url, INFLUXDB2_ADDRESS); // Modify URL parameters accordingly

        curl_easy_setopt(curl, CURLOPT_URL, url);
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

void* influxdb_write_task(void* arg) {
    Queue* q = (Queue*)arg;
    while (1) {
        char data[2048];
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
                sendDataToInfluxDBv2(influxData);
               
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
