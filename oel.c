#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

#define MAX_URL_LEN 200
#define MAX_CITY_LEN 50
#define FIXED_CITY "Karachi"

// Structure to store temperature data
struct TemperatureData {
    float temperature;
    time_t timestamp;
};

#define MAX_TEMPERATURE_ENTRIES 24
struct TemperatureData temperatureData[MAX_TEMPERATURE_ENTRIES];
int temperatureDataCount = 0;

void processWeatherData(char *jsonString);

size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char *result = (char *)userp;

    // Append the received data to the result buffer
    strncat(result, (char *)contents, realsize);

    // Check if a complete JSON object is received
    char *jsonObjectEnd = strstr(result, "}\n");
    while (jsonObjectEnd != NULL) {
        // Extract the JSON object
        size_t jsonObjectLength = jsonObjectEnd - result + 2;
        char jsonObject[MAX_URL_LEN];  // Adjust the buffer size as needed
        strncpy(jsonObject, result, jsonObjectLength);
        jsonObject[jsonObjectLength] = '\0';

        // Process the JSON object
        processWeatherData(jsonObject);

        // Remove the processed JSON object from the result buffer
        memmove(result, result + jsonObjectLength, realsize - jsonObjectLength + 1);

        // Check for another JSON object in the remaining data
        jsonObjectEnd = strstr(result, "}\n");
    }

    return realsize;
}

void makeRequest(char *url, char *result) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, result);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

void saveToFile(char *filename, char *data) {
    FILE *file = fopen(filename, "a");  // Open in append mode
    if (file != NULL) {
        fprintf(file, "%s\n", data);  // Append with a newline
        fclose(file);
    } else {
        fprintf(stderr, "Unable to open file: %s\n", filename);
    }
}

void processWeatherData(char *jsonString) {
    cJSON *root = cJSON_Parse(jsonString);

    if (root != NULL) {
        // Save the raw JSON data to a file
        saveToFile("rawData.json", jsonString);

        cJSON *current = cJSON_GetObjectItem(root, "current");
        if (current != NULL) {
            cJSON *temperature = cJSON_GetObjectItem(current, "temp_c");
            if (temperature != NULL && cJSON_IsNumber(temperature)) {
                // Save temperature with timestamp to the array
                if (temperatureDataCount < MAX_TEMPERATURE_ENTRIES) {
                    temperatureData[temperatureDataCount].temperature = cJSON_GetNumberValue(temperature);
                    temperatureData[temperatureDataCount].timestamp = time(NULL);
                    temperatureDataCount++;

                    // Save temperature data for each iteration to processedData.txt
                    char tempData[100];
                    snprintf(tempData, sizeof(tempData), "%.2f", cJSON_GetNumberValue(temperature));
                    saveToFile("processedData.txt", tempData);
                }
            } else if (temperature == NULL) {
                fprintf(stderr, "Temperature field not found in JSON data\n");
            } else {
                fprintf(stderr, "Temperature field is not a number in JSON data\n");
            }
        } else {
            fprintf(stderr, "Current data not found in JSON\n");
        }

        cJSON_Delete(root);
    } else {
        fprintf(stderr, "Error parsing JSON data\n");
    }
}

int main() {
    char key[] = "c98706cc8a494b46855184724230409";
    char url[MAX_URL_LEN];

    snprintf(url, sizeof(url), "http://api.weatherapi.com/v1/current.json?key=%s&q=%s", key, FIXED_CITY);

    char result[4096];
    result[0] = '\0';

    makeRequest(url, result);
    processWeatherData(result);

    return 0;
}
