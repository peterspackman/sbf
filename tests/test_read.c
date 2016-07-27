#include "sbf.h"

int main(void) {
    sbf_File file = sbf_new_file;
    file.mode = SBF_FILE_READONLY;
    file.filename = "test.sbf";
    sbf_open(&file);
    sbf_read(&file);
    sbf_close(&file);
    return 0;
}
