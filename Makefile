CXX := gcc
WARN := -Wall -Werror -Wextra -pedantic 
OPT := -g -lm
CFLAGS := $(OPT) $(WARN)

HEADERS := barrier.h utils.h
OBJECTS := utils.o
DEPS := $(HEADERS) $(OBJECTS)

BINARIES = main test_centralized test_unused test_dissemination

all: main

bench.png: data.csv pyproject.toml main.py
	poetry run python main.py

data.csv: main
	./$^ > $@

main: main.c $(DEPS)
	$(CXX) -o $@ $^ $(CFLAGS)

test: test_centralized test_dissemination
	./test_centralized
	./test_dissemination

test_%: test.o utils.o %.o
	$(CXX) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CXX) -c $< $(CFLAGS)

clean:
	-rm -f *.o
	-rm -f $(BINARIES)
	-rm -f data.csv
	-rm -f bench.png

.PHONY: all clean test
