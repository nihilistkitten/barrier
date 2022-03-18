#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "barrier.h"
#include "utils.h"

typedef uint32_t round_t;

typedef struct GlobalState {
  bool ***flags;
  n_threads_t p; // stored for destructor
} GlobalState;

typedef struct LocalState {
  bool sense;
  int parity;
} LocalState;

n_threads_t num_rounds(n_threads_t p) { return ceil(log2(p)); }

void barrier(const n_threads_t p, const pthread_t id __attribute__((unused)),
             void *local_state, void *global_state) {
  round_t round;
  pthread_t partner;
  LocalState *local = local_state;
  GlobalState *global = global_state;
  const n_threads_t rounds = num_rounds(p);

  for (round = 0; round < rounds; round++) {
    partner = (id + (1 << round)) % p;
    global->flags[partner][local->parity][round] = !local->sense;
    while (global->flags[id][local->parity][round] == local->sense) {
    }
  }
  if (local->parity == 1) {
    local->sense = !local->sense;
  }
  local->parity = 1 - local->parity;
}

void *init_global_barrier_state(const n_threads_t p) {
  GlobalState *state;
  n_threads_t counter;
  const n_threads_t rounds = num_rounds(p);

  state = alloc(1, sizeof(GlobalState));
  state->p = p;

  state->flags = alloc(p, sizeof(bool **));

  for (counter = 0; counter < p; counter++) {
    state->flags[counter] = alloc(2, sizeof(bool *));
    state->flags[counter][0] = alloc(rounds, sizeof(bool));
    state->flags[counter][1] = alloc(rounds, sizeof(bool));
  }

  return state;
}

void *init_local_barrier_state(const n_threads_t p __attribute__((unused))) {
  LocalState *state;
  state = alloc(1, sizeof(LocalState));

  state->sense = false;
  state->parity = 0;

  return state;
}

/// Free the local state.
void free_local_barrier_state(void *state) { free(state); }

/// Free the global state.
void free_global_barrier_state(void *state) {
  GlobalState *global = state;
  n_threads_t counter;

  for (counter = 0; counter < global->p; counter++) {
    free(global->flags[counter][0]);
    free(global->flags[counter][1]);
    free(global->flags[counter]);
  }

  free(global->flags);
  free(global);
}
