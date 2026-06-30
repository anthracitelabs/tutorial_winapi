#include <windows.h>

#include <stdint.h> // for predefined sized types
#include <stdbool.h>
#include <stdio.h>

#include "mysound.cpp"

static bool Running;
static BITMAPINFO BitmapInfo;
static void* BitmapMemory;
static int WindowWidth;
static int WindowHeight;
static int64_t GlobalPerfCountFrequency;
static int test_cursor;

inline LARGE_INTEGER Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return (Result);
}

inline float Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    float Result = (float)(End.QuadPart - Start.QuadPart) / (float)GlobalPerfCountFrequency;
    return (Result);
}


#define BUFFER_WIDTH	32
#define BUFFER_HEIGHT	24

static void Win32ResizeDIBSection(int Width, int Height)
{
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = -Height; // negative value: top-down pitch
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
	uint32_t *Row = (uint32_t *)BitmapMemory;
	uint32_t *Pixel = (uint32_t *)BitmapMemory;
	
	uint32_t ColorPattern[5] = { 0x00FF0000, 0x0000FF00, 0x000000FF, 0x0000FFFF, 0x00FF00FF };
			// 32 bit variable assigned to memory as a whole	XX RR GG BB
			// 8 bits variable assigned to memory by iteration	BB GG RR XX
	
	for (int Y = 0; Y < Height; ++Y)
	{
		for(int X = 0; X < Width; ++X)
		{
			// Write color to pixel
			*Pixel = ColorPattern[test_cursor];
			++Pixel;  
		}
		++Row;
	}
}

static void Win32UpdateWindow(HDC DeviceContext)
{
	// StretchDIBits is a rectangle-to-rectangle image copy. If the destination rectangle is bigger, the image is increased, if not, shrunk down.
	StretchDIBits(DeviceContext, 
			0, 0, WindowWidth, WindowHeight,
			0, 0, BUFFER_WIDTH, BUFFER_HEIGHT,
			BitmapMemory,
			&BitmapInfo,
			DIB_RGB_COLORS, SRCCOPY);
}

// Forward declaration of the window procedure callback function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void Win32ProcessPendingMessages()
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

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
                {
                    uint32_t VKCode = (uint32_t)Message.wParam;
                    bool IsDown = ((Message.lParam & (1 << 31)) == 0);
                    bool WasDown = ((Message.lParam & (1 << 30)) != 0);

                    if(IsDown != WasDown)
                    {
                        if (VKCode == VK_UP)
                        {
				play_sound();

				test_cursor = 0;
				Win32ResizeDIBSection(BUFFER_WIDTH, BUFFER_HEIGHT);
                        }
                        else if (VKCode == VK_DOWN)
                        {
				test_cursor = 1;
				Win32ResizeDIBSection(BUFFER_WIDTH, BUFFER_HEIGHT);
                        }
                        else if (VKCode == VK_LEFT)
                        {
				test_cursor = 2;
				Win32ResizeDIBSection(BUFFER_WIDTH, BUFFER_HEIGHT);
                        }
                        else if (VKCode == VK_RIGHT)
                        {
				test_cursor = 3;
				Win32ResizeDIBSection(BUFFER_WIDTH, BUFFER_HEIGHT);
                        }
                        else if (VKCode == VK_ESCAPE)
                        {
                            Running = false;
                        }

                        bool AltKeyWasDown = ((Message.lParam & (1 << 29)) != 0);
                        if((VKCode == VK_F4) && AltKeyWasDown)
                        {
                            Running = false;
                        }
                    }
                } break;

            default:
                {    
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                } break;
        }
    }
}

// The application entry point (instead of main)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) 
{
    test_cursor = 2;

    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    // NOTE(casey): Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular
    UINT DesiredSchedulerMS = 1;
    bool SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);


    // 1. Register the Window Class
    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProc;       // Pointer to the event handler function
    wc.hInstance     = hInstance;        // Handle to the application instance
    wc.lpszClassName = "Sample Window";       // Unique name identifying this class
    RegisterClass(&wc);

    // 2. Create the Window Instance
    HWND Window = CreateWindowEx(
        0,                               // Optional window styles
        wc.lpszClassName,                      // Window class name
        "My First WinAPI Window",       // Window title text
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,             // Standard window style (minimize, maximize, borders)

        // Size and position (using default OS coordinates)
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

	if (Window)
        {
		sound_init();

		// Window creation successful!
		Running = true;

		int MonitorRefreshHz = 60;
		HDC RefreshDC = GetDC(Window);
		int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
		ReleaseDC(Window, RefreshDC);
		if(Win32RefreshRate > 1)
		{
			MonitorRefreshHz = Win32RefreshRate;
		}

		float GameUpdateHz = (MonitorRefreshHz / 2.0f);
		float TargetSecondsPerFrame = 1.0f / (float)GameUpdateHz;
		Win32ResizeDIBSection(BUFFER_WIDTH, BUFFER_HEIGHT);

		LARGE_INTEGER LastCounter = Win32GetWallClock();
		LARGE_INTEGER FlipWallClock = Win32GetWallClock();
		uint64_t LastCycleCount = __rdtsc();

		play_music();

		while (Running) 
		{
			Win32ProcessPendingMessages();

			LARGE_INTEGER WorkCounter = Win32GetWallClock();
			float WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

			float SecondsElapsedForFrame = WorkSecondsElapsed;
			if(SecondsElapsedForFrame < TargetSecondsPerFrame)
			{
				if(SleepIsGranular)
				{
					DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
					if(SleepMS > 0)
					{
						Sleep(SleepMS);
					}
				}

				float TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
				if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
				{
					// TODO : Log missed sleep here
				}

				while(SecondsElapsedForFrame < TargetSecondsPerFrame)
				{
					SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
				}
			}
			else
			{
				// TODO : Log missed frame !!
			}

			LARGE_INTEGER EndCounter = Win32GetWallClock();
			float MSPerFrame = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
			LastCounter = EndCounter;
			HDC DeviceContext = GetDC(Window);

			Win32UpdateWindow(DeviceContext);
			//ShowWindow(Window, nCmdShow); // either this or the WS_VISIBLE flag when creating window 
			ReleaseDC(Window, DeviceContext);
			
			FlipWallClock = Win32GetWallClock();
			uint64_t EndCycleCount = __rdtsc();
			int64_t CyclesElapsed = EndCycleCount - LastCycleCount;
			LastCycleCount = EndCycleCount;



                    char FPSBuffer[256];
                    float TargetMsPerFrame = 1000.0f * TargetSecondsPerFrame;
                    _snprintf_s(FPSBuffer, sizeof(FPSBuffer), "%.02fms/f, %.02fms/f\n", MSPerFrame, TargetMsPerFrame);
                    OutputDebugStringA(FPSBuffer);
		}

		sound_close();
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
				// update width, height globals for StretchDIBits
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);
				WindowWidth = ClientRect.right - ClientRect.left;
				WindowHeight = ClientRect.bottom - ClientRect.top;
				
				Win32ResizeDIBSection(BUFFER_WIDTH, BUFFER_HEIGHT);
			} break;

		case WM_PAINT: // if this does not exist, a bigger resized section will not be painted, otherwise no major or functional effect 
			{
				PAINTSTRUCT Paint;
				HDC DeviceContext = BeginPaint(Window, &Paint);

				int X = Paint.rcPaint.left;
				int Y = Paint.rcPaint.top;
				int Width = Paint.rcPaint.right - Paint.rcPaint.left;
				int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

				Win32UpdateWindow(DeviceContext);

				EndPaint(Window, &Paint);
			} break;
    }
    // Pass any unhandled messages to default Windows behavior
    return DefWindowProc(Window, uMsg, wParam, lParam);
}
