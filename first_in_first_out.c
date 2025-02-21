#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"
#include "inputreader.h"

/* First in, first out */

typedef struct node {
    KeyType key;
    ValueType value;
} * CacheNode;

#define MAX_KEY MAX_ROD_LENGTH
#define CACHE_SIZE 10
#define MAP_SIZE MAX_KEY + 1

#define VALUE_NOT_PRESENT 0
#define KEY_NOT_PRESENT -1

CacheNode queue[CACHE_SIZE];
int key_map[MAP_SIZE];  // maps the real key to an index in the queue

size_t q_tail        = 0;  // idx to insert at

bool show_debug_info = false;

int cache_requests;
int cache_hits;
int cache_misses;

ProviderFunction _downstream = NULL;


CacheNode node_new(KeyType key, ValueType val) {
    CacheNode n_node = malloc(sizeof(struct node));
    n_node->key      = key;
    n_node->value    = val;
    return n_node;
}

void node_free(CacheNode c_node) {
    if (c_node->value)
        free(c_node->value);
    free(c_node);
}

void initialize(void) {
    if (show_debug_info)
        fprintf(stderr, __FILE__ " initialize()\n");

    cache_requests = 0;
    cache_hits     = 0;
    cache_misses   = 0;

    for (int ix = 0; ix < CACHE_SIZE; ix++)
        queue[ix] = NULL;
    for (int iy = 0; iy < MAP_SIZE; iy++)
        key_map[iy] = KEY_NOT_PRESENT;
}

void cleanup(void) {
    if (show_debug_info)
        fprintf(stderr, __FILE__ " cleanup(): ");

    for (size_t ix = 0; ix < CACHE_SIZE; ix++)
        if (queue[ix] != NULL) {
            if (show_debug_info)
                printf(KEY_FMT " ", queue[ix]->key);
            node_free(queue[ix]);
        }

    if (show_debug_info)
        printf("freed\n");
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

bool _is_present(KeyType key) {
    bool present = key <= MAX_KEY && key_map[key] != KEY_NOT_PRESENT;

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

    if (queue[q_tail] != NULL) {
        CacheNode old_node = queue[q_tail];
        KeyType old_key    = old_node->key;

        key_map[old_key]   = KEY_NOT_PRESENT;
        node_free(old_node);

        if (show_debug_info)
            fprintf(stderr, ": removed key " KEY_FMT, old_key);
    }

    if (show_debug_info)
        fprintf(stderr, "\n");

    queue[q_tail] = node_new(key, value);
    key_map[key]  = q_tail;
    q_tail        = (q_tail + 1) % CACHE_SIZE;
}

ValueType _get(KeyType key) {
    if (key > MAX_KEY || key_map[key] == KEY_NOT_PRESENT)
        return VALUE_NOT_PRESENT;

    size_t map_idx   = key_map[key];
    ValueType result = queue[map_idx]->value;

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
