//#include "lyra_plugin.h"
#include "mosquitto_plugin.h"
#include "stdio.h"
#include "curl/curl.h"
#include "string.h"
#include "stdlib.h"
#include "lyra_plugin.h"
#include "cjson/cJSON.h"
#include "mosquitto.h"
#include "mosquitto_broker.h"


// Support only mqtt v5
int mosquitto_plugin_version(int supported_version_count, const int *supported_versions
)
{
    for(int i = 0; i < supported_version_count; i++)
    {
        fprintf(stdout, "version supported : %i\n", supported_versions[i]);
    }
    return 5;
}

int basic_auth_callback(int event, void *event_data, void *user_data)
{
    struct mosquitto_evt_basic_auth *data = (struct mosquitto_evt_basic_auth *) event_data;
    struct broker_settings *settings = (struct broker_settings *) user_data;
    struct memory_payload response = {.curl_status = CURLE_FAILED_INIT, .size = 0, .memory = malloc(1)};
    AUTH_RESULT result = UNAUTHORIZED;

    if(response.memory == NULL) {
        fprintf(stderr, "Failed to allocate dynamically memory for response payload.");
        goto cleanup;
    }

    int url_len = strlen(settings->protocol) + 3 + strlen(settings->host) + 1 + strlen(settings->port) + strlen(settings->endpoint_auth) + 1;

    char * url = malloc(url_len);
    if(url == NULL)
    {
        fprintf(stderr, "Failed to allocated memory for url\n");
        goto cleanup;
    }

    // construct url
    snprintf(url, url_len, "%s://%s:%s%s",
        settings->protocol,
        settings->host,
        settings->port,
        settings->endpoint_auth);


    struct payload_attributes attributes[3] = {{"client_id", mosquitto_client_id(data->client)}, {"username", data->username}, {"password", data->password}};
    

    call_api(url, attributes, 3, &response);

    result = validate_auth_response(&response);

    cleanup:

        if(response.memory) {
            free(response.memory);
        }

        if(url) {
            free(url);
        }

        if(response.curl_status != CURLE_OK) {
            return MOSQ_ERR_AUTH;
        }

    if(result == AUTHORIZED) {
        return MOSQ_ERR_SUCCESS;
    }

    return MOSQ_ERR_AUTH;

}

int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **userdata, struct mosquitto_opt *options, int option_count)
{
    
    struct broker_settings *settings = malloc(sizeof(struct broker_settings));

    for(int i = 0; i < option_count; i++)
    {
        if(strcmp(options[i].key, "host") == 0)
        {
            settings->host = strdup(options[i].value);
        }

        if(strcmp(options[i].key, "protocol") == 0)
        {
            settings->protocol = strdup(options[i].value);
        }

        if(strcmp(options[i].key, "port") == 0)
        {
            settings->port = strdup(options[i].value);
        }

        if(strcmp(options[i].key, "endpoint_auth") == 0)
        {
            settings->endpoint_auth = strdup(options[i].value);
        }

        if(strcmp(options[i].key, "endpoint_acls") == 0)
        {
            settings->endpoint_acls = strdup(options[i].value);
        }
    }

    mosquitto_callback_register(identifier, MOSQ_EVT_BASIC_AUTH, basic_auth_callback, NULL, settings);

    return MOSQ_ERR_SUCCESS;
}

int mosquitto_plugin_cleanup(void *userdata, struct mosquitto_opt *options, int option_count)
{
    struct broker_settings *data = (struct broker_settings*) userdata;

    if(data)
    {
        free_broker_settings_struct(data);
    }

    return 0;
}

void free_broker_settings_struct(struct broker_settings* data) {
    fprintf(stdout, "Release malloc ressources.");
    free(data->host);
    free(data);
}

CURLcode call_api(char *url, struct payload_attributes *attributes, int attribut_count, struct memory_payload *payload) {
    
    CURL *curl = NULL;
    CURLcode resp = CURLE_FAILED_INIT;
    cJSON *json = NULL;
    char *payload_attributes = NULL;
    struct curl_slist *headers = NULL;

    curl = curl_easy_init();
    if(curl == NULL) {
        fprintf(stderr, "Failed to init curl");
        goto cleanup;
    }

    headers = curl_slist_append(headers, "Content-Type: application/json");
    if(headers == NULL) {
        fprintf(stderr, "Failed to set headers");
        goto cleanup;
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    json = cJSON_CreateObject();
    if(json == NULL) {
        fprintf(stderr, "Failed to create cJSON object");
        goto cleanup;
    }

    for(int i = 0 ; i < attribut_count; i++) {
        if(cJSON_AddStringToObject(json, attributes[i].key, attributes[i].value) == NULL) {
            fprintf(stderr, "Failed to add item to json object");
            goto cleanup;
        }
    }

    payload_attributes = cJSON_Print(json);
    if(payload_attributes == NULL) {
        fprintf(stderr, "Failed to allocate char * buffer for payload string");
        goto cleanup;
    }

    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, payload_attributes);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response_to_memory);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)payload);

    
    resp = curl_easy_perform(curl);

    payload->curl_status = resp;


    cleanup:

        if(curl) {
            curl_easy_cleanup(curl);
        }

        if(headers) {
            curl_slist_free_all(headers);
        }

        if(json) {
            cJSON_Delete(json);
        }

        if(payload_attributes) {
            free(payload_attributes);
        }


        return resp;

}

static size_t write_response_to_memory(void *contents, size_t size, size_t nmemb, void *userp) {

size_t realsize = size * nmemb;
    struct memory_payload *mem = (struct memory_payload *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;

}

AUTH_RESULT validate_auth_response(struct memory_payload *payload) {

    cJSON *json = cJSON_Parse(payload->memory);
    cJSON *ok = NULL;
    cJSON *err = NULL;
    AUTH_RESULT result = UNAUTHORIZED;

    if(json == NULL) {
        fprintf(stderr, "Failed to allocate pointer for json deserialized");
        goto cleanup;
    }


    ok = cJSON_GetObjectItem(json, "Ok");
    if(!cJSON_IsBool(ok)) {
        goto cleanup;
    }

    fprintf(stderr, "curl code : %d, ok value : %d", payload->curl_status, ok->valueint);

    if(ok->valueint == 1 && payload->curl_status == CURLE_OK) {
        result = AUTHORIZED;
        goto cleanup;
    }


    err = cJSON_GetObjectItem(json, "Error");
    if(!cJSON_IsString(err)) {
        goto cleanup;
    }

    fprintf(stderr, "Authentication failed : %s", err->valuestring);

    cleanup:

        if(json) {
            cJSON_Delete(json);
        }

        return result;

}