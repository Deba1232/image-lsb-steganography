#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *argv[]){
    if(!strcmp(argv[1],"-e")){
        return e_encode;
    }
    else if(!strcmp(argv[1],"-d")){
        return e_decode;
    }
    else{
        return e_unsupported;
    }
}

int main(int argc,char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decodeInfo;
    uint img_size;

    // Check whether the proper amt. of CLA has been passed or not
    if(argc == 1){
        printf("%s: missing operands\n",argv[0]);
        printf("Try '%s --help' for more information\n",argv[0]);
    }
    else if(argc == 2){

        if(!strcmp(argv[1],"--help")){
            printf("Usage: %s -e <bmp_file> <secret_file> [output_bmp_file (optional)]\n",argv[0]);
            printf("Usage: %s -d <output_bmp_file> [output_decoded_file (optional)]\n",argv[0]);
            printf("Encode a secret message to a bitmap file or decode the secret message from a bitmap file\n");
            printf("\nOptions\n-e\tEncode the bitmap file\n-d\tDecode the bitmap file\n");
            printf("\nInput bitmap file and secret file to encode the given bitmap file are MANDATORY to provide during encoding whereas output encoded bitmap file is MANDATORY to provide during decoding\n");
        }
        else if(check_operation_type(argv) == e_encode || check_operation_type(argv) == e_decode){
            printf("%s: missing file operand\n",argv[0]);
            printf("Try '%s --help' for more information\n",argv[0]);
        }
        else{
            fprintf(stderr,"%s: invalid option '%s'\n",argv[0],argv[1]);
            printf("Try '%s --help' for more information\n",argv[0]);
        }
    }
    else if((argc > 3) && (argc < 6)){
        //Encode operation
        if(check_operation_type(argv) == e_encode){
            printf("## Starting encoding procedure ##\n");

            if((read_and_validate_encode_args(argv,&encInfo) == e_success)){

                if(do_encoding(&encInfo) == e_success){
                    copy_remaining_img_data(encInfo.fptr_src_image,encInfo.fptr_stego_image);
                    printf("\n## Encoding done successfully ##\n");
                }
                else{
                  fprintf(stderr,"## Aborting encoding procedure ##\n");  
                }
            }
            else{
                fprintf(stderr,"## Aborting encoding procedure ##\n");
            }
        }
        //Decode operation
        else if(check_operation_type(argv) == e_decode){
            
            if(argc < 5){
                printf("## Starting decoding procedure ##\n");

                if(read_and_validate_decode_args(argv,&decodeInfo) == d_success){
                    
                    if(read_and_validate_decode_args(argv,&decodeInfo) == d_success){
                    
                        if(do_decoding(&decodeInfo) == d_success){
                            // printf("\n## Decoding done successfully ##\n");
                        }
                        else{
                            fprintf(stderr,"## Aborting decoding procedure ##\n");
                        }
                    }
                }
                else{
                    fprintf(stderr,"## Aborting decoding procedure ##\n");
                }
            }
            else{
                printf("Usage: %s -e <bmp_file> <secret_file> [output_bmp_file (optional)]\n",argv[0]);
                printf("Usage: %s -d <output_bmp_file> [output_decoded_file (optional)]\n",argv[0]);
                printf("Encode a secret message to a bitmap file or decode the secret message from a bitmap file\n");
                printf("\nOptions\n-e\tEncode the bitmap file\n-d\tDecode the bitmap file\n");
                printf("\nInput bitmap file and secret file to encode the given bitmap file are MANDATORY to provide during encoding whereas output encoded bitmap file is MANDATORY to provide during decoding\n");
            }
        }
        else{
            fprintf(stderr,"%s: invalid option '%s'\n",argv[0],argv[1]);
            printf("Try '%s --help' for more information\n",argv[0]);
        }
    }
    else{

        if(argc == 3){

            if(check_operation_type(argv) == e_decode){
                printf("## Starting decoding procedure ##\n");

                if(read_and_validate_decode_args(argv,&decodeInfo) == d_success){
                    
                    if(do_decoding(&decodeInfo) == d_success){
                        // printf("\n## Decoding done successfully ##\n");
                    }
                    else{
                        fprintf(stderr,"## Aborting decoding procedure ##\n");
                    }
                }
                else{
                    fprintf(stderr,"## Aborting decoding procedure ##\n");
                }
            }
            else if(check_operation_type(argv) == e_encode){
                printf("Usage: %s -e <bmp_file> <secret_file> [output_bmp_file (optional)]\n",argv[0]);
                printf("Usage: %s -d <output_bmp_file> [output_decoded_file (optional)]\n",argv[0]);
                printf("Encode a secret message to a bitmap file or decode the secret message from a bitmap file\n");
                printf("\nOptions\n-e\tEncode the bitmap file\n-d\tDecode the bitmap file\n");
                printf("\nInput bitmap file and secret file to encode the given bitmap file are MANDATORY to provide during encoding whereas output encoded bitmap file is MANDATORY to provide during decoding\n");
            }
            else{
                fprintf(stderr,"%s: invalid option '%s'\n",argv[0],argv[1]);
                printf("Try '%s --help' for more information\n",argv[0]);
            }
        }
        else{
            printf("Usage: %s -e <bmp_file> <secret_file> [output_bmp_file (optional)]\n",argv[0]);
            printf("Usage: %s -d <output_bmp_file> [output_decoded_file (optional)]\n",argv[0]);
            printf("Encode a secret message to a bitmap file or decode the secret message from a bitmap file\n");
            printf("\nOptions\n-e\tEncode the bitmap file\n-d\tDecode the bitmap file\n");
            printf("\nInput bitmap file and secret file to encode the given bitmap file are MANDATORY to provide during encoding whereas output encoded bitmap file is MANDATORY to provide during decoding\n");
        }
    }

    return 0;
}
