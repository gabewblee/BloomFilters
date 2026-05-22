#include <algorithm>
#include <stdexcept>

#include "BloomFilter.hpp"

#include "../libs/xxhash.h"

uint64_t BloomFilter::hash(const void* data, size_t len, uint64_t seed) const {
    return XXH3_64bits_withSeed(data, len, seed);
}

void BloomFilter::set(size_t i) {
    _bitmap[i >> 3] |=  uint8_t(1u << (i & 7u));
}

bool BloomFilter::get(size_t i) const {
    return _bitmap[i >> 3] & uint8_t(1u << (i & 7u));
}

size_t BloomFilter::nth(uint64_t h1, uint64_t h2, size_t i) const {
    return size_t((h1 + uint64_t(i) * h2) % _nbits);
}

BloomFilter::BloomFilter(size_t nbits, size_t nhash) {
    if (nbits == 0)
        throw std::invalid_argument("Error: the number of bits must be greater than 0");

    if (nhash == 0)
        throw std::invalid_argument("Error: the number of hash functions must be greater than 0");

    _nbits = nbits;
    _nhash = nhash;
    _count = 0;
    _bitmap.assign((_nbits + 7) / 8, 0);
}

BloomFilter::~BloomFilter() {
    clear();
}

void BloomFilter::add(const char* s) {
    size_t len = std::strlen(s);
    uint64_t h1 = hash(s, len, 0);
    uint64_t h2 = hash(s, len, 1);
    for (size_t i = 0; i < _nhash; i++)
        set(nth(h1, h2, i));

    _count++;
}

bool BloomFilter::test(const char* s) const {
    size_t len = std::strlen(s);
    uint64_t h1 = hash(s, len, 0);
    uint64_t h2 = hash(s, len, 1);
    for (size_t i = 0; i < _nhash; i++)
        if (!get(nth(h1, h2, i)))
            return false;
    return true;
}

void BloomFilter::clear() {
    std::fill(_bitmap.begin(), _bitmap.end(), uint8_t{0});
    _count = 0;
}

size_t BloomFilter::bits() const {
    return _nbits;
}

size_t BloomFilter::hashes() const {
    return _nhash;
}

size_t BloomFilter::count() const {
    return _count;
}

double BloomFilter::frac() const {
    size_t on = 0;
    for (uint8_t b : _bitmap)
        on += __builtin_popcount(b);
    return double(on) / double(_nbits);
}
