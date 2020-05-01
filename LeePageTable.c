/*
 * Simulates a page table
 */

#include <limits.h>
#include "PageTable.h"

static const int EMPTY = -1;
static const unsigned int VALID_BIT = 0;

// page table entry
struct page_table_entry {
    // the page number
    int page_number;
    // the last frame this entry was in
    int frame_number;
    // indicates that a page is in memory
    unsigned int metadata; // if valid bit == 1, it is in memory
};

// Used for FIFO page swapping
struct page_queue {
    int front, rear, size;
    unsigned capacity;
    struct page_table_entry *array;
};

struct page_table {
    /*
     * Number of pages to keep track of; this also represents the length of the page_table_entry
     * array
     */
    int page_count;
    // array of all page table entries
    struct page_table_entry *entries;
    // number of frames in "memory"
    int frame_count;
    // array of frames used
    int *frames;
    // replacement algorithm to use for page swapping
    enum replacement_algorithm algorithm;
    int faults;
    // FIFO queue
    struct page_queue *fifo_queue;
    // number of times a frame was accessed
    int *frame_accesses;
};

char *replacement_algorithm[] = {
        "FIFO",
        "LRU",
        "MFU"
};

// Queue functions

/**
 * Function to create a queue of given capacity.
 * It initializes size of queue as 0.
 */
struct page_queue *create_queue(unsigned capacity) {
    struct page_queue *queue = (struct page_queue *) malloc(sizeof(struct page_queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;  // This is important, see the enqueue
    queue->array = (struct page_table_entry *) malloc(queue->capacity * sizeof(struct page_table_entry));
    return queue;
}

/**
 * Queue is full when size becomes equal to the capacity
 */
int is_full(struct page_queue *queue) { return (queue->size == queue->capacity); }

/**
 * Queue is empty when size is 0
 * @param queue the queue to check
 * @return nonzero if queue is empty
 */
int is_empty(struct page_queue *queue) { return (queue->size == 0); }

/**
 * Function to add an item to the queue.
 * It changes rear and size.
 * @param queue the queue to add to
 * @param item the item to add
 */
void enqueue(struct page_queue *queue, struct page_table_entry item) {
    if (is_full(queue)) {
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

/**
 * Function to remove an item from queue.
 * It changes front and size
 * @param queue the queue to remove from
 * @return the item that was removed
 */
struct page_table_entry *dequeue(struct page_queue *queue) {
    if (is_empty(queue)) {
        return NULL;
    }
    struct page_table_entry *item = &(queue->array[queue->front]);
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// - End of queue functions

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
    pt->algorithm = algorithm;
    pt->faults = 0;
    pt->entries = (struct page_table_entry *) malloc(sizeof(struct page_table_entry) * page_count);
    for (int i = 0; i < page_count; ++i) {
        pt->entries[i].page_number = i;
        pt->entries[i].metadata = 0;
        pt->entries[i].frame_number = EMPTY;
    }

    pt->frame_count = frame_count;
    pt->frames = (int *) malloc(sizeof(int) * frame_count);
    // initialize all frames to EMPTY
    for (int i = 0; i < frame_count; i++) {
        pt->frames[i] = EMPTY;
    }
    if (algorithm == FIFO) {
        // create a FIFO queue of size frame_count
        pt->fifo_queue = create_queue(frame_count);
    } else if (algorithm == LRU) {
        // create a tracker for how many times a frame has been accessed
        pt->frame_accesses = (int *) malloc(sizeof(int) * frame_count);
        // initialize all frame accesses to 0
        for (int i = 0; i < frame_count; i++) {
            pt->frame_accesses[i] = 0;
        }
    }
    if (verbose) {
        printf("Created page_table{page_count=%d, frame_count=%d, replacement_algorithm=%s}\n",
               pt->page_count, pt->frame_count, replacement_algorithm[algorithm]);
    }
    return pt;
}

void swap_fifo(const struct page_table *pt, int page);

/**
 * Place the specified page in memory
 * @param pt the page table
 * @param page the page to place in memory
 * @param frame the frame number to "put" the page in
 */
void place_in_memory(const struct page_table *pt, int page, int frame) {
    pt->frames[frame] = page;   // store the page in the used frames array
    pt->entries[page].frame_number = frame; // store the frame number in the page table entry
    pt->entries[page].metadata |= 1UL << VALID_BIT; // set the VALID bit
    if (pt->algorithm == FIFO) {
        enqueue(pt->fifo_queue, pt->entries[page]);  // put the page in the FIFO queue
    } else if (pt->algorithm == LRU) {
        (pt->frame_accesses[frame])++;  // increment the counter of accesses for frame
    }
}

/**
 * Destorys an existing page table object. Sets outside variable to NULL.
 *
 * @param pt A page table object.
 */
void page_table_destroy(struct page_table **pt) {
    free((*pt)->entries);
    free((*pt)->frames);
    if ((*pt)->algorithm == FIFO) {
        free((*pt)->fifo_queue);
    } else if ((*pt)->algorithm == LRU) {
        free((*pt)->frame_accesses);
    }
    free(*pt);
}

/**
 * Return true if a bit is set.
 * @param n number with the bit to check
 * @param bit the bit to check
 * @return true if a bit is set
 */
unsigned is_bit_set(unsigned n, unsigned bit) {
    return (n & (1ul << bit));
}

unsigned clear_bit(unsigned n, unsigned bit) {
    return (n & ~(1ul << VALID_BIT));
}

/**
 * Swap the page into memory by replacing the oldest page.
 * @param pt the page table
 * @param page the page number
 */
void swap_fifo(const struct page_table *pt, int page) {
    struct page_table_entry *fi_page = dequeue(pt->fifo_queue);

    // retrieve the original page table entry
    struct page_table_entry dequeued = pt->entries[fi_page->page_number];
    dequeued.metadata &= ~(1ul << VALID_BIT); // clear the bit
    pt->entries[fi_page->page_number] = dequeued;   // save into the entries

    place_in_memory(pt, page, fi_page->frame_number);
}

/**
 * Swap the page into memory by replacing the least recently used page.
 * @param pt the page table
 * @param page the page number
 */
void swap_lru(const struct page_table *pt, int page) {
    int lru_frame = 0;
    for (int i = 0; i < pt->frame_count; ++i) {
        if (pt->frame_accesses[i] < pt->frame_accesses[lru_frame]) {
            lru_frame = i;
        }
    }

    // retrieve the original page table entry
    int lru_page = pt->frames[lru_frame];
    struct page_table_entry lru_entry = pt->entries[lru_page];
    lru_entry.metadata = clear_bit(lru_entry.metadata, VALID_BIT); // clear the bit
    pt->entries[lru_page] = lru_entry;   // save into the entries

    place_in_memory(pt, page, lru_page);
}

/**
 * Simulates an instruction accessing a particular page in the page table.
 *
 * @param pt A page table object.
 * @param page The page being accessed.
 */
void page_table_access_page(struct page_table *pt, int page) {
    // if page is already in memory, return
    struct page_table_entry entry = pt->entries[page];
    if (entry.frame_number != EMPTY && is_bit_set(entry.metadata, VALID_BIT)) {
        if (pt->algorithm == LRU) {
            (pt->frame_accesses[entry.frame_number])++;
        }
        return;
    }
    // if you reached this point, that means there's a page fault
    (pt->faults)++;
    // see if there is a free frame
    for (int i = 0; i < pt->frame_count; ++i) {
        if (pt->frames[i] == EMPTY) {
            place_in_memory(pt, page, i);
            return;
        }
    }
    // if you reached this point, there was no empty slot
    if (pt->algorithm == FIFO) {
        swap_fifo(pt, page);
    } else if (pt->algorithm == LRU) {
        swap_lru(pt, page);
    }
}

/**
 * Displays page table replacement algorithm, number of page faults, and the
 * current contents of the page table.
 *
 * @param pt A page table object.
 */
void page_table_display(struct page_table *pt) {
    printf("==== Page Table ====\n");
    printf("Mode : %s\n", replacement_algorithm[pt->algorithm]);
    printf("Page Faults : %d\n", pt->faults);
    page_table_display_contents(pt);
}

/**
 * Displays the current contents of the page table.
 *
 * @param pt A page table object.
 */
void page_table_display_contents(struct page_table *pt) {
    printf("page frame | dirty valid\n");
    for (int i = 0; i < pt->page_count; ++i) {
        printf("%4d %4d | %5d %5d\n", i, pt->entries[i].frame_number, 0, pt->entries[i].metadata);
    }
}
