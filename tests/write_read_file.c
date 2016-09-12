#include "sbf.h"
#include "unit_test.h"

int tests_run = 0;
const char *test_filename = "/tmp/sbf_test_c.sbf";

static char *test_write() {
    sbf_File file = sbf_new_file;
    file.mode = SBF_FILE_WRITEONLY;
    file.filename = test_filename;
    sbf_result res;

    res = sbf_open(&file);
    assert("opening file not successful", res == SBF_RESULT_SUCCESS);

    sbf_integer ints[1000];
    for (int i = 0; i < 1000; i++) {
        ints[i] = i * i;
    }

    sbf_size shape_ints[SBF_MAX_DIM] = {0};

    shape_ints[0] = 1000;
    res = sbf_add_dataset(&file, "integer_dataset", SBF_INT, shape_ints, ints);
    assert("adding dataset unsuccessful", res == SBF_RESULT_SUCCESS);
    res = sbf_write(&file);
    assert("writing file unsuccessful", res == SBF_RESULT_SUCCESS);
    res = sbf_close(&file);
    assert("closing file unsuccessful", res == SBF_RESULT_SUCCESS);
    return 0;
}

static char *test_read() {
    sbf_File file = sbf_new_file;
    file.mode = SBF_FILE_READONLY;
    file.filename = test_filename;
    sbf_result res;
    res = sbf_open(&file);
    assert("opening file not successful", res == SBF_RESULT_SUCCESS);

    res = sbf_read_headers(&file);
    assert("reading headers not successful", res == SBF_RESULT_SUCCESS);

    assert("incorrect number of datasets in file", file.n_datasets == 1);

    sbf_DataHeader integer_dataset = file.datasets[0];
    assert("incorrect dataset name",
           strncmp(integer_dataset.name, "integer_dataset", SBF_NAME_LENGTH) ==
               0);
    assert("incorrect datatype size",
           sbf_datatype_size(integer_dataset) == sizeof(sbf_integer));

    int size = integer_dataset.shape[0];
    int dataset[size];
    res = sbf_read_dataset(&file, integer_dataset, dataset);
    assert("reading dataset not successful", res == SBF_RESULT_SUCCESS);
    int num_differences = 0;
    for (int i = 0; i < size; i++) {
        if (i * i != dataset[i])
            num_differences++;
    }
    assert("datasets contain different values", num_differences == 0);
    res = sbf_close(&file);
    assert("closing file unsuccessful", res == SBF_RESULT_SUCCESS);
    return 0;
}

static char *all_tests() {
    run_unit_test(test_write);
    run_unit_test(test_read);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
