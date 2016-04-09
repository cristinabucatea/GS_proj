#include <windows.h>          // Header File For Windows
#include <stdio.h>            // Header File For Standard Input/Output ( ADD )
#include <gl\gl.h>            // Header File For The OpenGL32 Library
#include <gl\glu.h>           // Header File For The GLu32 Library

#include <fstream>
#include <vector>

#define MAX_PARTICLES   5000        // Number Of Particles To Create ( NEW )

HDC     hDC = NULL;       // Private GDI Device Context
HGLRC       hRC = NULL;       // Permanent Rendering Context
HWND        hWnd = NULL;      // Holds Our Window Handle
HINSTANCE   hInstance;      // Holds The Instance Of The Application

bool    keys[256];          // Array Used For The Keyboard Routine
bool    active = TRUE;            // Window Active Flag Set To TRUE By Default
bool    fullscreen = TRUE;        // Fullscreen Flag Set To Fullscreen Mode By Default
float   slowdown = 10.0f;          // Slow Down Particles
float   zoom = -30.0f;            // Used To Zoom Out
GLuint  loop;               // Misc Loop Variable
GLuint  texture[1];         // Storage For Our Particle Texture

typedef struct                      // Create A Structure For Particle
{
	bool    active;                 // Active (Yes/No)
	float   life;                   // Particle Life
	float   fade;                   // Fade Speed
	float   x;                  // X Position
	float   y;                  // Y Position
	float   z;                  // Z Position
	float   xi;                 // X Direction
	float   yi;                 // Y Direction
	float   zi;                 // Z Direction
	float   r;                  // Red Value
	float   g;                  // Green Value
	float   b;                  // Blue Value

}
particles;                      // Particles Structure

particles particle[MAX_PARTICLES];          // Particle Array (Room For Particle Info)
static GLfloat color[3] = { 49.0f, 79.0f, 79.0f };

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);   // Declaration For WndProc

bool LoadBMP(const char* FilePath, std::vector<unsigned char> &Pixels)
{
	int width = 0;
	int height = 0;
	short BitsPerPixel = 0;

	std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
	if (!hFile.is_open())
		return false;

	hFile.seekg(0, std::ios::end);
	int Length = hFile.tellg();
	hFile.seekg(0, std::ios::beg);
	std::vector<std::uint8_t> FileInfo(Length);
	hFile.read(reinterpret_cast<char*>(FileInfo.data()), 54);

	if (FileInfo[0] != 'B' && FileInfo[1] != 'M')
	{
		hFile.close();
		return false;
	}

	if (FileInfo[28] != 24 && FileInfo[28] != 32)
	{
		hFile.close();
		return false;
	}

	BitsPerPixel = FileInfo[28];
	width = FileInfo[18] + (FileInfo[19] << 8);
	height = FileInfo[22] + (FileInfo[23] << 8);
	std::uint32_t PixelsOffset = FileInfo[10] + (FileInfo[11] << 8);
	std::uint32_t size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
	Pixels.resize(size);

	hFile.seekg(PixelsOffset, std::ios::beg);
	hFile.read(reinterpret_cast<char*>(Pixels.data()), size);
	hFile.close();

	return true;
}

int LoadGLTextures() // Load Bitmaps And Convert To Textures
{
	int Status = FALSE; // Status Indicator
	std::vector<unsigned char> Pixels;
	if (LoadBMP("Particle.bmp",Pixels)) // Load Particle Texture
	{
		Status = TRUE; // Set The Status To TRUE
		glGenTextures(1, &texture[0]); // Create One Textures

		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, Pixels.data());
	}

	return Status; // Return The Status
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) // Resize And Initialize The GL Window
{
	if (height == 0) // Prevent A Divide By Zero By
	{
		height = 1; // Making Height Equal One
	}

	glViewport(0, 0, width, height); // Reset The Current Viewport

	glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
	glLoadIdentity(); // Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 200.0f);

		glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
	glLoadIdentity(); // Reset The Modelview Matrix
}

int InitGL(GLvoid)                              // All Setup For OpenGL Goes Here
{
	if (!LoadGLTextures())                          // Jump To Texture Loading Routine
	{
		return FALSE;                           // If Texture Didn't Load Return FALSE
	}
	glShadeModel(GL_SMOOTH);                        // Enables Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);                  // Black Background
	glClearDepth(1.0f);                         // Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);                       // Disables Depth Testing
	glEnable(GL_BLEND);                         // Enable Blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);                   // Type Of Blending To Perform
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);           // Really Nice Perspective Calculations
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);                 // Really Nice Point Smoothing
	glEnable(GL_TEXTURE_2D);                        // Enable Texture Mapping
	glBindTexture(GL_TEXTURE_2D, texture[0]);                // Select Our Texture

	for (loop = 0;loop < MAX_PARTICLES;loop++)                   // Initialize All The Textures
	{
		particle[loop].active = true;                 // Make All The Particles Active
		particle[loop].life = 1.0f;                   // Give All The Particles Life
		particle[loop].fade = float(rand() % 100) / 1000.0f + 0.001f;       // Random Fade Speed
		particle[loop].r = color[0];        
		particle[loop].g = color[1];        
		particle[loop].b = color[2];    
		particle[loop].x = 0.0f;                  // Center On X Axis
		particle[loop].y = 0.0f;                  // Center On Y Axis
		particle[loop].z = 0.0f;                  // Center On Z Axis
		particle[loop].xi = 0.0f;       // Random Speed On X Axis
		particle[loop].yi = 0.0f;       // Random Speed On Y Axis
		particle[loop].zi = 0.0f;       // Random Speed On Z Axis
	}
	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)                             // Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         // Clear Screen And Depth Buffer
	glLoadIdentity();                           // Reset The ModelView Matrix									// Reset The Current Modelview Matrix

	for (loop = 0;loop < MAX_PARTICLES;loop++)                   // Loop Through All The Particles
	{
		if (particle[loop].active)                  // If The Particle Is Active
		{
			float x = particle[loop].x;               // Grab Our Particle X Position
			float y = particle[loop].y;               // Grab Our Particle Y Position
			float z = particle[loop].z + zoom;              // Particle Z Pos + Zoom
			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(particle[loop].r, particle[loop].g, particle[loop].b, particle[loop].life);
			glBegin(GL_TRIANGLE_STRIP);             // Build Quad From A Triangle Strip
			glTexCoord2d(1, 1); 
			glVertex3f(x + 0.5f, y + 0.5f, z); // Top Right
			glTexCoord2d(0, 1); 
			glVertex3f(x - 0.5f, y + 0.5f, z); // Top Left
			glTexCoord2d(1, 0); 
			glVertex3f(x + 0.5f, y - 0.5f, z); // Bottom Right
			glTexCoord2d(0, 0);
			glVertex3f(x - 0.5f, y - 0.5f, z); // Bottom Left
			glEnd();                        // Done Building Triangle Strip

				particle[loop].x += particle[loop].xi / (slowdown * 800);    // Move On The X Axis By X Speed
				particle[loop].y += particle[loop].yi / (slowdown * 800);    // Move On The Y Axis By Y Speed
				particle[loop].z += particle[loop].zi / (slowdown * 800);    // Move On The Z Axis By Z Speed
			
			particle[loop].life -= particle[loop].fade;       // Reduce Particles Life By 'Fade'
			if (particle[loop].life < 0.0f)                    // If Particle Is Burned Out
			{
				particle[loop].life = 1.0f;               // Give It New Life
				particle[loop].fade = float(rand() % 100) / 1000.0f + 0.001f;   // Random Fade Value
				particle[loop].x = 0.0f;                  // Center On X Axis
				particle[loop].y = 0.0f;                  // Center On Y Axis
				particle[loop].z = 0.0f;                  // Center On Z Axis
				particle[loop].xi = float((rand() % 60) - 30.0f);  // X Axis Speed And Direction
				particle[loop].yi = float((rand() % 60) + 30.0f);  // Y Axis Speed And Direction
				particle[loop].zi = float((rand() % 60) + 30.0f);     // Z Axis Speed And Direction
			}
		}
	}
	return TRUE;										// Everything Went OK
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, L"Release Of DC And RC Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, L"Release Rendering Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, L"Release Device Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, L"Could Not Release hWnd.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass(L"OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, L"Could Not Unregister Class.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
*	title			- Title To Appear At The Top Of The Window				*
*	width			- Width Of The GL Window Or Fullscreen Mode				*
*	height			- Height Of The GL Window Or Fullscreen Mode			*
*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(LPCWSTR title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height
	
	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = L"OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, L"Failed To Register The Window Class.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}


		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size
																		// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		L"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Window Creation Error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Create A GL Device Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Find A Suitable PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Set The PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Create A GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Activate The GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Initialization Failed.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
	case WM_ACTIVATE:							// Watch For Window Activate Message
	{
		if (!HIWORD(wParam))					// Check Minimization State
		{
			active = TRUE;						// Program Is Active
		}
		else
		{
			active = FALSE;						// Program Is No Longer Active
		}

		return 0;								// Return To The Message Loop
	}

	case WM_SYSCOMMAND:							// Intercept System Commands
	{
		switch (wParam)							// Check System Calls
		{
		case SC_SCREENSAVE:					// Screensaver Trying To Start?
		case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
			return 0;							// Prevent From Happening
		}
		break;									// Exit
	}

	case WM_CLOSE:								// Did We Receive A Close Message?
	{
		PostQuitMessage(0);						// Send A Quit Message
		return 0;								// Jump Back
	}

	case WM_KEYDOWN:							// Is A Key Being Held Down?
	{
		keys[wParam] = TRUE;					// If So, Mark It As TRUE
		return 0;								// Jump Back
	}

	case WM_KEYUP:								// Has A Key Been Released?
	{
		keys[wParam] = FALSE;					// If So, Mark It As FALSE
		return 0;								// Jump Back
	}

	case WM_SIZE:								// Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
		return 0;								// Jump Back
	}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE   hInstance,          // Instance
	HINSTANCE   hPrevInstance,          // Previous Instance
	LPSTR       lpCmdLine,          // Command Line Parameters
	int     nCmdShow)           // Window Show State
{
	MSG msg;                            // Windows Message Structure
	BOOL    done = FALSE;                     // Bool Variable To Exit Loop

	fullscreen = FALSE;        

	// Create Our OpenGL Window
	if (!CreateGLWindow(L"NeHe's Particle Tutorial", 640, 480, 16, fullscreen))
	{
		return 0;                       // Quit If Window Was Not Created
	}

	while (!done)                            // Loop That Runs Until done=TRUE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))       // Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)           // Have We Received A Quit Message?
			{
				done = TRUE;              // If So done=TRUE
			}
			else                        // If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);         // Translate The Message
				DispatchMessage(&msg);          // Dispatch The Message
			}
		}
		else                            // If There Are No Messages
		{
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])  // Updating View Only If Active
			{
				done = TRUE;              // ESC or DrawGLScene Signalled A Quit
			}
			else                        // Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);           // Swap Buffers (Double Buffering)
				if (keys[VK_ADD] && (slowdown > 1.0f))
					slowdown -= 0.01f;        // Speed Up Particles
				if (keys[VK_SUBTRACT] && (slowdown < 4.0f))
					slowdown += 0.01f;   // Slow Down Particles
				if (keys[VK_PRIOR])
					zoom += 0.1f;     // Zoom In
				if (keys[VK_NEXT])
					zoom -= 0.1f;      // Zoom Out
			}
		}
	}
	// Shutdown
	KillGLWindow();                     // Kill The Window
	return (msg.wParam);                    // Exit The Program
}
