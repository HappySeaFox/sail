#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int sail_strdup(char *input, char **output) {

    return sail_strdup_length(input, -1, output);
}

int sail_strdup_length(char *input, int length, char **output) {

    if (input == NULL) {
        *output = NULL;
        return 0;
    }

    const int len = length < 0 ? (int)strlen(input) : length;

    *output = (char *)malloc(len+1);

    if (*output == NULL) {
        return errno;
    }

    memcpy(*output, input, len);
    (*output)[len] = '\0';

    return 0;
}
