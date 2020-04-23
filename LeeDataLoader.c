/*
 * Helper functions to load a reference string.
 */

#include "DataLoader.h"

/**
 * Loads a test_scenario strut from a textfile.
 *
 * @param filename The name of the file to load.
 * @return A struct containing the loaded file.
 */
struct test_scenario* load_test_data(char* filename) {
    struct test_scenario *ts = (struct test_scenario *) malloc(sizeof(struct test_scenario));
    ts->frame_count = 0;
    ts->page_count = 0;
    for (int i = 0; i < 512; i++) {
        ts->refstr[i] = 0;
    }
    ts->refstr_len = 0;
    return ts;
}