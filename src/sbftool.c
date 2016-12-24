#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "sbf.h"
#include <unistd.h>
#include <ctype.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 
enum log_level {
    error = 0, warning = 1, info = 2, debug = 3
};
enum log_level GLOBAL_LOG_LEVEL = info;

#define log(level, message, ...) \
    do {                                        \
        FILE * fd = stdout;                     \
        switch(level) {                         \
            case error:                         \
                fd = stderr;                    \
            default:                            \
                fd = stdout;                    \
        }                                       \
        if(level <= GLOBAL_LOG_LEVEL)           \
            fprintf(fd, message, __VA_ARGS__);  \
    } while(0)

#define assert(message, test)                                                       \
    do {                                                                            \
        if (!(test))                                                                \
            fprintf(stdout, "%s\n", message); fflush(stderr); exit(EXIT_FAILURE);   \
    } while (0)

const char *test_filename = "/tmp/sbf_test_c.sbf";

void usage(const char * progname) {
    fprintf(stdout, "%s [-dl] filename\n\n", progname);
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "\t-l\tList all datasets in file.\n");
    fprintf(stdout, "\t-d\tspecify of all datasets in file.\n");
    fprintf(stdout, "\t-c\tCompare contents of two sbf file. (UNIMPLEMENTED)\n");
    fprintf(stdout, "\t-h\tPrint this help message.\n");
    fprintf(stdout, "\n");

    exit(EXIT_SUCCESS);
}


const char * sbf_datatype_name(sbf_byte data_type) {
    switch(data_type) {
        case SBF_DOUBLE:
            return "sbf_double";
        case SBF_INT:
            return "sbf_integer";
        case SBF_LONG:
            return "sbf_long_integer";
        case SBF_FLOAT:
            return "sbf_float";
        case SBF_CFLOAT:
            return "sbf_complex_float";
        case SBF_CDOUBLE:
            return "sbf_complex_double";
        default:
            return "sbf_byte";
    }
}

const char * format_string(sbf_byte data_type) {
    switch(data_type) {
        case SBF_DOUBLE:
            return "%s% 10.3g%s";
        case SBF_INT:
            return "%s% d%s";
        case SBF_LONG:
            return "%s% li%s";
        case SBF_FLOAT:
            return "%s% 10.3g%s";
        default:
            return "%s%c%s";
    }

}

void pretty_print_1d(void *data, const char *fmt_string, sbf_size stride,
                    sbf_size n, sbf_size block_size, sbf_data_type dtype, uint_fast8_t vertical) {

    ptrdiff_t offset = 0;
    const char * suffix = "";
    const char * prefix = "";
    if(vertical) suffix = "\n";
    if(dtype == SBF_BYTE) 
        suffix="";

    for(ptrdiff_t offset = 0; offset != n * stride; offset = offset + stride) {
        switch(dtype) {
            case(SBF_INT):
                fprintf(stdout, fmt_string, "", *(int *)(data + offset), suffix);
                break;
            case(SBF_LONG):
                fprintf(stdout, fmt_string, "", *(long int *)(data + offset), suffix);
                break;
            case(SBF_DOUBLE):
                fprintf(stdout, fmt_string, "", *(double *)(data + offset), suffix);
                break;
            case(SBF_FLOAT):
                fprintf(stdout, fmt_string, "", *(float *)(data + offset), suffix);
                break;
            default:
                fprintf(stdout, fmt_string, "",*(char *)(data + offset), suffix);
                break;
        }
    }
    fprintf(stdout, "\n");
}

void pretty_print_2d(void *data, const char *fmt_string, sbf_size rows, sbf_size cols,
                    sbf_size block_size, sbf_data_type dtype, uint_fast8_t column_major) {
    ptrdiff_t column_stride = block_size, row_stride = block_size * cols, stride = 0;

    if(!column_major) {
        stride = column_stride; column_stride = row_stride; row_stride = stride;
    }
    // print out row by row
    for(int row = 0; row < rows; row++) {
        pretty_print_1d(data + (row * row_stride), fmt_string, column_stride, cols, block_size, dtype, 0);
    }
}


void pretty_print_data(const sbf_DataHeader dset, void * data, const char *fmt_string) {
    sbf_size block_size = sbf_datatype_size(dset);
    sbf_size num_blocks = sbf_num_blocks(dset); // change
    sbf_size stride = block_size;
    sbf_byte dims = SBF_GET_DIMENSIONS(dset);
    uint_fast8_t column_major = SBF_CHECK_COLUMN_MAJOR_FLAG(dset);
    int i = 0, j = 1;
    if(dims == 1) {
        pretty_print_1d(data, fmt_string, stride, num_blocks, block_size, dset.data_type, 1);
    }
    else if(dims == 2) {
        sbf_size cols = dset.shape[column_major ? 0 : 1];
        sbf_size rows = dset.shape[column_major ? 1 : 0];
        pretty_print_2d(data, fmt_string, rows, cols, block_size, dset.data_type, column_major);
    }
    else {
        sbf_size cols = dset.shape[column_major ? 0 : 1];
        sbf_size rows = dset.shape[column_major ? 1 : 0];
        if(!column_major) {
             for(int k = dims-1; k > j; k--) {
                sbf_size k_dim = dset.shape[k];
                log(debug, "k_dim = %llu, i = %d, j = %d, k=%d\n", k_dim, i, j, k);
                for(int l = 0; l < k_dim; l++) {
                    fprintf(stdout, "[%d][:][:]\n", l);
                    ptrdiff_t offset = rows * cols * block_size * l;
                    pretty_print_2d(data + offset, fmt_string, rows, cols, block_size, dset.data_type, column_major);
                }
            }
        }
        else {
            for(int k = 0; k < i; k++) {
                sbf_size k_dim = dset.shape[k];
                log(debug, "k_dim = %llu, i = %d, j = %d, k=%d\n", k_dim, i, j, k);
                for(int l = 0; l < k_dim; l++) {
                    for(int idx=0; idx < dims; idx++) {
                        if (idx == i || idx == j)
                                fprintf(stdout, "[:]");
                        else if (idx == k)
                                fprintf(stdout, "[%d]",l);
                        else fprintf(stdout, "[?]");
                    }
                    fprintf(stdout, "\n");
                    ptrdiff_t offset = rows * cols * block_size * l;
                    pretty_print_2d(data + offset, fmt_string, rows, cols, block_size, dset.data_type, column_major);
                }
            }
        }
     }
    fprintf(stdout, "\n");
}

void dump_file_as_utf8(sbf_File * file, uint_fast8_t dump_all_data) {
    fprintf(stdout, "---- %s ----\n", file->filename);
    //TODO add version checking
    fprintf(stdout, "sbf %s\n", "v0.1.1");
    for(int i = 0; i < file->n_datasets; i++) {

        sbf_DataHeader dset = file->datasets[i];
        fprintf(stdout, "dataset:\t\t'%s'\ntype|bytes|width:\t%s|%llu|%llubit\n",
                dset.name, sbf_datatype_name(dset.data_type), sbf_datatype_size(dset), sbf_datatype_size(dset)*8);
        fprintf(stdout, "flags:\t\t\t"BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(dset.flags));
        uint_fast8_t dims = SBF_GET_DIMENSIONS(dset);
        fprintf(stdout, "dimensions:\t\t%d\n", dims);
        fprintf(stdout, "shape:\t\t\t");
        fprintf(stdout, "[%llu", dset.shape[0]);
        for(uint_fast8_t dim = 1; dim < dims; dim++) {
            fprintf(stdout, ", %llu", dset.shape[dim]);
        }
        fprintf(stdout, "]\n");
        uint_fast8_t column_major = SBF_CHECK_COLUMN_MAJOR_FLAG(dset);
        fprintf(stdout, "column major:\t\t%s\n", column_major ? "true": "false");
        if(dump_all_data) {
            sbf_size data_size = sbf_datatype_size(dset) * sbf_num_blocks(dset);
            uint8_t data[data_size];
            sbf_result res = sbf_read_dataset(file, dset, data);
            if(res != SBF_RESULT_SUCCESS) {
                log(error, "Problem reading dataset %s: %s\n", dset.name, strerror(errno));
            }
            fprintf(stdout, "{\n");
            pretty_print_data(dset, (void *) data, format_string(dset.data_type));
            fprintf(stdout, "}\n");
        }
        fprintf(stdout, "\n");
    }
}

int main(int argc, char *argv[]) {
    extern char *optarg;
    extern int optind;
    uint_fast8_t dump_file = 0, list_datasets = 0;
    int c;

    opterr = 0;
    while ((c = getopt (argc, argv, "cdlh")) != -1)
        switch (c)
        {
            case 'd':
                dump_file = 1;
                break;
            case 'l':
                list_datasets = 1;
                break;
            case 'h':
                usage(argv[0]);
                break;
            case '?':
                fprintf(stderr, "Unrecognised argument.\nTry %s -h for more information.\n", argv[0]);
            default:
                exit(EXIT_FAILURE);
        }
    log(debug, "flags:\n\tdump_file: %s\n\tlist_datasets: %s\n",
        dump_file ? "true":"false", list_datasets ? "true":"false");

    if(optind == argc) {
        log(error, "Expecting a filename e.g.: %s\n", "file.sbf");
        usage(argv[0]);
    }

    // try to read all of the remaining arguments as sbf files
    for (int index = optind; index < argc; index++) {
        char * filename = argv[index];
        sbf_File file = sbf_new_file;
        file.mode = SBF_FILE_READONLY;
        file.filename = filename;
        sbf_result res;
        res = sbf_open(&file);
        if(res != SBF_RESULT_SUCCESS) {
            log(error, "Could not open file %s: %s\n", file.filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
        res = sbf_read_headers(&file);
        if(res != SBF_RESULT_SUCCESS) {
            log(error, "Could not read headers: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if(list_datasets || dump_file) dump_file_as_utf8(&file, dump_file);
        res = sbf_close(&file);
        if(res != SBF_RESULT_SUCCESS) {
            log(error, "Problem closing file %s: %s\n", file.filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
