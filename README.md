# tutorial_winapi
basics of winapi

<p>Set up your environment to compile, execute and debug the code.
Install Visual Studio Community edition, open a command prompt, locate vcvarsall.bat to set up environment variables. 
Open code editor, write a simple main and execute cl.exe with the source file name. Now everything ready, start writing winapi code to open a window.
</p>

<p>Start with writing the entry point WinMain. Then follow these steps:</p>
  * Create and Register the WindowClass.
  * Create Main Windows Callback, Start Processing Basic Messages.
  * Create a Window.
  * Start the Message Loop with GetMessageA.
  * Create a Bitmap using CreateDIBSection.
  * Output the Bitmap to the Window using StretchDIBits.
