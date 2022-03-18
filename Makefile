CXX := gcc
WARN := -Wall -Werror -Wextra -pedantic 
OPT := -g
CFLAGS := $(OPT) $(WARN)

HEADERS := barrier.h
DEPS := $(HEADERS) $(OBJECTS)

TARGETS = main test_centralized


all: main

bench.png: data.csv pyproject.toml main.py
	poetry run python main.py

data.csv: main
	./$^ > $@

main: main.c $(DEPS)
	$(CXX) -o $@ $^ $(CFLAGS)

test: test_centralized
	./test_centralized

test_centralized: test.o centralized.o
	$(CXX) -o $@ $^ $(CFLAGS)

test_unused: test.o unused.o
	$(CXX) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CXX) -c $< $(CFLAGS)

clean:
	-rm -f *.o
	-rm -f $(TARGETS)
	-rm -f data.csv
	-rm -f bench.png

.PHONY: all clean test
