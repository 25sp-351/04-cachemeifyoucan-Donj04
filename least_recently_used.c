#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"

/* Least recently used */

typedef unsigned int TimeType;

typedef struct node {
    KeyType key;
    ValueType value;
    TimeType time_since_access;
} * LRUnode;

#define MAX_KEY 100000
#define CACHE_SIZE 50
#define MAP_SIZE MAX_KEY + 1

#define VALUE_NOT_PRESENT NULL
#define KEY_NOT_PRESENT -1
#define MAX_TIME UINT_MAX

LRUnode cache[CACHE_SIZE];
int key_map[MAP_SIZE];  // list of cache indexes
                        // maps the real key to an index in the cache

KeyType key_to_replace = 0;  // least recently used key

size_t saved_values    = 0;

int cache_requests;
int cache_hits;
int cache_misses;

ProviderFunction _downstream = NULL;


LRUnode node_new(KeyType key, ValueType val) {
    LRUnode node            = malloc(sizeof(struct node));
    node->key               = key;
    node->value             = val;
    node->time_since_access = 0;
    return node;
}


void node_free(LRUnode node) {
    key_map[node->key] = KEY_NOT_PRESENT;
    if (node->value)
        free(node->value);
    free(node);
}


void initialize(void) {
    DEBUG_PRINT(__FILE__ " initialize()\n");

    cache_requests = 0;
    cache_hits     = 0;
    cache_misses   = 0;

    for (size_t ix = 0; ix < CACHE_SIZE; ix++)
        cache[ix] = NULL;

    for (KeyType iy = 0; iy < MAP_SIZE; iy++)
        key_map[iy] = KEY_NOT_PRESENT;
}


void cleanup(void) {
    DEBUG_PRINT(__FILE__ " cleanup(): ");

    for (size_t ix = 0; ix < CACHE_SIZE; ix++) {
        if (cache[ix] != NULL) {
            DEBUG_PRINT(KEY_FMT " ", cache[ix]->key);
            node_free(cache[ix]);
        }
    }

    DEBUG_PRINT("freed\n");
}


void reset_statistics(void) {
    DEBUG_PRINT(__FILE__ " reset_statistics()\n");
    cache_requests = 0;
    cache_hits     = 0;
    cache_misses   = 0;
}


CacheStat* statistics(void) {
    DEBUG_PRINT(__FILE__ " statistics()\n");

    CacheStat* stats_cache = malloc(4 * sizeof(CacheStat));
    stats_cache[0]         = (CacheStat){Cache_requests, cache_requests};
    stats_cache[1]         = (CacheStat){Cache_hits, cache_hits};
    stats_cache[2]         = (CacheStat){Cache_misses, cache_misses};
    stats_cache[3]         = (CacheStat){Cache_size, CACHE_SIZE};
    stats_cache[4]         = (CacheStat){END_OF_STATS, 0};

    return stats_cache;
}


// Increment the times for each node in the cache
// Takes the most recently accessed key and resets its time to 0
// Also updates the least recently used key
void _update_times(KeyType last_used) {
    DEBUG_PRINT(__FILE__ " update_times():\n");

    KeyType new_replace = 0;
    TimeType oldest     = 0;

    for (size_t ix = 0; ix < CACHE_SIZE; ix++) {
        LRUnode node = cache[ix];

        if (node == NULL)
            continue;

        TimeType* time    = &(node->time_since_access);
        #ifdef DEBUG
        TimeType old_time = *time;
        #endif

        if (node->key == last_used) {
            *time = 0;  // reset time in node
            DEBUG_PRINT(">");

        } else {
            if (*time < MAX_TIME)  // increment time in node
                (*time)++;

            if (*time > oldest) {
                new_replace = node->key;
                oldest      = *time;
            }
            DEBUG_PRINT(" ");
        }
        DEBUG_PRINT(KEY_FMT ": %u -> %u\n", node->key, old_time, *time);
    }
    DEBUG_PRINT("\n");

    key_to_replace = new_replace;
}


bool _is_present(KeyType key) {
    bool present = key <= MAX_KEY && key_map[key] != KEY_NOT_PRESENT;

    DEBUG_PRINT(__FILE__ " is_present(" KEY_FMT ") = %s\n", key,
                present ? "true" : "false");

    return present;
}


void _insert(KeyType key, ValueType value) {
    if (key > MAX_KEY)
        return;

    DEBUG_PRINT(__FILE__ " insert(" KEY_FMT ")", key);

    bool space_available = saved_values < CACHE_SIZE;

    size_t insert_idx    = 0;

    // if not full, insert at end of used entries, else replace least recently
    // used
    if (space_available)
        insert_idx = saved_values;
    else if (key_map[key_to_replace] >= 0)
        insert_idx = key_map[key_to_replace];

    if (cache[insert_idx] != NULL) {
        DEBUG_PRINT(": evict key " KEY_FMT, key_to_replace);
        node_free(cache[insert_idx]);
    }
    DEBUG_PRINT("\n");

    // insert element
    cache[insert_idx] = node_new(key, value);
    key_map[key]      = insert_idx;

    if (space_available)
        saved_values++;

    _update_times(key);
}


ValueType _get(KeyType key) {
    if (key > MAX_KEY || key_map[key] == KEY_NOT_PRESENT)
        return VALUE_NOT_PRESENT;

    // map key to a cache index, get value from node in that index
    ValueType result = cache[key_map[key]]->value;

    _update_times(key);

    DEBUG_PRINT(__FILE__ " get(" KEY_FMT ")\n", key);

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
    DEBUG_PRINT(__FILE__ " set_provider()\n");
    _downstream = downstream;
    return _caching_provider;
}
