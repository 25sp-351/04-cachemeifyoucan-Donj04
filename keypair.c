#include "keypair.h"

#include <stdlib.h>

KeyPair createKeyPair(size_t key, int value) {
    KeyPair new_pair;
    new_pair.key   = key;
    new_pair.value = value;
    return new_pair;
}
