#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shared.h"

#define MAX_CONFIG_INT_LEN 4
#define MAX_CONFIG_STRING_LEN 11
#define CONFIG_LINE_BUFFER_SIZE 100

//types of servers
#define CONF_DIR "/etc/webserver/config.conf"

;int read_int_from_config_line(char* config_line) {    
    char prm_name[MAX_CONFIG_INT_LEN];
    int val;
    sscanf(config_line, "%s %d\n", prm_name, &val);
    return val;
}
void read_double_from_config_line(char* config_line, double* val) { 

    char prm_name[MAX_CONFIG_STRING_LEN];
    sscanf(config_line, "%s %lf\n", prm_name, val);
}
void read_str_from_config_line(char* config_line,char* val) {    
    char prm_name[MAX_CONFIG_STRING_LEN];
    sscanf(config_line, "%s %s", prm_name, val);
}


void read_config_file(struct config_struct *data,int serverType) {
    FILE *fp;
    char buf[CONFIG_LINE_BUFFER_SIZE];
    char *dir;
    dir = CONF_DIR;
    if ((fp=fopen(dir, "r")) == NULL) {
        fprintf(stderr, "Failed to open config file ");
        exit(EXIT_FAILURE);
    }
    while(! feof(fp)) {
        fgets(buf, CONFIG_LINE_BUFFER_SIZE, fp);
        if (buf[0] == '#' || strlen(buf) < 4) {
            continue;
        }
        if (strstr(buf, "PORT ")) {
            data->port = read_int_from_config_line(buf); 
        }
        if (strstr(buf, "LOGFILE ")) {
            char val[CONFIG_LINE_BUFFER_SIZE];
            read_str_from_config_line(buf,val);
            data->path= val;
        }
    }
}


