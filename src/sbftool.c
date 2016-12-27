#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sbf.h"
#include <unistd.h>
#include <complex.h>
#include <stdarg.h>
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
    error = 0, warning = 1, info = 2, verbose_info = 3, debug = 10
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
    fprintf(stdout, "%s [-cdl] filename\n\n", progname);
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "\t-l\tList all datasets in file.\n");
    fprintf(stdout, "\t-d\tspecify of all datasets in file.\n");
    fprintf(stdout, "\t-c\tCompare contents of two sbf file. (UNIMPLEMENTED)\n");
    fprintf(stdout, "\t-h\tPrint this help message.\n");
    fprintf(stdout, "\n");

    exit(EXIT_SUCCESS);
}

ptrdiff_t offset_of(sbf_size block_size, bool column_major,
                    int dims, const sbf_size shape[SBF_MAX_DIM], sbf_size idx[SBF_MAX_DIM]) {
    va_list ap;
    ptrdiff_t offset = 0;

    if(column_major) {
        for(int i = 0; i < dims; i++) {
            int product = 1;
            for(int j = 0; j < i; j++) {
                product = product * shape[j];
            }
            offset = offset + product * idx[i];
        }
    }
    else {
        for(int i = 0; i < dims; i++) {
            int product = 1;
            for(int j = i + 1; j < dims; j++) {
                product = product * shape[j];
            }
            offset = offset + product * idx[i];
        }

    }
    return offset * block_size;
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
            return "%s% 10.3f%s";
        case SBF_INT:
            return "%s% d%s";
        case SBF_LONG:
            return "%s% li%s";
        case SBF_FLOAT:
            return "%s% 10.3f%s";
        case SBF_CFLOAT:
            return "%s% 5.2f%+5.2fi";
        case SBF_CDOUBLE:
            return "%s% 5.2f%+5.2fi";
        default:
            return "%s%c%s";
    }

}

void pretty_print_block(void *data, const char *fmt_string, sbf_data_type dtype) {
    ptrdiff_t offset = 0;
    const char * suffix = "";
    const char * prefix = "";
    if(dtype == SBF_BYTE) 
        suffix="";
    switch(dtype) {
        case(SBF_INT):
            fprintf(stdout, fmt_string, "", *(int *)(data), suffix);
            break;
        case(SBF_LONG):
            fprintf(stdout, fmt_string, "", *(long int *)(data), suffix);
            break;
        case(SBF_DOUBLE):
            fprintf(stdout, fmt_string, "", *(double *)(data), suffix);
            break;
        case(SBF_FLOAT):
            fprintf(stdout, fmt_string, "", *(float *)(data), suffix);
            break;
        case(SBF_CFLOAT):
            fprintf(stdout, fmt_string, "", *(float *)(data), 
                                            *(float *)(data + sizeof(float)), 
                                            suffix);
            break;
        case(SBF_CDOUBLE):
            fprintf(stdout, fmt_string, "", *(double *)(data), 
                                            *(double *)(data + sizeof(double)), 
                                            suffix);
            break;
        default:
            fprintf(stdout, fmt_string, "",*(char *)(data), suffix);
            break;
    }
}

void increment_index(const sbf_size shape[SBF_MAX_DIM],
                     sbf_size idx[SBF_MAX_DIM], const sbf_byte dims) {
    for(int dim = dims -1; dim > -1; dim--) {
        idx[dim]++;
        if(idx[dim] == shape[dim]) idx[dim] = 0;
        else break;
    }
}

bool all_zero(sbf_size idx[SBF_MAX_DIM]) {
    for(int dim = 0; dim < SBF_MAX_DIM; dim++) 
        if(idx[dim] != 0) return 0;
    return 1;
}

void pretty_print_nd(const sbf_DataHeader dset, void *data, const char *fmt_string) {
    sbf_size idx[SBF_MAX_DIM] = {0};
    sbf_byte dims = SBF_GET_DIMENSIONS(dset);
    sbf_size rows = dset.shape[dims - 2];
    sbf_size cols = dset.shape[dims - 1];
    bool column_major = SBF_CHECK_COLUMN_MAJOR_FLAG(dset);
    sbf_size n = 0;
    do {
        if(n % rows == 0) {
            fprintf(stdout, "\n");
        }
        if(n % (rows * cols) == 0) {
            for(int i = 0; i < dims - 2; i++) printf("%llu,", idx[i]);
            if(dims > 2) fprintf(stdout, ":,:\n");
        }
        ptrdiff_t offset = offset_of(sbf_datatype_size(dset), column_major, dims, dset.shape, idx);
        pretty_print_block(data + offset, fmt_string, dset.data_type);
        n++;
        increment_index(dset.shape, idx, dims);
    } while(!all_zero(idx));
    fprintf(stdout, "\n");
}


void pretty_print_data(const sbf_DataHeader dset, void * data, const char *fmt_string) {
    sbf_byte dims = SBF_GET_DIMENSIONS(dset);
    if(dims == 0) {
        pretty_print_block(data, fmt_string, dset.data_type);
    }
    else {
        pretty_print_nd(dset, data, fmt_string);
    }
}

void dump_file_as_utf8(sbf_File * file, bool dump_all_data) {
    fprintf(stdout, "---- %s ----\n", file->filename);
    //TODO add version checking
    fprintf(stdout, "sbf %s\n", "v0.1.1");
    for(int i = 0; i < file->n_datasets; i++) {

        sbf_DataHeader dset = file->datasets[i];
        fprintf(stdout, "dataset:\t\t'%s'\n", dset.name);
        fprintf(stdout, "dtype:\t\t\t%s\n", sbf_datatype_name(dset.data_type));
        fprintf(stdout, "dtype size:\t\t%llu bit\n", sbf_datatype_size(dset)*8);
        fprintf(stdout, "flags:\t\t\t"BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(dset.flags));
        sbf_byte dims = SBF_GET_DIMENSIONS(dset);
        fprintf(stdout, "dimensions:\t\t%d\n", dims);
        fprintf(stdout, "shape:\t\t\t");
        fprintf(stdout, "[%llu", dset.shape[0]);
        for(sbf_byte dim = 1; dim < dims; dim++) {
            fprintf(stdout, ", %llu", dset.shape[dim]);
        }
        fprintf(stdout, "]\n");
        bool column_major = SBF_CHECK_COLUMN_MAJOR_FLAG(dset);
        fprintf(stdout, "storage:\t\t%s major\n", column_major ? "column": "row");
        bool endianness = SBF_CHECK_BIG_ENDIAN_FLAG(dset);
        fprintf(stdout, "endianness:\t\t%s endian\n", endianness ? "big": "little");
        if(dump_all_data) {
            sbf_size data_size = sbf_datatype_size(dset) * sbf_num_blocks(dset);
            uint8_t data[data_size];
            sbf_result res = sbf_read_dataset(file, dset, data);
            if(res != SBF_RESULT_SUCCESS) {
                log(error, "Problem reading dataset %s: %s\n", dset.name, strerror(errno));
            }
            else pretty_print_data(dset, (void *) data, format_string(dset.data_type));
        }
        fprintf(stdout, "\n");
    }
}

bool shape_equal(sbf_size shape1[SBF_MAX_DIM], sbf_size shape2[SBF_MAX_DIM]) {
    for(sbf_byte i = 0; i < SBF_MAX_DIM; i++) {
        if(shape1[i] != shape2[i]) return false;
    }
    return true;
}

int get_dataset(const char * name, const sbf_File * file) {
    int found = -1;
    for(sbf_byte i = 0; i < file->n_datasets; i++) {
        if(strncmp(name, file->datasets[i].name, SBF_NAME_LENGTH) == 0) {
            found = i;
            log(debug, "Found matching dset : '%s'\n", file->datasets[i].name);
        }
    }
    return found;
}

bool compare_blocks(void * a, void * b, sbf_data_type dtype) {
    switch(dtype) {
        case(SBF_INT):
            return *(int *)(a) == *(int *)(b);
        case(SBF_LONG):
            return *(long *)(a) == *(long *)(b);
        case(SBF_DOUBLE):
            return *(double *)(a) == *(double *)(b);
        case(SBF_FLOAT):
            return *(float *)(a) == *(float *)(b);
        case(SBF_CFLOAT):
            return (*(float *)(a) == *(float *)(b)) &&
                   (*(float *)(a+sizeof(float)) == *(float *)(b+sizeof(float))); 
        case(SBF_CDOUBLE):
            return (*(double *)(a) == *(double *)(b)) &&
                   (*(double *)(a+sizeof(double)) == *(double *)(b+sizeof(double))); 
        default:
            return *(char *)(a) == *(char *)(b);
    }

}

sbf_size diff_datablocks(const sbf_DataHeader dset1, void * data1,
                         const sbf_DataHeader dset2, void * data2) {
    sbf_size bytes = sbf_num_blocks(dset1) * sbf_datatype_size(dset1); 
    bool raw = false;
    if(raw) return memcmp(data1, data2, bytes);
    sbf_size diffs = 0;
    bool cmaj1 = SBF_CHECK_COLUMN_MAJOR_FLAG(dset1);
    bool cmaj2 = SBF_CHECK_COLUMN_MAJOR_FLAG(dset2);

    sbf_size idx[SBF_MAX_DIM] = {0};
    sbf_byte dims = SBF_GET_DIMENSIONS(dset1);
    const char * fmt_string = format_string(dset1.data_type);
    do {
        ptrdiff_t offset1 = offset_of(sbf_datatype_size(dset1), cmaj1, dims, dset1.shape, idx);
        ptrdiff_t offset2 = offset_of(sbf_datatype_size(dset2), cmaj2, dims, dset1.shape, idx);
        if(!compare_blocks(data1 + offset1, data2 + offset2, dset1.data_type)) {
            if(GLOBAL_LOG_LEVEL >= verbose_info) {
                log(verbose_info, "'%s' difference at ",dset1.name);
                for(sbf_byte dim = 0; dim < dims; dim++) log(verbose_info, "[%llu]", idx[dim]);
                pretty_print_block(data1 + offset1, fmt_string, dset1.data_type);
                fprintf(stdout, " != ");
                pretty_print_block(data2 + offset2, fmt_string, dset2.data_type);
                fprintf(stdout, "\n");
            }
            diffs++;
        }
        increment_index(dset1.shape, idx, dims);
    } while(!all_zero(idx));
    return diffs;
}

sbf_size diff_files(sbf_File * file1, sbf_File * file2) {
    sbf_byte n1 = file1->n_datasets; sbf_byte n2 = file2->n_datasets;
    sbf_size file_diffs = 0;
    bool deep_check = true;
    if(n1 != n2) {
        log(verbose_info, "Different number of datasets: %d, %d\n", n1, n2);
        return ++file_diffs;
    }
    for(sbf_byte i = 0; i < n1;  i++) {
        sbf_DataHeader dset = file1->datasets[i];
        sbf_size dset_diffs = 0;
        int dset_found = get_dataset(dset.name, file2);
        if(dset_found == -1) {
            log(verbose_info, "No matching dataset found for '%s' in %s\n", dset.name, file2->filename);
            file_diffs++;
        }
        else {
            sbf_DataHeader dset2 = file2->datasets[dset_found];
            bool flags_equal = (dset.flags == dset2.flags);
            bool dtypes_equal = (dset.data_type == dset2.data_type);
            bool shapes_equal = shape_equal(dset.shape, dset2.shape);

            if(!flags_equal) {
                log(verbose_info, "flags for '%s' differ.\n", dset.name);
                dset_diffs++;
            }
            if(!dtypes_equal) {
                log(verbose_info, "data types for '%s' differ.\n", dset.name);
                dset_diffs++;
            }
            if(!shapes_equal) {
                log(verbose_info, "shapes for '%s' differ.\n", dset.name);
                dset_diffs++;
            }
            if(deep_check && flags_equal && shapes_equal && dtypes_equal) {
                sbf_size data_size = sbf_datatype_size(dset) * sbf_num_blocks(dset);
                uint8_t data1[data_size];
                uint8_t data2[data_size];
                sbf_result res = sbf_read_dataset(file1, dset, data1);
                if(res != SBF_RESULT_SUCCESS) {
                    log(error, "Problem reading dataset '%s' in %s: %s\n", 
                        dset.name, file1->filename, strerror(errno));
                    break;
                }
                res = sbf_read_dataset(file2, dset2, data2);
                if(res != SBF_RESULT_SUCCESS) {
                    log(error, "Problem reading dataset %s: %s\n", dset.name, strerror(errno));
                    break;
                }
                dset_diffs = dset_diffs + diff_datablocks(dset, data1, dset2, data2);
            }
            log(verbose_info, "%llu differences in '%s'\n", dset_diffs, dset.name);
            file_diffs = file_diffs + dset_diffs;
        }
    }
    return file_diffs;
}

int main(int argc, char *argv[]) {
    extern char *optarg;
    extern int optind;
    bool dump_file = false, list_datasets = false, diff = false;
    int c;

    opterr = 0;
    while ((c = getopt (argc, argv, "cdlh")) != -1)
        switch (c)
        {
            case 'c':
                diff = true;
                break;
            case 'd':
                dump_file = true;
                break;
            case 'l':
                list_datasets = true;
                break;
            case 'h':
                usage(argv[0]);
                break;
            case '?':
                if(optopt == 'c' || optopt == 'd')
                    log(error, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    log(error, "Unknown option -%c.\n", optopt);
                else
                    log(error, "Unknown option character '\\x%x'.\n", optopt);
                exit(EXIT_FAILURE);
            default:
                exit(EXIT_FAILURE);
        }

    log(debug, "flags:\n\tdump_file: %s\n\tlist_datasets: %s\n",
        dump_file ? "true":"false", list_datasets ? "true":"false");

    if(diff) {
        if(optind != (argc - 2)) {
            log(error, "Option -c requires two filenames e.g.: %s %s\n",
                "file1.sbf", "file2.sbf");
            usage(argv[0]);
        }
        sbf_File file1 = sbf_new_file; sbf_File file2 = sbf_new_file;
        file1.mode = SBF_FILE_READONLY; file2.mode = SBF_FILE_READONLY;
        file1.filename = argv[optind]; file2.filename = argv[optind+1];
        sbf_result res;
        res = sbf_open(&file1);
        if(res != SBF_RESULT_SUCCESS) {
            exit(EXIT_FAILURE);
        }
        res = sbf_open(&file2);
        if(res != SBF_RESULT_SUCCESS) {
            exit(EXIT_FAILURE);
        }
        res = sbf_read_headers(&file1);
        if(res != SBF_RESULT_SUCCESS) {
            exit(EXIT_FAILURE);
        }
        res = sbf_read_headers(&file2);
        if(res != SBF_RESULT_SUCCESS) {
            exit(EXIT_FAILURE);
        }

        sbf_size diffs = diff_files(&file1, &file2);
        if(diffs)
            log(info, "%llu difference%s between %s and %s\n",
                diffs, diffs > 1 ? "s" : "", file1.filename, file2.filename);

        res = sbf_close(&file1);
        if(res != SBF_RESULT_SUCCESS) {
            exit(EXIT_FAILURE);
        }
        res = sbf_close(&file2);
        if(res != SBF_RESULT_SUCCESS) {
            exit(EXIT_FAILURE);
        }
        exit(diffs);
    }
    else {
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

            if(res != SBF_RESULT_SUCCESS) exit(EXIT_FAILURE);

            res = sbf_read_headers(&file);
            if(res != SBF_RESULT_SUCCESS) {
                log(error, "Could not read headers: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if(list_datasets || dump_file) dump_file_as_utf8(&file, dump_file);

            res = sbf_close(&file);
            if(res != SBF_RESULT_SUCCESS) exit(EXIT_FAILURE);
            
    }

   
    }

    return 0;
}
