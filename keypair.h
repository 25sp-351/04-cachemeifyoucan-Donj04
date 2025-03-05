#ifndef KEYPAIR_H
#define KEYPAIR_H

#include <stdlib.h>

typedef struct {
    size_t key;
    int value;
} KeyPair;


KeyPair createKeyPair(size_t key, int value);

#endif
