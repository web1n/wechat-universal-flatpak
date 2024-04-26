#include <stdio.h>
#include <string.h>


char *get_value(const char *func, const char *key) {
    char *value;

    FILE *fp = fopen("/app/etc/.kyact", "r");
    char buffer[50];
    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(key, strtok(buffer, "=")) == 0) {
            value = strtok(NULL, "=");
            break;
        }
    }
    fclose(fp);

    printf("%s: %s\n", func, value);
    return strdup(value);
}

char *activation_get_harddisk_id() {
    return get_value("activation_get_harddisk_id", "harddisk");
}

char *activation_get_interface_mac() {
    return get_value("activation_get_interface_mac", "netmac");
}

char *activation_new_register_number() {
    return get_value("activation_new_register_number", "registernum");
}

char *activation_get_system_uuid() {
    return get_value("activation_get_system_uuid", "systemuuid");
}

char *activation_get_serial_number() {
    return get_value("activation_get_serial_number", "serialnum");
}

int activation_status_code() {
    printf("activation_status_code\n");
    return 1;
}
