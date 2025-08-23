#include "cjson/cJSON.h"
#include "curl/curl.h"


struct broker_settings {
    char *protocol;
    char *host;
    char *port;
    char *endpoint_auth;
    char *endpoint_acls;
};


void free_broker_settings_struct(struct broker_settings* data);

void cleanup(CURL *curl, cJSON *json, char *payload, char *url);