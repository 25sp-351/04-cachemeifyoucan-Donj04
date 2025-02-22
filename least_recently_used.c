#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"
#include "inputreader.h"

/* Least recently used */

typedef struct mapnode {
    int cache_key;
    // increases for each cache access, resets when this node is accessed
    unsigned int time_since_access;
} * MapNode;

#define MAX_KEY MAX_ROD_LENGTH
#define CACHE_SIZE 50
#define MAP_SIZE MAX_KEY + 1

#define VALUE_NOT_PRESENT NULL
#define MAX_TIME UINT_MAX

ValueType cache[CACHE_SIZE];
MapNode key_map[MAP_SIZE];  // maps the real key to an index in the cache, and
                            // keeps track of when it was last accessed

KeyType key_to_replace = 0;  // least recently used key

int saved_values       = 0;

bool show_debug_info   = true;

int cache_requests;
int cache_hits;
int cache_misses;

ProviderFunction _downstream = NULL;

MapNode node_new(int cache_key, unsigned int time) {
    MapNode node            = malloc(sizeof(struct mapnode));
    node->cache_key         = cache_key;
    node->time_since_access = time;
    return node;
}

// make sure the cache entry this points to is also taken care of
void node_free(KeyType key) {
    if (key_map[key]) {
        free(key_map[key]);
        key_map[key] = NULL;
    }
}

// make sure the key pointing to this entry is also taken care of
void cache_entry_free(size_t idx) {
    if (cache[idx]) {
        free(cache[idx]);
        cache[idx] = NULL;
    }
}

void initialize(void) {
    if (show_debug_info)
        fprintf(stderr, __FILE__ " initialize()\n");

    cache_requests = 0;
    cache_hits     = 0;
    cache_misses   = 0;

    for (size_t ix = 0; ix < CACHE_SIZE; ix++)
        cache[ix] = NULL;

    for (KeyType iy = 0; iy < MAP_SIZE; iy++)
        key_map[iy] = NULL;
}

void cleanup(void) {
    if (show_debug_info)
        fprintf(stderr, __FILE__ " cleanup(): ");

    for (size_t ix = 0; ix < CACHE_SIZE; ix++)
        cache_entry_free(ix);

    for (KeyType iy = 0; iy < MAP_SIZE; iy++) {
        if (show_debug_info && key_map[iy] != NULL)
            fprintf(stderr, KEY_FMT " ", iy);
        node_free(iy);
    }

    if (show_debug_info)
        fprintf(stderr, "freed\n");
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

// also finds the least recently used key
void _update_times(KeyType reset_key) {
    if (show_debug_info)
        fprintf(stderr, __FILE__ " update_times():\n");

    KeyType new_replace   = 0;
    unsigned int time_max = 0;

    for (KeyType ix = 0; ix < MAP_SIZE; ix++) {
        MapNode node = key_map[ix];

        if (node == NULL)
            continue;

        unsigned int* time    = &(node->time_since_access);
        unsigned int old_time = *time;

        if (ix == reset_key) {
            *time = 0;
            if (show_debug_info)
                fprintf(stderr, "*");
        } else {
            if (*time < MAX_TIME)
                (*time)++;

            if (*time > time_max) {
                new_replace = ix;
                time_max    = *time;
            }
            if (show_debug_info)
                fprintf(stderr, " ");
        }
        fprintf(stderr, KEY_FMT ": %u -> %u\n", ix, old_time, *time);
    }

    if (new_replace > 0)
        key_to_replace = new_replace;
}

bool _is_present(KeyType key) {
    bool present = key <= MAX_KEY && key_map[key] != NULL;

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

    if (space_available)
        insert_cache_idx = saved_values;  // insert at end of used entries
    else {
        // get least recently used key, and get the cache index it's mapped to
        insert_cache_idx = key_map[key_to_replace]->cache_key;
    }

    cache_entry_free(insert_cache_idx);
    if (key_to_replace != 0) {
        node_free(key_to_replace);

        if (show_debug_info)
            fprintf(stderr, ": remove key " KEY_FMT, key_to_replace);
    }

    if (show_debug_info)
        fprintf(stderr, "\n");

    // insert element
    cache[insert_cache_idx] = value;
    key_map[key]            = node_new(insert_cache_idx, 0);

    if (space_available)
        saved_values++;

    _update_times(key);
}

ValueType _get(KeyType key) {
    if (key > MAX_KEY || key_map[key] == NULL)
        return VALUE_NOT_PRESENT;

    int cache_key    = key_map[key]->cache_key;
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
