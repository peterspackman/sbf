#include "sbf.h"
#include <assert.h>

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

int main(void) {

    sbf_DataHeader header = sbf_new_data_header;
    assert(SBF_GET_DIMENSIONS(header) == 0);

    SBF_SET_DIMENSIONS(header, 3);
    sbf_byte dim_3_flag = 0b00000011;
    assert(header.flags == dim_3_flag);

    assert(SBF_GET_DIMENSIONS(header) == 3);
    return 0;
}
