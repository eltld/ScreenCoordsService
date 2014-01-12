#include <stdio.h>
#include "native_log.h"

#define	TAG	"NATIVE_DEBUG"

typedef enum debug_mode {
	DEBUG_OFF = 0,
	DEBUG_ON = !0
} debug_mode ;
static debug_mode native_debug_mode = DEBUG_ON;

void setNativeDebugModeOn() {
	native_debug_mode = DEBUG_ON;
}

void setNativeDebugModeOff() {
	native_debug_mode = DEBUG_OFF;
}

void nativeDebug(const char *fmt, ...)
{
	if (native_debug_mode == DEBUG_OFF) {
		return;
	}

	char buf[4096];
	const int BUF_SZ = sizeof(buf) / sizeof(char);
	const int MSG_LEN = BUF_SZ - 1;
	int len;

	va_list args;
	va_start(args, fmt);
	len = vsnprintf(buf, MSG_LEN, fmt, args);
	va_end(args);

	buf[len] = '\0';

	LOGD("%s", buf);
}
