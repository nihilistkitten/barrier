# Reed CS385: Barrier Implementation and Analysis

## Building

The barriers only work with optimizations turned off; otherwise I think it
optimizes out the spinning, or something similar. The make targets should set
the correct flags for you.

I apologize for the general messiness of some of the code.

## Correctness

### Testing

There are four automated tests, contained in the `test.c` file. They can be run
with the `test` make target, which links the test object file against all three
barrier implementation files and runs the resulting executables. I'll note that
the code in this file is pretty messy; I'm not super familiar with paradigmatic
C and so I end up reusing a lot of code across tests for setting up thread
states, counters, etc.

`test_waw` spins two threads. Thread 0 sleeps for a second, writes a shared
variable to `1`, and then enters the barrier. Thread 1 enters the barrier and
then writes the shared variable to `2`. The main test function then asserts that
the variable is `2` after joining both threads. This tests the simple guarantee
that writes after a barrier resolve after writes before a barrier.

`test_raw` resets a global variable to `false` and then spins an arbitrary
number of threads. Each of these threads asserts that the global variable is
`true`, enters the barrier, and then writes the global to be `true`. If any of
the writes execute before any of the reads, the test will fail. This tests the
guarantee that writes after the barrier execute before reads before the barrier.

`test_twob` tests two threads with two barriers in sequence. Thread 0 writes a 1
to a global variable, then goes through both variables, then asserts the global
variable is 2, then writes it to 3. Thread 1 enters the first barrier, then
asserts the counter is 1, then writes it to 2, then enters the second barrier.
The main thread asserts it is 3 after joining both threads. This is a simple
test of memory sequencing guarantees across multiple barriers; in particular, it
tests the sense/parity implementations.

`test_manyb` tests many threads (the global `N_BARRIERS`) with many barriers in
sequence. It works similarly to `test_raw`: at barrier `N`, each thread asserts
that the global counter is `N`, enters the barrier, sleeps for a random amount
of time, and then writes the counter to `N+1`. If any thread executed any
instruction after the barrier before any thread executed any instruction before
the barrier, then the later thread would fail its assertion. The test file runs
this for every thread count up to 32.

These tests are of course not exhaustive, but they give me reasonably high
confidence in the guarantees made by the barriers, at least for simple cases.
