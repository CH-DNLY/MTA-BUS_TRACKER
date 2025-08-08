#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#define KEY "f73a02bc-9f41-431c-b0da-8cc450f2253d"

struct  data {
    char *data;    // Buffer to store the response
    size_t size;   // Current size of the data
};

//write function based off the curl example
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    struct data *res_data = (struct data *)userp;
    size_t total_size = size * nmemb;

    // Expand the buffer and append new data
    char *new_data = realloc(res_data->data, res_data->size + total_size + 1);
    if (!new_data) {
        fprintf(stderr, "Failed to allocate memory!\n");
        return 0;
    }

    res_data->data = new_data;
    memcpy(&res_data->data[res_data->size], buffer, total_size); // Append new chunk
    res_data->size += total_size;
    res_data->data[res_data->size] = '\0'; // Null-terminate

    return total_size;
}

//set up curl
//query url
//return respose
char *return_response(char *stop_id, char* route_id)
{
    char url[2048];
    //"%%" make sure the orignal %20 or space is recognized as a string
    snprintf(url, sizeof(url), "https://bustime.mta.info/api/siri/stop-monitoring.json?key=%s&OperatorRef=MTA&MonitoringRef=%s&LineRef=MTA%%20NYCT_%s", KEY, stop_id, route_id);

    struct data chunk = {0};
    CURLcode res;
    CURL *curl = curl_easy_init();
    if (curl) {
        //set url
        curl_easy_setopt(curl, CURLOPT_URL, url);

        //tunnel data thru write function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        //pass data struct to write func thru curl
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        //send request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            free(chunk.data);
            return NULL;
        }
        return chunk.data;
    }
   return NULL; 
}

//sends request to MTA
