#include <cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_SIZE 4096
struct stop
{
    char *name;
    char *id; // prolly just recast this later
};

// send processes JSON, return parent nodes as list of strings
int return_parent(cJSON *json_data, struct stop **stops)
{
    int total = cJSON_GetArraySize(json_data);
    *stops = malloc(total * sizeof(struct stop));
    if (*stops == NULL)
    {
        perror("[-] Malloc failed for stops array.");
        return 0;
    }

    for (int i = 0; i < total; i++)
    {
        cJSON *item = cJSON_GetArrayItem(json_data, i);
        if (item == NULL || item->string == NULL)
        {
            (*stops)[i].name = NULL;
            continue;
        }

        // Allocate and copy the name
        (*stops)[i].name = strdup(item->string);
        (*stops)[i].id = NULL; // or handle if needed
    }

    return total;
}

// takes in the parent name/route id, returns all children from parent node in array struct
int return_children(cJSON *json_data, const char *parent_name, char ***strings)
{
    const cJSON *parent = NULL;
    const cJSON *child = NULL;
    int i = 0;

    parent = cJSON_GetObjectItem(json_data, parent_name);
    int total = cJSON_GetArraySize(parent);
    *strings = malloc((total + 1) * sizeof(char *));
    if (*strings == NULL)
    {
        return 0; // Handle allocation failure
    }

    // child will be the whole list
    cJSON_ArrayForEach(child, parent)
    {
        int length = strlen(child->child->string);
        (*strings)[i] = malloc((length + 1) * sizeof(char *));
        if ((*strings)[i] == NULL)
        {
            // Handle allocation failure - should clean up previous allocations
            continue;
        }
        strcpy((*strings)[i], child->child->string);
        i++;
    }
    (*strings)[total] = NULL;
    return total;
}

int return_child(cJSON *json_data, const char *parent_name, struct stop **stops)
{
    const cJSON *parent = NULL;
    const cJSON *child = NULL;
    int i = 0;

    parent = cJSON_GetObjectItem(json_data, parent_name);
    if (!parent)
    {
        fprintf(stderr, "[-] Parent not found\n");
        return 0;
    }
    int total = cJSON_GetArraySize(parent);

    *stops = malloc((total + 1) * sizeof(struct stop)); // pre allocate memory for stops
    if (!stops)
    {
        perror("[-] Malloc Failed!.");
        return 0;
    }

    // populate with json info
    cJSON_ArrayForEach(child, parent)
    {
        // assign data
        (*stops)[i].name = strdup(child->child->valuestring);
        (*stops)[i].id = strdup(child->child->string);
        i++;
    }
    (*stops)[total].name = NULL;
    (*stops)[total].id = NULL;
    return total;
}

// need a function to translate stop to stop_id, returns the id
char *return_id(cJSON *json_data, const char *parent_name, const char *child_name)
{
    const cJSON *parent = NULL;
    const cJSON *child = NULL;
    char *child_id;
    int i = 0;

    parent = cJSON_GetObjectItem(json_data, parent_name);
    int total = cJSON_GetArraySize(parent);

    child = cJSON_GetObjectItemCaseSensitive(parent, child_name);

    child_id = strdup(child->child->string);

    return child_id;
}

int return_time_diff(char *eta_time, char *sys_time)
{
    // seperation for eta time
    int hr1;
    int min1;
    sscanf(eta_time, "%2d", &hr1);
    sscanf(eta_time, "%*[^:]:%2d", &min1);

    int hr2;
    int min2;
    sscanf(sys_time, "%2d", &hr2);
    sscanf(sys_time, "%*[^:]:%2d", &min2);

    int hrr = hr1 - hr2;
    int minr = min1 - min2;

    // turn hour into minutes
    if (hrr > 0)
    {
        hrr = hrr * 60;
        minr = hrr + minr;
    }

    // char *time_eta;
    // printf("time: %d:%d\n", hrr, minr);
    // snprintf(time_eta, sizeof(time_eta), ":%d", minr);
    return minr;
}

int return_ETA(char *JSONdata, char *sys_time, int **times)
{
    cJSON *json;
    cJSON *bus_list;
    cJSON *bus_info;
    cJSON *MonitoredVehicle;
    char *buffer = NULL;
    size_t buffer_len = 0;

    // throw it in JSON variable
    json = cJSON_Parse(JSONdata);
    bus_list = cJSON_GetObjectItemCaseSensitive(json, "Siri"); // list with buses and bus info
    // go through list
    cJSON *ServiceDelivery = cJSON_GetObjectItemCaseSensitive(bus_list, "ServiceDelivery");
    cJSON *StopMonitorDelivery = cJSON_GetObjectItemCaseSensitive(ServiceDelivery, "StopMonitoringDelivery");
    cJSON *MonitoredStopVisitOBJ = cJSON_GetArrayItem(StopMonitorDelivery, 0);
    cJSON *MonitoredStopVisit = cJSON_GetObjectItemCaseSensitive(MonitoredStopVisitOBJ, "MonitoredStopVisit");
    cJSON *MonitoredVehicleJourney;
    cJSON *monitoredCall;

    // get array size
    int total = cJSON_GetArraySize(MonitoredStopVisit);

    *times = malloc((total) * sizeof(int)); // pre allocate memory for stops
    if (!times)
    {
        perror("[-] Malloc Failed!.");
        return 0;
    }
    // go through each stop time and return eta
    int i = 0;
    cJSON_ArrayForEach(bus_info, MonitoredStopVisit)
    {
        // Go to "monitored Call" attribute
        MonitoredVehicleJourney = cJSON_GetObjectItemCaseSensitive(bus_info, "MonitoredVehicleJourney");
        monitoredCall = cJSON_GetObjectItemCaseSensitive(MonitoredVehicleJourney, "MonitoredCall");
        cJSON *ETA = cJSON_GetObjectItemCaseSensitive(monitoredCall, "ExpectedArrivalTime");

        // find item size
        size_t item_length = strlen(cJSON_Print(ETA));
        char eta[7];
        char *t2;
        sscanf(cJSON_Print(ETA), "%*[^T]T%5s", eta);
        (*times)[i] = return_time_diff(eta, sys_time);
        i++;

        // (*times)[i] = strdup(t2); //store into times list
        // (*times)[i].id = NULL;
    }
    // nul term
    //(*times)[total] = NULL;
    return total;
}