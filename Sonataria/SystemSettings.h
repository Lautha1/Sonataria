using namespace std;

class SystemSettings {
	
	public:
		enum class Setting {
			WIN_AUDIO
		};
		SystemSettings();
		~SystemSettings();
		void initSettings();
		void saveSettingsToFile();
		void updateSetting(Setting, float);
		void setAllSettings();

	private:
		float windowsAudioLevel;
};

extern SystemSettings systemSettings;