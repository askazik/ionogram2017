// ===========================================================================
// �������� � ���������������� ������ ������������.
// ===========================================================================
#include "config.h"

/// <summary>
/// �������� ������ � ����������� ��� ������ � ������������� ���������� � ������������.
/// </summary>
namespace parus {

	// ===========================================================================
	// ���������������� ���� XML
	// ===========================================================================

	/// <summary>
	/// ������������� ������ ������� ������ <see cref="xmlconfig"/>.
	/// </summary>
	/// <param name="fullName">��� ����������������� ����� xml. �� ���������: #define XML_CONFIG_DEFAULT_FILE_NAME.</param>
	/// <param name="mes">��� ������������ (����������/���������������). �� ���������: IONOGRAM.</param>
	xmlconfig::xmlconfig(std::string fullName, Measurement mes)
	{
		_fullFileName = fullName;
		_mes = mes;

		// ������� ����������.
		const XML::XMLElement *parent, *xml_mes;
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
		parent = _document.FirstChildElement();
		for (
         xml_mes = parent->FirstChildElement("Measurement");
         xml_mes;
         xml_mes = xml_mes->NextSiblingElement()
        ) 
		{
			if(!strcmp(xml_mes->Attribute("name"), mes_name.c_str()))
				break;
		}
 
		// ��������� �������� ������������ ����������.
		loadMeasurementHeader(xml_mes);
		// ��������� �������� ������������ ������������.
		switch(mes)
		{
		case IONOGRAM:
			loadIonogramConfig(xml_mes);
			break;
		case AMPLITUDES:
			loadAmplitudesConfig(xml_mes);
			break;
		}
	}

	/// <summary>
	/// ���������� ���������� � ���������� ����������. ���������� � ��������� ����� ������������.
	/// </summary>
	/// <param name="xml_mes">��������� �� XML �������, ���������� ���������� � ���������� ����������/������������.</param>
	void xmlconfig::loadMeasurementHeader(const XML::XMLElement *xml_mes)
	{
		const XML::XMLElement *xml_header, *xml_element;
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
	/// �������� ���������� � ������������ ��������� ���������.
	/// </summary>
	/// <param name="xml_mes">��������� �� XML �������, ���������� ���������� � ���������� ����������/������������.</param>
	void xmlconfig::loadIonogramConfig(const XML::XMLElement *xml_mes)
	{
		const XML::XMLElement *xml_module, *xml_element;
		int value = 0;

		// �������, ��� ������ ���������� ������������
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

	// ������������ ��������� ����� ���������
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
	/// �������� ���������� � ������������ ����������� ���������.
	/// </summary>
	/// <param name="xml_mes">��������� �� XML �������, ���������� ���������� � ���������� ����������/������������.</param>
	void xmlconfig::loadAmplitudesConfig(const XML::XMLElement *xml_mes)
	{
		const XML::XMLElement *xml_module, *xml_element;
		int value = 0;
		int i = 0;

		// ������� �� �������
		for (
         xml_module = xml_mes->FirstChildElement("module");
         xml_module;
         xml_module = xml_module->NextSiblingElement()
        ) 
		{
			xml_element = xml_module->FirstChildElement("frequency");
			xml_element->QueryIntText(&value);
				_amplitudes[i].frq = value;
			i++;
		}
	}
}