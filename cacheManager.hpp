#ifndef CACHE_MANAGER_HPP
#define CACHE_MANAGER_HPP

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

    rw_info();
};

struct Block{
    unsigned pc;
    int LRU_val;
    bool dirty;
    bool valid;

    Block();
};

class Cache{
    public: 
    Cache(int cache_size, int block_size, int n_ways, bool write_alloc);
    ~Cache() = default;
    // returns hit/miss/miss + writeback
    rw_info read(unsigned pc);
    // returns hit/miss/miss + writeback
    rw_info write(unsigned pc);
    // read or write, depending on type given
    rw_info access_cache(unsigned pc, char type);
    // update LRU fields in the set of pc if pc is there
    bool update_LRU(unsigned pc);
    // try to remove block containing pc
    int remove_block(unsigned pc);
    // prints cache, for debugging
    void print_cache() const;

    //private:
    //find place to evict at in set
    int evict_at(unsigned set) const;
    // push the block into the cache
    rw_info push_block(unsigned pc);
    //return -1 if block not found, else returns block idx inside set
    int find_block(unsigned pc) const;
    // returns idx to cache set
    inline unsigned set(unsigned pc) const;
    // tag calc
    inline unsigned tag(unsigned pc) const;

    int cache_size;
    int block_size;
    int set_size;
    int tag_size;
    int n_ways;
    bool write_alloc;

    vector<vector<Block>> cache_data;
    
};

class CacheManager{
    public:
    Cache L1, L2;
    int mem_rw_cycles;
    int l1_rw_cycles;
    int l2_rw_cycles;

    int l1_acc_amount;
    int l2_acc_amount;
    double l1_misses;
    double l2_misses;
    double total_cycles;
    
    CacheManager(int l1_cache_size, int l1_n_ways, 
                 int l1_rw_cycles, bool l1_write_alloc,
                 int l2_cache_size, int l2_n_ways, 
                 int l2_rw_cycles, bool l2_write_alloc,
                 int mem_rw_cycles, int block_size);

    // access to full cache chain
    void access(char type, unsigned pc);
    // return misrate for cache
    double cache_misrate(int cache) const;
    double avg_acc_time() const;
};

#endif