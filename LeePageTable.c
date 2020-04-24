/*
 * Simulates a page table
 */

#include "PageTable.h"

// page table entry
struct page_table_entry {
    /*
     * The number of the last frame that this page was in; if this page is in memory, then the
     * number of the current frame that this page is in.
     * This is initialized to -1 in the beginning.
     */
    int frame_number;
    // indicates that a page is in memory
    unsigned int metadata; // if valid bit == 1, it is in memory
    // the data in the page
    int reference;
};

struct page_table {
    // number of pages to keep track of; this also represents the length of the page_table_entry
    // array
    int page_count;
    // array of page table entries
    struct page_table_entry *pte;
    // number of frames in "memory"
    int frame_count;
    // replacement algorithm to use for page swapping
    enum replacement_algorithm algorithm;
};

char *replacement_algorithm[] = {
        "FIFO",
        "LRU",
        "MFU"
};

/**
 * Creates a new page table object. Returns a pointer to created page table.
 *
 * @param page_count Number of pages.
 * @param frame_count Numbers of frames.
 * @param algorithm Page replacement algorithm
 * @param verbose Enables showing verbose table contents.
 * @return A page table object.
 */
struct page_table *page_table_create(int page_count, int frame_count,
                                     enum replacement_algorithm algorithm, int verbose) {
    struct page_table *pt = (struct page_table *) malloc(sizeof(struct page_table));
    pt->page_count = page_count;
    pt->frame_count = frame_count;
    pt->algorithm = algorithm;
    pt->pte = (struct page_table_entry *) malloc(sizeof(struct page_table_entry) * page_count);
    for (int i = 0; i < page_count; ++i) {
        pt->pte[i].frame_number = -1;
        pt->pte[i].metadata = 0;
        pt->pte[i].reference = 0;
    }
    if (verbose) {
        printf("Created page_table{page_count=%d, frame_count=%d, replacement_algorithm=%s}\n",
                pt->page_count, pt->frame_count, replacement_algorithm[algorithm]);
    }
    return pt;
}

/**
 * Destorys an existing page table object. Sets outside variable to NULL.
 *
 * @param pt A page table object.
 */
void page_table_destroy(struct page_table **pt) {
    free((*pt)->pte);
    free(*pt);
}

/**
 * Simulates an instruction accessing a particular page in the page table.
 *
 * @param pt A page table object.
 * @param page The page being accessed.
 */
void page_table_access_page(struct page_table *pt, int page) {}

/**
 * Displays page table replacement algorithm, number of page faults, and the
 * current contents of the page table.
 *
 * @param pt A page table object.
 */
void page_table_display(struct page_table *pt) {}

/**
 * Displays the current contents of the page table.
 *
 * @param pt A page table object.
 */
void page_table_display_contents(struct page_table *pt) {}
