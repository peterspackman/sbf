#include "sbf.h"

int main(void) {
    sbf_File file = sbf_new_file;
    sbf_open(&file, "test.sbf");
    sbf_integer ints[1000];
    sbf_double dubs[1000];
    for (int i = 0; i < 1000; i++)
        ints[i] = i;
    sbf_size shape_ints[SBF_MAX_DIM] = {0};
    sbf_size shape_doubles[SBF_MAX_DIM] = {0};
    shape_ints[0] = 1000;
    shape_doubles[0] = 25;
    shape_doubles[1] = 40;
    sbf_add_dataset(&file, "integer_dataset", SBF_INT, shape_ints, ints);
    sbf_add_dataset(&file, "double_dataset", SBF_DOUBLE, shape_doubles, dubs);
    sbf_write(&file);
    sbf_close(&file);
    return 0;
}
