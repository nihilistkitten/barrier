#include <pthread.h>
#include <stdint.h>

#include "barrier.h"

/// Sanity check barrier which does nothing to verify tests fail on simple
/// incorrect barriers.
void barrier(const n_threads_t p __attribute__((unused)),
             const pthread_t id __attribute__((unused)),
             void *local __attribute__((unused)),
             void *global __attribute__((unused))) {}

void *init_global_barrier_state(const n_threads_t p __attribute__((unused))) {
  return NULL;
}
void *init_local_barrier_state(const n_threads_t p __attribute__((unused))) {
  return NULL;
}
