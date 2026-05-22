CPP=g++
CFLAGS=-Wall -Wextra -O3 -g -std=c++17

BUILD=build
LIBS=libs
RESULTS=results
SRC=src
TESTS=tests

.PHONY: build test experiment clean

build:
	mkdir -p $(BUILD)
	mkdir -p $(RESULTS)
	python3 -m venv .venv
	.venv/bin/pip install -r $(SRC)/requirements.txt

test: $(BUILD)/TestBloomFilter $(BUILD)/TestBlockedBloomFilter
	./$(BUILD)/TestBloomFilter
	./$(BUILD)/TestBlockedBloomFilter

experiment: $(BUILD)/Experiment
	./$(BUILD)/Experiment
	.venv/bin/python3 $(SRC)/plot.py

$(BUILD)/TestBloomFilter: $(TESTS)/TestBloomFilter.cpp $(SRC)/BloomFilter.cpp $(LIBS)/xxhash.c
	$(CPP) $(CFLAGS) $(TESTS)/TestBloomFilter.cpp $(SRC)/BloomFilter.cpp $(LIBS)/xxhash.c -o $(BUILD)/TestBloomFilter

$(BUILD)/TestBlockedBloomFilter: $(TESTS)/TestBlockedBloomFilter.cpp $(SRC)/BlockedBloomFilter.cpp $(LIBS)/xxhash.c
	$(CPP) $(CFLAGS) $(TESTS)/TestBlockedBloomFilter.cpp $(SRC)/BlockedBloomFilter.cpp $(LIBS)/xxhash.c -o $(BUILD)/TestBlockedBloomFilter

$(BUILD)/Experiment: $(SRC)/Experiments.cpp $(SRC)/BloomFilter.cpp $(SRC)/BlockedBloomFilter.cpp $(LIBS)/xxhash.c
	$(CPP) $(CFLAGS) $(SRC)/Experiments.cpp $(SRC)/BloomFilter.cpp $(SRC)/BlockedBloomFilter.cpp $(LIBS)/xxhash.c -o $(BUILD)/Experiment

clean:
	rm -rf $(BUILD)/*
	rm -rf $(RESULTS)/*