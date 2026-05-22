#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../src/BlockedBloomFilter.hpp"
#include "../src/BloomFilter.hpp"

#define SEED 42

/** Reads words from data/data.txt and splits them into positives and negatives */
static void init_words(std::vector<std::string>& positives, std::vector<std::string>& negatives) {
    FILE* f = fopen("data/data.txt", "r");
    if (!f) {
        std::cerr << "Error: could not open data/data.txt\n";
        return;
    }

    std::vector<std::string> words;

    /* Read all words from data/data.txt */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), f)) {
        std::string word(buffer);
        word.erase(word.find_last_not_of(" \n\r\t") + 1);
        if (!word.empty())
            words.push_back(word);
    }
    fclose(f);

    /* Shuffle the words */
    std::mt19937 rng(SEED);
    std::shuffle(words.begin(), words.end(), rng);

    /* Assign half to positives and half to negatives */
    size_t mid = words.size() / 2;
    positives = {words.begin(), words.begin() + mid};
    negatives = {words.begin() + mid, words.end()};
}

/* Experiment 1: FPR vs Number of Hash Functions */
static void exp1(std::vector<std::string>& positives, std::vector<std::string>& negatives) {
    std::cout << "Running experiment 1: FPR vs Number of Hash Functions...\n";
    std::ofstream csv("results/exp1.csv");
    if (!csv) {
        std::cerr << "Error: could not open results/exp1.csv\n";
        return;
    }
    csv << "number_of_hash_functions,bloom_filter_fpr,blocked_bloom_filter_fpr\n";

    static const size_t nbits = positives.size() * 10;
    for (size_t nhash = 1; nhash <= 20; nhash++) {
        BloomFilter bf(nbits, nhash);
        BlockedBloomFilter bbf(nbits, nhash);
        for (const auto& word : positives) {
            bf.add(word.c_str());
            bbf.add(word.c_str());
        }

        double bf_fpr = 0, bbf_fpr = 0;
        for (const std::string& word : negatives) {
            if (bf.test(word.c_str()))
                bf_fpr++;

            if (bbf.test(word.c_str()))
                bbf_fpr++;
        }

        bf_fpr = double(bf_fpr) / negatives.size();
        bbf_fpr = double(bbf_fpr) / negatives.size();
        csv << nhash << ","
            << std::fixed << std::setprecision(6) << bf_fpr << ","
            << std::fixed << std::setprecision(6) << bbf_fpr << "\n";
    }
}

/* Experiment 2: FPR vs Number of Bits per Entry */
static void exp2(std::vector<std::string>& positives, std::vector<std::string>& negatives) {
    std::cout << "Running experiment 2: FPR vs Number of Bits per Entry...\n";
    std::ofstream csv("results/exp2.csv");
    if (!csv) {
        std::cerr << "Error: could not open results/exp2.csv\n";
        return;
    }
    csv << "bits_per_element,nhash,bloom_filter_fpr,blocked_bloom_filter_fpr\n";

    /* Configuration vector: {bits per element, number of hash functions} */
    static const std::vector<std::pair<size_t, size_t>> configs = {
        {4, 3}, {6, 4}, {8, 6},
        {10, 7}, {12, 8}, {14, 10},
        {16, 11}, {18, 12}, {20, 14}
    };
    
    for (auto [bits_per_entry, nhash] : configs) {
        size_t nbits = positives.size() * bits_per_entry;
        BloomFilter bf(nbits, nhash);
        BlockedBloomFilter bbf(nbits, nhash);
        for (const auto& word : positives) {
            bf.add(word.c_str());
            bbf.add(word.c_str());
        }

        double bf_fpr = 0, bbf_fpr = 0;
        for (const std::string& word : negatives) {
            if (bf.test(word.c_str()))
                bf_fpr++;

            if (bbf.test(word.c_str()))
                bbf_fpr++;
        }

        bf_fpr = double(bf_fpr) / negatives.size();
        bbf_fpr = double(bbf_fpr) / negatives.size();
        csv << bits_per_entry << ","
            << nhash << ","
            << std::fixed << std::setprecision(6) << bf_fpr << ","
            << std::fixed << std::setprecision(6) << bbf_fpr << "\n";
    }
}

/* Experiment 3: Throughput vs Filter Size */
static void exp3(std::vector<std::string>& positives, std::vector<std::string>& negatives) {
    std::cout << "Running experiment 3: Throughput vs Filter Size...\n";
    std::ofstream csv("results/exp3.csv");
    if (!csv) {
        std::cerr << "Error: could not open results/exp3.csv\n";
        return;
    }
    csv << "filter_sz_bits,filter_t,insert_ns,hit_ns,miss_ns\n";

    static const size_t configs[] = {
        32   * 1024,               /* 32KB   — L1 */
        256  * 1024,               /* 256KB  — L2 */
        4    * 1024 * 1024,        /* 4MB    — L3 */
        32   * 1024 * 1024,        /* 32MB   — RAM */
        256  * 1024 * 1024,        /* 256MB  — RAM */
        1024 * 1024 * 1024,        /* 1GB    — RAM */
        4ULL * 1024 * 1024 * 1024, /* 4GB    — RAM */
    };
    
    const size_t nreps = 5, nhash = 7;
    for (size_t filter_sz_bits : configs) {
        for (size_t which = 0; which <= 1; which++) {
            double best_ins_ns = std::numeric_limits<double>::max();
            double best_hit_ns = std::numeric_limits<double>::max();
            double best_miss_ns = std::numeric_limits<double>::max();
            for (size_t rep = 0; rep < nreps; rep++) {
                if (which == 0) {
                    BloomFilter bf(filter_sz_bits, nhash);

                    /* Measure insertion time */
                    auto start_ins_ns = std::chrono::high_resolution_clock::now();
                    for (const auto& word : positives)
                        bf.add(word.c_str());
                    auto end_ins_ns = std::chrono::high_resolution_clock::now();

                    /* Measure hits time */
                    size_t sink = 0;
                    auto start_hit_ns = std::chrono::high_resolution_clock::now();
                    for (const auto& word : positives)
                        sink += bf.test(word.c_str());
                    auto end_hit_ns = std::chrono::high_resolution_clock::now();

                    /* Measure misses time */
                    auto start_miss_ns = std::chrono::high_resolution_clock::now();
                    for (const auto& word : negatives)
                        sink += bf.test(word.c_str());
                    auto end_miss_ns = std::chrono::high_resolution_clock::now();

                    (void)sink;
                    double ins_ns = std::chrono::duration<double, std::nano>(end_ins_ns - start_ins_ns).count() / positives.size();
                    double hit_ns = std::chrono::duration<double, std::nano>(end_hit_ns - start_hit_ns).count() / positives.size();
                    double miss_ns = std::chrono::duration<double, std::nano>(end_miss_ns - start_miss_ns).count() / negatives.size();

                    best_ins_ns = std::min(best_ins_ns, ins_ns);
                    best_hit_ns = std::min(best_hit_ns, hit_ns);
                    best_miss_ns = std::min(best_miss_ns, miss_ns);
                } else {
                    BlockedBloomFilter bbf(filter_sz_bits, nhash);

                    /* Measure insertion time */
                    auto start_ins_ns = std::chrono::high_resolution_clock::now();
                    for (const auto& word : positives)
                        bbf.add(word.c_str());
                    auto end_ins_ns = std::chrono::high_resolution_clock::now();

                    /* Measure hits time */
                    size_t sink = 0;
                    auto start_hit_ns = std::chrono::high_resolution_clock::now();
                    for (const auto& word : positives)
                        sink += bbf.test(word.c_str());
                    auto end_hit_ns = std::chrono::high_resolution_clock::now();

                    /* Measure misses time */
                    auto start_miss_ns = std::chrono::high_resolution_clock::now();
                    for (const auto& word : negatives)
                        sink += bbf.test(word.c_str());
                    auto end_miss_ns = std::chrono::high_resolution_clock::now();
                    (void)sink;

                    double ins_ns = std::chrono::duration<double, std::nano>(end_ins_ns - start_ins_ns).count() / positives.size();
                    double hit_ns = std::chrono::duration<double, std::nano>(end_hit_ns - start_hit_ns).count() / positives.size();
                    double miss_ns = std::chrono::duration<double, std::nano>(end_miss_ns - start_miss_ns).count() / negatives.size();

                    best_ins_ns = std::min(best_ins_ns, ins_ns);
                    best_hit_ns = std::min(best_hit_ns, hit_ns);
                    best_miss_ns = std::min(best_miss_ns, miss_ns);
                }
            }
            const char* name = (which == 0) ? "bloom_filter" : "blocked_bloom_filter";
            csv << filter_sz_bits << ","
                << name << ","
                << std::fixed << std::setprecision(2) << best_ins_ns << ","
                << std::fixed << std::setprecision(2) << best_hit_ns << ","
                << std::fixed << std::setprecision(2) << best_miss_ns << "\n";
        }
    }
}

/* Experiment 4: Throughput vs Load Factor */
static void exp4(std::vector<std::string>& positives, std::vector<std::string>& negatives) {
    std::cout << "Running experiment 4: Throughput vs load factor...\n";
    static const size_t nbits = 4 * 1024 * 1024 * 8;
    static const size_t nhash = 7;
    static const size_t nreps = 5;
    static const double loads[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};

    std::ofstream csv("results/exp4.csv");
    if (!csv) {
        std::cerr << "Error: could not open results/exp4.csv\n";
        return;
    }
    csv << "load,filter_t,ins_ns,hit_ns,miss_ns,fpr\n";

    for (double load : loads) {
        size_t ninserts = size_t(positives.size() * load);
        for (int which = 0; which <= 1; which++) {
            double best_ins = std::numeric_limits<double>::max();
            double best_hit = std::numeric_limits<double>::max();
            double best_miss = std::numeric_limits<double>::max();
            double fpr = 0.0;
            for (size_t rep = 0; rep < nreps; rep++) {
                if (which == 0) {
                    BloomFilter bf(nbits, nhash);

                    auto start1 = std::chrono::high_resolution_clock::now();
                    for (size_t i = 0; i < ninserts; i++)
                        bf.add(positives[i].c_str());
                    auto end1 = std::chrono::high_resolution_clock::now();

                    size_t sink = 0;
                    auto start2 = std::chrono::high_resolution_clock::now();
                    for (size_t i = 0; i < ninserts; i++)
                        sink += bf.test(positives[i].c_str());
                    auto end2 = std::chrono::high_resolution_clock::now();
                    (void)sink;

                    size_t misses = 0;
                    auto start3 = std::chrono::high_resolution_clock::now();
                    for (auto& neg : negatives)
                        misses += bf.test(neg.c_str());
                    auto end3 = std::chrono::high_resolution_clock::now();

                    fpr = double(misses) / negatives.size();

                    double ins = std::chrono::duration<double, std::nano>(end1 - start1).count() / ninserts;
                    double hit = std::chrono::duration<double, std::nano>(end2 - start2).count() / ninserts;
                    double miss = std::chrono::duration<double, std::nano>(end3 - start3).count() / negatives.size();

                    best_ins = std::min(best_ins,  ins);
                    best_hit = std::min(best_hit,  hit);
                    best_miss = std::min(best_miss, miss);
                } else {
                    BlockedBloomFilter bbf(nbits, nhash);

                    auto start1 = std::chrono::high_resolution_clock::now();
                    for (size_t i = 0; i < ninserts; i++)
                        bbf.add(positives[i].c_str());
                    auto end1 = std::chrono::high_resolution_clock::now();

                    size_t sink = 0;
                    auto start2 = std::chrono::high_resolution_clock::now();
                    for (size_t i = 0; i < ninserts; i++)
                        sink += bbf.test(positives[i].c_str());
                    auto end2 = std::chrono::high_resolution_clock::now();
                    (void)sink;

                    size_t misses = 0;
                    auto start3 = std::chrono::high_resolution_clock::now();
                    for (auto& neg : negatives)
                        misses += bbf.test(neg.c_str());
                    auto end3 = std::chrono::high_resolution_clock::now();

                    fpr = double(misses) / negatives.size();
                    double ins = std::chrono::duration<double, std::nano>(end1 - start1).count() / ninserts;
                    double hit = std::chrono::duration<double, std::nano>(end2 - start2).count() / ninserts;
                    double miss = std::chrono::duration<double, std::nano>(end3 - start3).count() / negatives.size();

                    best_ins = std::min(best_ins,  ins);
                    best_hit = std::min(best_hit,  hit);
                    best_miss = std::min(best_miss, miss);
                }
            }
            const char* name = (which == 0) ? "bloom_filter" : "blocked_bloom_filter";

            csv << load << ","
                << name << ","
                << std::fixed << std::setprecision(2) << best_ins << ","
                << best_hit << ","
                << best_miss << ","
                << std::setprecision(6) << fpr << "\n";
        }
    }
}

int main() {
    std::vector<std::string> positives;
    std::vector<std::string> negatives;
    init_words(positives, negatives);
    exp1(positives, negatives);
    exp2(positives, negatives);
    exp3(positives, negatives);
    exp4(positives, negatives);
    return 0;
}