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

unsigned _Atomic wbw_c = ATOMIC_VAR_INIT(0);

void *wbw_t0(void *global) {
  ThreadState *state = global;

  void *local = init_local_barrier_state(state->p);
  sleep(1);
  atomic_store(&wbw_c, 1);
  barrier(state->p, state->id, local, state->global);

  free_local_barrier_state(local);
  return NULL;
}

void *wbw_t1(void *global) {
  ThreadState *state = global;

  void *local = init_local_barrier_state(state->p);
  barrier(state->p, state->id, local, state->global);
  atomic_store(&wbw_c, 2);

  free_local_barrier_state(local);
  return NULL;
}

bool test_wbw() {
  pthread_t thread_id_zero, thread_id_one;
  void *global = init_global_barrier_state(2);

  atomic_store(&wbw_c, 0);

  ThreadState state_zero;
  state_zero.global = global;
  state_zero.id = 0;
  state_zero.p = 2;
  pthread_create(&thread_id_zero, NULL, *wbw_t0, &state_zero);

  ThreadState state_one;
  state_one.global = global;
  state_one.id = 1;
  state_one.p = 2;
  pthread_create(&thread_id_one, NULL, *wbw_t1, &state_one);

  pthread_join(thread_id_zero, NULL);
  pthread_join(thread_id_one, NULL);

  free_global_barrier_state(global);
  return (wbw_c == 2);
}

unsigned _Atomic oneb_c = ATOMIC_VAR_INIT(false);

/// A thread which guarantees no thread has left before it enters.
void *oneb_t(void *global) {
  ThreadState *state = global;
  bool out = true;

  void *local = init_local_barrier_state(state->p);
  if (oneb_c) {
    out = false;
  }
  barrier(state->p, state->id, local, state->global);
  atomic_store(&oneb_c, true);

  free_local_barrier_state(local);
  return (void *)out;
}

bool test_oneb(size_t p) {
  size_t counter;
  void *thread_ret;
  bool out = true;
  pthread_t *ids = alloc(p, sizeof(pthread_t));
  ThreadState *states = alloc(p, sizeof(ThreadState));

  void *global = init_global_barrier_state(p);

  for (counter = 0; counter < p; counter++) {
    states[counter].global = global;
    states[counter].id = counter;
    states[counter].p = p;
    pthread_create(&ids[counter], NULL, *oneb_t, &states[counter]);
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

int main() {
  assert_test(test_wbw(), "write barrier write",
              "thread one's pre-barrier write executed after thread two's "
              "post-barrier write");
  assert_test(test_oneb(32), "single barrier",
              "a thread left the barrier before every thread entered");
  assert_test(test_twob(), "two barriers",
              "two threads wrote in the wrong order");
}
