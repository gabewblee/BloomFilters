#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class BlockedBloomFilter {
private:
    size_t _cache_line_sz;          /* Cache line size in bytes                             */
    std::vector<uint8_t> _bitmap;   /* Bit vector to store the Blocked Bloom Filter         */
    size_t _nbits;                  /* Number of bits in the Blocked Bloom Filter           */
    size_t _nhash;                  /* Number of hash functions                             */
    size_t _count;                  /* Number of elements added to the Blocked Bloom Filter */

    /**
     * hash - Hash the data using the xxhash algorithm.
     * @param data The data to hash.
     * @param len The length of the data.
     * @param seed The seed for the hash function.
     * @returns The hash of the data.
     */
    uint64_t hash(const void* data, size_t len, uint64_t seed) const;

    /**
     * set - Set the bit at the index i in the Blocked Bloom Filter.
     * @param i The index to set.
     */
    void set(size_t i);

    /**
     * get - Get the bit at the index i in the Blocked Bloom Filter.
     * @param i The index to get.
     * @returns The bit at the index i.
     */
    bool get(size_t i) const;

    /**
     * block_bits - Get the number of bits in one block.
     * @returns The number of bits in one cache-line-sized block.
     */
    size_t block_bits() const;

    /**
     * block_cnt - Get the number of blocks in the filter.
     * @returns The number of blocks in the filter.
     */
    size_t block_cnt() const;

    /**
     * nth - Get the nth hash inside a block.
     * @param h1 The first hash of the data.
     * @param h2 The second hash of the data.
     * @param i The index to get.
     * @returns The nth offset within a block.
     */
    size_t nth(uint64_t h1, uint64_t h2, size_t i) const;

public:
    /**
     * BlockedBloomFilter - Constructor for the blocked Bloom Filter.
     * @param nbits The number of bits in the Blocked Bloom Filter.
     * @param nhash The number of hash functions to use.
     */
    BlockedBloomFilter(size_t nbits, size_t nhash);

    /**
     * ~BlockedBloomFilter - Destructor for the blocked Bloom Filter.
     */
    ~BlockedBloomFilter();

    /**
     * add - Add an element to the blocked Bloom Filter.
     * @param s The element to add.
     */
    void add(const char* s);

    /**
     * test - Test if an element is in the blocked Bloom Filter.
     * @param s The element to test.
     * @returns True if the element may be present, false otherwise.
     */
    bool test(const char* s) const;

    /**
     * clear - Clear the blocked Bloom Filter.
     */
    void clear();

    /**
     * bits - Get the number of bits in the blocked Bloom Filter.
     * @returns The number of bits in the blocked Bloom Filter.
     */
    size_t bits() const;

    /**
     * hashes - Get the number of hash functions in the blocked Bloom Filter.
     * @returns The number of hash functions.
     */
    size_t hashes() const;

    /**
     * count - Get the number of inserted elements.
     * @returns The number of inserted elements.
     */
    size_t count() const;

    /**
     * cache_line_sz - Get the cache line size used as a block size.
     * @returns The block size in bytes.
     */
    size_t cache_line_sz() const;

    /**
     * blocks - Get the number of blocks in the blocked Bloom Filter.
     * @returns The number of blocks.
     */
    size_t blocks() const;

    /**
     * frac - Get the fraction of bits that are set in the blocked Bloom Filter.
     * @returns The fraction of bits that are set.
     */
    double frac() const;
};