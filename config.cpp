// ===========================================================================
// Операции с конфигурационным файлом зондирования.
// ===========================================================================
#include "config.h"

/// <summary>
/// Содержит классы и определения для работы с конфигурацией аппаратуры и эксперимента.
/// </summary>
namespace parus {

	// ===========================================================================
	// Конфигурационный файл XML
	// ===========================================================================

	/// <summary>
	/// Инициализация нового объекта класса <see cref="xmlconfig"/>.
	/// </summary>
	/// <param name="fullName">Имя конфигурационного файла xml. По умолчанию: #define XML_CONFIG_DEFAULT_FILE_NAME.</param>
	/// <param name="mes">Вид эксперимента (ионограмма/амплитудограмма). По умолчанию: IONOGRAM.</param>
	xmlconfig::xmlconfig(std::string fullName, Measurement mes)
	{
		_fullFileName = fullName;
		_mes = mes;

		// Считаем информацию.
		XML::XMLElement *xml_mes, *parent;
		_document.LoadFile(_fullFileName.c_str());
 
		// Искомый параметр name для выбора измерения
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

		// Определим элемент с искомым параметром
		const char *attribval;
		parent = _document.FirstChildElement();
		// Перебор до нахождения аттрибута
		do
		{
			xml_mes = parent->FirstChildElement("Measurement");
			attribval = xml_mes->Attribute("name");
		} while(xml_mes != NULL && strcmp(attribval, mes_name.c_str()));
 
		// Считываем желаемую конфигурацию аппаратуры.
		loadMeasurementHeader(xml_mes);
		// Считываем желаемую конфигурацию эксперимента.
		switch(mes)
		{
		case IONOGRAM:
			loadIonogramConfig(xml_mes);
			break;
		case AMPLITUDES:
			
			break;
		}
	}

	/// <summary>
	/// Считывание информации о настройках аппаратуры. Содержится в заголовке блока эксперимента.
	/// </summary>
	/// <param name="xml_mes">Указатель на XML элемент, сожержащий информацию о настройках аппаратуры/эксперимента.</param>
	void xmlconfig::loadMeasurementHeader(XML::XMLElement *xml_mes)
	{
		XML::XMLElement *xml_header, *xml_element;
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

	/// <summary>
	/// Загрузка информации о конфигурации измерения ионограмм.
	/// </summary>
	/// <param name="xml_mes">Указатель на XML элемент, сожержащий информацию о настройках аппаратуры/эксперимента.</param>
	void xmlconfig::loadIonogramConfig(XML::XMLElement *xml_mes)
	{
		XML::XMLElement *xml_module, *xml_element;
		int value = 0;

		// Считаем, что модуль ионограммы единственный
		xml_module = xml_mes->FirstChildElement("module");
		xml_element = xml_module->FirstChildElement("fbeg");
			xml_element->QueryIntText(&value);
				_ionogram.fbeg = value;
		xml_element = xml_module->FirstChildElement("fend");
			xml_element->QueryIntText(&value);
				_ionogram.fend = value;
		xml_element = xml_module->FirstChildElement("fstep");
			xml_element->QueryIntText(&value);
				_ionogram.fstep = value;
	}

	// Формирование заголовка файла ионограмм
	ionHeaderNew2 xmlconfig::getIonogramHeader(void)
	{
		ionHeaderNew2 _out;

		_out.count_freq = 1 + (_ionogram.fend - _ionogram.fbeg)/_ionogram.fstep;
		_out.count_height = getHeightCount();
		_out.count_modules = getModulesCount();
		_out.freq_max = _ionogram.fend;
		_out.freq_min = _ionogram.fbeg;
		_out.height_min = 0;
		_out.height_step = getHeightStep();
		_out.switch_frequency = _device.switch_frequency;
		_out.ver = getVersion();

		return _out;
	}

	/// <summary>
	/// Загрузка информации о конфигурации амплитудных измерений.
	/// </summary>
	/// <param name="xml_mes">Указатель на XML элемент, сожержащий информацию о настройках аппаратуры/эксперимента.</param>
	void xmlconfig::loadAmplitudesConfig(XML::XMLElement *xml_mes)
	{
		// Определим элемент с искомым параметром
		//const char *attribval;
		//do
		//{
		//	xml_mes = _document.FirstChildElement("Measurement");
		//	attribval = xml_mes->Attribute("name");
		//} while(!strcmp(attribval, mes_name.c_str()));
	}

	// ===========================================================================
	// Ионограмма
	// ===========================================================================
	//confIonogram::confIonogram(void) :
	//	config(IONOGRAM_CONFIG_DEFAULT_FILE_NAME)
	//{
	//	readIonogramConf();
	//}

	//confIonogram::confIonogram(std::string fullName) :
	//	config(fullName)
	//{
	//	readIonogramConf();
	//}
 //   
	//confIonogram::~confIonogram(void)
	//{
	//	std::cout << "Выполнен деструктор конфигурационного файла." << std::endl;
	//}

	//void confIonogram::readIonogramConf(void)
	//{
	//	_fin.seekg(0);
	//	int i = 0; // счетчик значений
	//	bool key = false;
	//
	//	std::string line;
	//	while(!key)
	//	{
	//		getline(_fin, line);
	//		// trim from end of string (right)
	//		line.erase(line.find_last_not_of(config::_whitespaces) + 1);
	//		if(line[0] != '#' && line.size() > 1) // пропуск комментариев и пустых строк
	//		{
	//			i++;
	//			switch(i)
	//			{									
	//			case 10: // шаг по частоте ионограммы, кГц
	//				_ionogram.fstep = getValueFromString(line);
	//				break;
	//			case 11: // начальная частота зондирования, кГц
	//				_ionogram.fbeg = getValueFromString(line);
	//				break;
	//			case 12: // конечная частота зондирования, кГц
	//				_ionogram.fend = getValueFromString(line);
	//				key = true;
	//				break;
	//			}
	//		}                        
	//	}
	//}

	//// ===========================================================================
	//// Амплитуды
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
	//	std::cout << "Выполнен деструктор конфигурационного файла." << std::endl;
	//	if(_modules_count) delete [] _ptModules;
	//}

	//// Является ли файл конфигурационным файлом для ионограммы.
	//bool parusConfig::isValidConf(std::string fullName){
	//	bool key = false;

	//	// Откроем файл для чтения.
	//	std::ifstream fin(fullName.c_str());
	//	if(!fin)
	//		throw std::runtime_error("Error: Не могу открыть конфигурационный файл " + fullName);

	//	std::string line;
	//	while(getline(fin, line)){
	//		// trim from end of string (right)
	//		line.erase(line.find_last_not_of(_whitespaces) + 1);
	//		if(line[0] != '#' && line.size()) // пропуск комментариев и пустых строк
	//		{
	//			std::size_t pos = line.find("SERIAL");
	//			if (pos != std::string::npos && pos == 0){ // тег найден
	//				key = true;
	//				break; // заканчиваем цикл после нахождения первой значащей строки
	//			}
	//		}			
	//	}
	//	fin.close();
 //   
	//	return key;
	//}

	//// Загружаем конфигурационный файл по полному пути.
	//void parusConfig::loadConf(std::string fullName){
	//	int i = 0; // счетчик значений
	//	bool key = false;

	//	if(isValidConf(fullName)){
	//		// Откроем файл для чтения.
	//		std::ifstream fin(fullName.c_str());
	//		if(!fin)
	//			throw std::runtime_error("Error: Не могу открыть конфигурационный файл " + fullName);

	//		std::string line;
	//		while(!key) {
	//			getline(fin, line);
	//			// trim from end of string (right)
	//			line.erase(line.find_last_not_of(_whitespaces) + 1);
	//			if(line[0] != '#' && line.size()) { // пропуск комментариев и пустых строк
	//				i++;
	//				switch(i) {									
	//				case 1: // пропускаем тег принадлежности
	//					break;
	//				case 2: // версия формата
	//					_ver = getValueFromString(line);
	//					break;
	//				case 3: // шаг по высоте, м
	//					_height_step = getValueFromString(line);
	//					break;
	//				case 4: // количество отсчётов высоты, не более 4096
	//					_height_count = getValueFromString(line);
	//					break;
	//				case 5: // импульсов зондирования на каждой частоте
	//					_pulse_count = getValueFromString(line);
	//					break;
	//				case 6: // ослабление (аттенюатор) 1/0 = вкл/выкл
	//					_attenuation = getValueFromString(line);
	//					break;
	//				case 7: // усиление (g = value/6, 6дБ = приращение в 4 раза по мощности)
	//					_gain = getValueFromString(line);
	//					break;
	//				case 8: // частота зондирующих импульсов, Гц
	//					_pulse_frq = getValueFromString(line);
	//					break;
	//				case 9: // длительность зондирующих импульсов, мкс
	//					_pulse_duration = getValueFromString(line);
	//					break;
	//				case 10: // начальная высота, км (всё, что ниже при обработке отбрасывается)
	//					_height_min = getValueFromString(line);
	//					break;
	//				case 11: // конечная высота, км (всё, что выше при обработке отбрасывается)
	//					_height_max = getValueFromString(line);
	//					key = true; // заканчиваем сканирование
	//					break;
	//				}
	//			}                        
	//		}
	//		getModules(fin); // войдем в сканирование модулей
	//		fin.close();
	//	}
	//	else
	//		throw std::runtime_error("Error: Это не конфигурационный файл для ионограмм." + fullName);
	//}

	//// Определяет количество модулей зондирования. 
	//void parusConfig::getModulesCount(std::ifstream &fin){
	//	_modules_count = 0;
	//	std::string line;
	//	while(getline(fin, line)) {          
	//		// trim from end of string (right)
	//		line.erase(line.find_last_not_of(_whitespaces) + 1);
	//		if(line.size() && line[0] != '#') { // пропуск комментариев и пустых строк
	//			std::size_t pos = line.find("%%%");
	//			if (pos != std::string::npos && pos == 0) // тег найден
	//				_modules_count++;                            
	//		}
	//	}
	//	fin.clear();
	//	fin.seekg(0, fin.beg);
	//}

	//// Извлекает модули зондирования.    
	//void parusConfig::getModules(std::ifstream &fin){
	//	getModulesCount(fin);
	//	_ptModules = new myModule [_modules_count];

	//	// Считываем параметры в модулях.
	//	int i = 0;
	//	std::string line;
	//	while(getline(fin, line)) {
	//		// trim from end of string (right)
	//		line.erase(line.find_last_not_of(_whitespaces) + 1);
	//		if(line.size() && line[0] != '#') { // пропуск комментариев и пустых строк
	//			std::size_t pos = line.find("%%%");
	//			if (pos != std::string::npos && pos == 0) { // тег найден
	//				_ptModules[i] = getCurrentModule(fin);
	//				i++;
	//			}
	//		}
	//	}    

	//}

	//// Извлекает текущий модуль зондирования.    
	//myModule parusConfig::getCurrentModule(std::ifstream &fin){
	//	bool key = false;
	//	myModule out;
	//	int i = 0;

	//	std::string line;
	//	while(!key) {
	//		getline(fin, line);
	//		// trim from end of string (right)
	//		line.erase(line.find_last_not_of(_whitespaces) + 1);
	//		if(line.size() && line[0] != '#' ) { // пропуск комментариев и пустых строк
	//			i++;
	//			switch(i) {									
	//			case 1: // частота модуля, кГц
	//				out.frq = getValueFromString(line);
	//				key = true;
	//				break;
	//			//case 2: // конечная частота модуля, кГц
	//			//	out.fend = getValueFromString(line);
	//			//	break;
	//			//case 3: // шаг по частоте ионограммы, кГц
	//			//	out.fstep = getValueFromString(line);
	//			//	break;
	//			//case 4: // импульсов зондирования на каждой частоте
	//			//	out.pulse_count = getValueFromString(line);
	//			//	break;
	//			//case 5: // ослабление (аттенюатор) 1/0 = вкл/выкл
	//			//	out.attenuation = getValueFromString(line);
	//			//	break;
	//			//case 6: // усиление (g = value/6, 6дБ = приращение в 4 раза по мощности)
	//			//	out.gain = getValueFromString(line);
	//			//	break;
	//			//case 7: // частота зондирующих импульсов, Гц
	//			//	out.pulse_frq = getValueFromString(line);
	//			//	break;
	//			//case 8: // длительность зондирующих импульсов, мкс
	//			//	out.pulse_duration = getValueFromString(line);
	//			//	key = true;
	//			//	break;
	//			}
	//		}					
	//	}    

	//	return out;
	//}

	//// Определение суммарного количества частот зондирования по всем модулям.
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

	//// Получение целого значения из строки.
	//unsigned parusConfig::getValueFromString(std::string line){
	//	return std::stoi(line,nullptr);
	//}

}