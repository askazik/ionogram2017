// ===========================================================================
// ������������ ���� ��� ������ � ����������������� �������.
// ===========================================================================
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>

#include "config.h"
#include "tinyxml2.h"

#define IONOGRAM_CONFIG_DEFAULT_FILE_NAME "ionogram2017.conf"
#define AMPLITUDES_CONFIG_DEFAULT_FILE_NAME "amplitudes2017.conf"
#define XML_CONFIG_DEFAULT_FILE_NAME "config.xml"

namespace parus {

	// ===========================================================================
	// ���������������� ����
	// ===========================================================================
	
	// ����� ��������� ��� ���������������� ����������.
	struct ionosounder { 
		unsigned ver; // ����� ������ ��� ���������� ����� �����������
		unsigned height_step; // ��� �� ������
		unsigned height_count; // ���������� �����
		unsigned pulse_count; // ��������� ������������ �� ������ �������
		unsigned attenuation; // ���������� (����������) 1/0 = ���/����
		unsigned gain;	// ��������, �� (g = value/6, 6�� = ���������� � 4 ���� �� ��������)
		unsigned pulse_frq; // ������� ����������� ���������, ��
		unsigned pulse_duration; // ������������ ����������� ���������, ���
		unsigned switch_frequency; // ������� ������������ ������, ��� (�� ������������)
		unsigned modules_count; // ����� �������
	};

	// ����� ���� ����������������� �����
	class config {

	protected:
		ionosounder _device;
		static const std::string _whitespaces;
		std::string _fullFileName;
		std::ifstream _fin;

		unsigned getValueFromString(std::string line);
		void readDeviceConfig(void);

	public:
		config(std::string fullName);
		~config(void);

		static bool isTagConfig(std::string Tag, std::string fullName);
		static bool isIonogramConfig(std::string fullName = std::string(IONOGRAM_CONFIG_DEFAULT_FILE_NAME));
		static bool isAmplitudesConfig(std::string fullName = std::string(AMPLITUDES_CONFIG_DEFAULT_FILE_NAME));

		std::string getFileName(void){return _fullFileName;}

		std::string getTag(void){return _device.tag;}
		unsigned getVersion(void){return _device.ver;}
		unsigned getHeightStep(void){return _device.height_step;}
		void setHeightStep(double value){_device.height_step = static_cast<unsigned>(value);}
		unsigned getHeightCount(void){return _device.height_count;}
		unsigned getPulseCount(void){return _device.pulse_count;}
		unsigned getAttenuation(void){return _device.attenuation;}
		unsigned getGain(void){return _device.gain;}
		unsigned getPulseFrq(void){return _device.pulse_frq;}
		unsigned getPulseDuration(void){return _device.pulse_duration;}
	};

	// ����� ���� ����������������� xml-�����
	class xmlconfig {

	protected:
		ionosounder _device;
		std::string _fullFileName;
		std::ifstream _fin;

		unsigned getValueFromString(std::string line);
		void readDeviceConfig(void);

	public:
		xmlconfig(std::string fullName);
		~xmlconfig(void);

		std::string getFileName(void){return _fullFileName;}

		unsigned getVersion(void){return _device.ver;}
		unsigned getHeightStep(void){return _device.height_step;}
		void setHeightStep(double value){_device.height_step = static_cast<unsigned>(value);}
		unsigned getHeightCount(void){return _device.height_count;}
		unsigned getPulseCount(void){return _device.pulse_count;}
		unsigned getAttenuation(void){return _device.attenuation;}
		unsigned getGain(void){return _device.gain;}
		unsigned getPulseFrq(void){return _device.pulse_frq;}
		unsigned getPulseDuration(void){return _device.pulse_duration;}
	};

	// ===========================================================================
	// ����������
	// ===========================================================================
	
	struct ionogramSettings {  
		unsigned fstep;  // ��� �� ������� ����������, ���
		unsigned fbeg;   // ��������� ������� ������, ���
		unsigned fend;   // �������� ������� ������, ���	
	};

	// ����� ������� � ����������������� ����� ������������.
	class confIonogram : public config {
    
	protected:
		ionogramSettings _ionogram; // ��������� ����������

	public:
		confIonogram(void);
		confIonogram(std::string fullName);
		~confIonogram(void);

		void readIonogramConf(void);
		unsigned getFreq_step(void){return _ionogram.fstep;}
		unsigned getFreq_min(void){return _ionogram.fbeg;}
		unsigned getFreq_max(void){return _ionogram.fend;}
		unsigned getFreq_count(void){return (_ionogram.fend - _ionogram.fbeg) / _ionogram.fstep + 1;}
	};

	// ===========================================================================
	// ���������
	// ===========================================================================
	struct myModule {  
		unsigned frq;   // ������� ������, ���
	};

	// ����� ������� � ����������������� ����� ������������.
	class parusConfig  : public config {
    
		unsigned _ver; // ����� ������
		unsigned _height_step; // ��� �� ������, �
		unsigned _height_count; // ���������� �����
		unsigned _modules_count; // ���������� �������/������ ������������
		unsigned _pulse_count; // ��������� ������������ �� ������ �������
		unsigned _attenuation; // ���������� (����������) 1/0 = ���/����
		unsigned _gain;	// �������� (g = value/6, 6�� = ���������� � 4 ���� �� ��������)
		unsigned _pulse_frq; // ������� ����������� ���������, ��
		unsigned _pulse_duration; // ������������ ����������� ���������, ���
		unsigned _height_min; // ��������� ������, �� (��, ��� ���� ��� ��������� �������������)
		unsigned _height_max; // �������� ������, �� (��, ��� ���� ��� ��������� �������������)

		myModule	*_ptModules; // ��������� �� ������ �������

		void getModulesCount(std::ifstream &fin);
		void getModules(std::ifstream &fin);
		myModule getCurrentModule(std::ifstream &fin);
	public:
		parusConfig(std::string fullName);
		~parusConfig(void);

		void loadConf(std::string fullName);
		int getModulesCount(void){return _modules_count;}
		unsigned getHeightStep(void){return _height_step;}
		void setHeightStep(double value){_height_step = static_cast<unsigned>(value);}
		unsigned getHeightCount(void){return _height_count;}
		unsigned getVersion(void){return _ver;}
		unsigned getFreq(int num){return _ptModules[num].frq;}

		unsigned getAttenuation(void){return _attenuation;} // ���������� (����������) 1/0 = ���/����
		unsigned getGain(void){return _gain;}	// �������� (g = value/6, 6�� = ���������� � 4 ���� �� ��������)
		unsigned getPulse_frq(void){return _pulse_frq;} // ������� ����������� ���������, ��
		unsigned getPulse_duration(void){return _pulse_duration;} // ������������ ����������� ���������, ���

		unsigned getHeightMin(void){return _height_min;} // ��������� ������, �� (��, ��� ���� ��� ��������� �������������)
		unsigned getHeightMax(void){return _height_max;} // �������� ������, �� (��, ��� ���� ��� ��������� �������������)
	};

}; // end namespace parus

#endif // __CONFIG_H__