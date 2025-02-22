#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"
#include "inputreader.h"

/* Least recently used */

typedef struct {
    int cache_key;
    // increases for each cache access, resets when this node is accessed
    unsigned int time_since_access;
} MapNode;

#define MAX_KEY MAX_ROD_LENGTH
#define CACHE_SIZE 10
#define MAP_SIZE MAX_KEY + 1

#define VALUE_NOT_PRESENT NULL
#define KEY_NOT_PRESENT -1
#define MAX_TIME UINT_MAX

ValueType cache[CACHE_SIZE];
MapNode key_map[MAP_SIZE];  // maps the real key to an index in the cache, and
                            // keeps track of when it was last accessed

int saved_values     = 0;

bool show_debug_info = true;

int cache_requests;
int cache_hits;
int cache_misses;

ProviderFunction _downstream = NULL;

MapNode new_node(int cache_key, unsigned int time) {
    MapNode node;
    node.cache_key         = cache_key;
    node.time_since_access = time;
    return node;
}

void initialize(void) {
    if (show_debug_info)
        fprintf(stderr, __FILE__ " initialize()\n");

    cache_requests = 0;
    cache_hits     = 0;
    cache_misses   = 0;

    for (int ix = 0; ix < CACHE_SIZE; ix++)
        cache[ix] = NULL;

    for (int iy = 0; iy < MAP_SIZE; iy++)
        key_map[iy] = new_node(KEY_NOT_PRESENT, 0);
}

void cleanup(void) {
    if (show_debug_info)
        fprintf(stderr, __FILE__ " cleanup()\n");

    for (size_t ix = 0; ix < CACHE_SIZE; ix++)
        if (cache[ix] != NULL)
            free(cache[ix]);
}

void reset_statistics(void) {
    if (show_debug_info)
        fprintf(stderr, __FILE__ " reset_statistics()\n");
    cache_requests = 0;
    cache_hits     = 0;
    cache_misses   = 0;
}

CacheStat* statistics(void) {
    if (show_debug_info)
        fprintf(stderr, __FILE__ " statistics()\n");

    CacheStat* stats_cache = malloc(4 * sizeof(CacheStat));
    stats_cache[0]         = (CacheStat){Cache_requests, cache_requests};
    stats_cache[1]         = (CacheStat){Cache_hits, cache_hits};
    stats_cache[2]         = (CacheStat){Cache_misses, cache_misses};
    stats_cache[3]         = (CacheStat){Cache_size, CACHE_SIZE};
    stats_cache[4]         = (CacheStat){END_OF_STATS, 0};

    return stats_cache;
}

// find least recently used
// greatest time = least recent
KeyType _earliest_accessed_key() {
    KeyType key           = 0;
    unsigned int time_max = 0;

    for (size_t iy = 0; iy < MAP_SIZE; iy++) {
        MapNode node = key_map[iy];

        if ((node.cache_key != KEY_NOT_PRESENT) &&
            (node.time_since_access > time_max)) {
            key      = iy;
            time_max = node.time_since_access;
        }
    }
    return key;
}

void _update_times(KeyType ignore) {
    key_map[ignore].time_since_access = 0;

    if (show_debug_info)
        fprintf(stderr, __FILE__ " update_times():\n");

    for (size_t ix = 0; ix < MAP_SIZE; ix++) {
        if (ix == ignore || key_map[ix].cache_key == KEY_NOT_PRESENT)
            continue;

        unsigned int old_time = key_map[ix].time_since_access++;

        fprintf(stderr, KEY_FMT ": %u -> %u\n", ix, old_time,
                key_map[ix].time_since_access);
    }

    fprintf(stderr, "reset time for " KEY_FMT "\n\n", ignore);
}

bool _is_present(KeyType key) {
    bool present =
        (key <= MAX_KEY) && (key_map[key].cache_key != KEY_NOT_PRESENT);

    if (show_debug_info)
        fprintf(stderr, __FILE__ " is_present(" KEY_FMT ") = %s\n", key,
                present ? "true" : "false");

    return present;
}

void _insert(KeyType key, ValueType value) {
    if (key > MAX_KEY)
        return;

    if (show_debug_info)
        fprintf(stderr, __FILE__ " insert(" KEY_FMT ")", key);

    bool space_available = saved_values < CACHE_SIZE;

    int insert_cache_idx = 0;
    KeyType old_key      = 0;

    if (space_available)
        insert_cache_idx = saved_values;  // insert at end of used entries
    else {
        // find least recently used key, and get the cache index it's mapped to
        old_key          = _earliest_accessed_key();
        insert_cache_idx = key_map[old_key].cache_key;
    }

    if (cache[insert_cache_idx]) {
        free(cache[insert_cache_idx]);

        key_map[old_key] = new_node(KEY_NOT_PRESENT, 0);

        if (show_debug_info)
            fprintf(stderr, ": removed key " KEY_FMT, old_key);
    }

    if (show_debug_info)
        fprintf(stderr, "\n");

    // insert element
    cache[insert_cache_idx] = value;
    key_map[key]            = new_node(insert_cache_idx, 0);

    _update_times(key);

    if (space_available)
        saved_values++;
}

ValueType _get(KeyType key) {
    if ((key > MAX_KEY) || (key_map[key].cache_key == KEY_NOT_PRESENT))
        return VALUE_NOT_PRESENT;

    int cache_key    = key_map[key].cache_key;
    ValueType result = cache[cache_key];

    _update_times(key);

    if (show_debug_info)
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
    if (show_debug_info)
        fprintf(stderr, __FILE__ " set_provider()\n");
    _downstream = downstream;
    return _caching_provider;
}
