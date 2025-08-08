#ifndef PARSER_H
#define PARSER_H

#include <cJSON.h>
struct stop;
int return_parent(cJSON *json_data, struct stop **stops);
int return_children(cJSON *json_data, const char *parent_name, char ***strings);
int return_child(cJSON *json_data,const char *parent_name, struct stop **stops);
char *return_id(cJSON *json_data, const char *parent_name, const char *child_name);
int return_ETA(char *JSONdata, char *sys_time,  int **times);
int return_time_diff(char *eta_time, char *sys_time);


#endif