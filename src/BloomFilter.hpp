#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

class BloomFilter {
private:
    std::vector<uint8_t> _bitmap;   /* Bit vector to store the Bloom Filter         */
    size_t _nbits;                  /* Number of bits in the Bloom Filter           */
    size_t _nhash;                  /* Number of hash functions                     */
    size_t _count;                  /* Number of elements added to the Bloom Filter */

    /**
     * hash - Hash the data using the xxhash algorithm.
     * @param data The data to hash.
     * @param len The length of the data.
     * @param seed The seed for the hash function.
     * @returns The hash of the data.
     */
    uint64_t hash(const void* data, size_t len, uint64_t seed) const;

    /**
     * set - Set the bit at the index i in the Bloom Filter.
     * @param i The index to set.
     */
    void set(size_t i);

    /**
     * get - Get the bit at the index i in the Bloom Filter.
     * @param i The index to get.
     * @returns The bit at the index i.
     */
    bool get(size_t i) const;

    /**
     * nth - Get the nth hash of the data.
     * @param h1 The first hash of the data.
     * @param h2 The second hash of the data.
     * @param i The index to get.
     * @returns The nth hash of the data.
     */
    size_t nth(uint64_t h1, uint64_t h2, size_t i) const;

public:
    /**
     * BloomFilter - Constructor for the Bloom Filter.
     * @param nbits The number of bits in the Bloom Filter.
     * @param nhash The number of hash functions to use.
     */
    BloomFilter(size_t nbits, size_t nhash);

    /**
     * ~BloomFilter - Destructor for the Bloom Filter.
     */
    ~BloomFilter();
    
    /**
     * add - Add an element to the bloom filter.
     * @param s The element to add.
     */
    void add(const char* s);

    /**
     * test - Test if an element is in the bloom filter.
     * @param s The element to test.
     * @returns True if the element is in the bloom filter, false otherwise.
     */
    bool test(const char* s) const;

    /**
     * clear - Clear the bloom filter.
     */
    void clear();

    /**
     * bits - Get the number of bits in the bloom filter.
     * @returns The number of bits in the bloom filter.
     */
    size_t bits() const;

    /**
     * hashes - Get the number of hash functions in the bloom filter.
     */
    size_t hashes() const;

    /**
     * count - Get the number of elements in the Bloom Filter.
     * @returns The number of elements in the Bloom Filter.
     */
    size_t count() const;

    /**
     * frac - Get the fraction of bits that are set in the Bloom Filter.
     * @returns The fraction of bits that are set in the Bloom Filter.
     */
    double frac() const;
};
