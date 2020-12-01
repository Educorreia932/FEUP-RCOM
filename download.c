#include "utils.h"


int main(int argc, char** argv) {

    if(argc != 2){
        printf("Usage download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);
    }

    struct fields fields;
    parse_fields(argv[1], &fields);

    return 0;
}