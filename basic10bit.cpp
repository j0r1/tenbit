#include "stdafx.h"
#include <windows.h>
#include <GL/gl.h>
#include <iostream>
#include <string>

using namespace std;

#define ERRCODE_CANTOPENOPENGLDLL			62
#define ERRCODE_CANTGETWGLGETPROCADDR		63
#define ERRCODE_CANTGETFUNCTIONPOINTER		64
#define ERRCODE_CANTGET10BITFORMAT			65
#define ERRCODE_CANTCREATEVBO				66
#define ERRCODE_CANTCREATESHADER			67
#define ERRCODE_CANTMAKECONTEXTCURRENT		68
#define ERRCODE_CANTGETBASICPIXELFORMAT		69
#define ERRCODE_CANTGETBASICGLCONTEXT		70
#define ERRCODE_CANTGETOPENGLCONTEXT		71

void ErrorExit(int exitCode, const string &msg)
{
	cerr << endl << "ERROR: " << msg << endl << endl;
	cerr << "Exiting..." << endl;
	Sleep(2000);
	exit(exitCode);
}

#define GL_ARRAY_BUFFER								0x8892
#define GL_STATIC_DRAW								0x88E4
#define GL_FRAGMENT_SHADER							0x8B30
#define GL_VERTEX_SHADER							0x8B31

#define WGL_CONTEXT_MAJOR_VERSION_ARB				0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB				0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB				0x9126
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB	0x00000002

#define WGL_DRAW_TO_WINDOW_ARB						0x2001
#define WGL_SUPPORT_OPENGL_ARB						0x2010
#define WGL_DOUBLE_BUFFER_ARB						0x2011
#define WGL_PIXEL_TYPE_ARB							0x2013
#define WGL_RED_BITS_ARB							0x2015
#define WGL_GREEN_BITS_ARB							0x2017
#define WGL_BLUE_BITS_ARB							0x2019
#define WGL_DEPTH_BITS_ARB							0x2022
#define WGL_TYPE_RGBA_ARB							0x202B

using namespace std;

// Going to obtain function pointers from opengl32.dll, either using GetProcAddress or wglGetProcAddress
// To avoid conflicts, these things are put in their own namespace
namespace mygl
{
	HMODULE hGl = 0;

#ifdef _WIN64
	typedef int64_t GLsizeiptr;
#else
	typedef int32_t GLsizeiptr;
#endif

	PROC (WINAPI *wglGetProcAddress)(LPCSTR lpszProc);
	HGLRC (WINAPI *wglCreateContext)(HDC hdc);
	BOOL (WINAPI *wglMakeCurrent)(HDC hdc, HGLRC hglrc);
	HGLRC (WINAPI *wglCreateContextAttribsARB)(HDC hDC, HGLRC hshareContext, const int *attribList);
	BOOL (WINAPI *wglDeleteContext)(HGLRC hglrc);
	BOOL (WINAPI *wglSetPixelFormat)(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd);
	int (WINAPI *wglDescribePixelFormat)(HDC hdc, int iPixelFormat, UINT nBytes, PIXELFORMATDESCRIPTOR *ppfd);
	BOOL (WINAPI *wglChoosePixelFormatARB)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

	const GLubyte* (WINAPI *glGetString)(GLenum name);
	void (WINAPI *glClear)(GLbitfield  	mask);
	void (WINAPI *glFlush)(void);
	void (WINAPI *glFinish)(void);
	void (WINAPI *glGetIntegerv)(GLenum pname, GLint *params);
	void (WINAPI *glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	void (WINAPI *glBindVertexArray)(GLuint array);
	void (WINAPI *glUseProgram)(GLuint program);
	void (WINAPI *glDrawArrays)(GLenum mode, GLint, GLsizei);
	GLenum (WINAPI *glGetError)(void);
	void (WINAPI *glGenVertexArrays)(GLsizei n, GLuint *arrays);
	void (WINAPI *glGenBuffers)(GLsizei n, GLuint * buffers);
	void (WINAPI *glBindBuffer)(GLenum target, GLuint buffer);
	void (WINAPI *glBufferData)(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);
	void (WINAPI *glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);
	void (WINAPI *glEnableVertexAttribArray)(GLuint index);
	GLuint (WINAPI *glCreateShader)(GLenum shaderType);
	void (WINAPI *glShaderSource)(GLuint shader, GLsizei count, const char **string, const GLint *length);
	void (WINAPI *glCompileShader)(GLuint shader);
	GLuint (WINAPI *glCreateProgram)(void);
	void (WINAPI *glAttachShader)(GLuint program, GLuint shader);
	void (WINAPI *glLinkProgram)(GLuint program);
	void (WINAPI *glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

	// This define first tries to get a function pointer using wglGetProcAddress, with
	// GetProcAddress as a fallback mechanism
	#define GETPROC(x) \
	{\
		x = (decltype(x))wglGetProcAddress(#x); \
		if (!x)\
			x = (decltype(x))GetProcAddress(hGl, #x); \
		cout << "FUNCTION: " << #x << " = " << x << endl; \
		if (!x) \
			ErrorExit(ERRCODE_CANTGETFUNCTIONPOINTER, "Can't get pointer to function " #x); \
	}

	void internal_setupGLFunctions()
	{
		hGl = LoadLibraryA("opengl32.dll");
		if (!hGl)
			ErrorExit(ERRCODE_CANTOPENOPENGLDLL, "Unable to open opengl32.dll");

		wglGetProcAddress = (decltype(wglGetProcAddress))GetProcAddress(hGl, "wglGetProcAddress");
		if (!wglGetProcAddress)
			ErrorExit(ERRCODE_CANTGETWGLGETPROCADDR, "Unable to get function pointer for wglGetProcAddress");

		GETPROC(wglCreateContext);
		GETPROC(wglMakeCurrent);
		GETPROC(wglDeleteContext);
	}

	void internal_setupGLFunctions2()
	{
		GETPROC(wglCreateContextAttribsARB);
		GETPROC(wglSetPixelFormat);
		GETPROC(wglDescribePixelFormat);
		GETPROC(wglChoosePixelFormatARB);

		GETPROC(glGetString);
		GETPROC(glClear);
		GETPROC(glFlush);
		GETPROC(glFinish);
		GETPROC(glClearColor);
		GETPROC(glGetIntegerv);
		GETPROC(glBindVertexArray);
		GETPROC(glUseProgram);
		GETPROC(glDrawArrays);
		GETPROC(glGetError);
		GETPROC(glGenVertexArrays);
		GETPROC(glGenBuffers);
		GETPROC(glBindBuffer);
		GETPROC(glBufferData);
		GETPROC(glVertexAttribPointer);
		GETPROC(glEnableVertexAttribArray);
		GETPROC(glCreateShader);
		GETPROC(glShaderSource);
		GETPROC(glCompileShader);
		GETPROC(glCreateProgram);
		GETPROC(glAttachShader);
		GETPROC(glLinkProgram);
		GETPROC(glViewport);
	}

	// Gets pointers to the needed OpenGL functions
	void setup()
	{
		// First, open the dll and get a few function pointers to get started
		internal_setupGLFunctions();

		// We need to create a context and make it current to be able to get the
		// rest of the function pointers
		WNDCLASS wndCls;
		HINSTANCE hInst = GetModuleHandle(NULL);

		ZeroMemory(&wndCls, sizeof(wndCls));
		wndCls.lpfnWndProc = DefWindowProc;
		wndCls.hInstance = hInst;
		wndCls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wndCls.lpszClassName = TEXT("dummy");
		RegisterClass(&wndCls);

		HWND hWnd = CreateWindow(TEXT("dummy"), TEXT("dummy"), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, 
			                     0, 0, 0, 0, 0, 0, hInst, 0);
		HDC hDC = GetDC(hWnd);

		// This is just a very basic pixel format, so we can create a window and
		// gain access to other opengl functions
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL  | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,
			32,
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,
			8,
			0,
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		int pixelFormat = ChoosePixelFormat(hDC, &pfd);
		if (pixelFormat == 0)
			ErrorExit(ERRCODE_CANTGET10BITFORMAT, "Unable to get basic pixel format for dummy window");

		SetPixelFormat(hDC, pixelFormat, &pfd);

		HGLRC rc = mygl::wglCreateContext(hDC);
		if (!rc)
			ErrorExit(ERRCODE_CANTGETBASICGLCONTEXT, "Unable to get basic GL context for dummy window");

		mygl::wglMakeCurrent(hDC, rc);

		// Now we're able to get the rest of the function pointers
		internal_setupGLFunctions2();

		wglDeleteContext(rc);
		DeleteDC(hDC);
		DestroyWindow(hWnd);
	}
}

void PaintWindow(HWND hWnd);
void ResizeWindow(HWND hWnd, int w, int h);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

// Some global variables for the OpenGL code
GLuint VertexShaderId, FragmentShaderId1, FragmentShaderId2, ProgramId1, ProgramId2,
VaoId0, VaoId1, VboId0, VboId1, TexBufferId0, TexBufferId1, TexId0, TexId1;
HGLRC globalContext = NULL;
bool g_init = false;

int main()
{
	// Grab OpenGL functions
	mygl::setup();

	// Create and show a window, this window will display the black/white gradient
	// The lower half is forced to 8 bit precision, the top part uses 10 bit precision
	// if the monitor supports it
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASS wc;

	wc.lpszClassName = TEXT("myclass");
	wc.lpfnWndProc = MainWndProc;
	wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = TEXT("myclass");
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	RegisterClass(&wc);

	HWND hWnd = CreateWindow(TEXT("myclass"), TEXT("Black/White gradient"),
		                     WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
 		                     100, 100, 640, 480, NULL, NULL, hInstance, NULL);
	int nCmdShow = SW_SHOW;
	ShowWindow(hWnd, nCmdShow);

	// Let's see if we can get a 10 bit pixel format
	int attribsDesired[] = {
		WGL_SUPPORT_OPENGL_ARB, 1,
		WGL_DRAW_TO_WINDOW_ARB, 1,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_RED_BITS_ARB, 10,
		WGL_GREEN_BITS_ARB, 10,
		WGL_BLUE_BITS_ARB, 10,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_DOUBLE_BUFFER_ARB, 1,
		0, 0
	};

	int formats[1000];
	UINT numFormats = 0;
	HDC hDC = GetDC(hWnd);

	if (!mygl::wglChoosePixelFormatARB(hDC, attribsDesired, NULL, 1000, formats, &numFormats))
		ErrorExit(ERRCODE_CANTGET10BITFORMAT, "Unable to choose 10 bit pixel format");
	
	cout << "LOG: Got " << numFormats << " formats" << endl;
	if (numFormats < 1)
		ErrorExit(ERRCODE_CANTGET10BITFORMAT, "No useable 10 bit format detected");

	int pixelFormat = formats[0];
	cout << "LOG: Using first detected pixelformat: " << pixelFormat << endl;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));

	SetPixelFormat(hDC, pixelFormat, &pfd);

	// Initialize the OpenGL context
	int attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 	4,
		WGL_CONTEXT_MINOR_VERSION_ARB,	0,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0, 0
	};

	HGLRC rc = mygl::wglCreateContextAttribsARB(hDC, NULL, attribs);
	if (!rc)
		ErrorExit(ERRCODE_CANTGETOPENGLCONTEXT, "Unable to initialize OpenGL 4.0 context");

	// Keep this 10 bit context for later
	globalContext = rc;
	cout << "LOG: GL context is " << rc << endl;

	if (!mygl::wglMakeCurrent(hDC, globalContext))
		ErrorExit(ERRCODE_CANTMAKECONTEXTCURRENT, "Unable to make gl context current");

	cout << "VERSION: " << mygl::glGetString(GL_VERSION) << endl;
	cout << "VENDOR: " << mygl::glGetString(GL_VENDOR) << endl;

	// Let's see how many R, G and B bits we're using according to OpenGL
	int r = 0, g = 0, b = 0, a = 0;
	mygl::glGetIntegerv(GL_RED_BITS, &r);
	mygl::glGetIntegerv(GL_GREEN_BITS, &g);
	mygl::glGetIntegerv(GL_BLUE_BITS, &b);
	mygl::glGetIntegerv(GL_ALPHA_BITS, &a);

	cout << "BITDEPTH: " << r << ":" << g << ":" << b << ":" << a << endl;

	// The message loop

	MSG msg;
	BOOL bRet;

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			cerr << "WARNING: Unexpected return value from GetMessage" << bRet << endl;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

// This is the  window procedure for our window
// We're only going to do something special when painting and when resing.
// When the 
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
		PaintWindow(hWnd);
		break;
	case WM_SIZE:
	{
		WORD width = LOWORD(lParam);
		WORD height = HIWORD(lParam);
		ResizeWindow(hWnd, width, height);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

const char* VertexShader =
{
	"#version 400\n"\

	"layout(location=0) in vec4 in_Position;\n"\
	"layout(location=1) in vec4 in_Color;\n"\
	"out vec4 ex_Color;\n"\
	"out vec4 ex_Pos;\n"\

	"void main(void)\n"\
	"{\n"\
	"   gl_Position = in_Position;\n"\
	"   ex_Color = in_Color;\n"\
	"   ex_Pos = in_Position;\n"\
	"}\n"
};

const char* FragmentShaderNormal =
{
	"#version 400\n"\

	"in vec4 ex_Color;\n"\
	"in vec4 ex_Pos;\n"\
	"out vec4 out_Color;\n"\

	"void main(void)\n"\
	"{\n"\
	"   //out_Color = ex_Color;\n"\
	"   float x = (ex_Pos.x + 1.0)/2.0;\n"\
	"   out_Color = vec4(x, x, x, 1.0);\n"\
	"}\n"
};

const char* FragmentShader8Bit =
{
	"#version 400\n"\

	"in vec4 ex_Color;\n"\
	"in vec4 ex_Pos;\n"\
	"out vec4 out_Color;\n"\

	"void main(void)\n"\
	"{\n"\
	"   //out_Color = ex_Color;\n"\
	"   float x = (ex_Pos.x + 1.0)/2.0;\n"\
	"   x *= 255.0;\n"\
	"   x = floor(x);\n"\
	"   x /= 255.0;\n"\

	"   out_Color = vec4(x, x, x, 1.0);\n"\
	"}\n"
};

void CreateVBO(bool upper, GLuint &VaoId, GLuint &VboId)
{
	GLfloat Vertices[] = {
		-1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f,

		1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f
	};

	for (int i = 0; i < 6; i++)
	{
		int y = i * 4 + 1;

		if (upper)
			Vertices[y] = Vertices[y] / 2.0f + 0.5f;
		else
			Vertices[y] = Vertices[y] / 2.0f - 0.5f;
	}

	int ErrorCheckValue = mygl::glGetError();

	mygl::glGenVertexArrays(1, &VaoId);
	mygl::glBindVertexArray(VaoId);

	mygl::glGenBuffers(1, &VboId);
	mygl::glBindBuffer(GL_ARRAY_BUFFER, VboId);
	mygl::glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
	mygl::glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	mygl::glEnableVertexAttribArray(0);

	ErrorCheckValue = mygl::glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
		ErrorExit(ERRCODE_CANTCREATEVBO, "Could not create a VBO");
}

void CreateShaders(void)
{
	GLenum ErrorCheckValue = mygl::glGetError();

	VertexShaderId = mygl::glCreateShader(GL_VERTEX_SHADER);
	mygl::glShaderSource(VertexShaderId, 1, &VertexShader, NULL);
	mygl::glCompileShader(VertexShaderId);

	FragmentShaderId1 = mygl::glCreateShader(GL_FRAGMENT_SHADER);
	mygl::glShaderSource(FragmentShaderId1, 1, &FragmentShaderNormal, NULL);
	mygl::glCompileShader(FragmentShaderId1);

	ProgramId1 = mygl::glCreateProgram();
	mygl::glAttachShader(ProgramId1, VertexShaderId);
	mygl::glAttachShader(ProgramId1, FragmentShaderId1);
	mygl::glLinkProgram(ProgramId1);

	FragmentShaderId2 = mygl::glCreateShader(GL_FRAGMENT_SHADER);
	mygl::glShaderSource(FragmentShaderId2, 1, &FragmentShader8Bit, NULL);
	mygl::glCompileShader(FragmentShaderId2);

	ProgramId2 = mygl::glCreateProgram();
	mygl::glAttachShader(ProgramId2, VertexShaderId);
	mygl::glAttachShader(ProgramId2, FragmentShaderId2);
	mygl::glLinkProgram(ProgramId2);

	ErrorCheckValue = mygl::glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
		ErrorExit(ERRCODE_CANTCREATESHADER, "Could not create the shaders");
}

void initializeGL(HWND hWnd, HDC hDC)
{
	mygl::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateShaders();
	CreateVBO(true, VaoId0, VboId0);
	CreateVBO(false, VaoId1, VboId1);
}

void paintGL()
{
	mygl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mygl::glBindVertexArray(VaoId0);
	mygl::glUseProgram(ProgramId1);
	mygl::glDrawArrays(GL_TRIANGLES, 0, 6);

	mygl::glBindVertexArray(VaoId1);
	mygl::glUseProgram(ProgramId2);
	mygl::glDrawArrays(GL_TRIANGLES, 0, 6);

	mygl::glFlush();
}

void PaintWindow(HWND hWnd)
{
	HDC hDC = GetDC(hWnd);

	if (!mygl::wglMakeCurrent(hDC, globalContext))
		ErrorExit(ERRCODE_CANTMAKECONTEXTCURRENT, "Unable to make gl context current");

	if (!g_init)
	{
		g_init = true;
		initializeGL(hWnd, hDC);
	}

	paintGL();
	SwapBuffers(hDC); 

	mygl::wglMakeCurrent(hDC, 0);
}

void ResizeWindow(HWND hWnd, int w, int h)
{
	if (globalContext)
	{
		HDC hDC = GetDC(hWnd);
		mygl::wglMakeCurrent(hDC, globalContext);
		mygl::glViewport(0, 0, w, h);
	}
}
