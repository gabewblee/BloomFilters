#include <cassert>
#include <iostream>

#include "TestBlockedBloomFilter.hpp"

#include "../src/BlockedBloomFilter.hpp"

void test_blocked_bloom_filter_constructor() {
    BlockedBloomFilter bf(1024, 4);
    size_t bits_per_block = bf.cache_line_sz() * 8;

    assert(bf.bits() >= 1024);
    assert(bf.hashes() == 4);
    assert(bf.count() == 0);
    assert(bf.blocks() > 0);
    assert(bf.cache_line_sz() > 0);
    assert(bf.bits() % bits_per_block == 0);
    assert(bf.blocks() == bf.bits() / bits_per_block);
    assert(bf.frac() == 0.0);
    std::cout << "test_blocked_bloom_filter_constructor: passed\n";
}

void test_blocked_bloom_filter_insert_and_query() {
    BlockedBloomFilter bf(1024, 4);
    size_t blocks = bf.blocks();
    size_t cache_line_sz = bf.cache_line_sz();
    double frac;

    bf.add("hello");
    assert(bf.count() == 1);
    assert(bf.blocks() == blocks);
    assert(bf.cache_line_sz() == cache_line_sz);
    assert(bf.test("hello"));
    assert(bf.frac() > 0.0);

    frac = bf.frac();
    bf.add("world");

    assert(bf.test("hello"));
    assert(bf.test("world"));
    assert(bf.count() == 2);
    assert(bf.blocks() == blocks);
    assert(bf.cache_line_sz() == cache_line_sz);
    assert(bf.frac() >= frac);
    std::cout << "test_blocked_bloom_filter_insert_and_query: passed\n";
}

void test_blocked_bloom_filter_clear() {
    BlockedBloomFilter bf(1024, 4);
    size_t blocks = bf.blocks();
    size_t cache_line_sz = bf.cache_line_sz();

    bf.add("hello");
    assert(bf.count() == 1);

    bf.clear();

    assert(bf.blocks() == blocks);
    assert(bf.cache_line_sz() == cache_line_sz);
    assert(bf.count() == 0);
    assert(bf.frac() == 0.0);
    assert(!bf.test("hello"));
    std::cout << "test_blocked_bloom_filter_clear: passed\n";
}

int main() {
    std::cout << "Running Blocked Bloom Filter tests...\n";
    test_blocked_bloom_filter_constructor();
    test_blocked_bloom_filter_insert_and_query();
    test_blocked_bloom_filter_clear();

    return 0;
}
