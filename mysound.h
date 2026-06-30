#include <xaudio2.h>
#include <windows.h>
#include <cmath>
#include <vector>
#include <iostream>


struct xAudioVoice : IXAudio2VoiceCallback
{
	IXAudio2SourceVoice* voice;
	HANDLE hBufferEndEvent;

	xAudioVoice(): hBufferEndEvent( CreateEvent( NULL, FALSE, FALSE, NULL ) ){}
    	~xAudioVoice(){ CloseHandle( hBufferEndEvent ); }
  	bool playing;

	void OnStreamEnd() noexcept
	{
		//voice->Stop();
   	 	playing = false;
		printf("OnStreamEnd\n");
		SetEvent( hBufferEndEvent );
	}

	void OnBufferStart(void * pBufferContext) noexcept
	{
    		playing = true;
		printf("OnBufferStart\n");
	}

	void OnVoiceProcessingPassEnd() noexcept {
		//printf("OnVoiceProcessingPassEnd\n");
	}
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept {}
	void OnBufferEnd(void * pBufferContext) noexcept {
		playing = false;
		printf("OnBufferEnd\n");
	}
	void OnLoopEnd(void * pBufferContext) noexcept {
		printf("OnLoopEnd\n");
	}
	void OnVoiceError(void * pBufferContext, HRESULT Error) noexcept {}
};

static xAudioVoice Voice1;
static xAudioVoice Voice2;
static IXAudio2MasteringVoice* pMasteringVoice;
static IXAudio2* pXAudio2;
