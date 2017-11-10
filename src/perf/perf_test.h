//
// Created by robert on 7/6/17.
//

#include <set>
#include <string>
#include <algorithm>
#include "../tree/vanilla_b_plus_tree.h"
#include "../utils/generator.h"
#include "../utils/rdtsc.h"
#include "insert.h"
#include "update.h"

using namespace std;
template <typename K, typename V>
void benchmark(BTree<K, V> *tree, const string name, const int runs, const int ntuples, const int reads,
               const int nupdates, double skewness) {
    uint64_t total_start = ticks();
    double build_time = 0, search_time = 0, update_time = 0;
    int run = runs;
    int founds = 0;
    int errors = 0;
    uint64_t search_cycles = 0;
    ZipfGenerator generator(ntuples, skewness);

//    int *tuples = new int[ntuples];

    vector<pair<K, V>> tuples;
    vector<pair<K, V>> updates;

    for (int i = 0; i < ntuples; ++i) {
        tuples.push_back(make_pair(i, i));
    }
    random_shuffle(tuples.begin(), tuples.end());

    for (int i = 0; i < nupdates; ++i) {
        const int key = generator.gen();
        updates.push_back(make_pair(key, key));
    }

    int *search_keys = new int[reads];
    for (int i = 0; i < reads; ++i) {
        search_keys[i] = generator.gen();
    }

    printf("begin to run benchmark\n");
    while (run--) {
        tree->clear();
        std::set<int> s;

        uint64_t begin = ticks();
        insert<K, V>(tree, tuples, 1);
        uint64_t end = ticks();
        double elapsed_secs = cycles_to_seconds(end - begin);
        build_time += elapsed_secs;

        begin = ticks();
        update<K, V>(tree, updates, 1);
        end = ticks();
        update_time += cycles_to_seconds(end - begin);;


        begin = ticks();
        for (int i = 0; i < reads; ++i) {
            int value;
            const K key = search_keys[i];
            const bool is_found = tree->search(key, value);
            founds += is_found;
            // avoid the search operator to be wept out by the compile optimizer.
            if (is_found && value != key) {
                std::cout << std::endl;
            }
        }
        end = ticks();
        search_time += cycles_to_seconds(end - begin);;


    }
    delete[] search_keys;

    cout << ntuples << " tuples." << endl;

    cout << "[" << name.c_str() << "]: " << "#. of runs: " << runs << ", #. of tuples: " << ntuples
         << " reads: " << reads * runs <<" found: " << founds
         << ", Insert: " << ntuples * runs / build_time / 1000 << " K tuples / s"
         << ", Update: " << nupdates * runs / update_time / 1000 << " K tuples / s"
         << ", Search: " << reads * runs / search_time / 1000 << " K tuples / s"
         << ", found: " << founds
         << endl;

    uint64_t total_end = ticks();
    cout << "total execution time: " << cycles_to_seconds(total_end - total_start) << endl;
}