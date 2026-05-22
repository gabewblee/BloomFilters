#include <cassert>
#include <iostream>

#include "TestBloomFilter.hpp"

#include "../src/BloomFilter.hpp"

void test_bloom_filter_constructor() {
    BloomFilter bf(1024, 4);

    assert(bf.bits() == 1024);
    assert(bf.hashes() == 4);
    assert(bf.count() == 0);
    assert(bf.frac() == 0.0);
    std::cout << "test_bloom_filter_constructor: passed\n";
}

void test_bloom_filter_insert_and_query() {
    BloomFilter bf(1024, 4);
    double frac;

    assert(bf.bits() == 1024);
    assert(bf.hashes() == 4);

    bf.add("hello");
    assert(bf.count() == 1);
    assert(bf.bits() == 1024);
    assert(bf.hashes() == 4);
    assert(bf.test("hello"));
    assert(bf.frac() > 0.0);

    frac = bf.frac();
    bf.add("world");

    assert(bf.test("hello"));
    assert(bf.test("world"));
    assert(bf.count() == 2);
    assert(bf.bits() == 1024);
    assert(bf.hashes() == 4);
    assert(bf.frac() >= frac);
    std::cout << "test_bloom_filter_insert_and_query: passed\n";
}

void test_bloom_filter_clear() {
    BloomFilter bf(1024, 4);

    bf.add("hello");
    assert(bf.count() == 1);

    bf.clear();

    assert(bf.bits() == 1024);
    assert(bf.hashes() == 4);
    assert(bf.count() == 0);
    assert(bf.frac() == 0.0);
    assert(!bf.test("hello"));
    std::cout << "test_bloom_filter_clear: passed\n";
}

int main() {
    std::cout << "Running Bloom Filter tests...\n";
    test_bloom_filter_constructor();
    test_bloom_filter_insert_and_query();
    test_bloom_filter_clear();
    return 0;
}
