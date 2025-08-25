#include "cjson/cJSON.h"
#include "curl/curl.h"


struct broker_settings {
    char *protocol;
    char *host;
    char *port;
    char *endpoint_auth;
    char *endpoint_acls;
};

struct payload_attributes {
    char *key;
    char *value;
};

struct memory_payload {
    char *memory;
    size_t size;
    CURLcode curl_status;
};


typedef enum {
    UNAUTHORIZED = 0,
    AUTHORIZED

} AUTH_RESULT;


void free_broker_settings_struct(struct broker_settings* data);

CURLcode call_api(char *url, struct payload_attributes *attributes, int attribut_count, struct memory_payload *payload);

static size_t write_response_to_memory(void *contents, size_t size, size_t nmemb, void *userp);

AUTH_RESULT validate_auth_response(struct memory_payload *payload);