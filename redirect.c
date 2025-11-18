#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


static char real_xwechat_config_path[1024] = {0};
static size_t real_xwechat_config_path_len = 0;

static const char *DEFAULT_VAL = "MyDocument:";
static const char *NEW_VAL = "MyDocument:.var/app/com.tencent.WeChat";

static FILE *(*real_fopen)(const char *, const char *) = NULL;

__attribute__((constructor))
static void init_hook() {
    real_fopen = dlsym(RTLD_NEXT, "fopen");

    const char *home = getenv("HOME");
    if (!home) home = "/root";

    snprintf(real_xwechat_config_path, sizeof(real_xwechat_config_path), "%s/.xwechat/config/", home);
    real_xwechat_config_path_len = strlen(real_xwechat_config_path);

    mkdir(real_xwechat_config_path, 0755);
}

static bool is_xwechat_config_file(const char *path) {
    if (!path || strncmp(path, real_xwechat_config_path, real_xwechat_config_path_len) != 0) {
        return false;
    }

    const char *dot = strrchr(path, '.');
    if(!dot || strcmp(dot, ".ini") != 0) {
        return false;
    }

    return true;
}

static bool is_default_config(const char *path) {
    struct stat st;
    if (access(path, F_OK) != 0) return true;
    if (stat(path, &st) != 0) return false;

    size_t default_len = strlen(DEFAULT_VAL);
    if ((size_t)st.st_size != default_len) return false;

    FILE *fp = real_fopen(path, "r");
    if (!fp) return false;

    char buf[64];
    size_t n = fread(buf, 1, default_len, fp);
    fclose(fp);

    return (n == default_len && memcmp(buf, DEFAULT_VAL, default_len) == 0);
}

static void rewrite_default_config(const char *path) {
    char tmp_path[1024];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path);

    FILE *fp = real_fopen(tmp_path, "w");
    if (!fp) return;
    fputs(NEW_VAL, fp);
    fclose(fp);

    if (rename(tmp_path, path) != 0) {
        unlink(tmp_path);
    }
}

FILE *fopen(const char *path, const char *mode) {
    if (!real_fopen) {
        real_fopen = dlsym(RTLD_NEXT, "fopen");
    }

    if (strchr(mode, 'r') && is_xwechat_config_file(path)) {
        if (is_default_config(path)) {
            printf("[HOOK] Rewriting new default config: %s\n", path);
            rewrite_default_config(path);
        }
    }

    return real_fopen(path, mode);
}
