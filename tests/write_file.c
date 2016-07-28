#include "sbf.h"

int main(void) {
    sbf_File file = sbf_new_file;
    file.mode = SBF_FILE_WRITEONLY;
    file.filename = "test.sbf";

    sbf_open(&file);

    sbf_integer ints[1000];
    for (int i = 0; i < 1000; i++) {
        ints[i] = i * i;
    }

    sbf_size shape_ints[SBF_MAX_DIM] = {0};

    shape_ints[0] = 1000;
    sbf_add_dataset(&file, "integer_dataset", SBF_INT, shape_ints, ints);
    sbf_write(&file);
    sbf_close(&file);
    return 0;
}
