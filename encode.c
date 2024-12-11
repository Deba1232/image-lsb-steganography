#include <stdio.h>
#include <string.h>
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
    uint magic_string_size = 2 * 8;

    /*
        The number of bytes required to encode the length of the secret file extension.
        For example, if the file extension is ".txt" (4 characters), we need 4 * 8 bits 
        (or 32 bits) to store this length so the decoder can identify how many bytes to 
        read for the file extension.
    */
    uint secret_file_extn_length_size = strlen(encodeInfo->extn_secret_file) * 8;

    /*
        The number of bytes required to encode the actual secret file extension name.
        For instance, if the extension is ".txt" (4 characters), this requires 4 * 8 bits 
        (or 32 bits).
    */
    uint secret_file_extn_name_size = strlen(encodeInfo->extn_secret_file) * 8;

    //The number of bytes required to encode the length of the secret file's text content.
    uint secret_file_text_length_size = encodeInfo->size_secret_file * 8;

    //The number of bytes required to encode the actual text content of the secret file.
    uint secret_file_text_size = encodeInfo->size_secret_file * 8;

    uint expected_capacity = bmp_header_size + magic_string_size + secret_file_extn_length_size + secret_file_extn_name_size + secret_file_text_length_size + secret_file_text_size;

    if(encodeInfo->image_capacity <= expected_capacity){
        return e_failure;
    }
    
    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_stegano_image){
    printf("INFO: Copying image header\n");
    
}

Status do_encoding(EncodeInfo *encodeInfo){
    printf("INFO: Opening required files...\n");

    if(open_files(encodeInfo) == e_success){

        if(check_capacity(encodeInfo) == e_success){
            printf("INFO: Done. Found OK\n");
            printf("\n## Encoding procedure started ##\n");

            if(copy_bmp_header(encodeInfo->fptr_src_image,encodeInfo->fptr_stego_image) == e_success){
                printf("INFO: Done\n");
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