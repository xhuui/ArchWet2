#include "cache.hpp"

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

    void access(char type, unsigned pc){
        l1_acc_amount++;
        total_cycles += l1_rw_cycles;
        rw_info info;

        if(type == 'r'){
            if(!(info = L1.read(pc)).hit){
                l1_misses++;
                l2_acc_amount++;
                total_cycles += l2_rw_cycles;
                if(info.writeback)
                    L2.update_LRU(info.pc);
                if(!(info = L2.read(pc)).hit){
                    if(info.removed)
                        L1.remove_block(info.pc);
                    l2_misses++;
                    total_cycles += mem_rw_cycles;
                }
            }
        }

        if(type == 'w'){
            if(!(info = L1.write(pc)).hit){
                l1_misses++;
                l2_acc_amount++;
                total_cycles += l2_rw_cycles;
                if(info.writeback)
                    L2.update_LRU(info.pc);
                if(!(info = L2.write(pc)).hit){
                    if(info.removed)
                        L1.remove_block(info.pc);
                    l2_misses++;
                    total_cycles += mem_rw_cycles;
                }
            }
        }

        // if(type == 'w'){
        //     if(!(info = L1.write(pc)).hit){
        //         l1_misses++;
        //         if(info.writeback)
        //             L2.update_LRU(info.pc);
        //         if(L1.is_write_alloc()){
        //             total_cycles += l2_rw_cycles;
        //             l2_acc_amount++;
        //             if(!(info = L2.write(pc)).hit){
        //                 if(info.removed)
        //                     L1.remove_block(info.pc);
        //                 l2_misses++;
        //                 if(L2.is_write_alloc())
        //                     total_cycles += mem_rw_cycles;
        //             }
        //         }
                
        //     }
        // }

    }

    double cache_misrate(int cache) const {
        double misses = (cache == 1) ? l1_misses : l2_misses;
        double acc_amount = (cache == 1) ? l1_acc_amount : l2_acc_amount;
        return misses / acc_amount;
    }

    double avg_acc_time(){
        return total_cycles / l1_acc_amount;
    }
};