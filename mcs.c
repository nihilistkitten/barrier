#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "barrier.h"
#include "utils.h"

typedef uint32_t round_t;

typedef struct GlobalState {
  bool **flags;
  n_threads_t p;
} GlobalState;

#define STARTING_SENSE false;

size_t left_child(size_t of) { return of * 2 + 1; }
size_t right_child(size_t of) { return of * 2 + 2; }
size_t parent(size_t of) {
  if (of % 2 == 1) {
    return (of - 1) / 2;
  } else {
    return of / 2 - 1;
  }
}

bool right_child_done(size_t of, n_threads_t p, bool **flags, bool sense) {
  size_t right = right_child(of);
  if (right >= p) {
    return true;
  } else {
    return flags[right][0] == sense;
  }
}

bool left_child_done(size_t of, n_threads_t p, bool **flags, bool sense) {
  size_t left = left_child(of);
  if (left >= p) {
    return true;
  } else {
    return flags[left][0] == sense;
  }
}

/// are both children done with the first loop?
bool children_done(size_t of, n_threads_t p, bool **flags, bool sense) {
  return left_child_done(of, p, flags, sense) &&
         right_child_done(of, p, flags, sense);
}

/// is the parent done with the second loop?
bool parent_done(size_t of, bool **flags, bool sense) {
  if (of == 0) {
    return true;
  } else {
    return flags[parent(of)][1] == sense;
  }
}

/// A barrier.
void barrier(const uint32_t p, const pthread_t id, void *local_state,
             void *global_state) {
  GlobalState *global = global_state;
  bool *sense = local_state;

  while (!children_done(id, p, global->flags, *sense)) {
  }
  global->flags[id][0] = *sense;

  while (!parent_done(id, global->flags, *sense)) {
  }
  global->flags[id][1] = *sense;

  *sense = !*sense;
}

void *init_global_barrier_state(const n_threads_t p) {
  GlobalState *state;
  n_threads_t counter;

  state = alloc(1, sizeof(GlobalState));
  state->p = p;

  state->flags = alloc(p, sizeof(bool **));

  for (counter = 0; counter < p; counter++) {
    state->flags[counter] = alloc(2, sizeof(bool *));
    state->flags[counter][0] = STARTING_SENSE;
    state->flags[counter][1] = STARTING_SENSE;
  }

  return state;
}

/// Initialize the thread-local state.
void *init_local_barrier_state(const n_threads_t p __attribute__((unused))) {
  // need to heap-allocate this bool because we need the barrier to be passed a
  // pointer to the state so its modifications are persistent
  bool *out = alloc(1, sizeof(bool));
  *out = !STARTING_SENSE;
  return out;
}

/// Free the local state.
void free_local_barrier_state(void *state) { free(state); }

/// Free the global state.
void free_global_barrier_state(void *state) {
  n_threads_t counter;
  GlobalState *global = state;

  for (counter = 0; counter < global->p; counter++) {
    free(global->flags[counter]);
  }
  free(state);
}
