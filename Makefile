CXX := gcc
WARN := -Wall -Werror -Wextra -pedantic 
OPT := -lm -pthread
CFLAGS := $(OPT) $(WARN)

BARRIERS := centralized dissemination mcs

TESTS := $(patsubst %, test_%, $(BARRIERS))
BENCHES := $(patsubst %, bench_%, $(BARRIERS)) bench_overhead
DATA := $(patsubst %, %.csv, $(BARRIERS)) overhead.csv

bench.png: $(DATA) plot.py pyproject.toml
	poetry run python plot.py

test: $(TESTS)
	./test_centralized
	./test_dissemination
	./test_mcs

%.csv: bench_%
	./$^ > $@

test_%: test.o utils.o %.o
	$(CXX) -o $@ $^ $(CFLAGS)

bench_%: bench.o utils.o %.o
	$(CXX) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CXX) -c $< $(CFLAGS)

clean:
	-rm -f *.o
	-rm -f *.png
	-rm -f $(TESTS)
	-rm -f $(BENCHES)
	-rm -f $(DATA)

.PHONY: clean test bench
