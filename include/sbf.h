#pragma once
/*
 * sbf.h
 *
 * A simple binary format for storing data.
 * SBF files are designed to be as braindead as possible.
 *
 */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define sbf_print_error(...) fprintf(stderr, __VA_ARGS__)
#define FAIL_IF_NULL(arg)                                                      \
    if (arg == NULL)                                                           \
    return SBF_RESULT_NULL_FAILURE
#define SBF_MAX_DIM 16
#define SBF_MAX_DATASETS 32
#define SBF_NAME_LENGTH 32

// DATA TYPES
typedef uint64_t sbf_size;
typedef int32_t sbf_integer;
typedef int64_t sbf_long;
typedef float sbf_float;
typedef float sbf_double;
typedef char sbf_character;
// complex data types
typedef struct {
    sbf_float re;
    sbf_float im;
} sbf_complex_float;
typedef struct {
    sbf_double re;
    sbf_double im;
} sbf_complex_double;
// DATA TYPE FLAGS
typedef enum {
    SBF_INT = 1,
    SBF_LONG,
    SBF_FLOAT,
    SBF_DOUBLE,
    SBF_CFLOAT,
    SBF_CDOUBLE
} sbf_data_type;

// RESULT TYPE FLAGS
typedef enum {
    SBF_RESULT_SUCCESS = 1,
    SBF_RESULT_FILE_OPEN_FAILURE,
    SBF_RESULT_FILE_CLOSE_FAILURE,
    SBF_RESULT_WRITE_FAILURE,
    SBF_RESULT_READ_FAILURE,
    SBF_RESULT_NULL_FAILURE,
    SBF_RESULT_MAX_DATASETS_EXCEEDED_FAILURE
} sbf_result;

typedef struct {
    sbf_character token[4];
    sbf_character version_string[5];
    sbf_size n_datablocks;
} sbf_FileHeader;

typedef struct {
    sbf_character name[SBF_NAME_LENGTH]; // what is the name of this dataset
    sbf_data_type data_type;             // how big is each block of data
    sbf_size shape[SBF_MAX_DIM];         // how many blocks of data do we have
} sbf_DataHeader;

typedef struct {
    FILE *fp;
    sbf_size n_datasets;
    sbf_DataHeader datasets[SBF_MAX_DATASETS];
    void *dataset_pointers[SBF_MAX_DATASETS];
} sbf_File;

static const sbf_File sbf_new_file = {.n_datasets = 0};
static const sbf_FileHeader sbf_new_file_header = {
    .token = {'S', 'B', 'F', '\0'},
    .version_string = {'0', '.', '0', '.', '1'},
    .n_datablocks = 0};
static const sbf_DataHeader sbf_new_data_header = {
    .name = {0}, .data_type = 0, .shape = {0},
};

/*
 * Open 'filename', placing the resultant file pointer into 'sbf_file'
 *
 * Returns SBF_RESULT_SUCCESS if we opened the file, or corresponding error
 * values
 * if the file could not be opened.
 */
sbf_result sbf_open(sbf_File *sbf_file, const char *filename) {
    FAIL_IF_NULL(sbf_file);

    const char *mode = "wb";
    sbf_file->fp = fopen(filename, mode);

    if (sbf_file->fp == NULL) {
        sbf_print_error("Failed to open file (mode=%s): %s\n", mode,
                        strerror(errno));
        return SBF_RESULT_FILE_OPEN_FAILURE;
    }

    return SBF_RESULT_SUCCESS;
}

/*
 * Close the FILE * associated with 'sbf_file'
 *
 * Returns SBF_RESULT_SUCCESS if there was no failure,
 * or corresponding error values if the file could not be closed.
 */
sbf_result sbf_close(const sbf_File *file) {
    FAIL_IF_NULL(file);
    int ret = fclose(file->fp);
    if (ret != 0) {
        sbf_print_error("Failed to close file: %s\n", strerror(errno));
        return SBF_RESULT_FILE_CLOSE_FAILURE;
    }
    return SBF_RESULT_SUCCESS;
}

/*
 *  Add the dataset to the sbf_file, giving it 'name'
 *
 *  Creates a data header in the 'sbf_file' object for this dataset:
 *  Ensure the datatype is CORRECT
 *
 */
sbf_result sbf_add_dataset(sbf_File *sbf_file, const char *name,
                           sbf_data_type type, sbf_size shape[SBF_MAX_DIM],
                           void *data) {
    FAIL_IF_NULL(sbf_file);
    FAIL_IF_NULL(name);
    FAIL_IF_NULL(data);

    // Fail if there are already too many datasets in the file
    if (sbf_file->n_datasets >= SBF_MAX_DATASETS) {
        sbf_print_error(
            "Number of datasets (%llu) exceeded SBF_MAX_DATASETS (%d)\n",
            sbf_file->n_datasets, SBF_MAX_DATASETS);
        return SBF_RESULT_MAX_DATASETS_EXCEEDED_FAILURE;
    }

    sbf_DataHeader header = sbf_new_data_header;
    header.data_type = type;
    strncpy(header.name, name, SBF_NAME_LENGTH);
    for (int_fast32_t i = 0; i < SBF_MAX_DIM; ++i)
        header.shape[i] = shape[i];
    sbf_file->datasets[sbf_file->n_datasets] = header;
    sbf_file->dataset_pointers[sbf_file->n_datasets] = data;
    sbf_file->n_datasets++;
    return SBF_RESULT_SUCCESS;
}

/*
 * return the size of the datatype specified in 'header'
 * (basically a wrapper for sizeof(header.datatype) as that
 *  would not return what we want)
 */
sbf_size sbf_datatype_size(const sbf_DataHeader header) {
    sbf_size datatype_size = 0;
    switch (header.data_type) {
    case SBF_DOUBLE:
        datatype_size = sizeof(sbf_double);
        break;
    case SBF_INT:
        datatype_size = sizeof(sbf_integer);
        break;
    case SBF_LONG:
        datatype_size = sizeof(sbf_long);
        break;
    case SBF_FLOAT:
        datatype_size = sizeof(sbf_float);
        break;
    case SBF_CFLOAT:
        datatype_size = sizeof(sbf_complex_float);
        break;
    case SBF_CDOUBLE:
        datatype_size = sizeof(sbf_complex_double);
        break;
    }
    return datatype_size;
}

/*
 * Return the number of data blocks required to write
 * given the shape of the dataset.
 *
 * Basically a product over the shape array, ignoring 0 values.
 *
 * If shape[0] == 0, will return 0.
 */
sbf_size sbf_num_blocks(const sbf_DataHeader h) {
    sbf_size num_blocks = h.shape[0];
    for (int_fast32_t i = 1; i < SBF_MAX_DIM; i++) {
        if (h.shape[i] == 0)
            break;
        num_blocks *= h.shape[i];
    }
    return num_blocks;
}

/*
 * Write the contents of `sbf_file`' specified
 * in its dataheaders to the FILE * in `sbf_file->fp'.
 *
 * If it fails, it fails totally.
 */
sbf_result sbf_write(const sbf_File *sbf_file) {
    FAIL_IF_NULL(sbf_file);
    size_t write_size = 0, expected_write_size = 0;

    sbf_FileHeader header =
        sbf_new_file_header; // this gives us token/version at the beginning
    header.n_datablocks = sbf_file->n_datasets;

    expected_write_size = 1;
    write_size =
        fwrite(&header, sizeof(header), expected_write_size, sbf_file->fp);

    if (write_size != expected_write_size) {
        sbf_print_error("Failed to write file header to file.\n");
        fprintf(stderr, "Wrote: %lu, expected:%lu\n", write_size,
                expected_write_size);
        sbf_close(sbf_file);
        return SBF_RESULT_WRITE_FAILURE;
    }

    expected_write_size = sbf_file->n_datasets;
    write_size = fwrite(sbf_file->datasets, sizeof(sbf_file->datasets[0]),
                        expected_write_size, sbf_file->fp);

    if (write_size != expected_write_size) {
        sbf_print_error("Failed to write dataset headers to file.\n");
        fprintf(stderr, "Wrote: %lu, expected:%lu\n", write_size,
                expected_write_size);
        sbf_close(sbf_file);
        return SBF_RESULT_WRITE_FAILURE;
    }

    for (sbf_size dset = 0; dset < sbf_file->n_datasets; dset++) {
        sbf_size datatype_size = sbf_datatype_size(sbf_file->datasets[dset]);
        expected_write_size = sbf_num_blocks(sbf_file->datasets[dset]);
        write_size = fwrite(&sbf_file->dataset_pointers[dset], datatype_size,
                            expected_write_size, sbf_file->fp);
        if (write_size != expected_write_size) {
            sbf_print_error("Failed to write dataset %s to file.\n",
                            sbf_file->datasets[dset].name);
            sbf_close(sbf_file);
            return SBF_RESULT_WRITE_FAILURE;
        }
    }
    return SBF_RESULT_SUCCESS;
}
