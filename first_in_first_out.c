#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"

/* First in, first out */

typedef struct node {
    KeyType key;
    ValueType value;
} * FIFOnode;

#define MAX_KEY 100000
#define CACHE_SIZE 50
#define MAP_SIZE MAX_KEY + 1

#define VALUE_NOT_PRESENT NULL
#define KEY_NOT_PRESENT -1

FIFOnode cache[CACHE_SIZE];  // circular array acting as a queue

int key_map[MAP_SIZE];  // list of cache indexes
                        // maps the real key to an index in the cache

size_t q_tail = 0;  // queue tail, index to insert at

int cache_requests;
int cache_hits;
int cache_misses;

ProviderFunction _downstream = NULL;


FIFOnode node_new(KeyType key, ValueType val) {
    FIFOnode n_node = malloc(sizeof(struct node));
    n_node->key     = key;
    n_node->value   = val;
    return n_node;
}


void node_free(FIFOnode c_node) {
    if (c_node->value)
        free(c_node->value);
    free(c_node);
}


void initialize(void) {
    DEBUG_PRINT(__FILE__ " initialize()\n");

    cache_requests = 0;
    cache_hits     = 0;
    cache_misses   = 0;

    for (int ix = 0; ix < CACHE_SIZE; ix++)
        cache[ix] = NULL;

    for (int iy = 0; iy < MAP_SIZE; iy++)
        key_map[iy] = KEY_NOT_PRESENT;
}


void cleanup(void) {
    DEBUG_PRINT(__FILE__ " cleanup(): ");

    for (size_t ix = 0; ix < CACHE_SIZE; ix++)
        if (cache[ix] != NULL) {
            DEBUG_PRINT(KEY_FMT " ", cache[ix]->key);
            node_free(cache[ix]);
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


// print every cached key, and show where the tail currently is
void print_cache() {
    #ifdef DEBUG
    DEBUG_PRINT(__FILE__ " print_cache()\n");

    for (size_t ix = 0; ix < CACHE_SIZE; ix++) {
        if (cache[ix])
            DEBUG_PRINT(KEY_FMT, cache[ix]->key);
        else if (ix == q_tail)
            DEBUG_PRINT("null");

        if (ix == q_tail)
            DEBUG_PRINT(" < tail");

        if (cache[ix] != NULL || ix == q_tail)
            DEBUG_PRINT("\n");
    }
    DEBUG_PRINT("\n");
    #endif
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

    if (cache[q_tail] != NULL) {
        FIFOnode old_node = cache[q_tail];
        KeyType old_key   = old_node->key;

        key_map[old_key]  = KEY_NOT_PRESENT;
        node_free(old_node);

        DEBUG_PRINT(": evict key " KEY_FMT, old_key);
    }
    DEBUG_PRINT("\n");

    cache[q_tail] = node_new(key, value);
    key_map[key]  = q_tail;
    q_tail        = (q_tail + 1) % CACHE_SIZE;

    print_cache();  // for debugging
}


ValueType _get(KeyType key) {
    if (key > MAX_KEY || key_map[key] == KEY_NOT_PRESENT)
        return VALUE_NOT_PRESENT;

    size_t map_idx   = key_map[key];
    ValueType result = cache[map_idx]->value;

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
