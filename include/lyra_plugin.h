


struct broker_settings {
    char *protocol;
    char *host;
    char *port;
    char *endpoint_auth;
    char *endpoint_acls;
};


void free_broker_settings_struct(struct broker_settings* data);