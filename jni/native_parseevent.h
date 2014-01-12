#ifndef _NATIVE_PARSE_EVENT_H
#define _NATIVE_PARSE_EVENT_H

//#include <linux/input.h>	// not compiled
#include </usr/include/linux/input.h> // Ubuntu 12.04 LTE

typedef struct {
	long id;
	int action;
	float x;
	float y;
} coords_t;

/**
 * declare here to avoid modify the std getevent.h
 * impl in native_getevent.c
 */
int native_init();
int native_getevent();

/**
 * imple in native_parseevent.c
 */
long nativeGetDownTime();
long nativeGetEventTime();
int nativeGetAction();
float nativeGetPrecision(axis);
int nativeGetPointerCount();
coords_t* nativeGetPointerCoords();

void nativeSetResolution(const int axsiX, const int axsiY);

int nativeDevFilter(const char* name);
void nativeAddToEventHub(const char* device);
const char* nativeGetDevEvent();
void nativeSetAbs(int code, int value);
int nativeGetCalibratedWidth();
int nativeGetCalibratedHeight();
int nativeParseEvent(struct input_event event);

extern void (*METHOD_sync)(void);

#endif
