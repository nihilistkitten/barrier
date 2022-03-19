#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "barrier.h"
#include "utils.h"

void assert_test(bool result, char name[], char message[]) {
  if (!result) {
    printf("Test %s failed: %s\n", name, message);
  }
}

typedef struct ThreadState {
  // we maintain a separate id from the os to make math simple
  n_threads_t p;
  pthread_t id;
  void *global;
} ThreadState;

unsigned _Atomic waw_c = ATOMIC_VAR_INIT(0);

void *waw_t0(void *global) {
  ThreadState *state = global;

  void *local = init_local_barrier_state(state->p);
  sleep(1);
  atomic_store(&waw_c, 1);
  barrier(state->p, state->id, local, state->global);

  free_local_barrier_state(local);
  return NULL;
}

void *waw_t1(void *global) {
  ThreadState *state = global;

  void *local = init_local_barrier_state(state->p);
  barrier(state->p, state->id, local, state->global);
  atomic_store(&waw_c, 2);

  free_local_barrier_state(local);
  return NULL;
}

bool test_waw() {
  pthread_t thread_id_zero, thread_id_one;
  void *global = init_global_barrier_state(2);

  atomic_store(&waw_c, 0);

  ThreadState state_zero;
  state_zero.global = global;
  state_zero.id = 0;
  state_zero.p = 2;
  pthread_create(&thread_id_zero, NULL, *waw_t0, &state_zero);

  ThreadState state_one;
  state_one.global = global;
  state_one.id = 1;
  state_one.p = 2;
  pthread_create(&thread_id_one, NULL, *waw_t1, &state_one);

  pthread_join(thread_id_zero, NULL);
  pthread_join(thread_id_one, NULL);

  free_global_barrier_state(global);
  return (waw_c == 2);
}

unsigned _Atomic raw_c = ATOMIC_VAR_INIT(false);

/// A thread which guarantees no thread has left before it enters.
void *raw_t(void *global) {
  ThreadState *state = global;
  bool out;

  void *local = init_local_barrier_state(state->p);
  // if raw_c has been flipped, we know a thread has left the barrier, but we
  // haven't entered yet. otherwise no thread has executed an instruction after
  // the barrier and so from our perspective the barrier is correct
  out = !raw_c;
  barrier(state->p, state->id, local, state->global);
  atomic_store(&raw_c, true);

  free_local_barrier_state(local);
  return (void *)out;
}

/// Test many threads entering a single barrier.
bool test_raw(size_t p) {
  size_t counter;
  void *thread_ret;
  bool out = true;
  pthread_t *ids = alloc(p, sizeof(pthread_t));
  ThreadState *states = alloc(p, sizeof(ThreadState));

  void *global = init_global_barrier_state(p);
  atomic_store(&raw_c, false);

  for (counter = 0; counter < p; counter++) {
    states[counter].global = global;
    states[counter].id = counter;
    states[counter].p = p;
    pthread_create(&ids[counter], NULL, *raw_t, &states[counter]);
  }

  for (counter = 0; counter < p; counter++) {
    pthread_join(ids[counter], (void *)&thread_ret);
    // if any thread returns false we want to return false
    if (!(bool)thread_ret) {
      out = false;
    }
  }

  free(ids);
  free(states);
  free_global_barrier_state(global);

  return out;
}

unsigned _Atomic twob_c = ATOMIC_VAR_INIT(0);

void *twob_t0(void *global) {
  ThreadState *state = global;
  bool out = true;

  void *local = init_local_barrier_state(state->p);
  atomic_store(&twob_c, 1);
  barrier(state->p, state->id, local, state->global);
  barrier(state->p, state->id, local, state->global);
  if (twob_c != 2) {
    out = false;
  }
  atomic_store(&twob_c, 3);

  free_local_barrier_state(local);
  return (void *)out;
}

void *twob_t1(void *global) {
  ThreadState *state = global;
  bool out = true;

  void *local = init_local_barrier_state(state->p);
  barrier(state->p, state->id, local, state->global);
  if (twob_c != 1) {
    out = false;
  }
  atomic_store(&twob_c, 2);
  barrier(state->p, state->id, local, state->global);

  free_local_barrier_state(local);
  return (void *)out;
}

/// Test with two barriers in sequence with threads alternating writes.
bool test_twob() {
  pthread_t thread_id_zero, thread_id_one;
  bool out = true;
  void *thread_ret;
  void *global = init_global_barrier_state(2);

  atomic_store(&twob_c, 0);

  ThreadState state_zero;
  state_zero.global = global;
  state_zero.id = 0;
  state_zero.p = 2;
  pthread_create(&thread_id_zero, NULL, *twob_t0, &state_zero);

  ThreadState state_one;
  state_one.global = global;
  state_one.id = 1;
  state_one.p = 2;
  pthread_create(&thread_id_one, NULL, *twob_t1, &state_one);

  pthread_join(thread_id_zero, (void *)&thread_ret);
  if (!(bool)thread_ret) {
    out = false;
  }

  pthread_join(thread_id_one, (void *)&thread_ret);
  if (!(bool)thread_ret) {
    out = false;
  }

  if (twob_c != 3) {
    out = false;
  }

  free_global_barrier_state(global);
  return out;
}

#define N_BARRIERS 100
unsigned _Atomic manyb_c = ATOMIC_VAR_INIT(0);

/// A thread which guarantees no thread has left before it enters.
void *manyb_t(void *global) {
  size_t counter;
  ThreadState *state = global;
  bool out = true;

  void *local = init_local_barrier_state(state->p);

  for (counter = 0; counter < N_BARRIERS; counter++) {
    if (manyb_c != counter) {
      out = false;
    }
    barrier(state->p, state->id, local, state->global);
    atomic_store(&manyb_c, counter + 1);
  }

  free_local_barrier_state(local);
  return (void *)out;
}

/// Test many threads entering many barriers.
bool test_manyb(n_threads_t p) {
  size_t counter;
  void *thread_ret;
  bool out = true;
  pthread_t *ids = alloc(p, sizeof(pthread_t));
  ThreadState *states = alloc(p, sizeof(ThreadState));

  void *global = init_global_barrier_state(p);
  atomic_store(&manyb_c, 0);

  for (counter = 0; counter < p; counter++) {
    states[counter].global = global;
    states[counter].id = counter;
    states[counter].p = p;
    pthread_create(&ids[counter], NULL, *manyb_t, &states[counter]);
  }

  for (counter = 0; counter < p; counter++) {
    pthread_join(ids[counter], (void *)&thread_ret);
    // if any thread returns false we want to return false
    if (!(bool)thread_ret) {
      out = false;
    }
  }

  if (manyb_c != N_BARRIERS) {
    out = false;
  }

  free(ids);
  free(states);
  free_global_barrier_state(global);

  return out;
}

int main() {
  n_threads_t p;

  assert_test(test_waw(), "write barrier write",
              "thread one's pre-barrier write executed after thread two's "
              "post-barrier write");
  assert_test(test_raw(32), "single barrier",
              "a thread left the barrier before every thread entered");
  assert_test(test_twob(), "two barriers",
              "two threads wrote in the wrong order");

  for (p = 1; p < 32; p++) {
    assert_test(test_manyb(p), "many barriers",
                "threads wrote in the wrong order");
  }
}
