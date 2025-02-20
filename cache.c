#include "cache.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void _do_nothing(void) {
}

CacheStat *_do_nothing_stats(void) {
    return NULL;
}

Cache *load_cache_module(const char *libname) {
    void *handle = dlopen(libname, RTLD_NOW | RTLD_NODELETE);
    if (!handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return NULL;
    }

    Cache *hooks               = malloc(sizeof(Cache));

    Void_fptr cache_initialize = (Void_fptr)dlsym(handle, "initialize");
    hooks->set_provider_func = (SetProvider_fptr)dlsym(handle, "set_provider");
    hooks->get_statistics    = (Stats_fptr)dlsym(handle, "statistics");
    hooks->reset_statistics  = (Void_fptr)dlsym(handle, "reset_statistics");
    hooks->cache_cleanup     = (Void_fptr)dlsym(handle, "cleanup");

    dlclose(handle);

    if (!hooks->get_statistics)
        hooks->get_statistics = _do_nothing_stats;
    if (!hooks->reset_statistics)
        hooks->reset_statistics = _do_nothing;
    if (!hooks->cache_cleanup)
        hooks->cache_cleanup = _do_nothing;

    // Only require the required one
    if (!hooks->set_provider_func) {
        fprintf(stderr, "Error: could not resolve required symbol: (%p)\n",
                (void *)hooks->set_provider_func);
        free(hooks);
        hooks = NULL;
    }

    if (cache_initialize)
        cache_initialize();

    return hooks;
}

void print_cache_stats(int fd, CacheStat *stats) {
    if (!stats) {
        dprintf(fd, "No cache stats available\n");
        return;
    }

    printf("Cache Stats:\n");

    CacheStat *sptr = stats;
    while (sptr->type != END_OF_STATS) {
        dprintf(fd, "%-10s (%d) %4d\n", CacheStatNames[sptr->type], sptr->type,
                sptr->value);
        sptr++;
    }
}
