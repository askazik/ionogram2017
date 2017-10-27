// ===========================================================================
// Запись ионограммы в файл.
// ===========================================================================
#include "ionogram.h"

using namespace parus;

int main(void)
{	
	setlocale(LC_ALL,"Russian"); // настройка локали на вывод сообщений по-русски
	SetPriorityClass();

    // ===========================================================================================
    // 1. Читаем файл конфигурации.
    // ===========================================================================================
	confIonogram* conf = nullptr;

	if(config::isIonogramConfig())
	{
		conf = new confIonogram();
		std::cout << "Используем конфигурационный файл: <" << conf->getFileName() << ">." << std::endl;

		std::cout << "Параметры зондирования: " << std::endl;
		std::cout << "Частоты: " << conf->getFreq_min() << " кГц - " << conf->getFreq_max() 
				<< " кГц, усиление = " << conf->getGain() 
				<< " дБ, аттенюатор = " << conf->getAttenuation() << " выкл(0)/вкл(1)." << std::endl;
	}

    // ===========================================================================================
    // 2. Конфигурирование сеанса.
    // ===========================================================================================
	int RetStatus = 0;
	try	
	{
		// Подготовка аппаратуры к зондированию.
		// Открытие выходнго файла данных и запись заголовка.
		parusWork *work = new parusWork(conf);

		DWORD msTimeout = 4;
		unsigned short curFrq = conf->getFreq_min(); // текущая частота зондирования, кГц
		int counter = conf->getFreq_count() * conf->getPulseCount(); // число импульсов от генератора

		work->startGenerator(counter+1); // Запуск генератора импульсов.
		while(counter) // обрабатываем импульсы генератора
		{
			work->adjustSounding(curFrq);

			// Инициализация массива суммирования нулями.
			work->cleanLineAccumulator();
			for (unsigned k = 0; k < conf->getPulseCount(); k++) // счётчик циклов суммирования на одной частоте
			{
				work->ASYNC_TRANSFER(); // запустим АЦП
				
				// Цикл проверки до появления результатов в буфере.
				// READ_BUFISCOMPLETE - сбоит на частоте 47 Гц
				while(work->READ_ISCOMPLETE(msTimeout) == NULL);

				// Остановим АЦП
				work->READ_ABORTIO();					

				work->accumulateLine();
				counter--; // приступаем к обработке следующего импульса
			}
			// усредним по количеству импульсов зондирования на одной частоте
			work->averageLine(conf->getPulseCount()); 
			// Усечение данных до char (сдвиг на 6 бит) и сохранение линии в файле.
			work->saveLine(curFrq);

			curFrq += conf->getFreq_step(); // следующая частота зондирования
		}
		delete work;
	}
	catch(std::exception &e)
	{
		std::cerr << std::endl;
		std::cerr << "Сообщение: " << e.what() << std::endl;
		std::cerr << "Тип      : " << typeid(e).name() << std::endl;
		RetStatus = -1;
	}
	Beep( 1500, 300 );

	// Прибираем конфигурационный файл
	if(conf != nullptr)	delete conf;

	return RetStatus;
}

//	// Выделение памяти для записи отсчётов АЦП и их сортировки.
//    // Количество отсчетов = размер буфера FIFO АЦП в 32-разрядных словах.
//	try
//	{
//		unsigned count = conf.getHeightCount(); 
//		std::string stmp = std::to_string(static_cast<unsigned long long>(count));
//		if(count < 64 && count >= 4096) // 4096 - сбоит!!! 16К = 4096*4б
//			throw std::out_of_range("Размер буфера АЦП должен быть больше 64 и меньше 4096. <" + stmp + ">");
//		else
//			if(count & (count - 1))
//				throw std::out_of_range("Размер буфера АЦП должен быть степенью 2. <" + stmp + ">");
//
//		// Открываем устройство, используя глобальные переменные, установленные заранее.
//		M214x3M_DRVPARS DrvPars = initADC(count);
//		int daq = DAQ_open(DriverName.c_str(), &DrvPars); // NULL
//		if(!daq){
//			std::cerr << DAQ_GetErrorMessage(daq, RetStatus) << std::endl;
//			throw std::runtime_error("Не могу открыть модуль сбора данных.");
//		}
//
//		// Настройка построчного зондирования
//		unsigned long	buf_size, *dbuf;
//		buf_size = static_cast<unsigned long>(count * sizeof(unsigned long)); // размер буфера строки в байтах
//		dbuf =  new unsigned long [count];	// два сырых 16-битных чередующихся канала на одной частоте (count - количество 32-разрядных слов)
//		unsigned int *dataSum;
//		dataSum = new unsigned int [count]; // массив просуммированных данных
//			
//		int ReqDataSize = sizeof(DAQ_ASYNCREQDATA);
//        DAQ_ASYNCREQDATA *ReqData = (DAQ_ASYNCREQDATA *)new char[ReqDataSize];
//
//		ReqData->BufCnt = 1;
//		TBLENTRY *pTbl = &ReqData->Tbl[0];
//		pTbl[0].Addr = NULL;
//		pTbl[0].Size = buf_size;
//
//		// Request buffers of memory
//		RetStatus = DAQ_ioctl(daq, DAQ_ioctlREAD_MEM_REQUEST, ReqData); 
//		if(RetStatus != ERROR_SUCCESS)
//			throw(DAQ_GetErrorMessage(daq, RetStatus));
//
//		// Задаём частоту дискретизации АЦП, Гц.
//		double Frq = C/(2.*conf.getHeightStep());
//		// Внести пожелания о частоте дискретизации АЦП.
//		DAQ_ioctl(daq, DAQ_ioctlSETRATE, &Frq);
//		// Вернуть частоту дискретизации, реально установленную для АЦП.
//		DAQ_ioctl(daq, DAQ_ioctlGETRATE, &Frq);
//		// Пересчет шага по высоте, соответствующего реальной частоте дискретизации.
//		conf.setHeightStep(C/(2.*Frq)); // реальный шаг по высоте, м
//
//		// ===========================================================================================
//		// 3. Перебор по модулям зондирования.
//		// ===========================================================================================
//		moduleIonogram curModule;
//		unsigned f, f1, f2, df, ks, att, g;
//
//		// Выходной файл ионограммы.
//		HANDLE hFile = openIonogramFile(conf);
//
//		for(unsigned i=0; i<conf.getModulesCount(); i++) // перебор по модулям зондирования
//		{
//			// Получаем параметры зондирования текущего модуля.
//			curModule = conf.getModule(i);
//			// Определение параметров зондирования	
//			f1 = curModule.fbeg;
//			f2 = curModule.fend;
//			df = curModule.fstep;
//			ks = (f2-f1)/df + 1;	// Число строк ионограммы
//			att = curModule.attenuation;
//			g = curModule.gain/6;	// ??? 6дБ = приращение в 4 раза по мощности
//			if (g > 7) g = 7;
//
//			// Смена частоты синтезатора
//			f = f1;
//			startReciver(f, g, att);
//
//			// Begin read
//			DAQ_ASYNCXFERDATA xferData;
//			xferData.UseIrq = 1;	// 1 - использовать прерывания
//			xferData.Dir = 1;		// Направление обмена (1 - ввод, 2 - вывод данных)
//			// Для непрерывной передачи данных по замкнутому циклу необходимо установить автоинициализацию обмена.
//			xferData.AutoInit = 0;	// 1 - автоинициализация обмена
//
//			// Запуск АЦП в асинхронном режиме перед запуском излучения - нулевой уровень на ионограмме.
//			DAQ_ASYNCXFERSTATE xferState;
//			DAQ_ioctl(daq, DAQ_ioctlASYNC_TRANSFER, &xferData);
//
//			// Запуск синхроимпульсов для зондирования (+5 на случай потери зондирующих импульсов)
//			startPIC(ks, curModule.pulse_frq); // количество строк ионограммы; частота следования строк, Гц	
//
//			int buf_count = 0;
//			for (unsigned j = 0; j < ks; j++) // счётчик циклов перебора по частоте
//			{ 				
//				// Инициализация массива суммирования нулями.
//				// dataSum
//				for (unsigned k = 0; k < curModule.pulse_count; k++) // счётчик циклов суммирования на одной частоте
//				{
//					if (DAQ_ioctl(daq, DAQ_ioctlREAD_BUFISCOMPLETE, &msTimeout)) // Проверка наличия результатов
//					{ 
//						// Остановим АЦП
//						DAQ_ioctl(daq, DAQ_ioctlREAD_ABORTIO, &xferState);					
//						// Сохраним данные
//						memcpy(dbuf, pTbl[0].Addr, pTbl[0].Size);
//						// Преобразование "сырой" квадратурной строки в строку значений амплитуд без усечения данных.
//						unsigned int *dataLine = loadIonogramLine(dbuf, count);
//						// Суммирование последовательных линий ионограммы.
//						sumIonogramLine(dataSum, dataLine, count);						
//						// Чистка
//						delete [] dataLine;
//
//						buf_count++;
//					} 					
//				} // перебор по циклу суммирования
//					
//				// Усечение данных до char (сдвиг на 6 бит).
//				unsigned char* out = shiftLine6bits(dataSum, count);
//				// Сохранение линии в файле.
//				saveLine(out, count, conf.getHeightStep(), curModule, f, hFile);
//				delete [] out;
//
//				if(j < ks) 
//				{
//					// Смена частоты синтезатора
//					f += df;
//					startReciver(f, g, att);
//					// Запустим АЦП
//					DAQ_ioctl(daq, DAQ_ioctlASYNC_TRANSFER, &xferData);
//				}
//			} // цикл перебора по частотам
//			
//			std::cout << "Module № " << i << std::endl;
//			std::cout << "ks = " << ks << ", buf_count = " << buf_count << std::endl;
//			Sleep(100);
//		} // цикл перебора по модулям зондирования
//
//		delete ReqData;
//		delete [] dbuf;
//		delete [] dataSum;
//
//		// Free buffers of memory
//		DAQ_ioctl(daq, DAQ_ioctlREAD_MEM_RELEASE, NULL);
//		CloseHandle(hFile);
//		RetStatus = DAQ_close(daq);	
//	}
//	catch(std::exception &e)
//	{
//		std::cerr << std::endl;
//		std::cerr << "Сообщение: " << e.what() << std::endl;
//		std::cerr << "Тип      : " << typeid(e).name() << std::endl;
//		RetStatus = -1;
//	}
//	Beep( 1500, 300 );
//
//	return RetStatus;
//}

// ====================================================================================
// Внутренние функции
// ====================================================================================

//// Подготовка запуска АЦП.
//M214x3M_DRVPARS initADC(unsigned int count){
//	M214x3M_DRVPARS DrvPars;
//    
//	DrvPars.Pars = M214x3M_Params; // Устанавливаем значения по умолчанию для субмодуля ADM214x1M			
//	DrvPars.Carrier.Pars = AMB_Params; // Устанавливаем значения для базового модуля AMBPCI
//	DrvPars.Carrier.Pars.AdcFifoSize = count; // размер буфера FIFO АЦП в 32-разрядных словах;
//
//	// Включаем по одному входу в обоих каналах.
//	DrvPars.Carrier.Pars.ChanMask = 257; // Если бит равен 1, то соответствующий вход включен, иначе - отключен.
//	DrvPars.Pars.Gain[0].OnOff = 1;
//	DrvPars.Pars.Gain[8].OnOff = 1;
//
//	// Установка регистра режимов стартовой синхронизации АЦП
//	DrvPars.Carrier.Pars.Start.Start = 1; // запуск по сигналу компаратора 0
//	DrvPars.Carrier.Pars.Start.Src = 0;	// на компаратор 0 подается сигнал внешнего старта, на компаратор 1 подается сигнал с разъема X4
//	DrvPars.Carrier.Pars.Start.Cmp0Inv = 1;	// 1 соответствует инверсии сигнала с выхода компаратора 0
//	DrvPars.Carrier.Pars.Start.Cmp1Inv = 0;	// 1 соответствует инверсии сигнала с выхода компаратора 1
//	DrvPars.Carrier.Pars.Start.Pretrig = 0;	// 1 соответствует режиму претриггера
//	DrvPars.Carrier.Pars.Start.Trig.On = 1;	// 1 соответствует триггерному режиму запуска/остановки АЦП
//	DrvPars.Carrier.Pars.Start.Trig.Stop = 0;// Режим остановки АЦП в триггерном режиме запуска: 0 - программная остановка
//	DrvPars.Carrier.Pars.Start.Thr[0] = 1; // Пороговые значения в Вольтах для компараторов 0 и 1
//	DrvPars.Carrier.Pars.Start.Thr[1] = 1;
//
//	DrvPars.Carrier.HalPars = HAL_Params; // Устанавливаем значения по умолчанию для Слоя Аппаратных Абстракций (HAL)
//										  // VendorID, DeviceID, InstanceDevice, LatencyTimer
//
//	strcpy_s(DrvPars.Carrier.PldFileName, PLDFileName.c_str()); // Используем старую версию  PLD файла
//
//    return DrvPars;
//}
//
//// Открываем файл ионограммы для записи и вносим в него заголовок.
//HANDLE openIonogramFile(confIonogram &conf){
//	
//	// Obtain coordinated universal time (!!!! UTC !!!!!):
//    // ==================================================================================================
//    // The value returned generally represents the number of seconds since 00:00 hours, Jan 1, 1970 UTC
//    // (i.e., the current unix timestamp). Although libraries may use a different representation of time:
//    // Portable programs should not use the value returned by this function directly, but always rely on
//    // calls to other elements of the standard library to translate them to portable types (such as
//    // localtime, gmtime or difftime).
//    // ==================================================================================================
//	time_t ltime;
//    time(&ltime);
//	struct tm newtime;
//	
//	gmtime_s(&newtime, &ltime);
//
//    // Заполнение заголовка файла.
//    ionHeaderNew2 header;
//    header.ver = conf.getVersion();
//    header.time_sound = newtime;
//    header.height_min = 0; // начальная высота, м
//    header.height_step = conf.getHeightStep(); // шаг по высоте, м
//    header.count_height = conf.getHeightCount(); // число высот
//	header.freq_min = conf.getFreq_min(); // начальная частота, кГц (первого модуля)
//	header.freq_max = conf.getFreq_max(); // конечная частота, кГц (последнего модуля)   
//	header.count_freq = conf.getCount_freq(); // число частот во всех модулях
//    header.count_modules = conf.getModulesCount(); // количество модулей зондирования
//    header.switch_frequency = conf.getSwitchFrequency();
//
//    // Определение имя файла ионограммы.
//    std::stringstream name;
//    name << std::setfill('0');
//    name << std::setw(4);
//    name << newtime.tm_year+1900 << std::setw(2);
//    name << newtime.tm_mon+1 << std::setw(2) 
//		<< newtime.tm_mday << std::setw(2) 
//		<< newtime.tm_hour << std::setw(2) 
//		<< newtime.tm_min << std::setw(2) << newtime.tm_sec;
//    name << ".ion";
//
//    // Попытаемся открыть файл.
//    HANDLE hFile = NULL;
//	hFile = CreateFile(name.str().c_str(),		// name of the write
//                       GENERIC_WRITE,			// open for writing
//                       0,						// do not share
//                       NULL,					// default security
//                       CREATE_ALWAYS,			// create new file always
//                       FILE_ATTRIBUTE_NORMAL,	// normal file
//                       NULL);					// no attr. template
//    if (hFile == INVALID_HANDLE_VALUE)
//        throw std::runtime_error("Ошибка открытия файла ионограмм на запись.");
//
//    DWORD bytes;              // счётчик записи в файл
//    BOOL Retval;
//    Retval = WriteFile(hFile, // писать в файл
//              &header,        // адрес буфера: что писать
//              sizeof(header), // сколько писать
//              &bytes,         // адрес DWORD'a: на выходе - сколько записано
//              0);             // A pointer to an OVERLAPPED structure.
//    if (!Retval)
//        throw std::runtime_error("Ошибка записи заголовка ионограммы в файл.");    
//    
//	return	hFile;
//}
//
//// Программируем синтезатор приемника
//void startReciver(unsigned int f, unsigned int g, char att){
//// f -  частота, Гц
//// g - усиление
//// att - ослабление (вкл/выкл)
//	HANDLE			LPTPort;
//	char			ag, sb, j, jk, jk1, j1, j2, jk2;
//	int				i;
//	double			step = 2.5;
//	unsigned int	fp = 63000, nf, r = 4;
//	unsigned int	s_gr[9] = {0, 3555, 8675, 13795, 18915, 24035, 29155, 34280, 39395};
//	int				Address=888;
//
//	for ( i = 8; i > 0; )
//		if ( f > s_gr[--i] ){
//			sb = i;
//			break;
//		}
//	nf =(unsigned int)( ((double)f + (double)fp) / step );
//	
//	// Открываем LPT1 порт для записи через драйвер GiveIO.sys
//	LPTPort = CreateFile(TEXT("\\\\.\\giveio"),
//                         GENERIC_READ,
//                         0,
//                         NULL,
//                         OPEN_EXISTING,
//                         FILE_ATTRIBUTE_NORMAL,
//                         NULL);
//	if (LPTPort==INVALID_HANDLE_VALUE){
//        throw std::runtime_error("Error! Can't open driver! Press any key to exit...");
//	}
//
//	// Промежуточная частота
//	ag = att + sb*2 + g*16 + 128;
//    _outp(Address, ag);
//
//	// Запись шага по частоте в синтезатор
//	for ( i = 2; i >= 0;  i-- ) { // Код шага частоты делителя синтезатора
//		j = ( 0x1 & (r>>i) ) * 2 + 4;
//		j1 = j + 1;
//		jk = (j^0X0B)&0x0F;
//		_outp( Address+2, jk );
//		jk1 = (j1^0x0B)&0x0F;
//		_outp(Address+2, jk1);
//		_outp(Address+2, jk1);
//		_outp(Address+2, jk);
//	}
//
//	// Запись кода частоты в синтезатор
//	for ( i = 15; i >= 0; i-- )  { // Код частоты синтезатора
//		j = ( 0x1 & (nf>>i) ) * 2 + 4;
//		j1 = j2 = j + 1;
//		if ( i == 0) 
//			j2 -= 4;
//		jk = (j^0X0B)&0X0F;
//		_outp( Address+2, jk);
//		jk1 = (j1^0X0B)&0X0F;
//		_outp( Address+2, jk1);
//		jk2 = (j2^0X0B)&0X0F;
//		_outp( Address+2, jk2);
//		_outp( Address+2, jk);
//	}
//	
//	// Закрываем порт
//	CloseHandle(LPTPort);
//}
//
//// Программируем PIC на вывод синхроимпульсов сканирования ионограммы
//void startPIC(unsigned int nstr, unsigned int fsync){
//// nstr -  количество строк ионограммы
//// fsound - частота следования строк, Гц
//	HANDLE			COMPort;
//	double			fosc = 5e6;
//	unsigned char	cdat[8] = {103, 159, 205, 218, 144, 1, 0, 0}; // 50 Гц, 398 строк
//	unsigned int	pimp_n, nimp_n, lnumb;
//	unsigned long	NumberOfBytesWritten;
//	
//	lnumb = nstr;			// Число строк с запасом на сбои, передаваемое в PIC-контроллер
//
//	// ==========================================================================================
//	// Магические вычисления Геннадия Ивановича
//	pimp_n = (unsigned int)(fosc/(4.*(double)fsync));		// Циклов PIC на период	
//	nimp_n = 0x1000d - pimp_n;								// PIC_TMR1       Fimp_min = 19 Hz
//	cdat[0] = nimp_n%256;
//	cdat[1] = nimp_n/256;
//	cdat[2] = 0xCD;
//	cdat[3] = 0xDA;
//	cdat[4] = lnumb%256;
//	cdat[5] = lnumb/256;
//	cdat[6] = 0;
//	cdat[7] = 0;
//
//	// Завершение магических вычислений (Частота следования не менее 19 Гц.)
//	// ==========================================================================================
// 
//
//	// ==========================================================================================
//	// Запуск синхроимпульсов
//	// ==========================================================================================
//	// Открываем порт для записи
//	COMPort = CreateFile(TEXT("COM2"), 
//							GENERIC_READ | GENERIC_WRITE, 
//							0, 
//							NULL, 
//							OPEN_EXISTING, 
//							0, 
//							NULL);
//	if (COMPort == INVALID_HANDLE_VALUE) {
//        throw std::runtime_error("Error! Невозможно открыть COM порт!");
//	}
// 
//	// Build on the current configuration, and skip setting the size
//	// of the input and output buffers with SetupComm.
//	DCB dcb;
//	BOOL fSuccess;
//
//	// Проверка
//	fSuccess = GetCommState(COMPort, &dcb);
//	if (!fSuccess){
//		// Handle the error.
//        throw std::runtime_error("Error! GetCommState failed with error!");
//	}
//
//	// Подготовка
//	dcb.BaudRate = CBR_19200;     // set the baud rate
//	dcb.ByteSize = 8;             // data size, xmit, and rcv
//	dcb.Parity = NOPARITY;        // no parity bit
//	dcb.StopBits = TWOSTOPBITS;   // two stop bit
//
//	// Заполнение
//	fSuccess = SetCommState(COMPort, &dcb);
//	if (!fSuccess){
//		// Handle the error.
//        throw std::runtime_error("Error! SetCommState failed with error!");
//	}
//
//	// Проверка
//	fSuccess = GetCommState(COMPort, &dcb);
//	if (!fSuccess){
//		// Handle the error.
//		throw std::runtime_error("Error! GetCommState failed with error!");
//	}
//
//	// Передача параметров в кристалл PIC16F870
//	fSuccess = WriteFile(
//		COMPort,		// описатель сом порта
//		&cdat,		// Указатель на буфер, содержащий данные, которые будут записаны в файл. 
//		8,// Число байтов, которые будут записаны в файл.
//		&NumberOfBytesWritten,//  Указатель на переменную, которая получает число записанных байтов
//		NULL // Указатель на структуру OVERLAPPED
//	);
//    if (!fSuccess){
//		// Handle the error.
//		throw std::runtime_error("Ошибка передачи параметров в кристалл PIC16F870.");
//	}
//
//	// Закрываем порт
//	CloseHandle(COMPort);
//}
//
//// Преобразование "сырой" квадратурной строки в строку значений амплитуд с усечением данных.
//unsigned char* loadLine(unsigned long *data, int n)
//{
//	unsigned char *cdata = NULL;
//
//	// Обработка сырых данных.
//	cdata = new unsigned char [n];
//	int re, im, abstmp;
//    for(int i = 0; i < n; i++)
//		{
//            // Используем двухканальную интерпретацию через анонимную структуру
//            union {
//                unsigned long word;     // 4-байтное слово двухканального АЦП
//                adcTwoChannels twoCh;  // двухканальные (квадратурные) данные
//            };
//            
//			// Разбиение на квадратуры. 
//			// Значимы только старшие 14 бит. Младшие 2 бит - технологическая окраска.
//            word = data[i];
//            re = static_cast<int>(twoCh.re.value) >> 2;
//            im = static_cast<int>(twoCh.im.value) >> 2;
//
//            // Объединим квадратурную информацию в одну амплитуду.
//			abstmp = static_cast<int>(floor(sqrt(re*re + im*im*1.0)));
//
//			// Усечение данных до размера 8 бит (на 2 бита уже сместились ранее).
//			cdata[i] = static_cast<unsigned char>(abstmp >> 6);
//		}            
//    
//    return cdata;
//}
//
//// Преобразование "сырой" квадратурной строки в строку значений амплитуд без усечения данных.
//unsigned int* loadIonogramLine(unsigned long *data, int n)
//{
//	unsigned int *idata = NULL;
//
//	// Обработка сырых данных.
//	idata = new unsigned int [n];
//	int re, im, abstmp;
//    for(int i = 0; i < n; i++)
//		{
//            // Используем двухканальную интерпретацию через анонимную структуру
//            union {
//                unsigned long word;     // 4-байтное слово двухканального АЦП
//                adcTwoChannels twoCh;  // двухканальные (квадратурные) данные
//            };
//            
//			// Разбиение на квадратуры. 
//			// Значимы только старшие 14 бит. Младшие 2 бит - технологическая окраска.
//            word = data[i];
//            re = static_cast<int>(twoCh.re.value) >> 2;
//            im = static_cast<int>(twoCh.im.value) >> 2;
//
//            // Объединим квадратурную информацию в одну амплитуду.
//			abstmp = static_cast<int>(floor(sqrt(re*re + im*im*1.0)));
//		}            
//    
//    return idata;
//}
//
//// Суммирование последовательных линий ионограммы.
//void sumIonogramLine(unsigned int *dataDestination, unsigned int *dataSource, int n)
//{
//    for(int i = 0; i < n; i++)
//		dataDestination[i] = dataDestination[i]/2 + dataSource[i]/2;
//}
//
//// Усечение данных до char (сдвиг на 6 бит).
//unsigned char* shiftLine6bits(unsigned int *data, int n)
//{
//	unsigned char *cdata = NULL;
//
//	// Обработка сырых данных.
//	cdata = new unsigned char [n];
//    for(int i = 0; i < n; i++)
//		// Усечение данных до размера 8 бит (на 2 бита уже сместились ранее).
//		cdata[i] = static_cast<unsigned char>(data[i] >> 6);
//     
//	return cdata;
//}

void SetPriorityClass(void){
	HANDLE procHandle = GetCurrentProcess();
	DWORD priorityClass = GetPriorityClass(procHandle);

	if (!SetPriorityClass(procHandle, HIGH_PRIORITY_CLASS))
		std::cerr << "SetPriorityClass" << std::endl;

	priorityClass = GetPriorityClass(procHandle);
	std::cerr << "Priority Class is set to : ";
	switch(priorityClass)
	{
	case HIGH_PRIORITY_CLASS:
		std::cerr << "HIGH_PRIORITY_CLASS\r\n";
		break;
	case IDLE_PRIORITY_CLASS:
		std::cerr << "IDLE_PRIORITY_CLASS\r\n";
		break;
	case NORMAL_PRIORITY_CLASS:
		std::cerr << "NORMAL_PRIORITY_CLASS\r\n";
		break;
	case REALTIME_PRIORITY_CLASS:
		std::cerr << "REALTIME_PRIORITY_CLASS\r\n";
		break;
	default:
		std::cerr <<"Unknown priority class\r\n";
	}
}