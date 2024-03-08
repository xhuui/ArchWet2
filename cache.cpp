#include "cache.hpp"
#include <iostream>
#include <math.h>
#include <assert.h>


int Cache::read(unsigned pc){
    unsigned tag = this->tag(pc);
    unsigned level = this->level(pc);
    //cout << "read - level: " << level << " tag: " << tag << endl;

    if(int at = find_block(pc) != -1){
        return true;
        if(cache_data[level][at].dirty == 0)
            return true;
        //cache_data[level][at].dirty == 0;
        //return false;
    }
        
    push_block(pc);
    return false;
}

int Cache::write(unsigned pc){
    unsigned tag = this->tag(pc);
    unsigned level = this->level(pc);
    int idx = find_block(pc);
    if(idx != -1){
        cache_data[level][idx].dirty = 1;
        return true;
    }
    if(this->write_alloc){
        push_block(pc);
    }
    return false;
}

Cache::Cache(int cache_size, int block_size, int n_ways, bool write_alloc){
    this->cache_size = cache_size;
    this->block_size = block_size;
    this->n_ways = n_ways;
    this->write_alloc = write_alloc;

    this->set_size = cache_size - block_size - n_ways;
    this->tag_size = 32 - block_size - set_size;

    cache_data = vector<vector<Cachelevel>>(pow(2, set_size), 
                                            vector<Cachelevel>(pow(2, n_ways)));
}

void Cache::push_block(unsigned pc){
    unsigned tag = this->tag(pc);
    unsigned level = this->level(pc);
    int at = this->evict_at(level);

    assert(at != -1);

    Cachelevel* ent = &cache_data[level][at];

    ent->tag = tag;
    ent->dirty = false;
    ent->valid = true;

    //cout << "level to push: " << level << " tag to push: " << ent->tag << endl;
    
    this->update_LRU(pc);
}

int Cache::evict_at(unsigned level){
    for(int i = 0; i < cache_data[level].size(); i++){
        if(!cache_data[level][i].LRU_val){
            return i;
        }      
    }
    return -1;
}

//return -1 if block not found, else returns block idx inside level
int Cache::find_block(unsigned pc){
    unsigned level = this->level(pc);
    unsigned tag = this->tag(pc);
    for(int i = 0; i < cache_data[level].size(); i++){
        if(cache_data[level][i].tag == tag && cache_data[level][i].valid)
            return i;
    }
    return -1;
}

//use only after pushing in a new block!
void Cache::update_LRU(unsigned pc){
    unsigned level = this->level(pc);
    unsigned tag = this->tag(pc);
    //cout << "update_LRU level: " << level << " tag: " << tag << endl; 
    int cur_LRU = -1, cur_idx = -1;

    Cachelevel* ent;
    //cout << "cache_data size: " << cache_data[level].size() << endl;
    for(int i = 0; i < cache_data[level].size(); i++){
        ent = &cache_data[level][i];
        if(ent->tag == tag){
            cur_idx = i;
            cur_LRU = ent->LRU_val;
            ent->LRU_val = pow(2, n_ways) - 1;
            break;
        }      
    }

    assert(cur_LRU != -1);
    assert(cur_idx != -1);

    for(int j = 0; j < cache_data[level].size(); j++){
        ent = &cache_data[level][j];
        if(ent->LRU_val > cur_LRU && j != cur_idx)
            ent->LRU_val--;
    }
}

inline unsigned Cache::level(unsigned pc){
    unsigned set_mask = ~(-1 << set_size);
    return (pc >> (block_size)) & set_mask;
}

inline unsigned Cache::tag(unsigned pc){
    unsigned tag_mask = ~(-1 << tag_size);
    return (pc >> (set_size + block_size)) & tag_mask;
}

void Cache::print_cache(){
    for(auto& level : cache_data){
        cout << "level:" << endl;
        for(auto& level : level){
            cout << "tag: " << level.tag << " LRU: " << level.LRU_val
                 << " dirty: " << level.dirty << " valid: " << level.valid << endl;
        }
    }
}

int Cache::get_acc_time(){return rw_cycles;}

// int main(){
//     Cache c1(4, 3, 1, false);
//     cout << "set size: " << c1.set_size << endl;
//     cout << "tag size: " << c1.tag_size << endl;
//     cout << "block size: " << c1.block_size << endl;

//     c1.read(0);
//     cout << c1.tag(0) << endl;
//     cout << c1.tag(4) << endl;
//     cout << c1.tag(1048576) << endl;
//     cout << c1.tag(0) << endl;
//     cout << c1.tag(12) << endl;

//     cout << c1.tag(16) << endl;
//     cout << c1.tag(28) << endl;
//     cout << c1.tag(32) << endl;
//     cout << c1.tag(44) << endl;
//     cout << c1.tag(48) << endl;

//     cout << c1.tag(60) << endl;
//     cout << c1.tag(64) << endl;
//     cout << c1.tag(8) << endl;
//     cout << c1.tag(68) << endl;

//     c1.print_cache();
    
// }