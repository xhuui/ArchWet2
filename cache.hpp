#ifndef CACHE_HPP
#define CACHE_HPP

#include <vector>

using namespace std;

struct CacheEntry{
    unsigned tag;
    int LRU_val;
    bool dirty;
    bool valid;

    CacheEntry(){
        tag = 0;
        LRU_val = 0;
        dirty = true;
        valid = false;
    }
};

class Cache{
    public: 
    Cache(int cache_size, int block_size, int n_ways, int rw_cycles, bool write_alloc);
    ~Cache();
    //return true on hit and false on miss
    bool read(unsigned pc);
    //return true on hit and false on miss
    bool write(unsigned pc);
    int get_acc_time();
    void print_cache();
    //private:
    void evict();
    void update_LRU(unsigned pc);
    // returns idx to cache level
    unsigned entry(unsigned pc);
    unsigned tag(unsigned pc);

    int cache_size;
    int block_size;
    int set_size;
    int tag_size;
    int n_ways;
    int rw_cycles;
    bool write_alloc;


    vector<vector<CacheEntry>> cache_data;
    
};



#endif