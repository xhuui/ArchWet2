#include "cache.hpp"
#include <iostream>
#include <math.h>
#include <assert.h>

Cache::Cache(int cache_size, int block_size, int n_ways, int rw_cycles, bool write_alloc){
    this->cache_size = cache_size;
    this->block_size = block_size;
    this->n_ways = n_ways;
    this->rw_cycles = rw_cycles;
    this->write_alloc = write_alloc;

    this->set_size = cache_size - block_size - n_ways;
    this->tag_size = 32 - block_size - set_size;

    cache_data = vector<vector<CacheEntry>>(pow(2, set_size), 
                                            vector<CacheEntry>(pow(2, n_ways)));
}

Cache::~Cache(){
    return;
}

void Cache::print_cache(){
    for(auto& level : cache_data){
        cout << "level:" << endl;
        for(auto& entry : level){
            cout << "tag: " << entry.tag << " LRU: " << entry.LRU_val
                 << " dirty: " << entry.dirty << " valid: " << entry.valid << endl;
        }
    }
}

int Cache::get_acc_time(){return rw_cycles;}

unsigned Cache::entry(unsigned pc){
    unsigned set_mask = ~(-1 << set_size);
    cout << "entry: " << ((pc >> (block_size)) & set_mask);
    return (pc >> (2 + block_size)) & set_mask;
}

unsigned Cache::tag(unsigned pc){
    unsigned tag_mask = ~(-1 << tag_size);
    cout << " tag: " << ((pc >> (set_size + block_size)) & tag_mask) << endl;
    return (pc >> (2 + set_size + block_size)) & tag_mask;
}

void Cache::update_LRU(unsigned pc){
    unsigned entry = this->entry(pc);
    unsigned tag = this->tag(pc);
    cout << "update_LRU entry: " << entry << " tag: " << tag << endl; 
    int cur_LRU = -1, cur_idx = -1;

    CacheEntry* ent;
    cout << "cache_data size: " << cache_data[entry].size() << endl;
    for(int i = 0; i < cache_data[entry].size(); i++){
        ent = &cache_data[entry][i];
        if(ent->tag == tag){
            cur_idx = i;
            cur_LRU = ent->LRU_val;
            ent->LRU_val = pow(2, n_ways) - 1;
            break;
        }      
    }
    cout << cur_LRU << cur_idx << endl;

    assert(cur_LRU != -1);
    assert(cur_idx != -1);

    for(int j = 0; j < cache_data[entry].size(); j++){
        ent = &cache_data[entry][j];
        if(ent->LRU_val > cur_LRU && j != cur_idx)
            ent->LRU_val--;
    }
        
}

int main(){
    Cache c1(6, 2, 2, 4, 4);
    cout << "set size: " << c1.set_size << endl;
    cout << "tag size: " << c1.tag_size << endl;
    cout << "block size: " << c1.block_size << endl;
    c1.update_LRU(0);
    c1.print_cache();
    for(int i = 0; i < 50; i++){
        c1.entry(i << 2);
        c1.tag(i << 2);
    }
}