#ifndef ENCODE_H
#define ENCODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

// #define MAX_SECRET_BUF_SIZE 1
//#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 5
#define MAX_DATA_BUF_SIZE 5000000

typedef struct
{
    /* Source Image info */
    char *src_image_fname; //to store the source file name
    FILE *fptr_src_image; //to store src file
    uint image_capacity;
    //uint bits_per_pixel;
    //char image_data[MAX_IMAGE_BUF_SIZE];

    /* Secret File Info */
    char *secret_fname; //to store secret file name
    FILE *fptr_secret; //to store the secret file
    char extn_secret_file[MAX_FILE_SUFFIX];
    char secret_data[BUFSIZ];
    long size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname; //to store the output file name
    FILE *fptr_stego_image; //to store the output file

} EncodeInfo;


/* Encoding function prototype */

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint get_file_size(FILE *fptr);

/* Check operation type */
OperationType check_operation_type(char *argv[]);

/* Read and validate Encode args from argv */
EncodeStatus read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);

/* Get File pointers for i/p and o/p files */
EncodeStatus open_files(EncodeInfo *encInfo);

/* check capacity */
EncodeStatus check_capacity(EncodeInfo *encInfo);

/* Copy bmp image header */
EncodeStatus copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* Encode a byte into LSB of image data array */
EncodeStatus encode_byte_to_lsb(char data, char *image_buffer);

/* Encode size(which is an integer value) into LSB of image data array */
EncodeStatus encode_data_size_to_lsb(long data, char *image_buffer);

/* Encode function, which encodes data size to image */
EncodeStatus encode_data_size_to_image(long data_size, FILE *fptr_src_image, FILE *fptr_stego_image);

/* Encode function, which encodes data to image */
EncodeStatus encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image);

/* Store Magic String */
EncodeStatus encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Store secret file extension size */
EncodeStatus encode_secret_file_extn_size(int extn_size,EncodeInfo *encInfo);

/* Encode secret file size */
EncodeStatus encode_secret_file_size(long file_size, EncodeInfo *encInfo);

/* Encode secret file extenstion */
EncodeStatus encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

/* Encode secret file data*/
EncodeStatus encode_secret_file_data(EncodeInfo *encInfo);

/* Perform the encoding */
EncodeStatus do_encoding(EncodeInfo *encInfo);

/* Copy remaining image bytes from src to stego image after encoding */
EncodeStatus copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

#endif
