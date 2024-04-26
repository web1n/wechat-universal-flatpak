#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


void *(*real_dlopen)(const char *, int);
FILE *(*real_fopen64)(const char *, const char *);

void hook_path(const char *func, const char **path) {
    char *redirect = NULL;

    if (strcmp(*path, "/usr/lib/libactivation.so") == 0) {
        redirect = "/app/lib/libactivation.so";
    } else if (strcmp(*path, "/etc/lsb-release") == 0) {
        redirect = "/app/etc/lsb-release-ukui";
    }

    if (redirect != NULL) {
        printf("%s: redirect from %s to: %s\n", func, *path, redirect);
        *path = redirect;
    }
}

void *dlopen(const char *__file, int __mode) {
    if (real_dlopen == NULL) {
        real_dlopen = dlsym(RTLD_NEXT, "dlopen");
    }

    if (__file != NULL) {
        hook_path("dlopen", &__file);
    }

    return (*real_dlopen)(__file, __mode);
}

FILE *fopen64(const char *__file, const char *__mode) {
    if (real_fopen64 == NULL) {
        real_fopen64 = dlsym(RTLD_NEXT, "fopen64");
    }

    if (__file != NULL) {
        hook_path("fopen64", &__file);
    }

    return (*real_fopen64)(__file, __mode);
}
