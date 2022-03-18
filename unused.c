#include <pthread.h>
#include <stdint.h>

#include "barrier.h"

/// Sanity check barrier which does nothing to verify tests fail on simple
/// incorrect barriers.
void barrier(const uint32_t p __attribute__((unused)),
             const pthread_t id __attribute__((unused))) {}
