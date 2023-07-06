#include "prelude.h"

u32 len(const char* string) {
    u32 i = 0;
    for (; string[i] != '\0'; ++i) {
    }
    return i;
}

Bool eq(const char* a, const char* b) {
    u32 n = len(a);
    if (n != len(b)) {
        return FALSE;
    }
    for (u32 i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return FALSE;
        }
    }
    return TRUE;
}
