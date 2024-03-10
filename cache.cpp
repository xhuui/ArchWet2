#include "cache.hpp"
#include <iostream>
#include <math.h>
#include <assert.h>

Cache::Cache(int cache_size, int block_size, int n_ways, bool write_alloc){
    this->cache_size = cache_size;
    this->block_size = block_size;
    this->n_ways = n_ways;
    this->write_alloc = write_alloc;

    this->set_size = cache_size - block_size - n_ways;
    this->tag_size = 32 - block_size - set_size;

    cache_data = vector<vector<CacheEntry>>(pow(2, set_size), 
                                            vector<CacheEntry>(pow(2, n_ways)));
}

rw_info Cache::read(unsigned pc){
    rw_info ret;
    unsigned tag = this->tag(pc);
    unsigned entry = this->entry(pc);

    if(int at = find_block(pc) != -1){
        update_LRU(pc);
        ret.hit = true;
        return ret;
    }
        
    ret = push_block(pc);
    ret.hit = false;
    return ret;
}

rw_info Cache::write(unsigned pc){
    rw_info ret;
    unsigned tag = this->tag(pc);
    unsigned entry = this->entry(pc);
    int at = find_block(pc);
    if(at != -1){
        update_LRU(pc);
        cache_data[entry][at].dirty = 1;
        ret.hit = true;
        return ret;
    }
    if(this->write_alloc){
        ret = push_block(pc);
        at = find_block(pc);
        cache_data[entry][at].dirty = 1;
    }
    ret.hit = false;
    return ret;
}

rw_info Cache::push_block(unsigned pc){
    rw_info ret;
    ret.writeback = false;

    unsigned tag = this->tag(pc);
    unsigned entry = this->entry(pc);
    int at = this->evict_at(entry);

    assert(at != -1);

    CacheEntry* ent = &cache_data[entry][at];

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
    this->update_LRU(pc);

    return ret;
}

int Cache::remove_block(unsigned pc){
    unsigned tag = this->tag(pc);
    unsigned entry = this->entry(pc);
    CacheEntry* ent;
    for(int i = 0; i < cache_data[entry].size(); i++){
        ent = &cache_data[entry][i];
        if(this->tag((ent->pc)) == tag){
            ent->valid = false;
            return ent->dirty;
        }      
    }
    return -1;
}

int Cache::evict_at(unsigned entry){
    for(int i = 0; i < cache_data[entry].size(); i++){
        if(!cache_data[entry][i].LRU_val){
            return i;
        }      
    }
    return -1;
}

//return -1 if block not found, else returns block idx inside entry
int Cache::find_block(unsigned pc){
    unsigned entry = this->entry(pc);
    unsigned tag = this->tag(pc);
    //cout << "find_block entry: " << entry << endl;
    for(int i = 0; i < cache_data[entry].size(); i++){
        if(this->tag(cache_data[entry][i].pc) == tag && cache_data[entry][i].valid)
            return i;
    }
    return -1;
}

//updates LRU if pc exists in the cache
bool Cache::update_LRU(unsigned pc){
    unsigned entry = this->entry(pc);
    unsigned tag = this->tag(pc);
    int cur_LRU = -1, cur_idx = -1;

    CacheEntry* ent;
    for(int i = 0; i < cache_data[entry].size(); i++){
        ent = &cache_data[entry][i];
        if(ent->valid && this->tag(ent->pc) == tag){
            cur_idx = i;
            cur_LRU = ent->LRU_val;
            ent->LRU_val = pow(2, n_ways) - 1;
            break;
        }      
    }
    if(cur_LRU == -1 || cur_idx == -1){
        cout << "wtf";
        return false;
    }


    for(int j = 0; j < cache_data[entry].size(); j++){
        ent = &cache_data[entry][j];
        if(ent->LRU_val > cur_LRU && j != cur_idx)
            ent->LRU_val--;
    }

    return true;
}

bool Cache::is_write_alloc(){ return write_alloc; }

inline unsigned Cache::entry(unsigned pc){
    unsigned set_mask = ~(-1 << set_size);
    return (pc >> (block_size)) & set_mask;
}

inline unsigned Cache::tag(unsigned pc){
    unsigned tag_mask = ~(-1 << tag_size);
    return (pc >> (set_size + block_size)) & tag_mask;
}

void Cache::print_cache(){
    for(auto& entry : cache_data){
        cout << "level:" << endl;
        for(auto& entry : entry){
            cout << "pc: " << entry.pc << " tag: " << this->tag(entry.pc) << " LRU: " << entry.LRU_val
                 << " dirty: " << entry.dirty << " valid: " << entry.valid << endl;
        }
    }
}