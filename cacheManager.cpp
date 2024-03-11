#include "cacheManager.hpp"
#include <iostream>
#include <math.h>
#include <assert.h>

rw_info::rw_info(){
    hit = false;
    writeback = false;
    removed = false;
    pc = 0;
}

Block::Block(){
    pc = 0;
    LRU_val = 0;
    dirty = false;
    valid = false;
}

/**************************CACHE_MANAGER IMPLEMENTATION***********************/

CacheManager::CacheManager(int l1_cache_size, int l1_n_ways, 
                           int l1_rw_cycles, bool l1_write_alloc,
                           int l2_cache_size, int l2_n_ways, 
                           int l2_rw_cycles, bool l2_write_alloc,
                           int mem_rw_cycles, int block_size) : 
              L1(l1_cache_size, block_size, l1_n_ways, l1_write_alloc),
              L2(l2_cache_size, block_size, l2_n_ways, l2_write_alloc){

    this->mem_rw_cycles = mem_rw_cycles;
    this->l1_rw_cycles = l1_rw_cycles;
    this->l2_rw_cycles = l2_rw_cycles;

    l1_acc_amount = 0;
    l2_acc_amount = 0;
    l1_misses = 0;
    l2_misses = 0;
    total_cycles = 0;
}
// simulate 1 input line
void CacheManager::access(char type, unsigned pc){
    rw_info info_copy;
    l1_acc_amount++;
    total_cycles += l1_rw_cycles;
    rw_info info;

    if(!(info = L1.access_cache(pc, type)).hit){
        l1_misses++;
        l2_acc_amount++;
        total_cycles += l2_rw_cycles;
        info_copy = info;
                    
        if(!(info = L2.access_cache(pc, type)).hit){
            if(info.removed)
                L1.remove_block(info.pc);
            l2_misses++;
            total_cycles += mem_rw_cycles;
        }

        if(info_copy.writeback){
            L2.update_LRU(info_copy.pc);
        }
    }
        
}

double CacheManager::cache_misrate(int cache) const {
    double misses = (cache == 1) ? l1_misses : l2_misses;
    double acc_amount = (cache == 1) ? l1_acc_amount : l2_acc_amount;
    return misses / acc_amount;
}

double CacheManager::avg_acc_time() const {
    return total_cycles / l1_acc_amount;
}

/* ***************************CACHE IMPLEMENTATION*****************************/

Cache::Cache(int cache_size, int block_size, int n_ways, bool write_alloc){
    this->cache_size = cache_size;
    this->block_size = block_size;
    this->n_ways = n_ways;
    this->write_alloc = write_alloc;

    this->set_size = cache_size - block_size - n_ways;
    this->tag_size = 32 - block_size - set_size;

    cache_data = vector<vector<Block>>(pow(2, set_size), 
                                            vector<Block>(pow(2, n_ways)));
}

rw_info Cache::access_cache(unsigned pc, char type){
    if(type == 'r')
        return read(pc);
    if(type == 'w')
        return write(pc);
    return rw_info();
}

rw_info Cache::read(unsigned pc){
    rw_info ret;
    unsigned tag = this->tag(pc);
    unsigned set = this->set(pc);

    if(int at = find_block(pc) != -1){
        update_LRU(pc);
        ret.hit = true;
        return ret;
    }
    ret = push_block(pc);
    update_LRU(pc);
    ret.hit = false;
    return ret;
}

rw_info Cache::write(unsigned pc){
    rw_info ret;
    unsigned tag = this->tag(pc);
    unsigned set = this->set(pc);
    int at = find_block(pc);
    if(at != -1){
        update_LRU(pc);
        cache_data[set][at].dirty = 1;
        ret.hit = true;
        return ret;
    }
    if(this->write_alloc){
        ret = push_block(pc);
        update_LRU(pc);
        at = find_block(pc);
        cache_data[set][at].dirty = 1;
    }
    ret.hit = false;
    return ret;
}

rw_info Cache::push_block(unsigned pc){
    rw_info ret;
    ret.writeback = false;

    unsigned tag = this->tag(pc);
    unsigned set = this->set(pc);
    int at = this->evict_at(set);

    assert(at != -1);

    Block* ent = &cache_data[set][at];

    if(ent->valid){
        ret.pc = ent->pc;
        ret.removed = true;
        if(ent->dirty){
            ret.writeback = true;
        }
    }

    ent->pc = pc;
    ent->dirty = false;
    ent->valid = true;

    return ret;
}

int Cache::remove_block(unsigned pc){
    unsigned tag = this->tag(pc);
    unsigned set = this->set(pc);
    Block* ent;
    for(int i = 0; i < cache_data[set].size(); i++){
        ent = &cache_data[set][i];
        if(this->tag((ent->pc)) == tag){
            ent->valid = false;
            ent->LRU_val = 0;
            return ent->dirty;
        }      
    }
    return -1;
}

int Cache::evict_at(unsigned set) const {
    for(int i = 0; i < cache_data[set].size(); i++){
        if(!cache_data[set][i].valid || !cache_data[set][i].LRU_val){
            return i;
        }      
    }
    return -1;
}

int Cache::find_block(unsigned pc) const {
    unsigned set = this->set(pc);
    unsigned tag = this->tag(pc);
    for(int i = 0; i < cache_data[set].size(); i++){
        if(this->tag(cache_data[set][i].pc) == tag && cache_data[set][i].valid)
            return i;
    }
    return -1;
}

bool Cache::update_LRU(unsigned pc){
    unsigned set = this->set(pc);
    unsigned tag = this->tag(pc);
    int cur_LRU = -1, cur_idx = -1;

    Block* ent;
    for(int i = 0; i < cache_data[set].size(); i++){
        ent = &cache_data[set][i];
        if(ent->valid && this->tag(ent->pc) == tag){
            cur_idx = i;
            cur_LRU = ent->LRU_val;
            ent->LRU_val = pow(2, n_ways) - 1;
            break;
        }      
    }
    if(cur_LRU == -1 || cur_idx == -1){
        return false;
    }

    for(int j = 0; j < cache_data[set].size(); j++){
        ent = &cache_data[set][j];
        if(ent->LRU_val > cur_LRU && j != cur_idx)
            ent->LRU_val--;
    }

    return true;
}

inline unsigned Cache::set(unsigned pc) const {
    unsigned set_mask = ~(-1 << set_size);
    return (pc >> (block_size)) & set_mask;
}

inline unsigned Cache::tag(unsigned pc) const {
    unsigned tag_mask = ~(-1 << tag_size);
    return (pc >> (set_size + block_size)) & tag_mask;
}

void Cache::print_cache() const {
    for(auto& set : cache_data){
        cout << "level:" << endl;
        for(auto& entry : set){
            cout << "pc: " << entry.pc 
                 << " tag: " << tag(entry.pc) 
                 << " LRU: " << entry.LRU_val
                 << " dirty: " << entry.dirty 
                 << " valid: " << entry.valid << endl;
        }
    }
}