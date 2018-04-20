#include "comms.h"
#include <polysat.h>
/* This utility is simply for taking a properly formatted argv and translating it into a 
   string, which can then be translated back into an argv array.
   The string is just a concatenation of all the argv strings with null-bytes in separating 
   the different arguments 
*/

/* This returns a malloc'd string that can be decoded by argvDecode */
int argvEncode(int argc, char **argv, char **result) {
    size_t *lengths = malloc(sizeof(int) * argc);  // So we don't have to keep recomputing the length of the string
    int total_size = 0;                         // This counts the total number of bytes that argv will take up

    // We just count the total command size
    for (int i = 0; i < argc; i++) {
        lengths[i] = strlen(argv[i]) + 1; // Null byte
        total_size += lengths[i];
    }

    char *res = malloc(sizeof(char)*total_size);
    if (!res) {
        fprintf(stderr, "Could not malloc for argv");
        return -1;
    }

    int p = 0; // This denotes how far we are in the malloc'd memory RES.
    for (int i = 0; i < argc; i++) {
        strncpy(res + p, argv[i], lengths[i]);
        p += lengths[i];
    }
    free(lengths);

    *result = res;
    return total_size;
}

/* STR must be string provided by the output of argvEncode, returns a malloc'd  char **argv which needs to be free'd
   However, DO NOT free the underlying strings and DO NOT free the inputted STR. We simply point our ARGV pointers to 
   various locations on STR to create argv 
*/
char **argvDecode(char *str, int strLen, int argc) {
    char **argv = malloc(sizeof(char*) * (argc+1));
    int j = 0;
    int start = 1;
    for (int i = 0; i < strLen && j < argc; i++) {
        if (start) {
            start = 0;
            argv[j] = str+i;
            j++;
        }
        else if (str[i] == 0) {
            str[i] = 0;
            start = 1;
        }
    }
    argv[argc] = NULL;
    if (j != argc) {
        DBG_print(DBG_LEVEL_WARN, "Argument number mismatch\n");
    }
    return argv;
}
