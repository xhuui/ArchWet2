#ifndef CACHE_HPP
#define CACHE_HPP

#include <vector>

using namespace std;

struct Cachelevel{
    unsigned tag;
    int LRU_val;
    bool dirty;
    bool valid;

    Cachelevel(){
        tag = 0;
        LRU_val = 0;
        dirty = true;
        valid = false;
    }
};

class Cache{
    public: 
    Cache(int cache_size, int block_size, int n_ways, int rw_cycles, bool write_alloc);
    ~Cache() = default;
    //return true on hit and false on miss
    bool read(unsigned pc);
    //return true on hit and false on miss
    bool write(unsigned pc);

    int get_acc_time();
    void print_cache();
    //private:
    int evict_at(unsigned level);
    void push_block(unsigned pc);
    void update_LRU(unsigned pc);
    int find_block(unsigned pc);
    // returns idx to cache level
    inline unsigned level(unsigned pc);
    inline unsigned tag(unsigned pc);

    int cache_size;
    int block_size;
    int set_size;
    int tag_size;
    int n_ways;
    int rw_cycles;
    bool write_alloc;

    vector<vector<Cachelevel>> cache_data;
    
};

#endif