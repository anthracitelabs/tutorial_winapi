# tutorial_winapi
basics of winapi

<p>Set up your environment to compile, execute and debug the code.
Install Visual Studio Community edition, open a command prompt, locate vcvarsall.bat to set up environment variables. 
Open code editor, write a simple main and execute cl.exe with the source file name. Now everything ready, start writing winapi code to open a window.
</p>

Start with writing the entry point WinMain. Then follow these steps:
  * Create and Register the WindowClass.
  * Create Main Windows Callback, Start Processing Basic Messages.
  * Create a Window.
  * Start the Message Loop with GetMessageA.
  * Create a Bitmap using CreateDIBSection.
  * Output the Bitmap to the Window using StretchDIBits.

<p>wWinMain Entry Point: The entry function used by the operating system for graphical Win32 desktop applications instead of the typical standard console main function.</p>
<p>WNDCLASS Structure: Defines the structural behavior and visual attributes of your window class, such as linking it to its corresponding event handling callback.</p>
<p>CreateWindowEx Function: Tells Windows to physically create the system object based on the registered class name while configuring its initial borders, title text, and dimensions.</p>
<p>GetMessage Message Loop: Keeps the application running continuously by constantly checking for user inputs (like keyboard strokes or clicks) and feeding them to the window procedure.</p>
<p>WindowProc Callback: Handles targeted window events. Crucially, capturing WM_DESTROY tells the OS to shut down the main thread fully when the user clicks the "X" close button.</p>

~~~~~~~~~~~~ C
#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>

// Forward declaration of the window procedure callback function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// The application entry point (instead of main)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    
    // 1. Register the Window Class
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    
    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;       // Pointer to the event handler function
    wc.hInstance     = hInstance;        // Handle to the application instance
    wc.lpszClassName = CLASS_NAME;       // Unique name identifying this class

    RegisterClass(&wc);

    // 2. Create the Window Instance
    HWND hwnd = CreateWindowEx(
        0,                               // Optional window styles
        CLASS_NAME,                      // Window class name
        L"My First WinAPI Window",       // Window title text
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
    MSG msg = { };
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

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // All custom window rendering/drawing goes here
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
            
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    // Pass any unhandled messages to default Windows behavior
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
~~~~~~~~~~~~
