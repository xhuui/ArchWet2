#ifndef CACHE_HPP
#define CACHE_HPP

#include <vector>

using namespace std;

//info for read/write functions
//removed is true if the removed block was valid
//pc is the evicted pc if writeback happened
struct rw_info{
    bool hit;
    bool writeback;
    bool removed;
    unsigned pc;
};

struct CacheEntry{
    unsigned pc;
    int LRU_val;
    bool dirty;
    bool valid;

    CacheEntry(){
        pc = 0;
        LRU_val = 0;
        dirty = true;
        valid = false;
    }
};

class Cache{
    public: 
    Cache(int cache_size, int block_size, int n_ways, bool write_alloc);
    ~Cache() = default;
    // returns hit/miss/miss + writeback
    rw_info read(unsigned pc);
    // returns hit/miss/miss + writeback
    rw_info write(unsigned pc);
    bool update_LRU(unsigned pc);
    void remove_block(unsigned pc);
    void print_cache();
    bool is_write_alloc();
    //private:
    int evict_at(unsigned entry);
    rw_info push_block(unsigned pc);
    int find_block(unsigned pc);
    // returns idx to cache entry

    inline unsigned entry(unsigned pc);
    inline unsigned tag(unsigned pc);

    int cache_size;
    int block_size;
    int set_size;
    int tag_size;
    int n_ways;
    bool write_alloc;

    vector<vector<CacheEntry>> cache_data;
    
};

#endif