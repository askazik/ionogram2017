// ===========================================================================
// �������� � ���������������� ������ ������������.
// ===========================================================================
#include "config.h"

namespace parus {

	// ===========================================================================
	// ���������������� ����
	// ===========================================================================
	const std::string config::_whitespaces = " \t\f\v\n\r";

	config::config(std::string fullName)
	{
		_fullFileName = fullName;

		// ������� ���� ��� ������.
		_fin.open(_fullFileName);
		if(!_fin.is_open())
			throw std::runtime_error("Error: �� ���� ������� ���������������� ���� " + _fullFileName);
		readDeviceConfig();
	}

	config::~config(void){
		_fin.close();
	}

	// ��������� ������ �������� �� ������.
	unsigned config::getValueFromString(std::string line)
	{
		return std::stoi(line,nullptr);
	}

	bool config::isTagConfig(std::string Tag, std::string fullName)
	{
		bool key = false;

		// ������� ���� ��� ������.
		std::ifstream fin(fullName);
		if(!fin)
			throw std::runtime_error("Error: �� ���� ������� ���������������� ���� " + fullName);

		std::string line;
		while(getline(fin, line))
		{
			// trim from end of string (right)
			line.erase(line.find_last_not_of(_whitespaces) + 1);
			if(line[0] != '#' && line.size()) // ������� ������������ � ������ �����
			{
				std::size_t pos = line.find(Tag);
				if (pos != std::string::npos && pos == 0) // ��� ������
				{
					key = true;
					break; // ����������� ���� ����� ���������� ������ �������� ������
				}
			}			
		}
		fin.close();
    
		return key;
	}

	bool config::isIonogramConfig(std::string fullName)
	{
		return isTagConfig("IONOGRAM", fullName);
	}

	bool config::isAmplitudesConfig(std::string fullName)
	{
		return isTagConfig("AMPLITUDES", fullName);
	}

	void config::readDeviceConfig(void)
	{
		int i = 0; // ������� ��������
		bool key = false;
	
		std::string line;
		while(!key)
		{
			getline(_fin, line);
			// trim from end of string (right)
			line.erase(line.find_last_not_of(config::_whitespaces) + 1);
			if(line[0] != '#' && line.size()) // ������� ������������ � ������ �����
			{
				i++;
				switch(i)
				{									
				case 1: // ���������� ��� ��������������
					_device.tag = line;
					break;
				case 2: // ������ �������
					_device.ver = getValueFromString(line);
					break;
				case 3: // ��� �� ������, �
					_device.height_step = getValueFromString(line);
					break;
				case 4: // ���������� �������� ������, �� ����� 4096
					_device.height_count = getValueFromString(line);
					break;
				case 5: // ��������� ������������ �� ������ �������
					_device.pulse_count = getValueFromString(line);
					break;
				case 6: // ���������� (����������) 1/0 = ���/����
					_device.attenuation = getValueFromString(line);
					break;
				case 7: // �������� (g = value/6, 6�� = ���������� � 4 ���� �� ��������)
					_device.gain = getValueFromString(line);
					break;
				case 8: // ������� ����������� ���������, ��
					_device.pulse_frq = getValueFromString(line);
					break;
				case 9: // ������������ ����������� ���������, ���
					_device.pulse_duration = getValueFromString(line);
					key = true;
					break;
				}
			}                        
		}
	}

	// ===========================================================================
	// ���������������� ���� XML
	// ===========================================================================

	xmlconfig::xmlconfig(std::string fullName, Measurement mes)
	{
		_fullFileName = fullName;
		_mes = mes;

		// ������� ����������.
		const XML::XMLElement *xml_mes, *xml_header, *xml_element;
		_document.LoadFile(_fullFileName.c_str());
 
		// ������� �������� name ��� ������ ���������
		std::string mes_name;
		switch(mes)
		{
		case IONOGRAM:
			mes_name = "ionogram";
			break;
		case AMPLITUDES:
			mes_name = "amplitudes";
			break;
		}

		// ��������� ������� � ������� ����������
		const char *attribval;
		do
		{
			xml_mes = _document.FirstChildElement("Measurement");
			attribval = xml_mes->Attribute("name");
		} while(!strcmp(attribval, mes_name.c_str()));
 
		int value = 0;
		xml_header = xml_mes->FirstChildElement("header");
		xml_element = xml_header->FirstChildElement("version");
			xml_element->QueryIntText(&value);
				_device.ver = value;
		xml_element = xml_header->FirstChildElement("height_step");
			xml_element->QueryIntText(&value);
				_device.height_step = value;
		xml_element = xml_header->FirstChildElement("height_count");
			xml_element->QueryIntText(&value);
				_device.height_count = value;
		xml_element = xml_header->FirstChildElement("pulse_count");
			xml_element->QueryIntText(&value);
				_device.pulse_count = value;
		xml_element = xml_header->FirstChildElement("attenuation");
			xml_element->QueryIntText(&value);
				_device.attenuation = value;
		xml_element = xml_header->FirstChildElement("gain");
			xml_element->QueryIntText(&value);
				_device.gain = value;
		xml_element = xml_header->FirstChildElement("pulse_frq");
			xml_element->QueryIntText(&value);
				_device.pulse_frq = value;
		xml_element = xml_header->FirstChildElement("pulse_duration");
			xml_element->QueryIntText(&value);
				_device.pulse_duration = value;
		xml_element = xml_header->FirstChildElement("switch_frequency");
			xml_element->QueryIntText(&value);
				_device.switch_frequency = value;
		xml_element = xml_header->FirstChildElement("modules_count");
			xml_element->QueryIntText(&value);
				_device.modules_count = value;
	}

	// ===========================================================================
	// ����������
	// ===========================================================================
	confIonogram::confIonogram(void) :
		config(IONOGRAM_CONFIG_DEFAULT_FILE_NAME)
	{
		readIonogramConf();
	}

	confIonogram::confIonogram(std::string fullName) :
		config(fullName)
	{
		readIonogramConf();
	}
    
	confIonogram::~confIonogram(void)
	{
		std::cout << "�������� ���������� ����������������� �����." << std::endl;
	}

	void confIonogram::readIonogramConf(void)
	{
		_fin.seekg(0);
		int i = 0; // ������� ��������
		bool key = false;
	
		std::string line;
		while(!key)
		{
			getline(_fin, line);
			// trim from end of string (right)
			line.erase(line.find_last_not_of(config::_whitespaces) + 1);
			if(line[0] != '#' && line.size() > 1) // ������� ������������ � ������ �����
			{
				i++;
				switch(i)
				{									
				case 10: // ��� �� ������� ����������, ���
					_ionogram.fstep = getValueFromString(line);
					break;
				case 11: // ��������� ������� ������������, ���
					_ionogram.fbeg = getValueFromString(line);
					break;
				case 12: // �������� ������� ������������, ���
					_ionogram.fend = getValueFromString(line);
					key = true;
					break;
				}
			}                        
		}
	}

	//// ===========================================================================
	//// ���������
	//// ===========================================================================
	//parusConfig::parusConfig(std::string fullName) :
	//	_whitespaces (std::string(" \t\f\v\n\r")),
	//	_modules_count (0),
	//	_ptModules (nullptr)
	//{
	//	_fullFileName = fullName;
	//	loadConf(_fullFileName);
	//}
 //    
	//parusConfig::~parusConfig(void){
	//	std::cout << "�������� ���������� ����������������� �����." << std::endl;
	//	if(_modules_count) delete [] _ptModules;
	//}

	//// �������� �� ���� ���������������� ������ ��� ����������.
	//bool parusConfig::isValidConf(std::string fullName){
	//	bool key = false;

	//	// ������� ���� ��� ������.
	//	std::ifstream fin(fullName.c_str());
	//	if(!fin)
	//		throw std::runtime_error("Error: �� ���� ������� ���������������� ���� " + fullName);

	//	std::string line;
	//	while(getline(fin, line)){
	//		// trim from end of string (right)
	//		line.erase(line.find_last_not_of(_whitespaces) + 1);
	//		if(line[0] != '#' && line.size()) // ������� ������������ � ������ �����
	//		{
	//			std::size_t pos = line.find("SERIAL");
	//			if (pos != std::string::npos && pos == 0){ // ��� ������
	//				key = true;
	//				break; // ����������� ���� ����� ���������� ������ �������� ������
	//			}
	//		}			
	//	}
	//	fin.close();
 //   
	//	return key;
	//}

	//// ��������� ���������������� ���� �� ������� ����.
	//void parusConfig::loadConf(std::string fullName){
	//	int i = 0; // ������� ��������
	//	bool key = false;

	//	if(isValidConf(fullName)){
	//		// ������� ���� ��� ������.
	//		std::ifstream fin(fullName.c_str());
	//		if(!fin)
	//			throw std::runtime_error("Error: �� ���� ������� ���������������� ���� " + fullName);

	//		std::string line;
	//		while(!key) {
	//			getline(fin, line);
	//			// trim from end of string (right)
	//			line.erase(line.find_last_not_of(_whitespaces) + 1);
	//			if(line[0] != '#' && line.size()) { // ������� ������������ � ������ �����
	//				i++;
	//				switch(i) {									
	//				case 1: // ���������� ��� ��������������
	//					break;
	//				case 2: // ������ �������
	//					_ver = getValueFromString(line);
	//					break;
	//				case 3: // ��� �� ������, �
	//					_height_step = getValueFromString(line);
	//					break;
	//				case 4: // ���������� �������� ������, �� ����� 4096
	//					_height_count = getValueFromString(line);
	//					break;
	//				case 5: // ��������� ������������ �� ������ �������
	//					_pulse_count = getValueFromString(line);
	//					break;
	//				case 6: // ���������� (����������) 1/0 = ���/����
	//					_attenuation = getValueFromString(line);
	//					break;
	//				case 7: // �������� (g = value/6, 6�� = ���������� � 4 ���� �� ��������)
	//					_gain = getValueFromString(line);
	//					break;
	//				case 8: // ������� ����������� ���������, ��
	//					_pulse_frq = getValueFromString(line);
	//					break;
	//				case 9: // ������������ ����������� ���������, ���
	//					_pulse_duration = getValueFromString(line);
	//					break;
	//				case 10: // ��������� ������, �� (��, ��� ���� ��� ��������� �������������)
	//					_height_min = getValueFromString(line);
	//					break;
	//				case 11: // �������� ������, �� (��, ��� ���� ��� ��������� �������������)
	//					_height_max = getValueFromString(line);
	//					key = true; // ����������� ������������
	//					break;
	//				}
	//			}                        
	//		}
	//		getModules(fin); // ������ � ������������ �������
	//		fin.close();
	//	}
	//	else
	//		throw std::runtime_error("Error: ��� �� ���������������� ���� ��� ���������." + fullName);
	//}

	//// ���������� ���������� ������� ������������. 
	//void parusConfig::getModulesCount(std::ifstream &fin){
	//	_modules_count = 0;
	//	std::string line;
	//	while(getline(fin, line)) {          
	//		// trim from end of string (right)
	//		line.erase(line.find_last_not_of(_whitespaces) + 1);
	//		if(line.size() && line[0] != '#') { // ������� ������������ � ������ �����
	//			std::size_t pos = line.find("%%%");
	//			if (pos != std::string::npos && pos == 0) // ��� ������
	//				_modules_count++;                            
	//		}
	//	}
	//	fin.clear();
	//	fin.seekg(0, fin.beg);
	//}

	//// ��������� ������ ������������.    
	//void parusConfig::getModules(std::ifstream &fin){
	//	getModulesCount(fin);
	//	_ptModules = new myModule [_modules_count];

	//	// ��������� ��������� � �������.
	//	int i = 0;
	//	std::string line;
	//	while(getline(fin, line)) {
	//		// trim from end of string (right)
	//		line.erase(line.find_last_not_of(_whitespaces) + 1);
	//		if(line.size() && line[0] != '#') { // ������� ������������ � ������ �����
	//			std::size_t pos = line.find("%%%");
	//			if (pos != std::string::npos && pos == 0) { // ��� ������
	//				_ptModules[i] = getCurrentModule(fin);
	//				i++;
	//			}
	//		}
	//	}    

	//}

	//// ��������� ������� ������ ������������.    
	//myModule parusConfig::getCurrentModule(std::ifstream &fin){
	//	bool key = false;
	//	myModule out;
	//	int i = 0;

	//	std::string line;
	//	while(!key) {
	//		getline(fin, line);
	//		// trim from end of string (right)
	//		line.erase(line.find_last_not_of(_whitespaces) + 1);
	//		if(line.size() && line[0] != '#' ) { // ������� ������������ � ������ �����
	//			i++;
	//			switch(i) {									
	//			case 1: // ������� ������, ���
	//				out.frq = getValueFromString(line);
	//				key = true;
	//				break;
	//			//case 2: // �������� ������� ������, ���
	//			//	out.fend = getValueFromString(line);
	//			//	break;
	//			//case 3: // ��� �� ������� ����������, ���
	//			//	out.fstep = getValueFromString(line);
	//			//	break;
	//			//case 4: // ��������� ������������ �� ������ �������
	//			//	out.pulse_count = getValueFromString(line);
	//			//	break;
	//			//case 5: // ���������� (����������) 1/0 = ���/����
	//			//	out.attenuation = getValueFromString(line);
	//			//	break;
	//			//case 6: // �������� (g = value/6, 6�� = ���������� � 4 ���� �� ��������)
	//			//	out.gain = getValueFromString(line);
	//			//	break;
	//			//case 7: // ������� ����������� ���������, ��
	//			//	out.pulse_frq = getValueFromString(line);
	//			//	break;
	//			//case 8: // ������������ ����������� ���������, ���
	//			//	out.pulse_duration = getValueFromString(line);
	//			//	key = true;
	//			//	break;
	//			}
	//		}					
	//	}    

	//	return out;
	//}

	//// ����������� ���������� ���������� ������ ������������ �� ���� �������.
	////unsigned parusConfig::getCount_freq(void){
	////    unsigned count = 0;
	////    for(unsigned i = 0; i < modules_count; i++)
	////        {
	////            unsigned fcount = static_cast<unsigned>((ptModulesArray[i].fend - ptModulesArray[i].fbeg)/ptModulesArray[i].fstep + 1);
	////            count += fcount;
	////        }
	////
	////    return count;
	////}

	//// ��������� ������ �������� �� ������.
	//unsigned parusConfig::getValueFromString(std::string line){
	//	return std::stoi(line,nullptr);
	//}

}