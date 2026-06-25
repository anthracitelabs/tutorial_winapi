# tutorial_winapi

# Getting your environment ready

You can use any editor or IDE you like to follow this tutorial. I use a text editor (vim) to edit source files and command line to compile them into the final executable. You only need to download Visual Studio Build Tools.

Alternatively, to use Visual Studio as your source editor/debugger do the following in Visual Studio installation:

In the Workloads section, select only Desktop development with C++
You can further reduce total space required by leaving only the following tools (### stands for current version number of the tools):
MSVC v### - VS 2019 C++ x64/x86 build tools
C++/CLI support for v###
Windows 10 SDK (highest version available)
IntelliCode (If you are planning on using Visual Studio as your text editor)

<p>Set up your environment to compile, execute and debug the code.
Install Visual Studio Community edition, open a command prompt, locate vcvarsall.bat to set up environment variables. 
Open code editor, write a simple main and execute cl.exe with the source file name. Now everything ready, start writing winapi code to open a window.
</p>

# First Steps, Opening a Window

Start with writing the entry point WinMain. Then follow these steps:
  * Create and Register the WindowClass.
  * Create Main Windows Callback, Start Processing Basic Messages.
  * Create a Window.
  * Start the Message Loop with GetMessage.
  * Create a Bitmap using CreateDIBSection.
  * Output the Bitmap to the Window using StretchDIBits.

<p>wWinMain Entry Point: The entry function used by the operating system for graphical Win32 desktop applications instead of the typical standard console main function.</p>
<p>WNDCLASS Structure: Defines the structural behavior and visual attributes of your window class, such as linking it to its corresponding event handling callback.</p>
<p>CreateWindowEx Function: Tells Windows to physically create the system object based on the registered class name while configuring its initial borders, title text, and dimensions.</p>
<p>GetMessage Message Loop: Keeps the application running continuously by constantly checking for user inputs (like keyboard strokes or clicks) and feeding them to the window procedure.</p>
<p>WindowProc Callback: Handles targeted window events. Crucially, capturing WM_DESTROY tells the OS to shut down the main thread fully when the user clicks the "X" close button.</p>

~~~~~~~~~~~~ C
#include <windows.h>

// Forward declaration of the window procedure callback function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// The application entry point (instead of main)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    
    // 1. Register the Window Class
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;       // Pointer to the event handler function
    wc.hInstance     = hInstance;        // Handle to the application instance
    wc.lpszClassName = "Sample Window";       // Unique name identifying this class
    RegisterClass(&wc);

    // 2. Create the Window Instance
    HWND hwnd = CreateWindowEx(
        0,                               // Optional window styles
        "Sample Window",                      // Window class name
        "My First WinAPI Window",       // Window title text
        WS_OVERLAPPEDWINDOW,             // Standard window style (minimize, maximize, borders)

        // Size and position (using default OS coordinates)
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    // 3. Display the Window
    ShowWindow(hwnd, nCmdShow);

    // 4. Run the Message Loop (keeps the program alive and listening for events)
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// 5. The Window Procedure (handles events like clicking close, resizing, painting)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0); // Signals the message loop to stop when the window closes
            return 0;

        /*case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // All custom window rendering/drawing goes here
            //FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
            
            EndPaint(hwnd, &ps);
            return 0;
        }*/
    }
    // Pass any unhandled messages to default Windows behavior
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
~~~~~~~~~~~~

Note: CALLBACK and WINAPI are preprocessor macros that technically expand to the exact same calling convention (__stdcall) on modern x86 Windows platforms. CALLBACK is used for functions you write inside your application that the Windows operating system triggers internally. WINAPI is used for native functions the OS exposes that your application calls to perform system-level tasks. The distinction serves readability purposes.

Use the Visual Studio compiler cl.exe to compile the source file. Note that cl command also runs the linker by default. Open Visual Studio Command prompt and execute:

~~~~~~~~~~~~ cmd
cl main.c user32.lib
main.exe
~~~~~~~~~~~~

We give the necessary files as parameters to cl.exe , compiler needs them to compile our source file. Note that there're a one implicit library that is imported together with the CRT (kernel32.lib), so we did not specify this one. However for other stuff, like calling into the UI system to draw, we need to let the linker know about a new library (user32.lib) to import. When the linker combines all the .obj files and makes an executable, we should add an import library for the runtime functions that we use.

Note: -Zi generates a separate PDB file that contains all the symbolic debugging information for use with the debugger. Implicitly sets -DEBUG flag.

# Drawing Graphics on our Window

Now that we have our window ready, we will draw some graphics on it. We will use GDI library for this purpose. The idea is simply creating a buffer and sending it to the screen.

First let's introduce the function called ResizeDIBSection. It will serve to resize (or initialize if it doesn't exist) a Device independent Bitmap (DIB). This is the name Windows gives to the bitmaps that we can display using GDI. We will call it every time we get a WM_SIZE message.

A key piece of information that we will need to resize our buffer is the size of our window's client area, i.e. the area which we can draw into (it's smaller than the actual window area). The Windows API function responsible for it is called simply GetClientRect. 

We will allocate our video buffer inside ResizeDIBSection function. VirtualAlloc is the lowest level function available in Windows API, it allocates pages of memory at a time and clears them to 0. 

We have our BitmapMemory pointer, it will be drawn in full to the window, but before we're ready to do so, there's one question: how will it be read by StretchDIBits? How is StretchDIBits going to access this memory?

Memory is just a (giant) series of bytes, one following the other. On the other hand, Bitmap is a 2D grid, each square representing one pixel. We'll therefore need a convention to represent our 2D bitmap in 1D memory. The passage from one row to another is called pitch or a stride. It's typically a value that you add to the pointer to move the base from one row to another.

"The origin of a bottom-up DIB is the lower-left corner; the origin of a top-down DIB is the upper-left corner. [...] StretchDIBits creates a top-down image if the sign of the biHeight member of the BITMAPINFOHEADER structure for the DIB is negative."

What does it mean? If we want the rows to go sequentially from top-down, we need to update our BitmapInfo header height. Let's do it now:

~~~~~~~~~~~~ C
BitmapInfo.bmiHeader.biHeight = -BitmapHeight; // negative value: top-down pitch
~~~~~~~~~~~~


Graphics almost drawn:

~~~~ C
#include <windows.h>

#include <stdint.h> // for predefined sized types
#include <stdbool.h>


static bool Running;
static BITMAPINFO BitmapInfo;
static void* BitmapMemory;

static void Win32ResizeDIBSection(int Width, int Height)
{
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = -Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	if (BitmapMemory) // Same as writing (BitmapMemory != 0) or (BitmapMemory != NULL)
	{
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
		// Optionally, you can check if the result of VirtualFree is not zero.
		// Print out an error message if it is.
	}

	int BytesPerPixel = 4;
	int BitmapMemorySize = BytesPerPixel * (Width * Height);

	BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	uint8_t *Row = (uint8_t *)BitmapMemory;
	
	for (int Y = 0; Y < Height; ++Y)
	{
		for(int X = 0; X < Width; ++X)
		{
			// Write color to pixel
			//Pixel in memory: BB GG RR XX
			//
/*
			// Byte 0, blue
			*Pixel = 0;
			++Pixel;  

			// Byte 1, green
			*Pixel = 0;
			++Pixel;  

			// Byte 2, red
			*Pixel = 255;
			++Pixel;  

			// Byte 3, pad
			*Pixel = 0;
			++Pixel;  
*/
		}
	}
}

static void Win32UpdateWindow(HDC DeviceContext, int Width, int Height)
{
    // StretchDIBits is a rectangle-to-rectangle image copy. If the destination rectangle is bigger, the image is increased, if not, shrunk down.
    int result = StretchDIBits(DeviceContext, 
                  0, 0, Width, Height,
                  0, 0, Width, Height,
                  BitmapMemory,
                  &BitmapInfo,
                  DIB_RGB_COLORS, SRCCOPY);   
	int a = 0;

}

// Forward declaration of the window procedure callback function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// The application entry point (instead of main)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    
    // 1. Register the Window Class
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;       // Pointer to the event handler function
    wc.hInstance     = hInstance;        // Handle to the application instance
    wc.lpszClassName = "Sample Window";       // Unique name identifying this class
    RegisterClass(&wc);

    // 2. Create the Window Instance
    HWND Window = CreateWindowEx(
        0,                               // Optional window styles
        wc.lpszClassName,                      // Window class name
        "My First WinAPI Window",       // Window title text
        WS_OVERLAPPEDWINDOW,             // Standard window style (minimize, maximize, borders)

        // Size and position (using default OS coordinates)
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

	if (Window)
        {
            // Window creation successful!
            Running = true;

            Win32ResizeDIBSection(640, 480);

            while (Running) 
            {
		    MSG Message;
		    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		    {
			    switch(Message.message)
			    {
				    case WM_QUIT:
					    {
						    Running = false;
					    } break;

				    default:
					    {    
						    TranslateMessage(&Message);
						    DispatchMessage(&Message);
					    } break;
			    }
		    }

		    HDC DeviceContext = GetDC(Window);
		    Win32UpdateWindow(DeviceContext, 640, 480);
		    ShowWindow(Window, nCmdShow); 
		    ReleaseDC(Window, DeviceContext);
            }
        }
        else
        {
            // Window Creation failed! 
        }

    return 0;
}

// 5. The Window Procedure (handles events like clicking close, resizing, painting)
LRESULT CALLBACK WindowProc(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
	    Running = false;
            PostQuitMessage(0); // Signals the message loop to stop when the window closes
            return 0;
	
	case WM_CLOSE:
        {
            Running = false;
        } break;
	
	case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        } break;

        case WM_PAINT: {
	    PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);

            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            
            Win32UpdateWindow(DeviceContext, Width, Height);

            EndPaint(Window, &Paint);
        } break;
    }
    // Pass any unhandled messages to default Windows behavior
    return DefWindowProc(Window, uMsg, wParam, lParam);
}
~~~~





