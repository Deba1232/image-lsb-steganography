#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

#define MAX_FILE_NAME_SIZE 100

/* 
    Structure to store information required for decoding secret message from encoded image
*/

typedef struct{
    char *image_to_decode_fname; //to store the encoded image file name
    FILE *fptr_image_to_decode; //to store encoded file
    
    char decoded_output_fname[MAX_FILE_NAME_SIZE]; //to store the file name for decoded data
    FILE *fptr_decoded_output; //to store the decoded file
    char *decoded_magic_string; //to store the decoded magic string
    int decoded_extn_size; //to store the decoded extension size
    char *decoded_extn; //to store the decoded extension name

} DecodeInfo;

/* Read and validate decode args from argv */
DecodeStatus read_and_validate_decode_args(char *argv[],DecodeInfo *decodeInfo);

/* Encoded bit in LSB */
int encoded_lsb_bit(char data_buffer);

/* Decode function which decodes lsb data bits from encoded image */
char *decode_data_bits_from_encoded_image(int data_size,FILE *fptr_image_to_decode);

/* Decode the magic string */
DecodeStatus decode_magic_string(DecodeInfo *decodeInfo);

/* Decode the extension size for the encoded file extension */
DecodeStatus decode_extn_size(DecodeInfo *decodeInfo);

/* Decode the extension */
DecodeStatus decode_extn(DecodeInfo *decodeInfo);

/* Decode the size for the encoded secret data */
DecodeStatus decode_secret_data_size(DecodeInfo *decodeInfo);

/* Performing the decoding */
DecodeStatus do_decoding(DecodeInfo *decodeInfo);

#endif