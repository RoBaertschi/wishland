#include "wishland.h"

#include <stdio.h>

int main(void) {
    whl_string string = WHL_S("Hello World!");
    printf("%.*s\n", (int)string.len, string.data);
}
