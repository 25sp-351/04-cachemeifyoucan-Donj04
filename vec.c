#include "vec.h"

#include <stdlib.h>
#include <string.h>

#define SIZE_INCREMENT 20

// typedef struct vec {
//     void* base;
//     size_t element_size;
//     size_t allocated;
//     size_t length;
// } *Vec;


Vec new_vec(size_t element_size) {
    Vec v           = malloc(sizeof(struct vec));
    v->element_size = element_size;
    v->base         = NULL;
    v->allocated    = 0;
    v->length       = 0;
    return v;
}

Vec vec_copy(Vec v) {
    Vec nv            = malloc(sizeof(struct vec));
    nv->element_size  = v->element_size;
    nv->allocated     = v->allocated;
    nv->length        = v->length;
    size_t region_len = nv->element_size * nv->allocated;
    nv->base          = malloc(region_len);
    memcpy(nv->base, v->base, region_len);
    return nv;
}

void vec_free(Vec v) {
    if (v->base)
        free(v->base);
    free(v);
}

size_t vec_length(Vec v) {
    return v->length;
}

void vec_add(Vec v, void* item) {
    if (v->base == NULL) {
        v->allocated = SIZE_INCREMENT;
        v->base      = malloc(v->element_size * v->allocated);
        v->length    = 0;
    } else if (v->length == v->allocated) {
        v->allocated += SIZE_INCREMENT;
        v->base = realloc(v->base, v->element_size * v->allocated);
    }
    memcpy(v->base + v->length * v->element_size, item, v->element_size);
    v->length++;
}

void* vec_items(Vec v) {
    // this keeps clients from having to worry about getting a NULL
    if (v->base == NULL) {
        v->allocated = SIZE_INCREMENT;
        v->base      = malloc(v->element_size * v->allocated);
        v->length    = 0;
    }
    return v->base;
}

void* vec_get(Vec v, size_t index) {
    if (index >= vec_length(v))
        return NULL;
    // cast to char* to do pointer arithmetic correctly
    return (char*)vec_items(v) + (index * v->element_size);
}

KeyPair* vec_find_pair(const Vec vector, size_t key) {
    for (size_t ix = 0; ix < vec_length(vector); ix++) {
        KeyPair* pair = vec_get(vector, ix);
        if (pair->key == key)
            return pair;
    }
    return NULL;
}
