#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "barrier.h"

typedef struct CentralizedGlobalState {
  atomic_ulong counter;
  bool sense;
} State;

/// A centralized barrier.
void barrier(const n_threads_t p, const pthread_t id __attribute__((unused)),
             void *local __attribute__((unused)), void *global) {

  State *state = global;

  const bool local_sense = !state->sense;
  if (atomic_fetch_add(&state->counter, 1) == p) {
    atomic_store(&state->counter, 1);
    state->sense = local_sense;
  } else {
    while (state->sense != local_sense) {
    }
  }
}

void *init_global_barrier_state(const n_threads_t p __attribute__((unused))) {
  State *state = calloc(1, sizeof(State));

  if (!state) {
    fprintf(stderr, "Memory allocation failed!\n");
    exit(1);
  }

  atomic_store(&state->counter, 1);
  return state;
}
void *init_local_barrier_state(const n_threads_t p __attribute__((unused))) {
  return NULL;
}
