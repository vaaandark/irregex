#include "irregex.h"

void help() {
    fprintf(stderr, "irregex: usage: ");
    fprintf(stderr, "irregex {regex} {string}\n");
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        help();
        return 1;
    }
    const char *regex = argv[1];
    const char *string = argv[2];
    NFAGraph g = regex_compile(regex);
    int len = regex_execute(&g, string);
    if (len != -1) {
        char str[256];
        strcpy(str, string);
        str[len] = '\0';
        puts(str);
    }
    regex_free(&g);
    return 0;
}
