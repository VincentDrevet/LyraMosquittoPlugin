# Library required

- cjson
- libcurl


# Options to provide in mosquitto configuration file

```toml
plugin <path to plugin.so>

# Plugin parameter to set the API host
plugin_opt_host <host>

# Plugin parameter to set the API host
plugin_opt_port <port>

# Plugin parameter to set the API protocol
plugin_opt_port <procotol>

# Plugin parameter to set the API endpoint for device authentication
plugin_opt_endpoint_auth <endpoint_auth>

# Plugin parameter to set the API endpoint for device acl check
plugin_opt_endpoint_acls <endpoint_acls>
```

Example of valid configuration :

```toml
plugin <path to plugin.so>

# Plugin parameter to set the API host
plugin_opt_host lyra-api

# Plugin parameter to set the API host
plugin_opt_port 8000

# Plugin parameter to set the API protocol
plugin_opt_port http

# Plugin parameter to set the API endpoint for device authentication
plugin_opt_endpoint_auth /broker/device/auth

# Plugin parameter to set the API endpoint for device acl check
plugin_opt_endpoint_acls /broker/device/acls

```