#include <algorithm>
#include <cstring>
#include <new>
#include <stdexcept>

#include "BlockedBloomFilter.hpp"

#include "../libs/xxhash.h"

static constexpr size_t cache_line_def_sz = 64;

uint64_t BlockedBloomFilter::hash(const void* data, size_t len, uint64_t seed) const {
    return XXH3_64bits_withSeed(data, len, seed);
}

void BlockedBloomFilter::set(size_t i) {
    _bitmap[i >> 3] |= uint8_t(1u << (i & 7u));
}

bool BlockedBloomFilter::get(size_t i) const {
    return _bitmap[i >> 3] & uint8_t(1u << (i & 7u));
}

size_t BlockedBloomFilter::block_bits() const {
    return _cache_line_sz * 8;
}

size_t BlockedBloomFilter::block_cnt() const {
    return _nbits / block_bits();
}

size_t BlockedBloomFilter::nth(uint64_t h1, uint64_t h2, size_t i) const {
    return size_t((h1 + uint64_t(i) * h2) % block_bits());
}

BlockedBloomFilter::BlockedBloomFilter(size_t nbits, size_t nhash) {
    if (nbits == 0)
        throw std::invalid_argument("Error: the number of bits must be greater than 0");

    if (nhash == 0)
        throw std::invalid_argument("Error: the number of hash functions must be greater than 0");

#if defined(__cpp_lib_hardware_interference_size)
    _cache_line_sz = std::hardware_destructive_interference_size;
#else
    _cache_line_sz = cache_line_def_sz;
#endif

    if (_cache_line_sz == 0)
        _cache_line_sz = cache_line_def_sz;

    size_t bits_per_block = block_bits();
    size_t nblocks = (nbits + bits_per_block - 1) / bits_per_block;

    _nbits = nblocks * bits_per_block;
    _nhash = nhash;
    _count = 0;
    _bitmap.assign((_nbits + 7) / 8, 0);
}

BlockedBloomFilter::~BlockedBloomFilter() {
    clear();
}

void BlockedBloomFilter::add(const char* s) {
    size_t len = std::strlen(s);
    uint64_t h1 = hash(s, len, 0);
    uint64_t h2 = hash(s, len, 1);
    size_t nblocks = block_cnt();
    size_t block = size_t(h1 % nblocks);
    size_t base = block * block_bits();
    size_t step = size_t(((h1 >> 32u) ^ h2) | 1ULL);

    for (size_t i = 0; i < _nhash; i++)
        set(base + nth(h2, step, i));

    _count++;
}

bool BlockedBloomFilter::test(const char* s) const {
    size_t len = std::strlen(s);
    uint64_t h1 = hash(s, len, 0);
    uint64_t h2 = hash(s, len, 1);
    size_t nblocks = block_cnt();
    size_t block = size_t(h1 % nblocks);
    size_t base = block * block_bits();
    size_t step = size_t(((h1 >> 32u) ^ h2) | 1ULL);

    for (size_t i = 0; i < _nhash; i++)
        if (!get(base + nth(h2, step, i)))
            return false;

    return true;
}

void BlockedBloomFilter::clear() {
    std::fill(_bitmap.begin(), _bitmap.end(), uint8_t{0});
    _count = 0;
}

size_t BlockedBloomFilter::bits() const {
    return _nbits;
}

size_t BlockedBloomFilter::hashes() const {
    return _nhash;
}

size_t BlockedBloomFilter::count() const {
    return _count;
}

size_t BlockedBloomFilter::cache_line_sz() const {
    return _cache_line_sz;
}

size_t BlockedBloomFilter::blocks() const {
    return block_cnt();
}

double BlockedBloomFilter::frac() const {
    size_t on = 0;
    for (uint8_t b : _bitmap)
        on += __builtin_popcount(b);
    return double(on) / double(_nbits);
}