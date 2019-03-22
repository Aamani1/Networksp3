#include "types.h"
#include "pagesim.h"
#include "paging.h"
#include "swapops.h"
#include "stats.h"
#include "util.h"

pfn_t select_victim_frame(void);


/*  --------------------------------- PROBLEM 7 --------------------------------------
    Checkout PDF section 7 for this problem
    
    Make a free frame for the system to use.

    You will first call the page replacement algorithm to identify an
    "available" frame in the system.

    In some cases, the replacement algorithm will return a frame that
    is in use by another page mapping. In these cases, you must "evict"
    the frame by using the frame table to find the original mapping and
    setting it to invalid. If the frame is dirty, write its data to swap!
 * ----------------------------------------------------------------------------------
 */
pfn_t free_frame(void) {
    pfn_t victim_pfn;

    /* Call your function to find a frame to use, either one that is
       unused or has been selected as a "victim" to take from another
       mapping. */
    victim_pfn = select_victim_frame();

    /*
     * If victim frame is currently mapped, we must evict it:
     *
     * 1) Look up the corresponding page table entry
     * 2) If the entry is dirty, write it to disk with swap_write()
     * 3) Mark the original page table entry as invalid
     * 4) Unmap the corresponding frame table entry
     *
     */
    if (frame_table[victim_pfn].mapped) {
        fte_t *frameTableEntry = (fte_t*) (frame_table + victim_pfn);  //get the frame of the victim from frame table
        pte_t *pageTable = (pte_t *)(mem +  frameTableEntry->process->saved_ptbr * PAGE_SIZE); //get the page table of victim process
        pte_t *pageTableEntry = (pte_t *)(pageTable + frame_table[victim_pfn].vpn); //the page of the victim prcoess from page table

        if (pageTableEntry-> dirty) {    //if the page is dirty
            
            stats.writebacks++;        //increment write backs because we are writing to disk (I/O queue)
            swap_write(pageTableEntry, (mem + victim_pfn * PAGE_SIZE));
            pageTableEntry-> dirty = 0;    //page is not dirty anymore
        }
        
        pageTableEntry->valid = 0; //not in the frame anymore
        frame_table[victim_pfn].mapped = 0; //not in use
    }


    /* Return the pfn */
    return victim_pfn;
}



/*  --------------------------------- PROBLEM 9 --------------------------------------
    Checkout PDF section 7, 9, and 11 for this problem

    Finds a free physical frame. If none are available, uses either a
    randomized or FIFO algorithm to find a used frame for
    eviction.

    Return:
        The physical frame number of a free (or evictable) frame.

    HINTS: Use the global variables MEM_SIZE and PAGE_SIZE to calculate
    the number of entries in the frame table.
    ----------------------------------------------------------------------------------
*/
pfn_t select_victim_frame() {
    /* See if there are any free frames first */
    size_t num_entries = MEM_SIZE / PAGE_SIZE;
    for (size_t i = 0; i < num_entries; i++) {
        if (!frame_table[i].protected && !frame_table[i].mapped) {
            return i;
        }
    }

    if (replacement == RANDOM) {
        /* Play Russian Roulette to decide which frame to evict */
        pfn_t last_unprotected = NUM_FRAMES;
        for (pfn_t i = 0; i < num_entries; i++) {
            if (!frame_table[i].protected) {
                last_unprotected = i;
                if (prng_rand() % 2) {
                    return i;
                }
            }
        }
        /* If no victim found yet take the last unprotected frame
           seen */
        if (last_unprotected < NUM_FRAMES) {
            return last_unprotected;
        }
    } else if (replacement == FIFO) {
        /* Implement a FIFO algorithm here */
        // timestamp_t longest = get_current_timestamp();
        // pfn_t best_frame = 0;
        // timestamp_t best_time = frame_table[0].timestamp;

        // //pfn_t last_unprotected = NUM_FRAMES;
        // for (pfn_t i = 0; i < num_entries; i++) {
        //     if (!frame_table[i].protected && frame_table[i].timestamp < best_time) {
        //         best_frame = i;
        //         best_time = frame_table[i].timestamp;
        //     }
                
        // }
        // /* If no victim found yet take the last unprotected frame
        //    seen */
        // if (best_frame < NUM_FRAMES) {
        //     return best_frame;
        // }
        timestamp_t longest = get_current_timestamp();
        pfn_t head = 0;
        for (pfn_t i = 0; i < NUM_FRAMES; i++) {
            if ((frame_table[i].timestamp < longest) && !frame_table[i].protected) {
                longest = frame_table[i].timestamp;
            head = i;
            }
        }
        if (head > 0) {
            return head;
        }
    }

    } else if (replacement == CLOCKSWEEP) {
        /* Optionally, implement the clocksweep algorithm here */
        // while (1) {
        //     clock = clock % NUM_FRAMES;
        //     if (!frame_table[clock].protected) {
        //         if (frame_table[clock].referenced) {
        //             frame_table[clock].referenced = 0;
        //         } else {
        //             clock++;
        //             return clock - 1;
        //         }
        //     }
        //     clock++;
        // }
    }

    /* If every frame is protected, give up. This should never happen
       on the traces we provide you. */
    panic("System ran out of memory\n");
    exit(1);
}
