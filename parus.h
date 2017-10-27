// ===========================================================================
// Заголовочный файл для работы с аппаратурой.
// ===========================================================================
#ifndef __PARUS_H__
#define __PARUS_H__

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Помним, что структуры в памяти выравниваются в зависимости
// от компилятора, разрядности и архитектуры системы. 
// Оптимальный выход в использовании машинно независимых форматов
// данных. Например HDF, CDF и.т.п.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iomanip>
#include <cstring>
#include <sstream>
#include <ctime>
#include <stdexcept>
#include <conio.h>

// Для ускорения сохранения в файл используются функции Windows API.
#include <windows.h>

// Заголовочные файлы для работы с модулем АЦП.
// Включение в проект библиотеки импорта daqdrv.dll
#include "daqdef.h"
#pragma comment(lib, "daqdrv.lib") // for Microsoft Visual C++
#include "m14x3mDf.h"

// Заголовочные файлы для работы с данными.
#include "config.h"

// 14 бит - без знака, 13 бит по модулю со знаком для каждой квадратуры = 8191.
// sqrt(8191^2 + 8191^2) = 11583... Берем 11580.
#define __AMPLITUDE_MAX__ 11580 // константа, определяющая максимум амплитуды, выше которого подозреваем ограничение сигнала

namespace parus {

	#pragma pack(push, 1)
	struct adcChannel {
		unsigned short b0: 1;  // Устанавливается в 1 в первом отсчете блока для каждого канала АЦП (т.е. после старта АЦП или после каждого старта в старт-стопном режиме).
		unsigned short b1: 1;  // Устанавливается в 1 в отсчете, который соответствует последнему опрашиваемому входу (по порядку расположения номеров входов в памяти мультиплексора) для каждого канала АЦП.    
		short value : 14; // данные из одного канала АЦП, 14-разрядное слово
	};
	
	struct adcTwoChannels {
		adcChannel re; // условно первая квадратура
		adcChannel im; // условно вторая квадратура
	};

	// Формат до 2015 г.
	struct ionHeaderOld { 	        // === Заголовок файла измерений ===   
		unsigned char st;		// Station  <Ростов = 3>     > 1
		unsigned int f_beg;     // ORIGN FREQUENCY     	     > 2
		unsigned int f_step;    // STEP FREQUENCY            > 4
		unsigned int f_end;     // Final frequency           > 6
		double		 dh;		// Height step, m            > 8
		unsigned int scale;  	// Scale type & lines        >10
								// Format: <S_ppppppppppppp>
								// S=0 -> line  S=1 -> log
		unsigned char   sec;   	// Seconds                   >12
		unsigned char  	min;   	// Minutes                   >13
		unsigned char  	hour; 	// Hours                     >14
		unsigned char  	day;	// Data                      >15
		unsigned char	mon;	// Month                     >16
		unsigned char	year; 	// Year                      >17
		unsigned char 	fr;    	// Rept_freq (Hz)            >18
		unsigned char	an;    	// Ant_type & polarisation flag  >19
								//  & Doppler flag & number of pulses
								// Format: <AAPDnnnn>
		unsigned char	ka;    	// Attenuation               >20
								// Flags of configuration:
		unsigned char	p_len;	// Pulse length (n*50 mks)   >21
		unsigned char	power;	// Transmitter power n kWt   >22
		unsigned char	ch_r;  	//                       >23
		unsigned char	ver;   	// Version of program    >24
		unsigned int 	yday;	// Day of year (0 - 365; January 1 = 0) >25
		unsigned int    wday;	// Day of week (0 - 6; Sunday = 0)      >27
		unsigned int    ks;     //                       >29
		unsigned int    count;  //                       >31
	};

	struct ionHeaderNew1 { 	    // === Заголовок файла ионограмм ===   
		char project_name[16];	// Название проекта (PARUS)
		char format_version[8]; // Версия формата (ION1, ION2,...)
	
		unsigned char   sec;   	// Seconds   
		unsigned char  	min;   	// Minutes 
		unsigned char  	hour; 	// Hours 
		unsigned char  	day;	// Data 
		unsigned char	mon;	// Month
		unsigned char	year; 	// Year
	
		unsigned char 	imp_frq;// частота посылки зондирующего импульса, Гц (20...100)	
		unsigned char	imp_duration;// длительность зондирующих импульсов, мкс
		unsigned char	imp_count;// зондирующих импульсов на каждой частоте
		unsigned char	att;    	// ослабление (аттенюатор)
		unsigned char	power;	// усиление

		unsigned short f_beg;   // начальная частота, кГц
		unsigned short f_step;  // шаг по частоте, кГц
		unsigned short f_end;   // конечная частота, кГц

		unsigned short  count;  // количество отсчётов высоты
		double dh;              // шаг по высоте, км
	};

	struct ionHeaderNew2 { 	    // === Заголовок файла ионограмм ===
		unsigned ver; // номер версии
		struct tm time_sound; // GMT время получения ионограммы
		unsigned height_min; // начальная высота, м
		unsigned height_step; // шаг по высоте, м
		unsigned count_height; // число высот
		unsigned switch_frequency; // частота переключения антенн ионозонда 
		unsigned freq_min; // начальная частота, кГц (первого модуля)
		unsigned freq_max; // конечная частота, кГц (последнего модуля)   
		unsigned count_freq; // число частот во всех модулях
		unsigned count_modules; // количество модулей зондирования

		// Начальная инициализация структуры.
		ionHeaderNew2(void)
		{
			ver = 2;
			height_min = 0; // Это не означает, что зондирование от поверхности. Есть задержки!!!
			height_step = 0;
			count_height = 0;
			switch_frequency = 0;
			freq_min = 0;
			freq_max = 0;    
			count_freq = 0;
			count_modules = 0;
		}
	};

	struct ionPackedData { // Упакованные данные ионограммы.
		unsigned size; // Размер упакованной ионограммы в байтах.
		unsigned char *ptr;   // Указатель на блок данных упакованной ионограммы.
	};

	/* =====================================================================  */
	/* Родные структуры данных ИПГ-ИЗМИРАН */
	/* =====================================================================  */
	/* Каждая строка начинается с заголовка следующей структуры */
	struct FrequencyData {
		unsigned short frequency; //!< Частота зондирования, [кГц].
		unsigned short gain_control; // !< Значение ослабления входного аттенюатора дБ.
		unsigned short pulse_time; //!< Время зондирования на одной частоте, [мс].
		unsigned char pulse_length; //!< Длительность зондирующего импульса, [мкc].
		unsigned char band; //!< Полоса сигнала, [кГц].
		unsigned char type; //!< Вид модуляции (0 - гладкий импульс, 1 - ФКМ).
		unsigned char threshold_o; //!< Порог амплитуды обыкновенной волны, ниже которого отклики не будут записываться в файл, [Дб/ед. АЦП].
		unsigned char threshold_x; //!< Порог амплитуды необыкновенной волны, ниже которого отклики не будут записываться в файл, [Дб/ед. АЦП].
		unsigned char count_o; //!< Число сигналов компоненты O.
		unsigned char count_x; //!< Число сигналов компоненты X.
	};

	/* Сначала следуют FrequencyData::count_o структур SignalResponse, описывающих обыкновенную волну. */
	/* Cразу же после перечисления всех SignalResponse  и SignalSample  для обыкновенных откликов следуют FrequencyData::count_x */
	/* структур SignalResponse, описывающих необыкновенные отклики с массивом структур SignalSample после каждой из них. Величины */
	/* FrequencyData::count_o и FrequencyData::count_x могут быть равны нулю, тогда соответствующие данные отсутствуют. */
	struct SignalResponse {
		unsigned long height_begin; //!< начальная высота, [м]
		unsigned short count_samples; //!< Число дискретов
	};

	/* =====================================================================  */

	struct dataHeader { 	    // === Заголовок файла данных ===
		unsigned ver; // номер версии
		struct tm time_sound; // GMT время получения зондирования
		unsigned height_min; // начальная высота, м (всё, что ниже при обработке отбрасывается)
		unsigned height_max; // конечная высота, м (всё, что выше при обработке отбрасывается)
		unsigned height_step; // шаг по высоте, м (реальный шаг, вычисленный по частоте АЦП)
		unsigned count_height; // число высот (размер исходного буфера АЦП при зондировании, fifo нашего АЦП 4Кб. Т.е. не больше 1024 отсчётов для двух квадратурных каналов)
		unsigned count_modules; // количество модулей/частот зондирования
		unsigned pulse_frq; // частота зондирующих импульсов, Гц
	};
	#pragma pack(pop)

	// Класс работы с аппаратурой Паруса.
	class parusWork {

		HANDLE _hFile; // выходной файл данных

		unsigned int _g;
		char _att;
		unsigned int _fsync;
		unsigned int _pulse_duration;
		unsigned int _curFrq;
    
		unsigned _height_step; // шаг по высоте, м
		unsigned _height_count; // количество высот
		unsigned _height_min; // начальная высота, км (всё, что ниже при обработке отбрасывается)
		unsigned _height_max; // конечная высота, км (всё, что выше при обработке отбрасывается)

		int _RetStatus;
		M214x3M_DRVPARS _DrvPars;
		int _DAQ; // дескриптор устройства (если равен нулю, то значит произошла ошибка)
		DAQ_ASYNCREQDATA *_ReqData; // цепочка буферов АЦП
		DAQ_ASYNCXFERDATA _xferData; // параметры асинхронного режима
		DAQ_ASYNCXFERSTATE _curState;

		unsigned long *_fullBuf; // грязная строка данных
		unsigned long buf_size;

		BYTE *getBuffer(void){return (BYTE*)(&_ReqData->Tbl[0].Addr);}
		DWORD getBufferSize(void){return (DWORD)(&_ReqData->Tbl[0].Size);}

		HANDLE _LPTPort; // для конфигурирования синтезатора
		HANDLE initCOM2(void);
		void initLPT1(void);

		// Работа с файлами выходных данных
		void openIonogramFile(config* conf);
		void openDataFile(config* conf);
		void closeOutputFile(void);

		// Работа с ионограммами
		unsigned int *_sum_abs; // массив абсолютных значений

	public:
		// Глобальные описания
		static const std::string _PLDFileName;
		static const std::string _DriverName;
		static const std::string _DeviceName;
		static const double _C; // скорость света в вакууме

		parusWork(config* conf);
		~parusWork(void);

		M214x3M_DRVPARS initADC(unsigned int nHeights);

		void ASYNC_TRANSFER(void);
		int READ_BUFISCOMPLETE(unsigned long msTimeout);
		void READ_ABORTIO(void);
		void READ_GETIOSTATE(void);
		int READ_ISCOMPLETE(unsigned long msTimeout);

		void adjustSounding(unsigned int curFrq);
		void startGenerator(unsigned int nPulses);

		// Работа с ионограммами
		void cleanLineAccumulator(void);
		void accumulateLine(void); // суммирование по импульсам на одной частоте
		void averageLine(unsigned pulse_count); // усреднение по импульсам на одной частоте
		unsigned char getThereshold(unsigned char *arr, unsigned n);
		void saveLine(unsigned short curFrq); // Усечение данных до char (сдвиг на 6 бит) и сохранение линии в файле.

		// Работа с файлами выходных данных
		void saveFullData(void);
		void saveDataWithGain(void);
	};

	int comp(const void *i, const void *j);

} // namespace parus

#endif // __PARUS_H__