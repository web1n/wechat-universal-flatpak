#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


static char real_xwechat_config_path[1024] = {0};

static const char *default_config_value = "MyDocument:";
static const char *new_config_value = "MyDocument:.var/app/com.tencent.WeChat";

FILE *(*real_fopen)(char const *, char const *);


void init_target_dir() {
    if (real_xwechat_config_path[0] != '\0') return;
    const char *home = getenv("HOME");
    if (!home) home = "/root";
    snprintf(real_xwechat_config_path, sizeof(real_xwechat_config_path), "%s/.xwechat/config/", home);

    mkdir(real_xwechat_config_path, 0755);
}

bool is_xwechat_config_file(const char *path) {
    init_target_dir();

    if (strncmp(path, real_xwechat_config_path, strlen(real_xwechat_config_path)) != 0) {
        return false;
    }

    const char *dot = strrchr(path, '.');
    if(!dot || strcmp(dot, ".ini") != 0) {
        return false;
    }

    return true;
}

bool is_default_config(const char *path) {
    FILE *fp = real_fopen(path, "rb");
    if (!fp) return false;

    struct stat st;
    if (fstat(fileno(fp), &st) != 0) {
        fclose(fp);
        return false;
    }

    size_t expected = strlen(default_config_value);
    if ((size_t)st.st_size != expected) {
        fclose(fp);
        return false;
    }

    char *buf = malloc(expected + 1);
    if (!buf) {
        fclose(fp);
        return false;
    }

    size_t n = fread(buf, 1, expected, fp);
    fclose(fp);

    if (n != expected) {
        free(buf);
        return false;
    }

    bool match = (memcmp(buf, default_config_value, expected) == 0);
    free(buf);
    return match;
}

void rewrite_default_config(const char *path) {
    FILE *fp = real_fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "rewrite_default_config: failed to open %s for writing\n", path);
        return;
    }

    size_t written = fwrite(new_config_value, 1, strlen(new_config_value), fp);
    if (written != strlen(new_config_value)) {
        fprintf(stderr, "rewrite_default_config: failed to write full content\n");
    }

    fclose(fp);
}

FILE *fopen(const char *path, const char *mode) {
    if (!real_fopen) {
        real_fopen = dlsym(RTLD_NEXT, "fopen");
    }
    if (!strchr(mode, 'r') || !is_xwechat_config_file(path)) {
        return real_fopen(path, mode);
    }

    FILE* fp = real_fopen(path, mode);
    if (!fp) {
        if (access(path, F_OK) != 0) {
            printf("[HOOK] Config file does not exist, generate new: %s\n", path);

            rewrite_default_config(path);
            return real_fopen(path, mode);
        }

        return fp;
    }

    if (is_default_config(path)) {
        printf("[HOOK] Rewriting new default config: %s\n", path);
        fclose(fp);
        rewrite_default_config(path);
        return real_fopen(path, mode);
    }

    return fp;
}
