#ifndef __GL_PCH_H
#define __GL_PCH_H
#ifdef _WIN32
//#define __RPCNDR_H__		// this header causes problems!
//#define __wtypes_h__
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINDOWS 0x410
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501			// Support the mouse wheel and session notification.
#define _WIN32_IE 0x0500
#endif
#define DIRECTINPUT_VERSION 0x800
#define DIRECTDRAW_VERSION 0x0300

#define DWORD WINDOWS_DWORD	// I don't want to depend on this throughout the GL code!

#ifdef _MSC_VER
#pragma warning(disable : 4995)     // MIPS
#endif

#include <windows.h>
#include <mmsystem.h>
#include <winsock.h>
#ifndef __WINE__
#include <dshow.h>
#endif
#include <d3d9.h>
//#include <dsound.h>
//#include <dinput.h>
//#include <lmcons.h>
//#include <shlobj.h>
#endif

#undef DWORD
#ifndef CALLBACK
#define CALLBACK
#endif
#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
//#include <direct.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#if !defined(__APPLE__)
#include <malloc.h>
#endif
#include <time.h>

#ifdef _MSC_VER
#define    F_OK    0    /* Check for file existence */
#define    W_OK    2    /* Check for write permission */
#define    R_OK    4    /* Check for read permission */
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//GL headers
#ifdef USE_GLES

/*
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
void glVertexAttrib1f(	GLuint index,
                      GLfloat v0);

*/
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include "jwzgles.h"


#define glGenFramebuffersEXT glGenFramebuffersOES
#define glDeleteFramebuffersEXT glDeleteFramebuffersOES
#define glBindFramebufferEXT glBindFramebufferOES

#define GL_FRAMEBUFFER_BINDING_EXT GL_FRAMEBUFFER_BINDING_OES
#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER_OES
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES


#define glGenRenderbuffersEXT glGenRenderbuffersOES
#define glBindRenderbufferEXT glBindRenderbufferOES
#define glDeleteRenderbuffersEXT glDeleteRenderbuffersOES
#define glRenderbufferStorageEXT glRenderbufferStorageOES

#define GL_RENDERBUFFER_EXT GL_RENDERBUFFER_OES
#define GL_DEPTH24_STENCIL8_EXT GL_DEPTH24_STENCIL8_OES
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0_OES
#define GL_DEPTH_ATTACHMENT_EXT GL_DEPTH_ATTACHMENT_OES
#define GL_STENCIL_ATTACHMENT_EXT GL_STENCIL_ATTACHMENT_OES

#define glFramebufferTexture2DEXT glFramebufferTexture2DOES
#define glFramebufferRenderbufferEXT glFramebufferRenderbufferOES


#define glFlushMappedBufferRange glFlushMappedBufferRangeEXT
#define glMapBufferRange glMapBufferRangeEXT
#define glUnmapBuffer glUnmapBufferOES

#define GL_MAP_WRITE_BIT 0
#define GL_MAP_FLUSH_EXPLICIT_BIT 0
#define GL_MAP_UNSYNCHRONIZED_BIT 0

#define GL_ALPHA8 GL_ALPHA8_EXT
#define GL_RGBA8 GL_RGBA8_OES

//TODO
#define GL_RGB5_A1 GL_RGBA
#define GL_RGBA4 GL_RGBA
#define GL_RGBA2 GL_RGBA
// [BB] Added compressed texture formats.
#define GL_COMPRESSED_RGBA_ARB GL_RGBA
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT GL_RGBA
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT GL_RGBA
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT GL_RGBA


#else

#include "gl_load.h"

#if defined(__APPLE__)
	#include <OpenGL/OpenGL.h>
#endif


#endif

#ifdef _WIN32
#define DWORD WINDOWS_DWORD	// I don't want to depend on this throughout the GL code!
//#include "gl/api/wglext.h"
#ifndef __WINE__
#undef DWORD
#endif
#else
typedef unsigned char 	byte;
typedef float		FLOAT;
template <typename T>
inline T max( T a, T b) { return (((a)>(b)) ? (a) : (b)); }
#define __cdecl
#define _access(a,b)	access(a,b)
#endif


#ifdef LoadMenu
#undef LoadMenu
#endif
#ifdef DrawText
#undef DrawText
#endif
#ifdef GetCharWidth
#undef GetCharWidth
#endif

#undef S_NORMAL
#undef OPAQUE


#ifdef _MSC_VER
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)     // truncate from double to float
#endif

#ifdef WIN32
#undef WIN32
#endif
#endif //__GL_PCH_H
