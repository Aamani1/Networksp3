#include "paging.h"
#include "pagesim.h"
#include "page_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

 /* The frame table pointer. You will set this up in system_init. */
fte_t *frame_table;

/*  --------------------------------- PROBLEM 2 --------------------------------------
    Checkout PDF section 4 for this problem

    In this problem, you will initialize frame_table. This is the pointer to the first
    frame table entry.

    The frame table will be located at physical address 0 in our simulated
    memory. You will first assign the frame_table global variable to point to
    this location in memory. You should zero out this first frame table entry
    (remember, size of a frame table entry is PAGESIZE), in case for
    any reason physical memory is not clean.

    You should then mark the first entry in the frame table as protected. We do
    this because we do not want our free frame allocator to give out the frame
    used by the frame table.

    HINTS:
        You will need to use the following global variables:
        - mem: Simulated physical memory already allocated for you.
        - PAGE_SIZE: The size of one page.
        You will need to initialize (set) the following global variable:
        - frame_table: a pointer to the first entry of the frame table

    -----------------------------------------------------------------------------------
*/
void system_init(void) {
    /*
     * 1. Set the frame table pointer to point to the first frame in physical
     * memory. Zero out the memory used by the frame table.
     *
     * Address "0" in memory will be used for the frame table. This table will
     * contain n frame table entries (fte_t), where n is the number of
     * frames in memory. The frame table will be useful later if we need to
     * evict pages during page faults.
     */

    frame_table = (fte_t *) mem;          //getting space from memory for frame table
    memset(frame_table, 0, PAGE_SIZE);    //clearing the frame to 0


    /*
     * 2. Mark the first frame table entry as protected.
     *
     * The frame table contains entries for all of physical memory,
     * however, there are some frames we never want to evict.
     * We mark these special pages as "protected" to indicate this.
     */

    frame_table -> protected = 1;         //setting the frame protected

}

/*  --------------------------------- PROBLEM 3 --------------------------------------
    Checkout PDF section 4 for this problem

    This function gets called every time a new process is created.
    You will need to allocate a new page table for the process in memory using the
    free_frame function so that the process can store its page mappings. Then, you
    will need to store the PFN of this page table in the process's PCB.

    HINTS:
        - Look at the pcb_t struct defined in pagesim.h to know what to set inside.
        - You are not guaranteed that the memory returned by the free frame allocator
        is empty - an existing frame could have been evicted for our new page table.
        - As in the previous problem, think about whether we need to mark any entries
        in the frame_table as protected after we allocate memory for our page table.
    -----------------------------------------------------------------------------------
*/
void proc_init(pcb_t *proc) {
    /*
     * 1. Call the free frame allocator (free_frame) to return a free frame for
     * this process's page table. You should zero-out the memory.
     */
    // pfn_t frameNumber = free_frame();                       //getting an empty frame and naming it as a frame number
    // memset(mem + (frameNumber * PAGE_SIZE), 0, PAGE_SIZE); //going to that particular frame and clearing out values to 0

    pfn_t dip = free_frame();
    pte_t* page_table = (pte_t*) (mem + (dip * PAGE_SIZE));
    memset(page_table, 0, PAGE_SIZE);

    /*
     * 2. Update the process's PCB with the frame number
     * of the newly allocated page table.
     *
     * Additionally, mark the frame's frame table entry as protected. You do not
     * want your page table to be accidentally evicted.
     */
    // proc -> saved_ptbr = frameNumber;               //for the current process saves the processes' page table to the frame number.
    // (frame_table + frameNumber) -> protected = 1;      //setting the processes frame protected
    (*proc).saved_ptbr = dip;
    frame_table[dip].protected = 1;
    frame_table[dip].process = proc;
}

/*  --------------------------------- PROBLEM 4 --------------------------------------
    Checkout PDF section 5 for this problem
    
    Swaps the currently running process on the CPU to another process.

    Every process has its own page table, as you allocated in proc_init. You will
    need to tell the processor to use the new process's page table.

    HINTS:
        - Look at the global variables defined in pagesim.h. You may be interested in
        the definition of pcb_t as well.
    -----------------------------------------------------------------------------------
 */
void context_switch(pcb_t *proc) {
    PTBR = proc -> saved_ptbr;      /* PTBR = page table base register. It tells where to look to find the page table for the currently 
                                    running process. Now, we are setting PTBR to the curent proc page table. */

}

/*  --------------------------------- PROBLEM 5 --------------------------------------
    Checkout PDF section 6 for this problem
    
    Takes an input virtual address and returns the data from the corresponding
    physical memory address. The steps to do this are:

    1) Translate the virtual address into a physical address using the page table.
    2) Go into the memory and read/write the data at the translated address.

    Parameters:
        1) address     - The virtual address to be translated.
        2) rw          - 'r' if the access is a read, 'w' if a write
        3) data        - One byte of data to write to our memory, if the access is a write.
                         This byte will be NULL on read accesses.

    Return:
        The data at the address if the access is a read, or
        the data we just wrote if the access is a write.

    HINTS:
        - You will need to use the macros we defined in Problem 1 in this function.
        - You will need to access the global PTBR value. This will tell you where to
        find the page table. Be very careful when you think about what this register holds!
        - On a page fault, simply call the page_fault function defined in page_fault.c.
        You may assume that the pagefault handler allocated a page for your address
        in the page table after it returns.
        - Make sure to set the dirty bit in the page table entry if it's a write.
        - Make sure to update the stats variables correctly (see stats.h)
    -----------------------------------------------------------------------------------
 */
uint8_t mem_access(vaddr_t address, char rw, uint8_t data) {


    /* Split the address and find the page table entry.
       Remember to keep a pointer to the entry so you can modify it later. */
    // pte_t * PageTable = (pte_t *) (mem + (PTBR * PAGE_SIZE));         //getting the page table of the current process.

    // vpn_t vpn = vaddr_vpn(address);                                   //getting the vpn of the address.
    // uint16_t offset = vaddr_offset(address);                          //getting the offset of the address.

    // pte_t * entry = (pte_t *) (PageTable + vpn);                      //going into the actual page number in the page table.

    uint16_t vp = vaddr_vpn(address);
    pte_t* entry = (pte_t*) (mem + (PTBR * PAGE_SIZE)) + vp;

    /* If an entry is invalid, just page fault to allocate a page for the page table. */
    // if (!(entry -> valid)) {

    //     page_fault(address);
    // }
    if ((*entry).valid == 0) {
        page_fault(address);
        stats.page_faults += 1;
    }

    // pfn_t pfn = PageTable[vpn].pfn;                                 //getting the pfn from vpn through page table stored in frame
    // fte_t * FrameTableEntry = (fte_t *) (frame_table + pfn);        // finding the frame associated with the pfn from the page

    // FrameTableEntry -> timestamp = get_current_timestamp();
     frame_table[(*entry).pfn].referenced = 1;
    /*
        The physical address will be constructed like this:
        -------------------------------------
        |     PFN    |      Offset          |
        -------------------------------------
        where PFN is the value stored in the page table entry.
        We need to calculate the number of bits are in the offset.

        Create the physical address using your offset and the page
        table entry.

        Also, find the frame table entry corresponding to the VPN,
        and make sure set any relevant values.
    */

    uint16_t offset = vaddr_offset(address);
    paddr_t pAddress = (paddr_t) (((*entry).pfn * PAGE_SIZE) + offset);
    stats.accesses += 1;

    // paddr_t physicalAddress = ((paddr_t) pfn << OFFSET_LEN) | offset;     //calculating the physical address form offset and pfn
    /* Either read or write the data to the physical address
       depending on 'rw' */

    // if (rw == 'r') {
    //     stats.reads ++;                     // increment reading

    // } else {
    //     entry -> dirty = 1;                 //the disk needs to get the change of data that we write and so is dirty.
    //     stats.writes ++;                    //increment writing
    //     mem[physicalAddress] = data;        //saving data in memory

    // }

    // /* Return the data read/written */
    // return mem[physicalAddress];
    if (rw == 'r') {
        stats.reads += 1;
        return mem[pAddress];
    } else {
        mem[pAddress] = data;
        stats.writes += 1;
        (*entry).dirty = 1;
        return data;
    }
}

/*  --------------------------------- PROBLEM 8 --------------------------------------
    Checkout PDF section 8 for this problem
    
    When a process exits, you need to free any pages previously occupied by the
    process. Otherwise, every time you closed and re-opened Microsoft Word, it
    would gradually eat more and more of your computer's usable memory.

    To free a process, you must clear the "mapped" bit on every page the process
    has mapped. If the process has swapped any pages to disk, you must call
    swap_free() using the page table entry pointer as a parameter.

    You must also clear the "protected" bits for the page table itself.
    -----------------------------------------------------------------------------------
*/
void proc_cleanup(pcb_t *proc) {
    /* Look up the process's page table */
    // pte_t *currentPageTable = (pte_t*)(mem + proc->saved_ptbr * PAGE_SIZE);   // get the page table of the current process

    // pte_t *pageTabaleEntry;
    // fte_t *frameTableEntry;
    
    // /* Iterate the page table and clean up each valid page */
    // for (size_t i = 0; i < NUM_PAGES; i++) {
    //     pageTabaleEntry = currentPageTable + i;    // accessing each page in the page table

    //     if (pageTabaleEntry->valid) {

    //         frameTableEntry = (fte_t*) (frame_table + pageTabaleEntry-> pfn); //getting the frame for each page
    //         frameTableEntry -> mapped = 0;   // not in use (delete the link between page and frame.)
    //     } 

    //     if (pageTabaleEntry->swap) {
    //         swap_free(pageTabaleEntry);   // swap if needed.
    //     }
    // }

    // /* Free the page table itself in the frame table */
    // (frame_table+ (proc -> saved_ptbr)) -> protected = 0; //can be reused
    /* Look up the process's page table */
    pte_t* page_table = (pte_t*) (mem + ((*proc).saved_ptbr * PAGE_SIZE));
    /* Iterate the page table and clean up each valid page */
    for (size_t i = 0; i < NUM_PAGES; i++) {
        pfn_t pfn = page_table[i].pfn;
        if (page_table[i].valid == 1) {
            frame_table[pfn].mapped = 0;
        }
        page_table[i].valid = 0;
        if (swap_exists((pte_t*) &page_table[i])) {
            swap_free((pte_t*) &page_table[i]);
        }
    }

    /* Free the page table itself in the frame table */
    frame_table[(*proc).saved_ptbr].mapped = 0;
    frame_table[(*proc).saved_ptbr].protected = 0;
}

#pragma GCC diagnostic pop
