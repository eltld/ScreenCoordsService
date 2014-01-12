#include <stdio.h>
#include <string.h>
#include <linux/limits.h> // #define PATH_MAX
#include <android/input.h>

#include "getevent.h"
#include "native_parseevent.h"
#include "native_utils.h"
#include "native_log.h"

#ifndef PATH_MAX
#define PATH_MAX			4096	/* # chars in a path name including nul */
#endif

#define N_TOUCH_POINT		10
#define INVALID_ID			(~0)

enum {
	FALSE = 0,
	TRUE = !0,
};

enum {
	ACTION_DOWN = AMOTION_EVENT_ACTION_DOWN,
	ACTION_MOVE = AMOTION_EVENT_ACTION_MOVE,
	ACTION_UP = AMOTION_EVENT_ACTION_UP,
	ACTION_CANCEL = AMOTION_EVENT_ACTION_CANCEL,
	ACTION_POINTER_DOWN = AMOTION_EVENT_ACTION_POINTER_DOWN,
	ACTION_POINTER_UP = AMOTION_EVENT_ACTION_POINTER_UP,
};

extern void (*METHOD_sync)(void) = NULL;

/**
 * private global var & methods
 */
static union {
	char name[PATH_MAX];	// single touch screen, don't deal with multi-screen
	// NUL ('\0') == FALSE (0) actually;
	// in big-endian OS, it's the same as name[0]
	int found;
} sDevice;

static int sMaxSlot = 0;	// if 0 then SYN_MT_REPORT, otherwise ABS_MT_SLOT

static int sResolutionX;
static int sResolutionY;

static float sCalibratedWidth;
static float sCalibratedHeight;

static coords_t prevTouchPoints[N_TOUCH_POINT];
static coords_t sTouchPoints[N_TOUCH_POINT];
static int sTouchPointCnt = 0;

static long sDownTime;
static long sEventTime;
static int sAction;

static coords_t currEvent[N_TOUCH_POINT];
static int sEventPointCnt;

static const char* getActionLabel(int action) {
	switch(action) {
	case ACTION_DOWN:
		return "ACTION_DOWN";
	case ACTION_MOVE:
		return "ACTION_MOVE";
	case ACTION_UP:
		return "ACTION_UP";
	case ACTION_CANCEL:
		return "ACTION_CANCEL";
	case ACTION_POINTER_DOWN:
		return "ACTION_POINTER_DOWN";
	case ACTION_POINTER_UP:
		return "ACTION_POINTER_UP";
	}
}

static float getRawValue(int code, int value) {
	switch(code) {
	case ABS_MT_POSITION_X:
		return value * sResolutionX / sCalibratedWidth;
	case ABS_MT_POSITION_Y:
		return value * sResolutionY / sCalibratedHeight;
	default:
		return value;
	}
}

static void evncpy(coords_t* des, const coords_t* src, const int n) {
	int i = 0;
	while(i++ != n) {
		*des++ = *src++;
	}
}

static long get_time(struct timeval* tv) {
//	return 1000 * (tv->tv_sec) + (tv->tv_usec) / 1000;
	return 1000000 * (tv->tv_sec) + (tv->tv_usec); // 1/1000 ms unit
}

static void input_sync() {
	int i, k;
	for(i = 0; i != sMaxSlot; ++i) {
		if(!sTouchPoints[i].id || sTouchPoints[i].id == INVALID_ID) {
			continue;
		}

		if (prevTouchPoints[i].id) {
			switch (prevTouchPoints[i].action) {
			case ACTION_DOWN:
			case ACTION_POINTER_DOWN:
				if (sTouchPoints[i].action == ACTION_DOWN
						|| sTouchPoints[i].action == ACTION_POINTER_DOWN) {
					// ACTION_DOWN -> ACTION_MOVE
					sTouchPoints[i].action = ACTION_MOVE;
					sAction = ACTION_MOVE;
				}
				break; // ACTION_DOWN -> ACTION_UP, ...
			}
		}
	}

	k = 0;
	for(i = 0; i != sMaxSlot; ++i) {
		if(!sTouchPoints[i].id || sTouchPoints[i].id == INVALID_ID) {
			continue;
		}

		nativeDebug("[%2d]: %-24s X:%4.1f\tY:%4.1f", i,
				getActionLabel(sTouchPoints[i].action),
				sTouchPoints[i].x,
				sTouchPoints[i].y);
		currEvent[k++] = sTouchPoints[i];

		switch(sTouchPoints[i].action) {
		case ACTION_UP:
		case ACTION_POINTER_UP:
		case ACTION_CANCEL:
			if(prevTouchPoints[i].action == ACTION_UP
					|| prevTouchPoints[i].action == ACTION_POINTER_UP
					|| prevTouchPoints[i].action == ACTION_CANCEL) {
				LOGE("event parse error: %s -> %s",
						getActionLabel(prevTouchPoints[i].action),
						getActionLabel(sTouchPoints[i].action));
			}

			sTouchPoints[i].id = INVALID_ID; // important!
			break;
		}
	}
	sEventPointCnt = k;

	if(METHOD_sync != NULL) {
		METHOD_sync();
	}

	evncpy(prevTouchPoints, sTouchPoints, sMaxSlot);
}

/**
 * TRUE if using SYN_MT_REPORT to sync
 */
static int is_input_mt_sync() {
	return (sMaxSlot == 0);
}

static void input_mt_sync(struct input_event *ev) {
	// TODO: aha, my test dev nexus7 is ABS_MT_SLOT
	// google linux multi-touch protocol and program by yourself

	LOGW("no man's land");
}

static void input_mt_slot(struct input_event *ev) {
	static int slot = 0;

	// always update
	sEventTime = get_time(&(ev->time));

	switch(ev->code) {
	case ABS_MT_SLOT:
		slot = ev->value; // switch slot
		break;
	case ABS_MT_TRACKING_ID:
		if(ev->value == INVALID_ID) {
			--sTouchPointCnt;

			if(sTouchPointCnt < 0) {
				LOGE("event parse error: < 0 touch points");
				sTouchPointCnt = 0;
			} else if (sTouchPointCnt == 0) {
				sTouchPoints[slot].action = ACTION_UP;
				sAction = ACTION_UP;
			} else {
				sTouchPoints[slot].action = ACTION_POINTER_UP;
			}
		} else {
			if(sTouchPointCnt > sMaxSlot) {
				LOGE("event parse error: > %d touch points", sMaxSlot);
				sTouchPointCnt = sMaxSlot;
			}

			if (sTouchPointCnt == 0) {
				sTouchPoints[slot].action = ACTION_DOWN;
				sAction = ACTION_DOWN;
				sDownTime = sEventTime;
			} else {
				sTouchPoints[slot].action = ACTION_POINTER_DOWN;
			}

			sTouchPoints[slot].id = ev->value;

			++sTouchPointCnt;
		}
		break;
	case ABS_MT_POSITION_X:
		sTouchPoints[slot].x = getRawValue(ev->code, ev->value);
		break;
	case ABS_MT_POSITION_Y:
		sTouchPoints[slot].y = getRawValue(ev->code, ev->value);
		break;
	}
}

long nativeGetDownTime() {
	return sDownTime;
}

long nativeGetEventTime() {
	return sEventTime;
}

int nativeGetAction() {
	return sAction;
}

float nativeGetPrecision(axis) {
	if (axis == 0) {
		// AXIS_X
		return sCalibratedWidth / sResolutionX;
	} else {
		// AXIS_Y
		return sCalibratedHeight / sResolutionY;
	}
}

int nativeGetPointerCount() {
	return sEventPointCnt;
}

coords_t* nativeGetPointerCoords() {
	return currEvent;
}

void nativeSetResolution(const int axsiX, const int axsiY) {
	sResolutionX = axsiX;
	sResolutionY = axsiY;
}

int nativeDevFilter(const char* name) {
	static const char* keyWords[] = {"touch", "screen"};
	static const n = sizeof(keyWords) / sizeof(char*);
	int i;

	nativeDebug("scan %s...", name);
	for(i = 0; i != n; ++i) {
		if(strstr(name, keyWords[i]) != NULL) {
			return TRUE;
		}
	}

	return FALSE;
}

void nativeAddToEventHub(const char* device) {
//	if (!checkEndian()) {
//		// force copy
//		strncpy(sDevice.name, device, PATH_MAX);
//		return;
//	}

	if (!sDevice.found) {
		nativeDebug("lucky, android is big-endian");
		strncpy(sDevice.name, device, PATH_MAX);
	}
}

const char* nativeGetDevEvent() {
	return sDevice.name;
}

/**
 * value is almost abs.maximum
 */
void nativeSetAbs(int code, int value) {
	switch (code) {
	case ABS_MT_SLOT:
		sMaxSlot = value + 1; // f.e. max slot index is 9 in 10-point touch dev
		LOGW("%d-point touch device, %d slots available", sMaxSlot, N_TOUCH_POINT);
		break;
	case ABS_MT_POSITION_X:
		sCalibratedWidth = value + 1;
		break;
	case ABS_MT_POSITION_Y:
		sCalibratedHeight = value + 1;
		break;
	}
}

int nativeGetCalibratedWidth() {
	return sCalibratedWidth;
}

int nativeGetCalibratedHeight() {
	return sCalibratedHeight;
}

int nativeParseEvent(struct input_event event) {
	switch (event.type) {
	case EV_SYN:
		if (event.value == 0) {
			// EV_SYN(0000) SYN_REPORT(0000) 00000000
			input_sync();
		}
		// other, f.e. EV_SYN(0000) SYN_REPORT(0000) 00000001(?)
		break;
	case EV_ABS:
		if (is_input_mt_sync()) {
			input_mt_sync(&event);
		} else {
			input_mt_slot(&event);
		}
		break;
	}
}
