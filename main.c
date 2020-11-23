#include "utils.h"


int main() {
    char arguments[256] = "ftp://[<user>:<password>@]<host>/<url-path>";
    struct fields fields;

    parse_fields(arguments, &fields);

    return 0;
}