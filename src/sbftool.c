#include <errno.h>
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
    error, warning, info, debug
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
        if(level < GLOBAL_LOG_LEVEL)            \
            fprintf(fd, message, __VA_ARGS__);  \
    } while(0)

#define assert(message, test)                                                       \
    do {                                                                            \
        if (!(test))                                                                \
            fprintf(stdout, "%s\n", message); fflush(stderr); exit(EXIT_FAILURE);   \
    } while (0)

const char *test_filename = "/tmp/sbf_test_c.sbf";

void usage() {
    log(info, "Usage as:\n\t%s [-d] filenames...\n", "sbftool");
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

void pretty_print_data(const sbf_DataHeader dset, void * data) {
    return;
}

void dump_file_as_utf8(sbf_File * file, uint_fast8_t dump_all_data) {
    fprintf(stdout, "Contents of %s\n", file->filename);
    fprintf(stdout, "sbf %s\n", "v0.1.1");
    for(int i = 0; i < file->n_datasets; i++) {
        sbf_DataHeader dset = file->datasets[i];
        fprintf(stdout, "dataset\t\t\t'%s'\ntype|bytes|width \t%s|%llu|%llubit\n",
                dset.name, sbf_datatype_name(dset.data_type), sbf_datatype_size(dset), sbf_datatype_size(dset)*8);
        fprintf(stdout, "flags\t\t\t"BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(dset.flags));
        uint_fast8_t dims = SBF_GET_DIMENSIONS(dset);
        fprintf(stdout, "n dimensions\t\t%d\n", dims);
        fprintf(stdout, "shape\t\t\t");
        fprintf(stdout, "[%llu", dset.shape[0]);
        for(uint_fast8_t dim = 1; dim < dims; dim++) {
            fprintf(stdout, ", %llu", dset.shape[dim]);
        }
        fprintf(stdout, "]\n");
        uint_fast8_t column_major = SBF_CHECK_COLUMN_MAJOR_FLAG(dset);
        if(column_major) fprintf(stdout, "column major\n");
        if(dump_all_data) {
            fprintf(stdout, "TODO: dump out all data\n");
            sbf_size data_size = sbf_datatype_size(dset) * sbf_num_blocks(dset);
            uint8_t data[data_size];
            sbf_result res = sbf_read_dataset(file, dset, data);
            if(res != SBF_RESULT_SUCCESS) {
                log(error, "Problem reading dataset %s: %s\n", dset.name, strerror(errno));
                break;
            }
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
    while ((c = getopt (argc, argv, "dlh:")) != -1)
        switch (c)
        {
            case 'd':
                dump_file = 1;
                break;
            case 'l':
                list_datasets = 1;
                break;
            case 'h':
                usage();
                break;
            default:
                abort ();
        }
    log(debug, "dump_file = %d\n", dump_file);
    if(optind == argc) {
        log(error, "Expecting a filename e.g.: %s\n", "file.sbf");
        usage();
    }

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
