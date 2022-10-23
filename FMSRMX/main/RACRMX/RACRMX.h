#define _CRT_SECURE_NO_WARNINGS

#pragma once
#include "com_nextlabs_racrmx_RACRMXInstance.h"
#include "RMXTypeDef.h"
#include "RMXTagHlp.h"

#if defined(__cplusplus)
extern "C" {
#endif
	typedef enum {
		RMX_RIGHT_Nil = 0, /*Undefined rights, used to initialize*/
		RMX_RIGHT_VIEW = 1,
		RMX_RIGHT_EDIT = 2,
		RMX_RIGHT_PRINT = 4,
		RMX_RIGHT_CLIPBOARD = 8,
		RMX_RIGHT_SAVEAS = 0x10,
		RMX_RIGHT_DECRYPT = 0x20,
		RMX_RIGHT_SCREENCAPTURE = 0x40,
		RMX_RIGHT_SEND = 0x80,
		RMX_RIGHT_CLASSIFY = 0x100,
		RMX_RIGHT_SHARE = 0x200,
		RMX_RIGHT_DOWNLOAD = 0x400,
		RMX_RIGHT_WATERMARK = 0x40000000,
		RMX_RIGHT_NONE = 0x80000000 /*Always deny. No any right granted*/
	} RMXFileRight;

	typedef enum RMX_WATERMARK_FONTSTYLE {
		FontStyle_Regular = 0,
		FontStyle_Bold = 1,
		FontStyle_Italic = 2,
		FontStyle_BoldItalic = 3
	};

	typedef enum RMX_WATERMARK_TEXTALIGNMENT {
		TextAlign_Left = 0,
		TextAlign_Centre = 1,
		TextAlign_Right = 2
	};

#if defined(__cplusplus)
}
#endif

jbyteArray as_byte_array(JNIEnv *, unsigned char*, int);

unsigned char* as_unsigned_char_array(JNIEnv *, jbyteArray);

#pragma once
