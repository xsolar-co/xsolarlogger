#ifndef __INFLUXDB_H__
#define __INFLUXDB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <curl/curl.h>

void* influxdb_write_task(void* arg);

#endif // !__INFLUXDB_H__
#define __INFLUXDB_H__