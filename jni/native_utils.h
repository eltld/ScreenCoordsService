#ifndef _NATIVE_UTILS_H
#define _NATIVE_UTILS_H

int isRootAvailable();
int sudo(const char* fmt, ...);
/**
 * non-zero if it's big-endian os
 */
int checkEndian();

#endif
