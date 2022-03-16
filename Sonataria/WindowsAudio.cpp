#include "WindowsAudio.h"
#include "SystemSettings.h"

WindowsAudio windowsAudio;

/**
 * Default constructor.
 * 
 */
WindowsAudio::WindowsAudio() {

}

/**
 * Default deconstructor.
 * 
 */
WindowsAudio::~WindowsAudio() {
	
}

/**
 * Get the current system volume.
 * 
 * @param vUnit the type of Unit to get the volume in
 * @return the float of the volume in the unit specified
 */
float WindowsAudio::GetSystemVolume(VolumeUnit vUnit) {
	HRESULT hr;

	CoInitialize(NULL);
	IMMDeviceEnumerator* deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);
	IMMDevice* defaultDevice = NULL;

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
	deviceEnumerator->Release();
	deviceEnumerator = NULL;

	IAudioEndpointVolume* endpointVolume = NULL;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
	defaultDevice->Release();
	defaultDevice = NULL;

	float currentVolume = 0;
	if (vUnit == VolumeUnit::Decibal) {
		// Current Volume in dB
		hr = endpointVolume->GetMasterVolumeLevel(&currentVolume);
	} 
	else if (vUnit == VolumeUnit::Scalar) {
		// Current Volume as a scalar
		hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
	}
	endpointVolume->Release();
	CoUninitialize();

	return currentVolume;
}

/**
 * Set the system volume.
 * 
 * @param newVolume the new volume to set
 * @param vUnit the units of the new volume being set
 */
void WindowsAudio::SetSystemVolume(double newVolume, VolumeUnit vUnit) {
	HRESULT hr;

	CoInitialize(NULL);
	IMMDeviceEnumerator* deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);
	IMMDevice* defaultDevice = NULL;

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
	deviceEnumerator->Release();
	deviceEnumerator = NULL;

	IAudioEndpointVolume* endpointVolume = NULL;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
	defaultDevice->Release();
	defaultDevice = NULL;

	if (vUnit == VolumeUnit::Decibal) {
		hr = endpointVolume->SetMasterVolumeLevel((float)newVolume, NULL);
	}
	else if (vUnit == VolumeUnit::Scalar) {
		hr = endpointVolume->SetMasterVolumeLevelScalar((float)newVolume, NULL);
	}
	endpointVolume->Release();

	CoUninitialize();

	// Save the value in settings
	systemSettings.updateSetting(SystemSettings::Setting::WIN_AUDIO, (float)newVolume);
}