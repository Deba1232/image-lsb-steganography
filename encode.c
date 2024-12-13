#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "encode.h"
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr_secret_file){
    //Seek to end of file
    fseek(fptr_secret_file,0L,SEEK_END);

    // Return the file size 
    return ftell(fptr_secret_file);
}

/*
    Check whether files with proper extensions have been passed or not in CLA
*/
Status read_and_validate_encode_args(char *argv[],EncodeInfo *encodeInfo){
    if(strstr(argv[2],".bmp")){
        
        if(*(strstr(argv[2],".bmp") + 4) == '\0'){
           encodeInfo->src_image_fname = argv[2];
           
           if(strstr(argv[3],".")){
                //Check if the extension name of the file with secret code is proper or not
                if((strlen(strstr(argv[3],".")) > 1) && (strlen(strstr(argv[3],".")) < 5)){
                    strcpy(encodeInfo->extn_secret_file,strstr(argv[3],"."));

                    for(int i=1;i<strlen(encodeInfo->extn_secret_file);i++){
                        if(!(encodeInfo->extn_secret_file[i]>='A' && encodeInfo->extn_secret_file[i]<='Z') && !(encodeInfo->extn_secret_file[i]>='a' && encodeInfo->extn_secret_file[i]<='z') && !(encodeInfo->extn_secret_file[i]>='0' && encodeInfo->extn_secret_file[i]<='9')){
                            fprintf(stderr,"ERROR: Not a proper extension name for secret file\n");  
                            return e_failure;
                        }
                    }
                }
                else{
                    fprintf(stderr,"ERROR: Not a proper extension for secret file\n");  
                    return e_failure;
                }

                encodeInfo->secret_fname = argv[3];

                if(argv[4]){
                    if(strstr(argv[4],".bmp")){

                        if(*(strstr(argv[4],".bmp") + 4) == '\0'){
                            encodeInfo->stego_image_fname = argv[4];
                        }
                        else{
                            fprintf(stderr,"ERROR: Extension of output image file is not proper\n");
                            return e_failure;
                        }
                    }
                    else{
                        fprintf(stderr,"ERROR: Output file not a .bmp file\n");
                        return e_failure;
                    }
                }
                else{
                    printf("INFO: Output File not mentioned. Creating \"steged_img.bmp\" as default\n");
                    encodeInfo->stego_image_fname = "steged_img.bmp";
                }
            }
           else{
              fprintf(stderr,"ERROR: Please provide an extension for the secret file\n");  
              return e_failure;
            }
        }
        else{
            fprintf(stderr,"ERROR: Extension of image file to be encoded is not proper\n");
            return e_failure;
        }
    }
    else{
        fprintf(stderr,"ERROR: File to be encoded not a .bmp file\n");
        return e_failure;
    }

    return e_success;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encodeInfo)
{
    // Src Image file
    encodeInfo->fptr_src_image = fopen(encodeInfo->src_image_fname, "rb");
    // Do Error handling
    if (encodeInfo->fptr_src_image == NULL)
    {
    	fprintf(stderr, "ERROR: Unable to open image file %s\n", encodeInfo->src_image_fname);
        perror("fopen");

    	return e_failure;
    }
    printf("INFO: Opened %s\n",encodeInfo->src_image_fname);

    // Secret file
    encodeInfo->fptr_secret = fopen(encodeInfo->secret_fname, "rb");
    // Do Error handling
    if (encodeInfo->fptr_secret == NULL)
    {
    	fprintf(stderr, "ERROR: Unable to open secret file %s\n", encodeInfo->secret_fname);
        perror("fopen");

    	return e_failure;
    }
    printf("INFO: Opened %s\n",encodeInfo->secret_fname);

    // Stego Image file
    encodeInfo->fptr_stego_image = fopen(encodeInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encodeInfo->fptr_stego_image == NULL)
    {
    	fprintf(stderr, "ERROR: Unable to open output file %s\n", encodeInfo->stego_image_fname);
        perror("fopen");

    	return e_failure;
    }
    printf("INFO: Opened %s\n",encodeInfo->stego_image_fname);
    printf("INFO: Done\n");

    // No failure return e_success
    return e_success;
}

/*
    Check whether the source bmp file has the capacity to encode contents of secret file
*/

Status check_capacity(EncodeInfo *encodeInfo){
    printf("INFO: Checking for %s size\n",encodeInfo->secret_fname);
    encodeInfo->size_secret_file = get_file_size(encodeInfo->fptr_secret);

    if(encodeInfo->size_secret_file){
        printf("INFO: Done. Not empty\n");
    }
    else{
        printf("INFO: Secret file %s is empty, nothing to encode\n",encodeInfo->secret_fname);
    }


    printf("INFO: Checking for %s capacity to handle %s\n",encodeInfo->src_image_fname,encodeInfo->secret_fname);

    encodeInfo->image_capacity = get_image_size_for_bmp(encodeInfo->fptr_src_image);


    // The header of a .bmp file is 54 bytes by default.
    uint bmp_header_size = 54;

    /*
        Each RGB pixel in a BMP image contains 3 channels (Red, Green, Blue), 
        with each channel being 1 byte (8 bits). To encode one byte of data 
        (using the least significant bit of each channel), 8 bytes of RGB data 
        are required.
    */

    // Bytes required to encode the magic string "#."
    uint magic_string_size = strlen(MAGIC_STRING) * 8;

    /*
        The number of bytes required to encode the length of the secret file extension (which will be an integer value).
    */
    size_t secret_file_extn_length_size = sizeof((int)strlen(encodeInfo->extn_secret_file)) * 8;

    /*
        The number of bytes required to encode the actual secret file extension name.
        For example, if the file extension is ".txt" (4 characters), we need 4 * 8 bytes 
        (or 32 bytes) to store this length so the decoder can identify how many bytes to 
        read for the file extension.
    */
    uint secret_file_extn_name_size = strlen(encodeInfo->extn_secret_file) * 8;

    //The number of bytes required to encode the length of the secret file's text content.
    size_t secret_file_content_length_size = sizeof((int)encodeInfo->size_secret_file) * 8;

    //The number of bytes required to encode the actual text content of the secret file.
    uint secret_file_content_size = encodeInfo->size_secret_file * 8;

    uint expected_capacity = bmp_header_size + magic_string_size + secret_file_extn_length_size + secret_file_extn_name_size + secret_file_content_length_size + secret_file_content_size;

    if(encodeInfo->image_capacity <= expected_capacity){
        return e_failure;
    }
    
    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_stegano_image){
    printf("INFO: Copying image header\n");
    //since fptr_src_image now points to the 28th byte, rewind the file position indicator to the start
    rewind(fptr_src_image);

    char *bmp_header = (char *)calloc(54,sizeof(char));
    if(bmp_header == NULL){
        fprintf(stderr,"Memory couldn't be allocated for bmp_header\n");
        exit(EXIT_FAILURE);
    }

    fread(bmp_header,54,1,fptr_src_image);
    
    if(fwrite(bmp_header,54,1,fptr_stegano_image) != 1){
        return e_failure;
    }

    free(bmp_header);

    return e_success;
}

Status encode_data_size_to_lsb(size_t data_size, char *image_bytes_buffer){
    /*
        here the loop will iterate through the 32 bytes in image_buffer and encode each bit of integer data in LSB
    */
    for(int i=0;i<32;i++){
        //clearing the LSB bit
        image_bytes_buffer[i]&=0xFE;
        //setting the LSB bit with data_size bit
        image_bytes_buffer[i]|=(data_size & (1 << (31-i))) >> (31-i);
    }

}

Status encode_byte_to_lsb(char data, char *image_bytes_buffer){
    /*
        here the loop will iterate through the 8 bytes in image_buffer and encode each bit of character data in LSB
    */
    for(int i=0;i<8;i++){
        //clearing the LSB bit
        image_bytes_buffer[i]&=0xFE;
        //setting the LSB bit with data bit 
        image_bytes_buffer[i]|=(data & (1 << (7-i))) >> (7-i);
    }
}

Status encode_data_size_to_image(size_t data_size,FILE *fptr_src_img, FILE *fptr_stegano_img){
    char *image_bytes_buffer = (char *)calloc(32,sizeof(char));
    if(image_bytes_buffer == NULL){
        fprintf(stderr,"Memory couldn't be allocated for image_bytes_buffer\n");
        exit(EXIT_FAILURE);
    }
    /* 
        since sizeof(int) is 4bytes, and to encode each byte 8bytes of data is required from the src image file,  read 32bytes from source file and write to steganoed image file
    */
    fread(image_bytes_buffer,32,1,fptr_src_img);

    encode_data_size_to_lsb(data_size,image_bytes_buffer);

    fwrite(image_bytes_buffer,32,1,fptr_stegano_img);
    
    free(image_bytes_buffer);

    return e_success;
}

Status encode_data_to_image(const char *data,int data_size,FILE *fptr_src_img,FILE *fptr_stegano_img){
    char *image_bytes_buffer = (char *)calloc(8,sizeof(char));
    if(image_bytes_buffer == NULL){
        fprintf(stderr,"Memory couldn't be allocated for image_bytes_buffer\n");
        exit(EXIT_FAILURE);
    }
    /*
        since 8bytes from source bmp file are needed to encode 1byte of data, read 8bytes from source file and write to steganoed image file
    */
    for(int i=0;i<data_size;i++){
        fread(image_bytes_buffer,8,1,fptr_src_img);

        encode_byte_to_lsb(data[i],image_bytes_buffer);

        fwrite(image_bytes_buffer,8,1,fptr_stegano_img);
    }

    free(image_bytes_buffer);

    return e_success;
}

Status encode_magic_string(const char *magic_string,EncodeInfo *encodeInfo){
    printf("INFO: Encoding magic string signature\n");

    uint magic_string_length = strlen(magic_string);

    if(encode_data_to_image(magic_string,magic_string_length,encodeInfo->fptr_src_image,encodeInfo->fptr_stego_image) == e_success){
        // printf("INFO: Done\n");
    }
    else{
        fprintf(stderr,"ERROR: Magic string couldn't be encoded\n");
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_extn_size(size_t extn_size,EncodeInfo *encodeInfo){
    printf("INFO: Encoding size of secret file extension (an integer value)\n");

    if(encode_data_size_to_image(extn_size,encodeInfo->fptr_src_image,encodeInfo->fptr_stego_image) == e_success){

    }
    else{
        fprintf(stderr,"ERROR: Secret file extension size couldn't be encoded\n");
        return e_failure;
    }
    
    return e_success;
}

Status encode_secret_file_size(long file_size,EncodeInfo *encodeInfo){
    printf("INFO: Encoding size of secret file (an integer value)\n");

    if(encode_data_size_to_image(file_size,encodeInfo->fptr_src_image,encodeInfo->fptr_stego_image) == e_success){

    }
    else{
        fprintf(stderr,"ERROR: Secret file content size couldn't be encoded\n");
        return e_failure;
    }
    
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encodeInfo){
    printf("INFO: Encoding secret file extension\n");

    if(encode_data_to_image(file_extn,strlen(file_extn),encodeInfo->fptr_src_image,encodeInfo->fptr_stego_image) == e_success){

    }
    else{
        fprintf(stderr,"ERROR: Secret file extension couldn't be encoded\n");
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encodeInfo){
    printf("INFO: Encoding secret file data\n");

    rewind(encodeInfo->fptr_secret);

    fread(encodeInfo->secret_data,encodeInfo->size_secret_file,1,encodeInfo->fptr_secret);

    if(encode_data_to_image(encodeInfo->secret_data,strlen(encodeInfo->secret_data),encodeInfo->fptr_src_image,encodeInfo->fptr_stego_image) == e_success){

    }
    else{
        fprintf(stderr,"ERROR: Secret file data couldn't be encoded\n");
        return e_failure;
    }
    
    return e_success;
}

Status do_encoding(EncodeInfo *encodeInfo){
    printf("INFO: Opening required files...\n");

    if(open_files(encodeInfo) == e_success){

        if(check_capacity(encodeInfo) == e_success){
            printf("INFO: Done. Found OK\n");
            printf("\n## Encoding procedure started ##\n");

            if(copy_bmp_header(encodeInfo->fptr_src_image,encodeInfo->fptr_stego_image) == e_success){
                printf("INFO: Done\n");

                if(encode_magic_string(MAGIC_STRING,encodeInfo) == e_success){
                    printf("INFO: Done\n");

                    if(encode_secret_file_extn_size(sizeof((int)strlen(encodeInfo->extn_secret_file)),encodeInfo) == e_success){
                        printf("INFO: Done\n");

                        if(encode_secret_file_extn(encodeInfo->extn_secret_file,encodeInfo) == e_success){
                            printf("INFO: Done\n");

                            if(encode_secret_file_size(sizeof((int)encodeInfo->size_secret_file),encodeInfo) == e_success){
                                printf("INFO: Done\n");

                                if(encode_secret_file_data(encodeInfo) == e_success){
                                    printf("INFO: Done\n");
                                    fclose(encodeInfo->fptr_secret);
                                }
                                else{
                                    return e_failure;
                                }
                            }
                            else{
                                return e_failure;
                            }
                        }
                        else{
                            return e_failure;
                        }
                    }
                    else{
                        return e_failure;
                    }
                }
                else{
                    return e_failure; 
                }
            }
            else{
                fprintf(stderr,"ERROR: Image header couldn't be copied\n");
                return e_failure;
            }
        }
        else{
            fprintf(stderr,"ERROR: \"%s\" doesn't have the capacity to encode \"%s\"\n",encodeInfo->src_image_fname,encodeInfo->secret_fname);
            return e_failure;
        }
    }
    else{
        return e_failure;
    }
    
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src_img, FILE *fptr_stegano_img){
    /*
        here when we try to use fgetc(), the issue arises when we run the loop till EOF, since EOF expands to -1 and the src bmp file may contain 0xFF as a value for a color channel, which also converts to -1, due to which the loop stops whenever it encounters 0xFF, and doesn't run till the EOF, due to which we do not get a proper output for the steganoed bmp file. Due to this reason, fread() and fwrite() functions are used
    */ 
    char remaining_data_buffer[MAX_DATA_BUF_SIZE];    

    long file_pointer_pos_after_encode_operation = ftell(fptr_src_img);

    fseek(fptr_src_img,0L,SEEK_END);

    long end_of_file_pos = ftell(fptr_src_img);

    fseek(fptr_src_img,file_pointer_pos_after_encode_operation,SEEK_SET);

    //read from the current file pointer position till the end of file and write in output bmp file
    fread(remaining_data_buffer,(end_of_file_pos - file_pointer_pos_after_encode_operation),1,fptr_src_img);

    fwrite(remaining_data_buffer,(end_of_file_pos - file_pointer_pos_after_encode_operation),1,fptr_stegano_img);

    fclose(fptr_src_img);
    fclose(fptr_stegano_img);

    return e_success;
}