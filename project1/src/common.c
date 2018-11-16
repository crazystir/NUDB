#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#include "common.h"

void Concatenate(char *str, ...) {
  va_list valist;
  char *tmp = NULL;

  va_start(valist, str);

  while ((tmp = va_arg(valist, char *))) {
    strcat(str, tmp);
    //printf("%s\n", str);
  }
}

int NextString(char *dest, char *str, int pos) {
    if (!dest || !str || pos < 0) {
        return -1;
    }

    int i = 0;

    while (str[pos] != '\0' && str[pos] == ' ') {
        pos++;
    }

    if (str[pos] == '\0') {
        return -1;
    }

    while (str[pos] != '\0' && str[pos] != ' ') {
        dest[i++] = str[pos++];
    }

    dest[i] = '\0';
    return pos;
}

