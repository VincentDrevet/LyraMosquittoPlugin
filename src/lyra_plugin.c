//#include "lyra_plugin.h"
#include "mosquitto_plugin.h"
#include "stdio.h"
#include "curl/curl.h"
#include "string.h"
#include "stdlib.h"
#include "lyra_plugin.h"
#include "cjson/cJSON.h"

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
    CURL *curl;
    CURLcode response;

    struct mosquitto_evt_basic_auth *data = (struct mosquitto_evt_basic_auth *) event_data;
    struct broker_settings *settings = (struct broker_settings *) user_data;
    
    // get size of protocol
    int url_len = strlen(settings->protocol) + 3 + strlen(settings->host) + 1 + strlen(settings->port) + strlen(settings->endpoint_auth) + 1;

    char * url = malloc(url_len);

    // construct url
    snprintf(url, url_len, "%s://%s:%s%s",
        settings->protocol,
        settings->host,
        settings->port,
        settings->endpoint_auth);

    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 12L);


    // construct json body for post request

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "client_id", mosquitto_client_id(data->client));
    cJSON_AddStringToObject(json, "username", data->username);
    cJSON_AddStringToObject(json, "password", data->password);

    char *payload = cJSON_Print(json);

    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, payload);

    response = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return 1;

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

void free_broker_settings_struct(struct broker_settings* data)
{
    fprintf(stdout, "Release malloc ressources.");
    free(data->host);
    free(data);
}