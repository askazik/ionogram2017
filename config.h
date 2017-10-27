// ===========================================================================
// Заголовочный файл для работы с конфигурационными файлами.
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
	// Конфигурационный файл
	// ===========================================================================
	
	// Общие параметры для программирования устройства.
	struct ionosounder { 
		unsigned ver; // номер версии для сохранения файла результатов
		unsigned height_step; // шаг по высоте
		unsigned height_count; // количество высот
		unsigned pulse_count; // импульсов зондирования на каждой частоте
		unsigned attenuation; // ослабление (аттенюатор) 1/0 = вкл/выкл
		unsigned gain;	// усиление, дБ (g = value/6, 6дБ = приращение в 4 раза по мощности)
		unsigned pulse_frq; // частота зондирующих импульсов, Гц
		unsigned pulse_duration; // длительность зондирующих импульсов, мкс
		unsigned switch_frequency; // Частота переключения антенн, кГц (не используется)
		unsigned modules_count; // Число модулей
	};

	// Общий блок конфигурационного файла
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

	// Общий блок конфигурационного xml-файла
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
	// Ионограмма
	// ===========================================================================
	
	struct ionogramSettings {  
		unsigned fstep;  // шаг по частоте ионограммы, кГц
		unsigned fbeg;   // начальная частота модуля, кГц
		unsigned fend;   // конечная частота модуля, кГц	
	};

	// Класс доступа к конфигурационному файлу зондирования.
	class confIonogram : public config {
    
	protected:
		ionogramSettings _ionogram; // параметры ионограммы

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
	// Амплитуды
	// ===========================================================================
	struct myModule {  
		unsigned frq;   // частота модуля, кГц
	};

	// Класс доступа к конфигурационному файлу зондирования.
	class parusConfig  : public config {
    
		unsigned _ver; // номер версии
		unsigned _height_step; // шаг по высоте, м
		unsigned _height_count; // количество высот
		unsigned _modules_count; // количество модулей/частот зондирования
		unsigned _pulse_count; // импульсов зондирования на каждой частоте
		unsigned _attenuation; // ослабление (аттенюатор) 1/0 = вкл/выкл
		unsigned _gain;	// усиление (g = value/6, 6дБ = приращение в 4 раза по мощности)
		unsigned _pulse_frq; // частота зондирующих импульсов, Гц
		unsigned _pulse_duration; // длительность зондирующих импульсов, мкс
		unsigned _height_min; // начальная высота, км (всё, что ниже при обработке отбрасывается)
		unsigned _height_max; // конечная высота, км (всё, что выше при обработке отбрасывается)

		myModule	*_ptModules; // указатель на массив модулей

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

		unsigned getAttenuation(void){return _attenuation;} // ослабление (аттенюатор) 1/0 = вкл/выкл
		unsigned getGain(void){return _gain;}	// усиление (g = value/6, 6дБ = приращение в 4 раза по мощности)
		unsigned getPulse_frq(void){return _pulse_frq;} // частота зондирующих импульсов, Гц
		unsigned getPulse_duration(void){return _pulse_duration;} // длительность зондирующих импульсов, мкс

		unsigned getHeightMin(void){return _height_min;} // начальная высота, км (всё, что ниже при обработке отбрасывается)
		unsigned getHeightMax(void){return _height_max;} // конечная высота, км (всё, что выше при обработке отбрасывается)
	};

}; // end namespace parus

#endif // __CONFIG_H__