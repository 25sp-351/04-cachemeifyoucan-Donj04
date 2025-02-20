#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"

/* Simple memoizer */

#define MAX_KEY 1000
#define CACHE_SIZE MAX_KEY + 1
ValueType cache[CACHE_SIZE];

#define VALUE_NOT_PRESENT 0

int cache_requests;
int cache_hits;
int cache_misses;

ProviderFunction _downstream = NULL;


void initialize(void) {
    fprintf(stderr, __FILE__ " initialize()\n");

    cache_requests = 0;
    cache_hits     = 0;
    cache_misses   = 0;

    for (int ix = 0; ix < CACHE_SIZE; ix++)
        cache[ix] = VALUE_NOT_PRESENT;
}


void cleanup(void) {
    fprintf(stderr, __FILE__ " cleanup()\n");
    printf("free: ");
    for (size_t ix = 0; ix < CACHE_SIZE; ix++)
        if (cache[ix] != NULL) {
            printf("%2zu ", ix);
            free(cache[ix]);
        }
    printf("\n");
}


void reset_statistics(void) {
    fprintf(stderr, __FILE__ " reset_statistics()\n");
    cache_requests = 0;
    cache_hits     = 0;
    cache_misses   = 0;
}


CacheStat* statistics(void) {
    fprintf(stderr, __FILE__ " statistics()\n");

    CacheStat* stats_cache = malloc(4 * sizeof(CacheStat));
    stats_cache[0]         = (CacheStat){Cache_requests, cache_requests};
    stats_cache[1]         = (CacheStat){Cache_hits, cache_hits};
    stats_cache[2]         = (CacheStat){Cache_misses, cache_misses};
    stats_cache[3]         = (CacheStat){Cache_size, CACHE_SIZE};
    stats_cache[4]         = (CacheStat){END_OF_STATS, 0};

    return stats_cache;
}

bool _is_present(KeyType key) {
    bool present =
        key >= 0 && key <= MAX_KEY && cache[key] != VALUE_NOT_PRESENT;

    fprintf(stderr, __FILE__ " is_present(" KEY_FMT ") = %s\n", key,
            present ? "true" : "false");
    return present;
}

void _insert(KeyType key, ValueType value) {
    if (key < 0 || key > MAX_KEY)
        return;

    fprintf(stderr, __FILE__ " insert(" KEY_FMT "):\n" VALUE_FMT "\n", key,
            value);
    cache[key] = value;
}

ValueType _get(KeyType key) {
    if (key < 0 || key > MAX_KEY)
        return VALUE_NOT_PRESENT;

    ValueType result = cache[key];
    fprintf(stderr, __FILE__ " get(" KEY_FMT "):\n" VALUE_FMT "\n", key,
            result);
    return result;
}

// used externally but not referenced externally --
// only by the set_provider function
ValueType _caching_provider(Vec lengths, KeyType key) {
    cache_requests++;

    if (_is_present(key)) {
        cache_hits++;
        return _get(key);
    } else
        cache_misses++;

    ValueType result = (*_downstream)(lengths, key);
    _insert(key, result);

    return result;
}

ProviderFunction set_provider(ProviderFunction downstream) {
    fprintf(stderr, __FILE__ " set_provider()\n");
    _downstream = downstream;
    return _caching_provider;
}
