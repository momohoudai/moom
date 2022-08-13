#ifndef WIN_WASAPI
#define WIN_WASAPI

struct Win_Wasapi;
struct Win_Wasapi_Notif_Client {
  IMMNotificationClient imm_notifs;
  Win_Wasapi* wasapi;
  LONG ref;
};

struct Win_Wasapi {
  Platform_Audio platform_audio; // Must be first member
  
  Win_Wasapi_Notif_Client notifs;
  IMMDeviceEnumerator * mm_device_enum;
  IAudioClient2* audio_client;
  IAudioRenderClient* audio_render_client;
  
  // "Secondary" buffer
  U32 buffer_size;
  S16* buffer;
  
  // Other variables for tracking purposes
  U32 latency_sample_count;
  U32 samples_per_second;
  U16 bits_per_sample;
  U16 channels;
    
	B32 is_device_changed;
	B32 is_device_ready;

  Bump_Allocator allocator;
};

///////////////////////////////////////////////////////////
// IMPLEMENTATION
//
DEFINE_GUID(CLSID_MMDeviceEnumerator,  0xbcde0395, 0xe52f, 0x467c, 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e);
DEFINE_GUID(IID_IMMDeviceEnumerator,   0xa95664d2, 0x9614, 0x4f35, 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6);
DEFINE_GUID(IID_IAudioRenderClient,    0xf294acfc, 0x3146, 0x4483, 0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2);
DEFINE_GUID(IID_IAudioClient2,         0x726778cd, 0xf60a, 0x4eda, 0x82, 0xde, 0xe4, 0x76, 0x10, 0xcd,0x78, 0xaa);
DEFINE_GUID(IID_IMMNotificationClient, 0x7991eec9, 0x7e89, 0x4d85, 0x83, 0x90, 0x6c, 0x70, 0x3c, 0xec, 0x60, 0xc0);



//////////////////////////////////////////////////////////
// Win Audio Notifs implementation
static STDMETHODIMP_(ULONG)  
_win_wasapi_notif_client_AddRef(IMMNotificationClient* mm_notif) {
  Win_Wasapi_Notif_Client* win_notif = (Win_Wasapi_Notif_Client*)mm_notif;
  return InterlockedIncrement(&win_notif->ref);
}
static STDMETHODIMP_(ULONG)  
_win_wasapi_notif_client_Release(IMMNotificationClient* mm_notif) {
  Win_Wasapi_Notif_Client* win_notif = (Win_Wasapi_Notif_Client*)mm_notif;
  return InterlockedDecrement(&win_notif->ref);
}
static STDMETHODIMP_(HRESULT)  
_win_wasapi_notif_client_QueryInterface(IMMNotificationClient* mm_notif,
                                       REFIID riid,
                                       VOID **ppvInterface) 
{
  if (IID_IUnknown == riid)
  {
    _win_wasapi_notif_client_AddRef(mm_notif);
    *ppvInterface = (IUnknown*)mm_notif;
  }
  else if (IID_IMMNotificationClient == riid)
  {
    _win_wasapi_notif_client_AddRef(mm_notif);
    *ppvInterface = (IMMNotificationClient*)mm_notif;
  }
  else
  {
    *ppvInterface = 0;
    return E_NOINTERFACE;
  }
  return S_OK;
}
static STDMETHODIMP_(HRESULT) 
_win_wasapi_notif_client_OnDefaultDeviceChange(IMMNotificationClient* mm_notif,
                                              EDataFlow flow,
                                              ERole role,
                                              LPCWSTR pwstr_device_id)
{
  Win_Wasapi_Notif_Client* win_notif = (Win_Wasapi_Notif_Client*)mm_notif;
  win_notif->wasapi->is_device_changed = true;
  return S_OK;
}
static STDMETHODIMP_(HRESULT) 
_win_wasapi_notif_client_OnDeviceAdded(IMMNotificationClient* client, LPCWSTR pwstr_device_id)
{
  return S_OK;
}
static STDMETHODIMP_(HRESULT) 
_win_wasapi_notif_client_OnDeviceRemoved(IMMNotificationClient* client, LPCWSTR pwstr_device_id)
{
  return S_OK;
}

static STDMETHODIMP_(HRESULT) 
_win_wasapi_notif_client_OnDeviceStateChanged(IMMNotificationClient* client,
                                             LPCWSTR pwstr_device_id, 
                                             DWORD dwNewState)
{
  return S_OK;
}

static STDMETHODIMP_(HRESULT) 
_win_wasapi_notif_client_OnPropertyValueChanged(IMMNotificationClient* client,
                                               LPCWSTR pwstr_device_id, 
                                               const PROPERTYKEY key)
{
  return S_OK;
}


////////////////////////////////////////////////////
// Win Audio implementation
//
static IMMNotificationClientVtbl _win_wasapi_notifs_vtable {
  _win_wasapi_notif_client_QueryInterface,
  _win_wasapi_notif_client_AddRef,
  _win_wasapi_notif_client_Release,
  _win_wasapi_notif_client_OnDeviceStateChanged,
  _win_wasapi_notif_client_OnDeviceAdded,
  _win_wasapi_notif_client_OnDeviceRemoved,
  _win_wasapi_notif_client_OnDefaultDeviceChange,
  _win_wasapi_notif_client_OnPropertyValueChanged,
};

static B32 
_win_wasapi_set_default_device_as_current_device(Win_Wasapi* wasapi) {
  IMMDevice* device;
  HRESULT hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(wasapi->mm_device_enum, 
                                                           eRender, 
                                                           eConsole, 
                                                           &device);
  if (FAILED(hr)) {
    win_log("[win_wasapi] Failed to get wasapi endpoint\n");
    return false;
  }
  defer { IMMDevice_Release(device); };
  
  hr = IMMDevice_Activate(device,
                          IID_IAudioClient2, 
                          CLSCTX_ALL, 
                          0, 
                          (LPVOID*)&wasapi->audio_client);
  if(FAILED(hr)) {
    win_log("[win_wasapi] Failed to create IAudioClient\n");
    return false;
  }
  
  WAVEFORMATEX wave_format = {};
  wave_format.wFormatTag = WAVE_FORMAT_PCM;
  wave_format.wBitsPerSample = wasapi->bits_per_sample;
  wave_format.nChannels = wasapi->channels;
  wave_format.nSamplesPerSec = wasapi->samples_per_second;
  wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample / 8);
  wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
  
  REFERENCE_TIME buffer_duration = 0;
  hr = IAudioClient2_GetDevicePeriod(wasapi->audio_client, 0, &buffer_duration);
  
  DWORD stream_flags = (AUDCLNT_STREAMFLAGS_RATEADJUST |
                        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
                        AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY);

  hr = IAudioClient2_Initialize(wasapi->audio_client,
                                AUDCLNT_SHAREMODE_SHARED, 
                                stream_flags, 
                                buffer_duration,
                                0, 
                                &wave_format, 
                                0);
  if (FAILED(hr))
  {
    win_log("[win_wasapi] Failed to initialize wasapi client\n");
    return false;
  }
  
  hr = IAudioClient2_GetService(wasapi->audio_client, 
                               IID_IAudioRenderClient, 
                               (LPVOID*)&wasapi->audio_render_client);
  if (FAILED(hr))
  {
    win_log("[win_wasapi] Failed to create IAudioClient\n");
    return false;
  }
  
  UINT32 sound_frame_count;
  hr = IAudioClient2_GetBufferSize(wasapi->audio_client, &sound_frame_count);
  if (FAILED(hr))
  {
    win_log("[win_wasapi] Failed to get buffer size\n");
    return false;
  }

  ba_clear(&wasapi->allocator);
  wasapi->buffer_size = sound_frame_count;
  wasapi->buffer = ba_push<S16>(&wasapi->allocator, wasapi->buffer_size);
  if (!wasapi->buffer) {
    win_log("[win_wasapi] Failed to allocate secondary buffer\n");
    return false;
  }
  IAudioClient2_Start(wasapi->audio_client);
	wasapi->is_device_ready = true;
  return true;
}

static B32
win_wasapi_init(Win_Wasapi* wasapi,
                U32 samples_per_second, 
                U16 bits_per_sample,
                U16 channels,
                U32 latency_frames,
                U32 refresh_rate,
                Bump_Allocator* allocator)
{
  wasapi->channels = channels;
  wasapi->bits_per_sample = bits_per_sample;
  wasapi->samples_per_second = samples_per_second;
  wasapi->latency_sample_count = (samples_per_second / refresh_rate) * latency_frames;

  if (!ba_partition_with_remaining(allocator, &wasapi->allocator)) return false;
  
  HRESULT hr = CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY);
  if (FAILED(hr)) {
    win_log("[win_wasapi] Failed CoInitializeEx\n");
    return false;
  }
  
  hr = CoCreateInstance(CLSID_MMDeviceEnumerator, 
                        0,
                        CLSCTX_ALL, 
                        IID_IMMDeviceEnumerator,
                        (LPVOID*)(&wasapi->mm_device_enum));
  if (FAILED(hr)) {
    win_log("[win_wasapi] Failed to create IMMDeviceEnumerator\n");
    goto cleanup_1;
  }
   
  wasapi->notifs.imm_notifs.lpVtbl = &_win_wasapi_notifs_vtable;
	wasapi->notifs.ref = 1;
	wasapi->notifs.wasapi = wasapi;
  hr = IMMDeviceEnumerator_RegisterEndpointNotificationCallback(wasapi->mm_device_enum, &wasapi->notifs.imm_notifs);

	if(FAILED(hr)) {
		win_log("[win_wasapi] Failed to register notification callback\n");
		goto cleanup_2;
	}
	
	// NOTE(Momo): Allocate the maximum buffer possible given allowed latency
	wasapi->buffer_size = wasapi->latency_sample_count * sizeof(S16);
  wasapi->buffer = ba_push<S16>(&wasapi->allocator, wasapi->buffer_size);
  if (!wasapi->buffer) {
    win_log("[win_wasapi] Failed to allocate memory\n");
    goto cleanup_3;
  }

  // Does the success of this matter?
  // Do we even need to return success for this method??
  _win_wasapi_set_default_device_as_current_device(wasapi);

	return true;
	
	// NOTE(Momo): Cleanup
	cleanup_3: 	
    ba_clear(&wasapi->allocator);
	cleanup_2: 
		IMMDeviceEnumerator_UnregisterEndpointNotificationCallback(wasapi->mm_device_enum, &wasapi->notifs.imm_notifs);
	cleanup_1:
		IMMDeviceEnumerator_Release(wasapi->mm_device_enum);

	return false;
}

static inline void 
_win_wasapi_release_current_device(Win_Wasapi* wasapi) {
	if (wasapi->audio_client) {
		IAudioClient2_Stop(wasapi->audio_client);
		IAudioClient2_Release(wasapi->audio_client);
		wasapi->audio_client = 0;
	}
	
	if (wasapi->audio_render_client) {
		IAudioRenderClient_Release(wasapi->audio_render_client);
		wasapi->audio_render_client = 0;
	}
	wasapi->is_device_ready = false;
}

static void
win_wasapi_free(Win_Wasapi* wasapi) {
  _win_wasapi_release_current_device(wasapi);
	IMMDeviceEnumerator_UnregisterEndpointNotificationCallback(wasapi->mm_device_enum, &wasapi->notifs.imm_notifs);
	IMMDeviceEnumerator_Release(wasapi->mm_device_enum);
  ba_clear(&wasapi->allocator);
}

static void 
win_wasapi_begin_frame(Win_Wasapi* wasapi) {
	if (wasapi->is_device_changed) {
		win_log("[win_wasapi] Resetting wasapi device\n");
		// Attempt to change device
		_win_wasapi_release_current_device(wasapi);
		_win_wasapi_set_default_device_as_current_device(wasapi);
		wasapi->is_device_changed = false;
	}
	
  UINT32 sound_padding_size;
  UINT32 samples_to_write = 0;
    
	if (wasapi->is_device_ready) {
		// Padding is how much valid data is queued up in the sound buffer
		// if there's enough padding then we could skip writing more data
		HRESULT hr = IAudioClient2_GetCurrentPadding(wasapi->audio_client, &sound_padding_size);
		
		if (SUCCEEDED(hr)) {
			samples_to_write = (UINT32)wasapi->buffer_size - sound_padding_size;
			
			// Cap the samples to write to how much latency is allowed.
			if (samples_to_write > wasapi->latency_sample_count) {
				samples_to_write = wasapi->latency_sample_count;
			}
		}
	}
	else {
		// NOTE(Momo): if there is no device avaliable,
		// just write to the whole 'dummy' buffer.
		samples_to_write = wasapi->buffer_size;
	}

  // Get Platform_Audio
  wasapi->platform_audio.sample_buffer = wasapi->buffer;
  wasapi->platform_audio.sample_count = samples_to_write; 
  wasapi->platform_audio.channels = wasapi->channels;

}
static void
win_wasapi_end_frame(Win_Wasapi* wasapi) 
{
	if (!wasapi->is_device_ready) return;
  Platform_Audio* output = &wasapi->platform_audio;

  // NOTE(Momo): Kinda assumes 16-bit Sound
  BYTE* sound_buffer_data;
  HRESULT hr = IAudioRenderClient_GetBuffer(wasapi->audio_render_client, 
                                            (UINT32)output->sample_count, 
                                            &sound_buffer_data);
  if (FAILED(hr)) return;

  S16* src_sample = output->sample_buffer;
  S16* dest_sample = (S16*)sound_buffer_data;
  // buffer structure for stereo:
  // S16   S16    S16  S16   S16  S16
  // [LEFT RIGHT] LEFT RIGHT LEFT RIGHT....
  for(U32 sample_index = 0; sample_index < output->sample_count; ++sample_index){
    for (U32 channel_index = 0; channel_index < wasapi->channels; ++channel_index) {
      *dest_sample++ = *src_sample++;
    }
  }
  IAudioRenderClient_ReleaseBuffer(wasapi->audio_render_client, (UINT32)output->sample_count, 0);
}

#endif 