#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
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
    uint img_size;

    // Check whether the proper amt. of CLA has been passed or not
    if(argc == 1){
        printf("%s: missing operands\n",argv[0]);
        printf("Try '%s --help' for more information\n",argv[0]);
    }
    else if(argc == 2){

        if(!strcmp(argv[1],"--help")){
            printf("Usage: %s [OPTION] <bmp_file> <secret_file> [output_bmp_file (optional)]\n",argv[0]);
            printf("Encode a secret message to a bitmap file or decode the secret message from a bitmap file\n");
            printf("\nOptions\n-e\tEncode the bitmap file\n-d\tDecode the bitmap file\n");
            printf("\nInput bitmap file and secret file to encode bitmap file are MANDATORY to provide\n");
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

        if(check_operation_type(argv) == e_encode){
            printf("## Starting encoding procedure ##\n");

            if((read_and_validate_encode_args(argv,&encInfo) == e_success)){

                if(do_encoding(&encInfo) == e_success){
                    // printf("## Encoding done successfully ##\n");
                }
                else{
                  fprintf(stderr,"## Aborting encoding procedure ##\n");  
                }
            }
            else{
                fprintf(stderr,"## Aborting encoding procedure ##\n");
            }
        }
        else if(check_operation_type(argv) == e_decode){
            //decode operation to be added
        }
        else{
            fprintf(stderr,"%s: invalid option '%s'\n",argv[0],argv[1]);
            printf("Try '%s --help' for more information\n",argv[0]);
        }
    }
    else{
        printf("Usage: %s [OPTION] <bmp_file> <secret_file> [output_bmp_file (optional)]\n",argv[0]);
        printf("Encode a secret message to a bitmap file or decode the secret message from a bitmap file\n");
        printf("\nOptions\n-e\tEncode the bitmap file\n-d\tDecode the bitmap file\n");
        printf("\nInput bitmap file and secret file to encode bitmap file are MANDATORY to provide\n");
    }

    return 0;
}
