#pragma once
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

class WindowsAudio {

	private:
		

	public:
		enum class VolumeUnit {
			Decibal,
			Scalar
		};
		WindowsAudio();
		~WindowsAudio();
		float GetSystemVolume(VolumeUnit vUnit);
		void SetSystemVolume(double newVolume, VolumeUnit vUnit);
	
};

extern WindowsAudio windowsAudio;
