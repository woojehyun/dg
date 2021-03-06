#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "wgl_ext.h"

#if defined(__APPLE__)
#include <mach-o/dyld.h>

static void* AppleGLGetProcAddress (const GLubyte *name)
{
  static const struct mach_header* image = NULL;
  NSSymbol symbol;
  char* symbolName;
  if (NULL == image)
  {
    image = NSAddImage("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
  }
  /* prepend a '_' for the Unix C symbol mangling convention */
  symbolName = malloc(strlen((const char*)name) + 2);
  strcpy(symbolName+1, (const char*)name);
  symbolName[0] = '_';
  symbol = NULL;
  /* if (NSIsSymbolNameDefined(symbolName))
	 symbol = NSLookupAndBindSymbol(symbolName); */
  symbol = image ? NSLookupSymbolInImage(image, symbolName, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR) : NULL;
  free(symbolName);
  return symbol ? NSAddressOfSymbol(symbol) : NULL;
}
#endif /* __APPLE__ */

#if defined(__sgi) || defined (__sun)
#include <dlfcn.h>
#include <stdio.h>

static void* SunGetProcAddress (const GLubyte* name)
{
  static void* h = NULL;
  static void* gpa;

  if (h == NULL)
  {
    if ((h = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL)) == NULL) return NULL;
    gpa = dlsym(h, "glXGetProcAddress");
  }

  if (gpa != NULL)
    return ((void*(*)(const GLubyte*))gpa)(name);
  else
    return dlsym(h, (const char*)name);
}
#endif /* __sgi || __sun */

#if defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(disable: 4055)
#pragma warning(disable: 4054)
#endif

static int WglTestPointer(const PROC pTest)
{
	ptrdiff_t iTest;
	if(!pTest) return 0;
	iTest = (ptrdiff_t)pTest;
	
	if(iTest == 1 || iTest == 2 || iTest == 3 || iTest == -1) return 0;
	
	return 1;
}

static PROC WglWinGetProcAddress(const char *name)
{
	HMODULE glMod = NULL;
	PROC pFunc = wglGetProcAddress((LPCSTR)name);
	if(WglTestPointer(pFunc))
	{
		return pFunc;
	}
	glMod = GetModuleHandleA("OpenGL32.dll");
	return (PROC)GetProcAddress(glMod, (LPCSTR)name);
}
	
#define WglIntGetProcAddress(name) WglWinGetProcAddress(name)
#else
	#if defined(__APPLE__)
		#define WglIntGetProcAddress(name) AppleGLGetProcAddress(name)
	#else
		#if defined(__sgi) || defined(__sun)
			#define WglIntGetProcAddress(name) SunGetProcAddress(name)
		#else /* GLX */
		    #include <GL/glx.h>

			#define WglIntGetProcAddress(name) (*glXGetProcAddressARB)((const GLubyte*)name)
		#endif
	#endif
#endif

int wgl_ext_ARB_multisample = wgl_LOAD_FAILED;
int wgl_ext_ARB_extensions_string = wgl_LOAD_FAILED;
int wgl_ext_ARB_pixel_format = wgl_LOAD_FAILED;
int wgl_ext_ARB_pixel_format_float = wgl_LOAD_FAILED;
int wgl_ext_ARB_framebuffer_sRGB = wgl_LOAD_FAILED;
int wgl_ext_ARB_create_context = wgl_LOAD_FAILED;
int wgl_ext_ARB_create_context_profile = wgl_LOAD_FAILED;
int wgl_ext_ARB_create_context_robustness = wgl_LOAD_FAILED;
int wgl_ext_EXT_swap_control = wgl_LOAD_FAILED;
int wgl_ext_EXT_pixel_format_packed_float = wgl_LOAD_FAILED;
int wgl_ext_EXT_create_context_es2_profile = wgl_LOAD_FAILED;
int wgl_ext_EXT_swap_control_tear = wgl_LOAD_FAILED;
int wgl_ext_NV_swap_group = wgl_LOAD_FAILED;

const char * (CODEGEN_FUNCPTR *_ptrc_wglGetExtensionsStringARB)(HDC) = NULL;

static int WglLoad_ARB_extensions_string()
{
	int numFailed = 0;
	_ptrc_wglGetExtensionsStringARB = (const char * (CODEGEN_FUNCPTR *)(HDC))WglIntGetProcAddress("wglGetExtensionsStringARB");
	if(!_ptrc_wglGetExtensionsStringARB) numFailed++;
	return numFailed;
}

BOOL (CODEGEN_FUNCPTR *_ptrc_wglChoosePixelFormatARB)(HDC, const int *, const FLOAT *, UINT, int *, UINT *) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglGetPixelFormatAttribfvARB)(HDC, int, int, UINT, const int *, FLOAT *) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglGetPixelFormatAttribivARB)(HDC, int, int, UINT, const int *, int *) = NULL;

static int Load_ARB_pixel_format()
{
	int numFailed = 0;
	_ptrc_wglChoosePixelFormatARB = (BOOL (CODEGEN_FUNCPTR *)(HDC, const int *, const FLOAT *, UINT, int *, UINT *))WglIntGetProcAddress("wglChoosePixelFormatARB");
	if(!_ptrc_wglChoosePixelFormatARB) numFailed++;
	_ptrc_wglGetPixelFormatAttribfvARB = (BOOL (CODEGEN_FUNCPTR *)(HDC, int, int, UINT, const int *, FLOAT *))WglIntGetProcAddress("wglGetPixelFormatAttribfvARB");
	if(!_ptrc_wglGetPixelFormatAttribfvARB) numFailed++;
	_ptrc_wglGetPixelFormatAttribivARB = (BOOL (CODEGEN_FUNCPTR *)(HDC, int, int, UINT, const int *, int *))WglIntGetProcAddress("wglGetPixelFormatAttribivARB");
	if(!_ptrc_wglGetPixelFormatAttribivARB) numFailed++;
	return numFailed;
}

HGLRC (CODEGEN_FUNCPTR *_ptrc_wglCreateContextAttribsARB)(HDC, HGLRC, const int *) = NULL;

static int Load_ARB_create_context()
{
	int numFailed = 0;
	_ptrc_wglCreateContextAttribsARB = (HGLRC (CODEGEN_FUNCPTR *)(HDC, HGLRC, const int *))WglIntGetProcAddress("wglCreateContextAttribsARB");
	if(!_ptrc_wglCreateContextAttribsARB) numFailed++;
	return numFailed;
}

int (CODEGEN_FUNCPTR *_ptrc_wglGetSwapIntervalEXT)() = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglSwapIntervalEXT)(int) = NULL;

static int Load_EXT_swap_control()
{
	int numFailed = 0;
	_ptrc_wglGetSwapIntervalEXT = (int (CODEGEN_FUNCPTR *)())WglIntGetProcAddress("wglGetSwapIntervalEXT");
	if(!_ptrc_wglGetSwapIntervalEXT) numFailed++;
	_ptrc_wglSwapIntervalEXT = (BOOL (CODEGEN_FUNCPTR *)(int))WglIntGetProcAddress("wglSwapIntervalEXT");
	if(!_ptrc_wglSwapIntervalEXT) numFailed++;
	return numFailed;
}

BOOL (CODEGEN_FUNCPTR *_ptrc_wglBindSwapBarrierNV)(GLuint, GLuint) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglJoinSwapGroupNV)(HDC, GLuint) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglQueryFrameCountNV)(HDC, GLuint *) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglQueryMaxSwapGroupsNV)(HDC, GLuint *, GLuint *) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglQuerySwapGroupNV)(HDC, GLuint *, GLuint *) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglResetFrameCountNV)(HDC) = NULL;

static int Load_NV_swap_group()
{
	int numFailed = 0;
	_ptrc_wglBindSwapBarrierNV = (BOOL (CODEGEN_FUNCPTR *)(GLuint, GLuint))WglIntGetProcAddress("wglBindSwapBarrierNV");
	if(!_ptrc_wglBindSwapBarrierNV) numFailed++;
	_ptrc_wglJoinSwapGroupNV = (BOOL (CODEGEN_FUNCPTR *)(HDC, GLuint))WglIntGetProcAddress("wglJoinSwapGroupNV");
	if(!_ptrc_wglJoinSwapGroupNV) numFailed++;
	_ptrc_wglQueryFrameCountNV = (BOOL (CODEGEN_FUNCPTR *)(HDC, GLuint *))WglIntGetProcAddress("wglQueryFrameCountNV");
	if(!_ptrc_wglQueryFrameCountNV) numFailed++;
	_ptrc_wglQueryMaxSwapGroupsNV = (BOOL (CODEGEN_FUNCPTR *)(HDC, GLuint *, GLuint *))WglIntGetProcAddress("wglQueryMaxSwapGroupsNV");
	if(!_ptrc_wglQueryMaxSwapGroupsNV) numFailed++;
	_ptrc_wglQuerySwapGroupNV = (BOOL (CODEGEN_FUNCPTR *)(HDC, GLuint *, GLuint *))WglIntGetProcAddress("wglQuerySwapGroupNV");
	if(!_ptrc_wglQuerySwapGroupNV) numFailed++;
	_ptrc_wglResetFrameCountNV = (BOOL (CODEGEN_FUNCPTR *)(HDC))WglIntGetProcAddress("wglResetFrameCountNV");
	if(!_ptrc_wglResetFrameCountNV) numFailed++;
	return numFailed;
}

typedef int (*PFN_LOADFUNCPOINTERS)();
typedef struct wgl_StrToExtMap_s
{
	char *extensionName;
	int *extensionVariable;
	PFN_LOADFUNCPOINTERS LoadExtension;
} wgl_StrToExtMap;

static wgl_StrToExtMap WglExtensionMap[13] = {
	{"WGL_ARB_multisample", &wgl_ext_ARB_multisample, NULL},
	{"WGL_ARB_extensions_string", &wgl_ext_ARB_extensions_string, WglLoad_ARB_extensions_string},
	{"WGL_ARB_pixel_format", &wgl_ext_ARB_pixel_format, Load_ARB_pixel_format},
	{"WGL_ARB_pixel_format_float", &wgl_ext_ARB_pixel_format_float, NULL},
	{"WGL_ARB_framebuffer_sRGB", &wgl_ext_ARB_framebuffer_sRGB, NULL},
	{"WGL_ARB_create_context", &wgl_ext_ARB_create_context, Load_ARB_create_context},
	{"WGL_ARB_create_context_profile", &wgl_ext_ARB_create_context_profile, NULL},
	{"WGL_ARB_create_context_robustness", &wgl_ext_ARB_create_context_robustness, NULL},
	{"WGL_EXT_swap_control", &wgl_ext_EXT_swap_control, Load_EXT_swap_control},
	{"WGL_EXT_pixel_format_packed_float", &wgl_ext_EXT_pixel_format_packed_float, NULL},
	{"WGL_EXT_create_context_es2_profile", &wgl_ext_EXT_create_context_es2_profile, NULL},
	{"WGL_EXT_swap_control_tear", &wgl_ext_EXT_swap_control_tear, NULL},
	{"WGL_NV_swap_group", &wgl_ext_NV_swap_group, Load_NV_swap_group},
};

static int g_wglEextensionMapSize = 13;

static wgl_StrToExtMap *WglFindExtEntry(const char *extensionName)
{
	int loop;
	wgl_StrToExtMap *currLoc = WglExtensionMap;
	for(loop = 0; loop < g_wglEextensionMapSize; ++loop, ++currLoc)
	{
		if(strcmp(extensionName, currLoc->extensionName) == 0)
			return currLoc;
	}
	
	return NULL;
}

static void WglClearExtensionVars()
{
	wgl_ext_ARB_multisample = wgl_LOAD_FAILED;
	wgl_ext_ARB_extensions_string = wgl_LOAD_FAILED;
	wgl_ext_ARB_pixel_format = wgl_LOAD_FAILED;
	wgl_ext_ARB_pixel_format_float = wgl_LOAD_FAILED;
	wgl_ext_ARB_framebuffer_sRGB = wgl_LOAD_FAILED;
	wgl_ext_ARB_create_context = wgl_LOAD_FAILED;
	wgl_ext_ARB_create_context_profile = wgl_LOAD_FAILED;
	wgl_ext_ARB_create_context_robustness = wgl_LOAD_FAILED;
	wgl_ext_EXT_swap_control = wgl_LOAD_FAILED;
	wgl_ext_EXT_pixel_format_packed_float = wgl_LOAD_FAILED;
	wgl_ext_EXT_create_context_es2_profile = wgl_LOAD_FAILED;
	wgl_ext_EXT_swap_control_tear = wgl_LOAD_FAILED;
	wgl_ext_NV_swap_group = wgl_LOAD_FAILED;
}


static void WglLoadExtByName(const char *extensionName)
{
	wgl_StrToExtMap *entry = NULL;
	entry = WglFindExtEntry(extensionName);
	if(entry)
	{
		if(entry->LoadExtension)
		{
			int numFailed = entry->LoadExtension();
			if(numFailed == 0)
			{
				*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED;
			}
			else
			{
				*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED + numFailed;
			}
		}
		else
		{
			*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED;
		}
	}
}


static void ProcExtsFromExtString(const char *strExtList)
{
	size_t iExtListLen = strlen(strExtList);
	const char *strExtListEnd = strExtList + iExtListLen;
	const char *strCurrPos = strExtList;
	char strWorkBuff[256];

	while(*strCurrPos)
	{
		/*Get the extension at our position.*/
		int iStrLen = 0;
		const char *strEndStr = strchr(strCurrPos, ' ');
		int iStop = 0;
		if(strEndStr == NULL)
		{
			strEndStr = strExtListEnd;
			iStop = 1;
		}

		iStrLen = (int)((ptrdiff_t)strEndStr - (ptrdiff_t)strCurrPos);

		if(iStrLen > 255)
			return;

		strncpy(strWorkBuff, strCurrPos, iStrLen);
		strWorkBuff[iStrLen] = '\0';

		WglLoadExtByName(strWorkBuff);

		strCurrPos = strEndStr + 1;
		if(iStop) break;
	}
}

int wgl_LoadFunctions(HDC hdc)
{
	WglClearExtensionVars();
	
	_ptrc_wglGetExtensionsStringARB = (const char * (CODEGEN_FUNCPTR *)(HDC))WglIntGetProcAddress("wglGetExtensionsStringARB");
	if(!_ptrc_wglGetExtensionsStringARB) return wgl_LOAD_FAILED;
	
	ProcExtsFromExtString((const char *)_ptrc_wglGetExtensionsStringARB(hdc));
	return wgl_LOAD_SUCCEEDED;
}

