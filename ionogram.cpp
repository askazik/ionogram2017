// ===========================================================================
// ������ ���������� � ����.
// ===========================================================================
#include "ionogram.h"

using namespace parus;

int main(void)
{	
	setlocale(LC_ALL,"Russian"); // ��������� ������ �� ����� ��������� ��-������
	SetPriorityClass();

    // ===========================================================================================
    // 1. ������ ���� ������������.
    // ===========================================================================================
	confIonogram* conf = nullptr;

	if(config::isIonogramConfig())
	{
		conf = new confIonogram();
		std::cout << "���������� ���������������� ����: <" << conf->getFileName() << ">." << std::endl;

		std::cout << "��������� ������������: " << std::endl;
		std::cout << "�������: " << conf->getFreq_min() << " ��� - " << conf->getFreq_max() 
				<< " ���, �������� = " << conf->getGain() 
				<< " ��, ���������� = " << conf->getAttenuation() << " ����(0)/���(1)." << std::endl;
	}

    // ===========================================================================================
    // 2. ���������������� ������.
    // ===========================================================================================
	int RetStatus = 0;
	try	
	{
		// ���������� ���������� � ������������.
		// �������� �������� ����� ������ � ������ ���������.
		parusWork *work = new parusWork(conf);

		DWORD msTimeout = 4;
		unsigned short curFrq = conf->getFreq_min(); // ������� ������� ������������, ���
		int counter = conf->getFreq_count() * conf->getPulseCount(); // ����� ��������� �� ����������

		work->startGenerator(counter+1); // ������ ���������� ���������.
		while(counter) // ������������ �������� ����������
		{
			work->adjustSounding(curFrq);

			// ������������� ������� ������������ ������.
			work->cleanLineAccumulator();
			for (unsigned k = 0; k < conf->getPulseCount(); k++) // ������� ������ ������������ �� ����� �������
			{
				work->ASYNC_TRANSFER(); // �������� ���
				
				// ���� �������� �� ��������� ����������� � ������.
				// READ_BUFISCOMPLETE - ����� �� ������� 47 ��
				while(work->READ_ISCOMPLETE(msTimeout) == NULL);

				// ��������� ���
				work->READ_ABORTIO();					

				work->accumulateLine();
				counter--; // ���������� � ��������� ���������� ��������
			}
			// �������� �� ���������� ��������� ������������ �� ����� �������
			work->averageLine(conf->getPulseCount()); 
			// �������� ������ �� char (����� �� 6 ���) � ���������� ����� � �����.
			work->saveLine(curFrq);

			curFrq += conf->getFreq_step(); // ��������� ������� ������������
		}
		delete work;
	}
	catch(std::exception &e)
	{
		std::cerr << std::endl;
		std::cerr << "���������: " << e.what() << std::endl;
		std::cerr << "���      : " << typeid(e).name() << std::endl;
		RetStatus = -1;
	}
	Beep( 1500, 300 );

	// ��������� ���������������� ����
	if(conf != nullptr)	delete conf;

	return RetStatus;
}

//	// ��������� ������ ��� ������ �������� ��� � �� ����������.
//    // ���������� �������� = ������ ������ FIFO ��� � 32-��������� ������.
//	try
//	{
//		unsigned count = conf.getHeightCount(); 
//		std::string stmp = std::to_string(static_cast<unsigned long long>(count));
//		if(count < 64 && count >= 4096) // 4096 - �����!!! 16� = 4096*4�
//			throw std::out_of_range("������ ������ ��� ������ ���� ������ 64 � ������ 4096. <" + stmp + ">");
//		else
//			if(count & (count - 1))
//				throw std::out_of_range("������ ������ ��� ������ ���� �������� 2. <" + stmp + ">");
//
//		// ��������� ����������, ��������� ���������� ����������, ������������� �������.
//		M214x3M_DRVPARS DrvPars = initADC(count);
//		int daq = DAQ_open(DriverName.c_str(), &DrvPars); // NULL
//		if(!daq){
//			std::cerr << DAQ_GetErrorMessage(daq, RetStatus) << std::endl;
//			throw std::runtime_error("�� ���� ������� ������ ����� ������.");
//		}
//
//		// ��������� ����������� ������������
//		unsigned long	buf_size, *dbuf;
//		buf_size = static_cast<unsigned long>(count * sizeof(unsigned long)); // ������ ������ ������ � ������
//		dbuf =  new unsigned long [count];	// ��� ����� 16-������ ������������ ������ �� ����� ������� (count - ���������� 32-��������� ����)
//		unsigned int *dataSum;
//		dataSum = new unsigned int [count]; // ������ ���������������� ������
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
//		// ����� ������� ������������� ���, ��.
//		double Frq = C/(2.*conf.getHeightStep());
//		// ������ ��������� � ������� ������������� ���.
//		DAQ_ioctl(daq, DAQ_ioctlSETRATE, &Frq);
//		// ������� ������� �������������, ������� ������������� ��� ���.
//		DAQ_ioctl(daq, DAQ_ioctlGETRATE, &Frq);
//		// �������� ���� �� ������, ���������������� �������� ������� �������������.
//		conf.setHeightStep(C/(2.*Frq)); // �������� ��� �� ������, �
//
//		// ===========================================================================================
//		// 3. ������� �� ������� ������������.
//		// ===========================================================================================
//		moduleIonogram curModule;
//		unsigned f, f1, f2, df, ks, att, g;
//
//		// �������� ���� ����������.
//		HANDLE hFile = openIonogramFile(conf);
//
//		for(unsigned i=0; i<conf.getModulesCount(); i++) // ������� �� ������� ������������
//		{
//			// �������� ��������� ������������ �������� ������.
//			curModule = conf.getModule(i);
//			// ����������� ���������� ������������	
//			f1 = curModule.fbeg;
//			f2 = curModule.fend;
//			df = curModule.fstep;
//			ks = (f2-f1)/df + 1;	// ����� ����� ����������
//			att = curModule.attenuation;
//			g = curModule.gain/6;	// ??? 6�� = ���������� � 4 ���� �� ��������
//			if (g > 7) g = 7;
//
//			// ����� ������� �����������
//			f = f1;
//			startReciver(f, g, att);
//
//			// Begin read
//			DAQ_ASYNCXFERDATA xferData;
//			xferData.UseIrq = 1;	// 1 - ������������ ����������
//			xferData.Dir = 1;		// ����������� ������ (1 - ����, 2 - ����� ������)
//			// ��� ����������� �������� ������ �� ���������� ����� ���������� ���������� ����������������� ������.
//			xferData.AutoInit = 0;	// 1 - ����������������� ������
//
//			// ������ ��� � ����������� ������ ����� �������� ��������� - ������� ������� �� ����������.
//			DAQ_ASYNCXFERSTATE xferState;
//			DAQ_ioctl(daq, DAQ_ioctlASYNC_TRANSFER, &xferData);
//
//			// ������ ��������������� ��� ������������ (+5 �� ������ ������ ����������� ���������)
//			startPIC(ks, curModule.pulse_frq); // ���������� ����� ����������; ������� ���������� �����, ��	
//
//			int buf_count = 0;
//			for (unsigned j = 0; j < ks; j++) // ������� ������ �������� �� �������
//			{ 				
//				// ������������� ������� ������������ ������.
//				// dataSum
//				for (unsigned k = 0; k < curModule.pulse_count; k++) // ������� ������ ������������ �� ����� �������
//				{
//					if (DAQ_ioctl(daq, DAQ_ioctlREAD_BUFISCOMPLETE, &msTimeout)) // �������� ������� �����������
//					{ 
//						// ��������� ���
//						DAQ_ioctl(daq, DAQ_ioctlREAD_ABORTIO, &xferState);					
//						// �������� ������
//						memcpy(dbuf, pTbl[0].Addr, pTbl[0].Size);
//						// �������������� "�����" ������������ ������ � ������ �������� �������� ��� �������� ������.
//						unsigned int *dataLine = loadIonogramLine(dbuf, count);
//						// ������������ ���������������� ����� ����������.
//						sumIonogramLine(dataSum, dataLine, count);						
//						// ������
//						delete [] dataLine;
//
//						buf_count++;
//					} 					
//				} // ������� �� ����� ������������
//					
//				// �������� ������ �� char (����� �� 6 ���).
//				unsigned char* out = shiftLine6bits(dataSum, count);
//				// ���������� ����� � �����.
//				saveLine(out, count, conf.getHeightStep(), curModule, f, hFile);
//				delete [] out;
//
//				if(j < ks) 
//				{
//					// ����� ������� �����������
//					f += df;
//					startReciver(f, g, att);
//					// �������� ���
//					DAQ_ioctl(daq, DAQ_ioctlASYNC_TRANSFER, &xferData);
//				}
//			} // ���� �������� �� ��������
//			
//			std::cout << "Module � " << i << std::endl;
//			std::cout << "ks = " << ks << ", buf_count = " << buf_count << std::endl;
//			Sleep(100);
//		} // ���� �������� �� ������� ������������
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
//		std::cerr << "���������: " << e.what() << std::endl;
//		std::cerr << "���      : " << typeid(e).name() << std::endl;
//		RetStatus = -1;
//	}
//	Beep( 1500, 300 );
//
//	return RetStatus;
//}

// ====================================================================================
// ���������� �������
// ====================================================================================

//// ���������� ������� ���.
//M214x3M_DRVPARS initADC(unsigned int count){
//	M214x3M_DRVPARS DrvPars;
//    
//	DrvPars.Pars = M214x3M_Params; // ������������� �������� �� ��������� ��� ��������� ADM214x1M			
//	DrvPars.Carrier.Pars = AMB_Params; // ������������� �������� ��� �������� ������ AMBPCI
//	DrvPars.Carrier.Pars.AdcFifoSize = count; // ������ ������ FIFO ��� � 32-��������� ������;
//
//	// �������� �� ������ ����� � ����� �������.
//	DrvPars.Carrier.Pars.ChanMask = 257; // ���� ��� ����� 1, �� ��������������� ���� �������, ����� - ��������.
//	DrvPars.Pars.Gain[0].OnOff = 1;
//	DrvPars.Pars.Gain[8].OnOff = 1;
//
//	// ��������� �������� ������� ��������� ������������� ���
//	DrvPars.Carrier.Pars.Start.Start = 1; // ������ �� ������� ����������� 0
//	DrvPars.Carrier.Pars.Start.Src = 0;	// �� ���������� 0 �������� ������ �������� ������, �� ���������� 1 �������� ������ � ������� X4
//	DrvPars.Carrier.Pars.Start.Cmp0Inv = 1;	// 1 ������������� �������� ������� � ������ ����������� 0
//	DrvPars.Carrier.Pars.Start.Cmp1Inv = 0;	// 1 ������������� �������� ������� � ������ ����������� 1
//	DrvPars.Carrier.Pars.Start.Pretrig = 0;	// 1 ������������� ������ �����������
//	DrvPars.Carrier.Pars.Start.Trig.On = 1;	// 1 ������������� ����������� ������ �������/��������� ���
//	DrvPars.Carrier.Pars.Start.Trig.Stop = 0;// ����� ��������� ��� � ���������� ������ �������: 0 - ����������� ���������
//	DrvPars.Carrier.Pars.Start.Thr[0] = 1; // ��������� �������� � ������� ��� ������������ 0 � 1
//	DrvPars.Carrier.Pars.Start.Thr[1] = 1;
//
//	DrvPars.Carrier.HalPars = HAL_Params; // ������������� �������� �� ��������� ��� ���� ���������� ���������� (HAL)
//										  // VendorID, DeviceID, InstanceDevice, LatencyTimer
//
//	strcpy_s(DrvPars.Carrier.PldFileName, PLDFileName.c_str()); // ���������� ������ ������  PLD �����
//
//    return DrvPars;
//}
//
//// ��������� ���� ���������� ��� ������ � ������ � ���� ���������.
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
//    // ���������� ��������� �����.
//    ionHeaderNew2 header;
//    header.ver = conf.getVersion();
//    header.time_sound = newtime;
//    header.height_min = 0; // ��������� ������, �
//    header.height_step = conf.getHeightStep(); // ��� �� ������, �
//    header.count_height = conf.getHeightCount(); // ����� �����
//	header.freq_min = conf.getFreq_min(); // ��������� �������, ��� (������� ������)
//	header.freq_max = conf.getFreq_max(); // �������� �������, ��� (���������� ������)   
//	header.count_freq = conf.getCount_freq(); // ����� ������ �� ���� �������
//    header.count_modules = conf.getModulesCount(); // ���������� ������� ������������
//    header.switch_frequency = conf.getSwitchFrequency();
//
//    // ����������� ��� ����� ����������.
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
//    // ���������� ������� ����.
//    HANDLE hFile = NULL;
//	hFile = CreateFile(name.str().c_str(),		// name of the write
//                       GENERIC_WRITE,			// open for writing
//                       0,						// do not share
//                       NULL,					// default security
//                       CREATE_ALWAYS,			// create new file always
//                       FILE_ATTRIBUTE_NORMAL,	// normal file
//                       NULL);					// no attr. template
//    if (hFile == INVALID_HANDLE_VALUE)
//        throw std::runtime_error("������ �������� ����� ��������� �� ������.");
//
//    DWORD bytes;              // ������� ������ � ����
//    BOOL Retval;
//    Retval = WriteFile(hFile, // ������ � ����
//              &header,        // ����� ������: ��� ������
//              sizeof(header), // ������� ������
//              &bytes,         // ����� DWORD'a: �� ������ - ������� ��������
//              0);             // A pointer to an OVERLAPPED structure.
//    if (!Retval)
//        throw std::runtime_error("������ ������ ��������� ���������� � ����.");    
//    
//	return	hFile;
//}
//
//// ������������� ���������� ���������
//void startReciver(unsigned int f, unsigned int g, char att){
//// f -  �������, ��
//// g - ��������
//// att - ���������� (���/����)
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
//	// ��������� LPT1 ���� ��� ������ ����� ������� GiveIO.sys
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
//	// ������������� �������
//	ag = att + sb*2 + g*16 + 128;
//    _outp(Address, ag);
//
//	// ������ ���� �� ������� � ����������
//	for ( i = 2; i >= 0;  i-- ) { // ��� ���� ������� �������� �����������
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
//	// ������ ���� ������� � ����������
//	for ( i = 15; i >= 0; i-- )  { // ��� ������� �����������
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
//	// ��������� ����
//	CloseHandle(LPTPort);
//}
//
//// ������������� PIC �� ����� ��������������� ������������ ����������
//void startPIC(unsigned int nstr, unsigned int fsync){
//// nstr -  ���������� ����� ����������
//// fsound - ������� ���������� �����, ��
//	HANDLE			COMPort;
//	double			fosc = 5e6;
//	unsigned char	cdat[8] = {103, 159, 205, 218, 144, 1, 0, 0}; // 50 ��, 398 �����
//	unsigned int	pimp_n, nimp_n, lnumb;
//	unsigned long	NumberOfBytesWritten;
//	
//	lnumb = nstr;			// ����� ����� � ������� �� ����, ������������ � PIC-����������
//
//	// ==========================================================================================
//	// ���������� ���������� �������� ���������
//	pimp_n = (unsigned int)(fosc/(4.*(double)fsync));		// ������ PIC �� ������	
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
//	// ���������� ���������� ���������� (������� ���������� �� ����� 19 ��.)
//	// ==========================================================================================
// 
//
//	// ==========================================================================================
//	// ������ ���������������
//	// ==========================================================================================
//	// ��������� ���� ��� ������
//	COMPort = CreateFile(TEXT("COM2"), 
//							GENERIC_READ | GENERIC_WRITE, 
//							0, 
//							NULL, 
//							OPEN_EXISTING, 
//							0, 
//							NULL);
//	if (COMPort == INVALID_HANDLE_VALUE) {
//        throw std::runtime_error("Error! ���������� ������� COM ����!");
//	}
// 
//	// Build on the current configuration, and skip setting the size
//	// of the input and output buffers with SetupComm.
//	DCB dcb;
//	BOOL fSuccess;
//
//	// ��������
//	fSuccess = GetCommState(COMPort, &dcb);
//	if (!fSuccess){
//		// Handle the error.
//        throw std::runtime_error("Error! GetCommState failed with error!");
//	}
//
//	// ����������
//	dcb.BaudRate = CBR_19200;     // set the baud rate
//	dcb.ByteSize = 8;             // data size, xmit, and rcv
//	dcb.Parity = NOPARITY;        // no parity bit
//	dcb.StopBits = TWOSTOPBITS;   // two stop bit
//
//	// ����������
//	fSuccess = SetCommState(COMPort, &dcb);
//	if (!fSuccess){
//		// Handle the error.
//        throw std::runtime_error("Error! SetCommState failed with error!");
//	}
//
//	// ��������
//	fSuccess = GetCommState(COMPort, &dcb);
//	if (!fSuccess){
//		// Handle the error.
//		throw std::runtime_error("Error! GetCommState failed with error!");
//	}
//
//	// �������� ���������� � �������� PIC16F870
//	fSuccess = WriteFile(
//		COMPort,		// ��������� ��� �����
//		&cdat,		// ��������� �� �����, ���������� ������, ������� ����� �������� � ����. 
//		8,// ����� ������, ������� ����� �������� � ����.
//		&NumberOfBytesWritten,//  ��������� �� ����������, ������� �������� ����� ���������� ������
//		NULL // ��������� �� ��������� OVERLAPPED
//	);
//    if (!fSuccess){
//		// Handle the error.
//		throw std::runtime_error("������ �������� ���������� � �������� PIC16F870.");
//	}
//
//	// ��������� ����
//	CloseHandle(COMPort);
//}
//
//// �������������� "�����" ������������ ������ � ������ �������� �������� � ��������� ������.
//unsigned char* loadLine(unsigned long *data, int n)
//{
//	unsigned char *cdata = NULL;
//
//	// ��������� ����� ������.
//	cdata = new unsigned char [n];
//	int re, im, abstmp;
//    for(int i = 0; i < n; i++)
//		{
//            // ���������� ������������� ������������� ����� ��������� ���������
//            union {
//                unsigned long word;     // 4-������� ����� �������������� ���
//                adcTwoChannels twoCh;  // ������������� (������������) ������
//            };
//            
//			// ��������� �� ����������. 
//			// ������� ������ ������� 14 ���. ������� 2 ��� - ��������������� �������.
//            word = data[i];
//            re = static_cast<int>(twoCh.re.value) >> 2;
//            im = static_cast<int>(twoCh.im.value) >> 2;
//
//            // ��������� ������������ ���������� � ���� ���������.
//			abstmp = static_cast<int>(floor(sqrt(re*re + im*im*1.0)));
//
//			// �������� ������ �� ������� 8 ��� (�� 2 ���� ��� ���������� �����).
//			cdata[i] = static_cast<unsigned char>(abstmp >> 6);
//		}            
//    
//    return cdata;
//}
//
//// �������������� "�����" ������������ ������ � ������ �������� �������� ��� �������� ������.
//unsigned int* loadIonogramLine(unsigned long *data, int n)
//{
//	unsigned int *idata = NULL;
//
//	// ��������� ����� ������.
//	idata = new unsigned int [n];
//	int re, im, abstmp;
//    for(int i = 0; i < n; i++)
//		{
//            // ���������� ������������� ������������� ����� ��������� ���������
//            union {
//                unsigned long word;     // 4-������� ����� �������������� ���
//                adcTwoChannels twoCh;  // ������������� (������������) ������
//            };
//            
//			// ��������� �� ����������. 
//			// ������� ������ ������� 14 ���. ������� 2 ��� - ��������������� �������.
//            word = data[i];
//            re = static_cast<int>(twoCh.re.value) >> 2;
//            im = static_cast<int>(twoCh.im.value) >> 2;
//
//            // ��������� ������������ ���������� � ���� ���������.
//			abstmp = static_cast<int>(floor(sqrt(re*re + im*im*1.0)));
//		}            
//    
//    return idata;
//}
//
//// ������������ ���������������� ����� ����������.
//void sumIonogramLine(unsigned int *dataDestination, unsigned int *dataSource, int n)
//{
//    for(int i = 0; i < n; i++)
//		dataDestination[i] = dataDestination[i]/2 + dataSource[i]/2;
//}
//
//// �������� ������ �� char (����� �� 6 ���).
//unsigned char* shiftLine6bits(unsigned int *data, int n)
//{
//	unsigned char *cdata = NULL;
//
//	// ��������� ����� ������.
//	cdata = new unsigned char [n];
//    for(int i = 0; i < n; i++)
//		// �������� ������ �� ������� 8 ��� (�� 2 ���� ��� ���������� �����).
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