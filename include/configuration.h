#ifndef __CONFIG_MODULE__
#define __CONFIG_MODULE__

// Define a structure to hold configuration values
typedef struct {
    int port;
    char address[255];
    // Add more configuration variables as needed
} Config;

int load_config(const char* filename, Config* config);

#endif // !__CONFIG_MODULE__

