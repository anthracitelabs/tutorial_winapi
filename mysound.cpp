
#include "mysound.h"


struct read_file_result
{
    uint32_t ContentsSize;
    void *Contents;
};

read_file_result ReadEntireFile(char *Filename)
{
    read_file_result Result = {};

    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            // FileSize.QuadPart is the 64-bit value of the size.
            uint32_t FileSize32 = (uint32_t)FileSize.QuadPart;
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead))
                {
                    // File read successfully
                    Result.ContentsSize = BytesRead;
                }
                else
                {
                    // Read failed
		    if(Result.Contents)
		    {
        		VirtualFree(Result.Contents, 0, MEM_RELEASE);
		    }
                    Result.Contents = 0;
                }
            }
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // Error: handle creation failed
        // TODO: Logging
    }

    return (Result);
}

bool WriteEntireFile(char *Filename, uint32_t MemorySize, void *Memory)
{
    bool Result = false;

    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            // File written successfully
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            // Write failed
        }
        CloseHandle(FileHandle);
    }
    else
    {
        // Handle creation failed
    }

    return (Result);
}




void sound_init()
{
	// 1. Initialize COM library
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		std::cerr << "Failed to initialize COM: " << hr << std::endl;
	}

	// 2. Create the XAudio2 Engine Instance
	pXAudio2 = nullptr;
	hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR); //
	if (FAILED(hr)) {
		std::cerr << "Failed to create XAudio2 engine: " << hr << std::endl;
		CoUninitialize();
	}

	// 3. Create the Mastering Voice (represents the default audio output device)
	pMasteringVoice = nullptr;
	hr = pXAudio2->CreateMasteringVoice(&pMasteringVoice); //
	if (FAILED(hr)) {
		std::cerr << "Failed to create mastering voice: " << hr << std::endl;
		pXAudio2->Release();
		CoUninitialize();
	}

	// 4. Define the Audio Format (CD Quality: 44100Hz, 16-bit Mono PCM)
	const DWORD SAMPLE_RATE = 44100; //
	const double FREQUENCY = 440.0;  // 440 Hz pitch (A4)
	const double PI = 3.14159265358979323846;

	WAVEFORMATEX wfx = {};
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2; // Stereo
	wfx.nSamplesPerSec = SAMPLE_RATE;
	wfx.wBitsPerSample = 16; // 16-bit audio
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.cbSize = 0;

	// 5. Load raw files
	read_file_result Result = ReadEntireFile("data/music.raw");
	uint8_t* RawByte = (uint8_t*)Result.Contents;
	std::vector<BYTE> audioBuffer1(Result.ContentsSize);
	for (size_t i = 0; i < Result.ContentsSize; ++i)
	{
		audioBuffer1[i] = *RawByte++;
	}

	read_file_result Result2 = ReadEntireFile("data/congratulations.raw");
	uint8_t* RawByte2 = (uint8_t*)Result2.Contents;
	std::vector<BYTE> audioBuffer2(Result2.ContentsSize);
	for (size_t i = 0; i < Result2.ContentsSize; ++i)
	{
		audioBuffer2[i] = *RawByte2++;
	}

	// 6. Create the Source Voice
       	Voice1.voice = nullptr;	
       	Voice2.voice = nullptr;	
	hr = pXAudio2->CreateSourceVoice(&Voice1.voice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &Voice1, NULL, NULL);
	hr = pXAudio2->CreateSourceVoice(&Voice2.voice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &Voice2, NULL, NULL);
	
	if (FAILED(hr)) {
		std::cout << "Failed to create source voice: " << hr << std::endl;
		pMasteringVoice->DestroyVoice();
		pXAudio2->Release();
		CoUninitialize();
	}

	// 7. Configure the XAudio2 Data Buffers for Infinite Looping
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = static_cast<UINT32>(audioBuffer1.size() * sizeof(BYTE));
	buffer.pAudioData = audioBuffer1.data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount = 0;//XAUDIO2_LOOP_INFINITE; // Tells XAudio2 to loop this buffer forever

	XAUDIO2_BUFFER buffer2 = {};
	buffer2.AudioBytes = static_cast<UINT32>(audioBuffer2.size() * sizeof(BYTE));
	buffer2.pAudioData = audioBuffer2.data();
	buffer2.Flags = XAUDIO2_END_OF_STREAM;
	buffer2.LoopCount = 5;//XAUDIO2_LOOP_INFINITE; // Tells XAudio2 to loop this buffer forever

	// 8. Submit and Play the Audio
	hr = Voice1.voice->SubmitSourceBuffer(&buffer); //
	hr = Voice2.voice->SubmitSourceBuffer(&buffer2); //
	/*if (SUCCEEDED(hr)) {
		Voice1.voice->Start(0); //
		Voice2.voice->Start(0); //
		std::cout << "Congratulations!" << std::endl;

		WaitForSingleObjectEx( Voice1.hBufferEndEvent, INFINITE, TRUE );
	}*/

}

void play_music()
{
	Voice1.voice->Start(0); //
}

void play_sound()
{
	Voice2.voice->Start(0); //
}

void sound_close()
{
	// 9. Clean up resources in reverse order of creation
	Voice1.voice->DestroyVoice();
	Voice2.voice->DestroyVoice();
	pMasteringVoice->DestroyVoice();
	pXAudio2->Release();
	CoUninitialize();
}

