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
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Cannot open file %s\n", filename);
        return NULL;
    }

    int num_matched = fscanf(fp, "%d", &(ts->page_count));
    if (num_matched != 1) {
        printf("Read of number of pages failed!\n");
        return NULL;
    }

    num_matched = fscanf(fp, "%d", &(ts->frame_count));
    if (num_matched != 1) {
        printf("Read of number of frames failed!\n");
        return NULL;
    }

    int refstr_len;
    num_matched = fscanf(fp, "%d", &(refstr_len));
    if (num_matched != 1) {
        printf("Read of number of entries failed!\n");
        return NULL;
    }

    ts->refstr_len = refstr_len;
    for (int i = 0; i < refstr_len; i++) {
        num_matched = fscanf(fp, "%d", &(ts->refstr[i]));
        if (num_matched != 1) {
            printf("Read of reference string failed!\n");
            return NULL;
        }
    }

    fclose(fp);

    return ts;
}