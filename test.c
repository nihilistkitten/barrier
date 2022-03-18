#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "barrier.h"

void assert_test(bool result, char name[], char message[]) {
  if (!result) {
    printf("Test %s failed: %s", name, message);
  }
}

unsigned _Atomic __test_counter = ATOMIC_VAR_INIT(0);

void *test_thread_one() {
  sleep(3);
  atomic_store(&__test_counter, 1);
  barrier(2, pthread_self());
  return NULL;
}

void *test_thread_two() {
  barrier(2, pthread_self());
  atomic_store(&__test_counter, 2);
  return NULL;
}

bool test_simple() {
  atomic_store(&__test_counter, 0);

  pthread_t thread_id_one, thread_id_two;
  pthread_create(&thread_id_one, NULL, *test_thread_one, NULL);
  pthread_create(&thread_id_two, NULL, *test_thread_two, NULL);

  pthread_join(thread_id_one, NULL);
  pthread_join(thread_id_two, NULL);

  return (__test_counter == 2);
}

int main() {
  bool out = test_simple();
  assert_test(out, "simple", "thread one wrote after thread two");
}
