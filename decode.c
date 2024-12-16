#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "common.h"

int power_of_two(int power_value){
    if(power_value == 0){
        return 1;
    }
    return 2 * power_of_two(power_value - 1);
}

/*
    Check whether files with proper extension have been passed or not in CLA
*/
DecodeStatus read_and_validate_decode_args(char *argv[],DecodeInfo *decodeInfo){
    if(strstr(argv[2],".bmp")){

        if(*(strstr(argv[2],".bmp") + 4) == '\0'){
            decodeInfo->image_to_decode_fname = argv[2];

            if(argv[3]){
                
                if(strstr(argv[3],".")){
                    strcpy(decodeInfo->decoded_output_fname,strtok(argv[3],"."));
                }
                else{
                    char *temp = argv[3];

                    while(*temp){
                        if(!(*temp>='A' && *temp<='Z') && !(*temp>='a' && *temp<='z') && !(*temp>='0' && *temp<='9')){
                            fprintf(stderr,"ERROR: Please provide a proper name for the decoded data file\n");
                            return d_failure;
                        }
                        temp++;
                    }
                    strcpy(decodeInfo->decoded_output_fname,argv[3]);
                }
            }
            else{
                printf("INFO: Output File not mentioned. Creating \"decoded_data\" as default file name\n");
                strcpy(decodeInfo->decoded_output_fname,"decoded_data");
            }
        }
        else{
            fprintf(stderr,"ERROR: Extension for encoded image file is not proper\n");
            return d_failure;
        }
    }
    else{
        fprintf(stderr,"ERROR: Not a bmp file\n");
        return d_failure;
    }

    return d_success;
}

int encoded_lsb_bit(char data_buffer){
    return data_buffer & 0x1;
}

char *decode_data_bits_from_encoded_image(int data_size,FILE *fptr_image_to_decode){
    /*
        Since data is encoded in LSB, to decode each character byte, we need 8bytes of data from the encoded image
    */
    char *bytes_to_read_buffer = (char *)calloc(data_size*8,sizeof(char));
    if(bytes_to_read_buffer == NULL){
        fprintf(stderr,"Memory couldn't be allocated for bytes to read buffer\n");
        exit(EXIT_FAILURE);
    }
    //Store the decoded data
    char *decoded_data_bits = (char *)calloc(data_size*8,sizeof(char));
    if(decoded_data_bits == NULL){
        fprintf(stderr,"Memory couldn't be allocated for decoded data\n");
        exit(EXIT_FAILURE);
    }

    fread(bytes_to_read_buffer,data_size*8,1,fptr_image_to_decode);

    for(int i=0;i<data_size*8;i++){
        decoded_data_bits[i] = encoded_lsb_bit(bytes_to_read_buffer[i]);
    }
    
    free(bytes_to_read_buffer);

    return decoded_data_bits;
}

DecodeStatus decode_magic_string(DecodeInfo *decodeInfo){
    printf("INFO: Decoding magic string signature\n");

    int idx = 0;

    char *decoded_magic_string_bits = (char *)calloc(strlen(MAGIC_STRING),sizeof(char));
    if(decoded_magic_string_bits == NULL){
        fprintf(stderr,"Memory couldn't be allocated for decoded magic string bits\n");
        exit(EXIT_FAILURE);
    }

    char *decoded_magic_string = (char *)calloc(strlen(MAGIC_STRING),sizeof(char));
    if(decoded_magic_string == NULL){
        fprintf(stderr,"Memory couldn't be allocated for decoded magic string\n");
        exit(EXIT_FAILURE);
    }

    decoded_magic_string_bits = decode_data_bits_from_encoded_image(strlen(MAGIC_STRING),decodeInfo->fptr_image_to_decode);
    /*
        Since each character consists of 8bits, to convert to decimal, the highest power that 2 can have is 7
    */
    for(int i=0;i<strlen(MAGIC_STRING)*8;i+=8){
        int bin_to_decimal_value = 0;
        for(int j=i;j<(i+8);j++){
            //here j value ranges from i to i+7
            bin_to_decimal_value = bin_to_decimal_value + (decoded_magic_string_bits[j] * power_of_two(7-(j-i)));
        }
        decoded_magic_string[idx++] = bin_to_decimal_value;
    }
    decoded_magic_string[idx] = '\0';

    if(!strcmp(decoded_magic_string,MAGIC_STRING)){
        return d_success;
    }
    else{
        fprintf(stderr,"ERROR: Decoded magic string doesn't match with the provided magic string\n");
        return d_failure;
    }
    
    free(decoded_magic_string_bits);
    free(decoded_magic_string);
}

DecodeStatus decode_extn_size(DecodeInfo *decodeInfo){
    printf("INFO: Decoding the extension size in order to get the encoded extension properly\n");

    int bin_to_decimal_value = 0;
    /*
        Since size of extn is essentially an integer value, sizeof(int)*8 bytes of data is required to decode the extn size
    */
    char *decoded_extn_size_bits = (char *)calloc(sizeof(int)*8,sizeof(char));
    if(decoded_extn_size_bits == NULL){
        fprintf(stderr,"Memory couldn't be allocated for decoded extn size bits\n");
        exit(EXIT_FAILURE);
    }

    char *decoded_extn_size = (char *)calloc(sizeof(int)*8,sizeof(char));
    if(decoded_extn_size == NULL){
        fprintf(stderr,"Memory couldn't be allocated for decoded extn size\n");
        exit(EXIT_FAILURE);
    }

    decoded_extn_size_bits = decode_data_bits_from_encoded_image(sizeof(int),decodeInfo->fptr_image_to_decode);

    for(int i=0;i<sizeof(int)*8;i++){
    /*
        Since size of integer values is 32bits, to convert to decimal, the highest power that 2 can have is 31
    */
        bin_to_decimal_value = bin_to_decimal_value + (decoded_extn_size_bits[i] * power_of_two(31-i));
    }

    decodeInfo->decoded_extn_size = bin_to_decimal_value;

    free(decoded_extn_size_bits);
    free(decoded_extn_size);
    
    return d_success;
}

DecodeStatus decode_secret_data_size(DecodeInfo *decodeInfo){
    printf("INFO: Decoding file size\n");
    
    return d_success;
}

DecodeStatus decode_extn(DecodeInfo *decodeInfo){
    printf("INFO: Decoding output file extension\n");

    int idx = 0;
    
    char *decoded_extn_bits = (char *)calloc(decodeInfo->decoded_extn_size*8,sizeof(char));
    if(decoded_extn_bits == NULL){
        fprintf(stderr,"Memory couldn't be allocated for decoded magic string bits\n");
        exit(EXIT_FAILURE);
    }

    char *decoded_extn = (char *)calloc(decodeInfo->decoded_extn_size*8,sizeof(char));
    if(decoded_extn == NULL){
        fprintf(stderr,"Memory couldn't be allocated for decoded magic string\n");
        exit(EXIT_FAILURE);
    }

    decoded_extn_bits = decode_data_bits_from_encoded_image(decodeInfo->decoded_extn_size,decodeInfo->fptr_image_to_decode);

    for(int i=0;i<decodeInfo->decoded_extn_size*8;i+=8){
    /*
        Since each character consists of 8bits, to convert to decimal, the highest power that 2 can have is 7
    */
        int bin_to_decimal_value = 0;
        
        for(int j=i;j<(i+8);j++){
            bin_to_decimal_value = bin_to_decimal_value + (decoded_extn_bits[j] * power_of_two(7-(j-i)));
        }
        decoded_extn[idx++] = bin_to_decimal_value;
   
    }
    decoded_extn[idx] = '\0';

    strcpy(decodeInfo->decoded_output_fname,strcat(decodeInfo->decoded_output_fname,decoded_extn));

    return d_success;
}

DecodeStatus do_decoding(DecodeInfo *decodeInfo){
    printf("INFO: Opening required files\n");

    //Encode image file
    decodeInfo->fptr_image_to_decode = fopen(decodeInfo->image_to_decode_fname,"rb");

    if(decodeInfo->fptr_image_to_decode == NULL){
        fprintf(stderr,"ERROR: Unable to open encoded image file %s\n", decodeInfo->image_to_decode_fname);
        perror("fopen");

        return d_failure;
    }
    printf("INFO: Opened encoded bmp file\n");
    //Skipping 54bytes of header data from encoded image file
    fseek(decodeInfo->fptr_image_to_decode,54,SEEK_SET);

    if(decode_magic_string(decodeInfo) == d_success){
        printf("INFO: Done\n");

        if(decode_extn_size(decodeInfo) == d_success){
            printf("INFO: Done\n");

            if(decode_extn(decodeInfo) == d_success){
                printf("INFO: Done\n");
                //Decoded data output file
                decodeInfo->fptr_decoded_output = fopen(decodeInfo->decoded_output_fname,"wb");
                if(decodeInfo->fptr_decoded_output == NULL){
                    fprintf(stderr,"ERROR: Unable to open output file for decoded data %s\n", decodeInfo->decoded_output_fname);
                    perror("fopen");

                    return d_failure;
                }
                printf("INFO: Opened %s\n",decodeInfo->decoded_output_fname);

                if(decode_secret_data_size(decodeInfo) == d_success){
                    printf("INFO: Done\n");
                }
                else{
                    return d_failure;
                }
            }
            else{
                return d_failure;
            }
        }
        else{
            return d_failure;
        }
    }
    else{
        return d_failure;
    }

    return d_success;
}