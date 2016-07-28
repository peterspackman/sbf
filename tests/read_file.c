#include "sbf.h"
#include <assert.h>

int main(void) {
    sbf_File file = sbf_new_file;
    file.mode = SBF_FILE_READONLY;
    file.filename = "test.sbf";
    sbf_open(&file);
    sbf_read_headers(&file);
    assert(file.n_datasets == 1);

    sbf_DataHeader integer_dataset = file.datasets[0];
    assert(strncmp(integer_dataset.name, "integer_dataset", SBF_NAME_LENGTH) ==
           0);
    assert(sbf_datatype_size(integer_dataset) == sizeof(sbf_integer));

    int size = integer_dataset.shape[0];
    int dataset[size];
    sbf_read_dataset(&file, integer_dataset, dataset);
    for (int i = 0; i < size; i++) {
        assert(i * i == dataset[i]);
    }

    sbf_close(&file);
    return 0;
}
