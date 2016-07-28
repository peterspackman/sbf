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
#include <stdlib.h>
#include <string.h>

#ifdef SBF_DEBUG_OUTPUT
#define SBF_DEBUG(...) fprintf(stderr, __VA_ARGS__);
#else
#define SBF_DEBUG
#endif

#define SBF_VERSION_MAJOR '0'
#define SBF_VERSION_MINOR '1'
#define SBF_VERSION_MINOR_MINOR '1'

#define SBF_MAX_DIM 8
#define SBF_MAX_DATASETS 16
#define SBF_NAME_LENGTH 32
#define SBF_ROW_MAJOR 0b01000000
#define SBF_BIG_ENDIAN 0b1000000
#define SBF_DIMENSION_BITS 0b00001111

#define SBF_GET_DIMENSIONS(data_header) \
    (data_header.flags & (SBF_DIMENSION_BITS))

#define SBF_SET_DIMENSIONS(data_header, dims) \
    (data_header.flags |=  ((dims & SBF_DIMENSION_BITS)))

#define SBF_SET_BIG_ENDIAN_FLAG(data_header, flag) \
    (data_header.flags |= (flag << 7))

#define SBF_CHECK_BIG_ENDIAN_FLAG(data_header) \
    (data_header.flags & SBF_BIG_ENDIAN)

#define SBF_CHECK_ROW_MAJOR_FLAG(data_header) \
    (data_header.flags & SBF_ROW_MAJOR)

#define SBF_SET_ROW_MAJOR_FLAG(data_header) \
    (data_header.flags |= SBF_ROW_MAJOR)

#define SBF_PERROR(...) fprintf(stderr, __VA_ARGS__)

#define FAIL_IF_NULL(arg)                                                      \
    if (arg == NULL)                                                           \
    return SBF_RESULT_NULL_FAILURE

#define SBF_WRITE_RAW(data, block_size, num_blocks, fp)                        \
    do {                                                                       \
        if (fwrite(data, block_size, num_blocks, fp) != num_blocks) {          \
            SBF_PERROR("Failed to write to file, ferror=%d\nClosing file\n",   \
                       ferror(fp));                                            \
            sbf_close(sbf);                                                    \
            return SBF_RESULT_WRITE_FAILURE;                                   \
        }                                                                      \
    } while (0)

#define SBF_READ_RAW(data, block_size, num_blocks, fp)                         \
    do {                                                                       \
        if (fread(data, block_size, num_blocks, fp) != num_blocks) {           \
            SBF_PERROR("Failed to read from file, ferror=%d\nClosing file\n",  \
                       ferror(fp));                                            \
            sbf_close(sbf);                                                    \
            return SBF_RESULT_WRITE_FAILURE;                                   \
        }                                                                      \
    } while (0)

// DATA TYPES
typedef uint8_t sbf_byte;
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

typedef enum {
    SBF_FILE_READONLY,
    SBF_FILE_WRITEONLY,
    SBF_FILE_READWRITE
} sbf_mode;

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
    sbf_character token[3];
    sbf_character version_string[3];
    sbf_byte n_datasets;
} sbf_FileHeader;

typedef struct {
    sbf_character name[SBF_NAME_LENGTH]; // what is the name of this dataset
    sbf_byte flags;
    sbf_data_type data_type;             // how big is each block of data
    sbf_size shape[SBF_MAX_DIM];         // how many blocks of data do we have
} sbf_DataHeader;

typedef struct {
    sbf_mode mode;
    const char *filename;
    FILE *fp;
    sbf_byte n_datasets;
    sbf_DataHeader datasets[SBF_MAX_DATASETS];
    void *dataset_pointers[SBF_MAX_DATASETS];
} sbf_File;

static const sbf_File sbf_new_file = {
    .mode = SBF_FILE_READONLY, .filename = NULL, .fp = NULL, .n_datasets = 0,
};

static const sbf_FileHeader sbf_new_file_header = {
    .token = {'S', 'B', 'F'},
    .version_string = {SBF_VERSION_MAJOR, SBF_VERSION_MINOR, SBF_VERSION_MINOR_MINOR},
    .n_datasets = 0
};

static const sbf_DataHeader sbf_new_data_header = {
    .name = {0}, .data_type = 0, .shape = {0},
};

/*
 * Open 'filename', placing the resultant file pointer into 'sbf'
 *
 * Returns SBF_RESULT_SUCCESS if we opened the file, or corresponding error
 * values
 * if the file could not be opened.
 */
sbf_result sbf_open(sbf_File *sbf) {
    FAIL_IF_NULL(sbf);

    switch (sbf->mode) {
    case SBF_FILE_WRITEONLY:
        sbf->fp = fopen(sbf->filename, "wb");
        break;
    case SBF_FILE_READONLY:
        sbf->fp = fopen(sbf->filename, "rb");
        break;
    case SBF_FILE_READWRITE:
        sbf->fp = fopen(sbf->filename, "w+b");
        break;
    }

    if (sbf->fp == NULL) {
        SBF_PERROR("Failed to open file: %s\n", strerror(errno));
        return SBF_RESULT_FILE_OPEN_FAILURE;
    }

    return SBF_RESULT_SUCCESS;
}

/*
 * Close the FILE * associated with 'sbf'
 *
 * Returns SBF_RESULT_SUCCESS if there was no failure,
 * or corresponding error values if the file could not be closed.
 */
sbf_result sbf_close(const sbf_File *file) {
    FAIL_IF_NULL(file);
    int ret = fclose(file->fp);
    if (ret != 0) {
        SBF_PERROR("Failed to close file: %s\n", strerror(errno));
        return SBF_RESULT_FILE_CLOSE_FAILURE;
    }
    return SBF_RESULT_SUCCESS;
}

/*
 *  Add the dataset to the sbf, giving it 'name'
 *
 *  Creates a data header in the 'sbf' object for this dataset:
 *  Ensure the datatype is CORRECT
 *
 */
sbf_result sbf_add_dataset(sbf_File *sbf, const char *name, sbf_data_type type,
                           sbf_size shape[SBF_MAX_DIM], void *data) {
    FAIL_IF_NULL(sbf);
    FAIL_IF_NULL(name);
    FAIL_IF_NULL(data);

    // Fail if there are already too many datasets in the file
    if (sbf->n_datasets >= SBF_MAX_DATASETS) {
        SBF_PERROR("Number of datasets (%d) exceeded SBF_MAX_DATASETS (%d)\n",
                   sbf->n_datasets, SBF_MAX_DATASETS);
        return SBF_RESULT_MAX_DATASETS_EXCEEDED_FAILURE;
    }

    sbf_DataHeader header = sbf_new_data_header;
    header.data_type = type;
    strncpy(header.name, name, SBF_NAME_LENGTH);
    for (int_fast32_t i = 0; i < SBF_MAX_DIM; ++i)
        header.shape[i] = shape[i];
    sbf->datasets[sbf->n_datasets] = header;
    sbf->dataset_pointers[sbf->n_datasets] = data;
    sbf->n_datasets++;
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

sbf_result sbf_write_headers(const sbf_File *sbf) {
    FAIL_IF_NULL(sbf);
    FAIL_IF_NULL(sbf->fp);

    sbf_FileHeader header =
        sbf_new_file_header; // this gives us token/version at the beginning
    header.n_datasets = sbf->n_datasets;

    SBF_WRITE_RAW(&header, sizeof(header), 1, sbf->fp);

    SBF_WRITE_RAW(sbf->datasets, sizeof(sbf->datasets[0]), header.n_datasets,
                  sbf->fp);

    return SBF_RESULT_SUCCESS;
}

/*
 * Write the contents of 'sbf' specified
 * in its dataheaders to the FILE * in 'sbf->fp'.
 *
 * If it fails, it fails totally.
 */
sbf_result sbf_write(const sbf_File *sbf) {
    FAIL_IF_NULL(sbf);
    FAIL_IF_NULL(sbf->fp);

    sbf_write_headers(sbf);

    for (sbf_size dset = 0; dset < sbf->n_datasets; dset++) {
        sbf_size datatype_size = sbf_datatype_size(sbf->datasets[dset]);
        sbf_size expected_write_size = sbf_num_blocks(sbf->datasets[dset]);

        SBF_WRITE_RAW(sbf->dataset_pointers[dset], datatype_size,
                      expected_write_size, sbf->fp);
    }
    return SBF_RESULT_SUCCESS;
}

/*
 * Read the contents of a dataset in the file pointed to by 'sbf'
 * Expects 'data' to be an array already allocated of the correct size.
 */
sbf_result sbf_read_dataset(sbf_File *sbf, const sbf_DataHeader header,
                            void *data) {
    FAIL_IF_NULL(sbf);
    FAIL_IF_NULL(sbf->fp);
    FAIL_IF_NULL(data);

    sbf_size num_blocks = sbf_num_blocks(header);
    sbf_size datatype_size = sbf_datatype_size(header);

    SBF_READ_RAW(data, datatype_size, num_blocks, sbf->fp);

    return SBF_RESULT_SUCCESS;
}

/*
 * Read the contents of the headers in the file pointed to by 'sbf'
 * Sets the relevant information into 'sbf'
 */
sbf_result sbf_read_headers(sbf_File *sbf) {
    FAIL_IF_NULL(sbf);
    FAIL_IF_NULL(sbf->fp);

    sbf_FileHeader header =
        sbf_new_file_header; // this gives us token/version at the beginning
    SBF_READ_RAW(&header, sizeof(header), 1, sbf->fp);

    sbf->n_datasets = header.n_datasets;

    SBF_READ_RAW(&(sbf->datasets[0]), sizeof(sbf_DataHeader), header.n_datasets,
                 sbf->fp);

    return SBF_RESULT_SUCCESS;
}
