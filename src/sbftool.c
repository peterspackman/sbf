#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <complex.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>
#include "sbf.h"
#define SBFTOOL_VERSION "0.3.0"

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
    error = 0, warning = 1, info = 2, verbose_info = 3, very_verbose_info = 4, debug = 5
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

#define SBF_ASSERT_SUCCESSFUL(res)           \
    do {                                     \
        if (res != SBF_RESULT_SUCCESS)       \
            exit(EXIT_FAILURE);              \
    } while (0)

float eps = 1e-5;

void usage(const char * progname) {
    fprintf(stdout,
    "sbftool %s (SBF v%s)\n"
    "Usage:\n"
    "\tsbftool [-dhvp] filename\n"
    "\tsbftool [-cm] filename1 filename2\n"
    "Options:\n"
        "\t-d\tspecify a dataset.\n"
        "\t-p\tprint out contents of dataset(s).\n"
        "\t-c\tCompare contents of two sbf files (like diff).\n"
        "\t-m\tOnly compare dataset metadata of the two sbf files.\n"
        "\t-v\tIncrease verbosity (up to three times).\n\n"
        "\t-h\tPrint this help message.\n\n"
    "By default sbftool simply prints out info about datasets in the file(s) provided.\n",
        SBFTOOL_VERSION, SBF_VERSION);

    exit(EXIT_SUCCESS);
}

int_fast8_t get_dataset(const char * name, const sbf_File * file) {
    int_fast8_t found = -1;
    for(sbf_byte i = 0; i < file->n_datasets; i++) {
        if(strncmp(name, file->datasets[i].name, SBF_NAME_LENGTH) == 0) {
            found = i;
            log(debug, "Found matching dataset '%s' in '%s'\n", file->datasets[i].name,
                    file->filename);
        }
    }
    return found;
}

bool shape_equal(sbf_size shape1[SBF_MAX_DIM], sbf_size shape2[SBF_MAX_DIM]) {
    for(sbf_byte i = 0; i < SBF_MAX_DIM; i++) {
        if(shape1[i] != shape2[i]) return false;
    }
    return true;
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

ptrdiff_t offset_of(sbf_size block_size, bool column_major, sbf_byte dims,
                    const sbf_size shape[SBF_MAX_DIM], sbf_size idx[SBF_MAX_DIM]) {
    ptrdiff_t offset = 0;

    if(column_major) {
        for(int_fast8_t i = 0; i < dims; i++) {
            sbf_size product = 1;
            for(int j = 0; j < i; j++) {
                product = product * shape[j];
            }
            offset = offset + product * idx[i];
        }
    }
    else {
        for(int_fast8_t i = 0; i < dims; i++) {
            sbf_size product = 1;
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
        case SBF_DOUBLE: return "sbf_double";
        case SBF_INT: return "sbf_integer";
        case SBF_LONG: return "sbf_long_integer";
        case SBF_FLOAT: return "sbf_float";
        case SBF_CFLOAT: return "sbf_complex_float";
        case SBF_CDOUBLE: return "sbf_complex_double";
        case SBF_CHAR: return "sbf_character";
        default: return "sbf_byte";
    }
}

const char * format_string(sbf_byte data_type) {
    switch(data_type) {
        case SBF_DOUBLE: return "%s% 7.5g%s";
        case SBF_INT: return "%s% 7"PRIi32"%s";
        case SBF_LONG: return "%s% 7"PRIi64"%s";
        case SBF_FLOAT: return "%s% 7.5g%s";
        case SBF_CFLOAT: return "%s%3.1g%+3.1gi";
        case SBF_CDOUBLE: return "%s%3.1g%+3.1gi";
        case SBF_CHAR: return "%s%c%s";
        default: return "%s%0x %s";
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
            fprintf(stdout, fmt_string, "", *(int32_t *)(data), suffix);
            break;
        case(SBF_LONG):
            fprintf(stdout, fmt_string, "", *(int64_t *)(data), suffix);
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

void pretty_print_nd(const sbf_DataHeader dset, void *data, const char *fmt_string) {
    sbf_size idx[SBF_MAX_DIM] = {0};
    sbf_byte dims = SBF_GET_DIMENSIONS(dset);
    sbf_size rows = dset.shape[dims - 2];
    sbf_size cols = dset.shape[dims - 1];
    bool column_major = SBF_CHECK_COLUMN_MAJOR_FLAG(dset);
    bool print_columns = (dims == 1) && (dset.data_type != SBF_CHAR);
    sbf_size n = 0;
    do {
        if(n % (rows * cols) == 0) {
            for(int i = 0; i < dims - 2; i++) printf("%"PRIu64",", idx[i]);
            if(dims > 2) fprintf(stdout, ":,:\n");
        }
        ptrdiff_t offset = offset_of(sbf_datatype_size(dset),
                                     column_major, dims, dset.shape, idx);
        pretty_print_block(data + offset, fmt_string, dset.data_type);
        n++;
        if(((n % rows == 0)) || print_columns) {
            fprintf(stdout, "\n");
        }
        increment_index(dset.shape, idx, dims);
    } while(!all_zero(idx));
    fprintf(stdout, "\n");
}


void pretty_print_data(const sbf_DataHeader dset, void * data, const char *fmt_string) {
    sbf_byte dims = SBF_GET_DIMENSIONS(dset);
    if(dims == 0) {
        pretty_print_block(data, fmt_string, dset.data_type);
        fprintf(stdout, "\n");
    }
    else {
        pretty_print_nd(dset, data, fmt_string);
    }
}

void dump_file_as_utf8(sbf_File * file, bool dump_all_data) {
    for(int_fast8_t i = 0; i < file->n_datasets; i++) {

        sbf_DataHeader dset = file->datasets[i];
        fprintf(stdout, "dataset:\t'%s'\n", dset.name);
        fprintf(stdout, "dtype:\t\t%s\n", sbf_datatype_name(dset.data_type));
        fprintf(stdout, "dtype size:\t%"PRIu64" bit\n", sbf_datatype_size(dset)*8);
        fprintf(stdout, "flags:\t\t"BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(dset.flags));
        sbf_byte dims = SBF_GET_DIMENSIONS(dset);
        fprintf(stdout, "dimensions:\t%d\n", dims);
        fprintf(stdout, "shape:\t\t");
        fprintf(stdout, "[%"PRIu64, dset.shape[0]);
        for(sbf_byte dim = 1; dim < dims; dim++) {
            fprintf(stdout, ", %"PRIu64, dset.shape[dim]);
        }
        fprintf(stdout, "]\n");
        bool column_major = SBF_CHECK_COLUMN_MAJOR_FLAG(dset);
        fprintf(stdout, "storage:\t%s major\n", column_major ? "column": "row");
        bool endianness = SBF_CHECK_BIG_ENDIAN_FLAG(dset);
        fprintf(stdout, "endianness:\t%s endian\n", endianness ? "big": "little");

        if(dump_all_data) {
            sbf_size data_size = sbf_datatype_size(dset) * sbf_num_blocks(dset);
            sbf_byte data[data_size];
            sbf_result res = sbf_read_dataset(file, dset, data);
            if(res != SBF_RESULT_SUCCESS) {
                log(error, "Problem reading dataset %s: %s\n", dset.name, strerror(errno));
            }
            else {
                fprintf(stdout, "\n--- contents ---\n");
                pretty_print_data(dset, (void *) data, format_string(dset.data_type));
                fprintf(stdout, "----------------\n");
            }
        }
        fprintf(stdout, "\n");
    }
}

bool compare_blocks(void * a, void * b, sbf_data_type dtype) {
    switch(dtype) {
        case(SBF_INT):
            return *(int *)(a) == *(int *)(b);
        case(SBF_LONG):
            return *(long *)(a) == *(long *)(b);
        case(SBF_DOUBLE):
            return (fabs(*(double *)(a) - *(double *)(b)) < eps);
        case(SBF_FLOAT):
            return (fabs(*(float *)(a) - *(float *)(b)) < eps);
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
                log(verbose_info, "D '%s' @(",dset1.name);
                for(sbf_byte dim = 0; dim < dims; dim++) log(verbose_info, "%s%"PRIu64, (dim ==0)? "":",", idx[dim]);
                fprintf(stdout, "):");
                pretty_print_block(data1 + offset1, fmt_string, dset1.data_type);
                fprintf(stdout, " < >");
                pretty_print_block(data2 + offset2, fmt_string, dset2.data_type);
                fprintf(stdout, "\n");
            }
            diffs++;
        }
        increment_index(dset1.shape, idx, dims);
    } while(!all_zero(idx));
    return diffs;
}

sbf_result load_datasets(sbf_File *file) {
    //TODO error check
    for(int_fast8_t i = 0; i < file->n_datasets; i++) {
        sbf_DataHeader dset = file->datasets[i];
        file->dataset_pointers[i] = calloc(sbf_datatype_size(dset), sbf_num_blocks(dset));
        sbf_result res = sbf_read_dataset(file, dset, file->dataset_pointers[i]);
        if(res != SBF_RESULT_SUCCESS) {
            log(error, "Problem reading dataset '%s' in %s: %s\n", 
                dset.name, file->filename,
                (res == SBF_RESULT_NULL_FAILURE) ? "failure allocating memory" : strerror(errno));
            return res;
        }
    }
    return SBF_RESULT_SUCCESS;
}

sbf_result cleanup_datasets(sbf_File *file) {
    for(int_fast8_t i = 0; i < file->n_datasets; i++) {
        free(file->dataset_pointers[i]);
    }
    return SBF_RESULT_SUCCESS;
}

sbf_size diff_files(sbf_File * file1, sbf_File * file2) {
    sbf_byte n1 = file1->n_datasets; sbf_byte n2 = file2->n_datasets;
    if(n2 > n1) {
        sbf_File * tmp; sbf_byte n_tmp;
        tmp = file1; file1 = file2; file2 = tmp;
        n_tmp = n2; n1 = n2; n2 = n_tmp;
    }
    sbf_size file_diffs = 0;
    bool deep_check = true;
    if(n1 != n2) {
        log(verbose_info, "Different number of datasets: %d, %d\n", n1, n2);
        ++file_diffs;
    }

    if(deep_check) {
        load_datasets(file1);
        load_datasets(file2);
    }

    // TODO process longest file first
    for(sbf_byte i = 0; i < n1;  i++) {
        log(debug, "Checking dataset %d in %s\n", i, file1->filename);
        sbf_DataHeader dset1 = file1->datasets[i];
        sbf_size dset_diffs = 0;
        int dset_found = get_dataset(dset1.name, file2);
        if(dset_found == -1) {
            log(verbose_info, "No matching dataset found for '%s' in %s\n",
                dset1.name, file2->filename);
            file_diffs++;
        }
        else {
            sbf_DataHeader dset2 = file2->datasets[dset_found];
            void * data2 = file2->dataset_pointers[i];
            bool dims_equal = (SBF_GET_DIMENSIONS(dset1) == SBF_GET_DIMENSIONS(dset2));
            bool dtypes_equal = (dset1.data_type == dset2.data_type);
            bool shapes_equal = shape_equal(dset1.shape, dset2.shape);

            if(!dims_equal) {
                log(verbose_info, "D '%s' incompatible dimensions: %d < > %d\n",
                    dset1.name, SBF_GET_DIMENSIONS(dset1), SBF_GET_DIMENSIONS(dset2));
                dset_diffs++;
            }
            if(!dtypes_equal) {
                log(verbose_info, "D '%s' incompatible data types: %s < > %s\n",
                    dset1.name, sbf_datatype_name(dset1.data_type), sbf_datatype_name(dset2.data_type));
                dset_diffs++;
            }
            if(!shapes_equal) {
                log(verbose_info, "D '%s' incompatible shapes: ", dset1.name);
                for(int_fast8_t i = 0; i < SBF_GET_DIMENSIONS(dset1); i++) log(verbose_info, "[%"PRIu64"]", dset1.shape[i]);
                log(verbose_info, " %s ", "< >");
                for(int_fast8_t i = 0; i < SBF_GET_DIMENSIONS(dset2); i++) log(verbose_info, "[%"PRIu64"]", dset2.shape[i]);
                log(verbose_info, "%s\n", "");
                dset_diffs++;
            }
            if(deep_check && dims_equal && shapes_equal && dtypes_equal) {
                dset_diffs = dset_diffs + diff_datablocks(dset1, file1->dataset_pointers[i],
                                                          dset2, file2->dataset_pointers[dset_found]);
            }
            log(verbose_info, "%"PRIu64" differences in dataset '%s'\n", dset_diffs, dset1.name);
            file_diffs = file_diffs + dset_diffs;
        }
    }
    if(deep_check) {
        cleanup_datasets(file1);
        cleanup_datasets(file2);
    }
    return file_diffs;
}

int main(int argc, char *argv[]) {
    extern char *optarg;
    extern int optind;
    bool dump_file = false, list_datasets = true, diff = false;
    char * e_arg = NULL;
    int c;
    int retcode = 0;

    opterr = 0;
    while ((c = getopt (argc, argv, "e:cplhv")) != -1)
        switch (c)
        {
            case 'c':
                diff = true;
                break;
            case 'p':
                dump_file = true;
                break;
            case 'l':
                list_datasets = true;
                break;
            case 'h':
                usage(argv[0]);
                break;
            case 'v':
                if(GLOBAL_LOG_LEVEL < info) GLOBAL_LOG_LEVEL = info;
                else GLOBAL_LOG_LEVEL++;
                break;
            case 'e':
                eps = strtod(optarg, NULL);
                break;
            case '?':
                if(optopt == 'e')
                    log(error, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    log(error, "Unknown option -%c.\n", optopt);
                else
                    log(error, "Unknown option character '\\x%x'.\n", optopt);
            default:
                exit(EXIT_FAILURE);
        }

    if(diff) {
        if(optind != (argc - 2)) {
            log(error, "Option -c requires two filenames e.g.: %s %s\n",
                "file1.sbf", "file2.sbf");
            usage(argv[0]);
        }
        sbf_File file1 = sbf_new_file; sbf_File file2 = sbf_new_file;
        file1.mode = SBF_FILE_READONLY; file2.mode = SBF_FILE_READONLY;
        file1.filename = argv[optind]; file2.filename = argv[optind+1];
        SBF_ASSERT_SUCCESSFUL(sbf_open(&file1));
        SBF_ASSERT_SUCCESSFUL(sbf_open(&file2));
        SBF_ASSERT_SUCCESSFUL(sbf_read_headers(&file1));
        SBF_ASSERT_SUCCESSFUL(sbf_read_headers(&file2));

        sbf_size diffs = diff_files(&file1, &file2);
        // If we found differences in the file, print the number out
        if(diffs)
            log(info, "%"PRIu64" difference%s between %s and %s\n",
                diffs, diffs > 1 ? "s" : "", file1.filename, file2.filename);

        SBF_ASSERT_SUCCESSFUL(sbf_close(&file1));
        SBF_ASSERT_SUCCESSFUL(sbf_close(&file2));
        // and return it as an exit value
        retcode = diffs;
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
            log(info, "Processing '%s'...\n", file.filename);
            sbf_result res;
            SBF_ASSERT_SUCCESSFUL(sbf_open(&file));
            if(sbf_read_headers(&file) != SBF_RESULT_SUCCESS) {
                retcode = EXIT_FAILURE;
                continue;
            }
            if(list_datasets || dump_file) dump_file_as_utf8(&file, dump_file);
            SBF_ASSERT_SUCCESSFUL(sbf_close(&file));
        }
   
    }
    return retcode;
}
