#include "sbf.h"
#include "unit_test.h"

int tests_run = 0;

void print_bits(sbf_byte byte) {
    const int bits_per_byte = 8;
    char bits[bits_per_byte + 1];
    for (int i = bits_per_byte - 1; i >= 0; i -= 1) {
        bits[i] = '0' + (byte & 0x01);
        byte >>= 1;
    }
    bits[bits_per_byte] = 0;
    printf("Byte contains: 0b%s\n", bits);
}

static char *test_header() {
    sbf_DataHeader header = sbf_new_data_header;
    assert("header size", sizeof(header) == 128);
    assert("initial header dimensions nonzero",
           SBF_GET_DIMENSIONS(header) == 0);

    SBF_SET_DIMENSIONS(header, 3);
    sbf_byte dim_3_flag = 0b00000011;
    assert("setting dimensions incorrect", header.flags == dim_3_flag);
    assert("getting dimensions incorrect", SBF_GET_DIMENSIONS(header) == 3);
    return 0;
}

static char *all_tests() {
    run_unit_test(test_header);
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
