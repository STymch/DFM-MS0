// 
// 
// 

#include "CTemperatureSensor.h"

const int   DS_PIN = 5;     // номер входа, к которому подключен температурный датчик

// Detect temperature sensor
// Return:	0 - sensor OK, 
//			-1 - no sensor,
//			1 - no more addresses, 
//			2 - CRC is not valid.
int CTemperatureSensor::Detect()
{
	int		retcode = -1;
	
	// Detect sensor
	if (m_pDS->search(p_bAddr))	// read adresses
		if (OneWire::crc8(p_bAddr, 7) == p_bAddr[7]) 	// check CRC
		{
			// The first ROM byte indicates which chip
			switch (p_bAddr[0]) {
			case 0x10:	// Chip = DS18S20 or old DS1820
				m_nTypeSensor = 1;
				break;
			case 0x28:	// Chip = DS18B20
				m_nTypeSensor = 2;
				break;
			case 0x22:	// Chip = DS1822
				m_nTypeSensor = 3;
				break;
			default:	// Device is not a DS18x20 family
				m_nTypeSensor = -1;
				break;
			}
			retcode = 0;
		}
		else	// CRC is not valid
			retcode = 2;
	else		// No more addresses
		retcode = 1;

	return retcode;
}

// Get temperature from sensor: 0 - celsius, 1 - fahrenheit
float CTemperatureSensor::GetTemperature(int nTypeScale = 0)
{
	byte  data[12];

	m_pDS->reset();
	m_pDS->select(p_bAddr);
	m_pDS->write(0x44, 1);

	delay(1000);

	m_pDS->reset();
	m_pDS->select(p_bAddr);
	m_pDS->write(0xBE);         // read Scratchpad

	// Read data	
	for (int i = 0; i < 9; i++) data[i] = m_pDS->read();	// we need 9 bytes


	// Convert the data to actual temperature
	// because the result is a 16 bit signed integer, it should
	// be stored to an "int16_t" type, which is always 16 bits
	// even when compiled on a 32 bit processor.
	int16_t raw = (data[1] << 8) | data[0];
	if (m_nTypeSensor == 1) {
		raw = raw << 3; // 9 bit resolution default
		if (data[7] == 0x10) {
			// "count remain" gives full 12 bit resolution
			raw = (raw & 0xFFF0) + 12 - data[6];
		}
	}
	else {
		byte cfg = (data[4] & 0x60);
		// at lower res, the low bits are undefined, so let's zero them
		if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
		else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
		else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
											  //// default is 12 bit resolution, 750 ms conversion time
	}

	if (!nTypeScale) return (float)raw / 16.0;
	else return ((float)raw / 16.0) * 1.8 + 32.0;
}

/*void CTemperatureSensor::Init()
{
	
	float Tcelsius, Tfahrenheit;
	byte i;
	byte  addr[8];
	byte  type_s;
	byte  present = 0;
	byte  data[12];
	int   nInpByte;  //хранение полученного из порта байта
	
	// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
	OneWire DS(DS_PIN);

	// Инициализация входов/выходов
	pinMode(DS_PIN, INPUT);  // вход температурного датчика

	
	// Определяем датчик
	if (!DS.search(addr)) {
		Serial.println("No more addresses.");
		Serial.println();
		DS.reset_search();
		delay(250);
		return;
	}

	Serial.print("ROM =");
	for (i = 0; i < 8; i++) {
		Serial.write(' ');
		Serial.print(addr[i], HEX);
	}

	if (OneWire::crc8(addr, 7) != addr[7]) {
		Serial.println("CRC is not valid!");
		return;
	}
	Serial.print("   CRC is valid = 0x");
	Serial.println(OneWire::crc8(data, 8), HEX);

	// the first ROM byte indicates which chip
	switch (addr[0]) {
	case 0x10:
		Serial.println("Chip = DS18S20");  // or old DS1820
		type_s = 1;
		break;
	case 0x28:
		Serial.println("Chip = DS18B20");
		type_s = 0;
		break;
	case 0x22:
		Serial.println("Chip = DS1822");
		type_s = 0;
		break;
	default:
		Serial.println("DEVICE IS NOT A DS18x20 FAMILY DEVICES!!!");
		return;
	}

	/* // Ожидание команды из последовательного порта на измерение температуры
	Serial.println("Waiting command...");

	while (Serial.available() <= 0);

	Serial.print("Received command: ");
	// считываем байт данных
	nInpByte = Serial.read();
	Serial.println(nInpByte);

	// анализируем команду
	if (nInpByte == 49) // 1
	{
	*/
/*		Serial.println("Requesting temperatures...");
		DS.reset();
		DS.select(addr);
		//oneWire.write(0xCC);
		DS.write(0x44, 1);

		delay(1000);

		present = DS.reset();
		DS.select(addr);
		DS.write(0xBE);         // Read Scratchpad

		Serial.print("Data = ");
		Serial.print(present, HEX);
		Serial.print(" ");
		for (i = 0; i < 9; i++) {           // we need 9 bytes
			data[i] = DS.read();
			Serial.print(data[i], HEX);
			Serial.print(" ");
		}
		Serial.print(" CRC = 0x");
		Serial.print(OneWire::crc8(data, 8), HEX);
		Serial.println();

		// Convert the data to actual temperature
		// because the result is a 16 bit signed integer, it should
		// be stored to an "int16_t" type, which is always 16 bits
		// even when compiled on a 32 bit processor.
		int16_t raw = (data[1] << 8) | data[0];
		if (type_s) {
			raw = raw << 3; // 9 bit resolution default
			if (data[7] == 0x10) {
				// "count remain" gives full 12 bit resolution
				raw = (raw & 0xFFF0) + 12 - data[6];
			}
		}
		else {
			byte cfg = (data[4] & 0x60);
			// at lower res, the low bits are undefined, so let's zero them
			if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
			else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
			else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
												  //// default is 12 bit resolution, 750 ms conversion time
		}

		Tcelsius = (float)raw / 16.0;
		Tfahrenheit = Tcelsius * 1.8 + 32.0;

		/*
		oneWire.reset();
		oneWire.write(0xCC);
		oneWire.write(0xBE);
		data[0] = oneWire.read();
		data[1] = oneWire.read();

		T = (float)((data[1]<< 8)+data[0])/16.0;
		*/
/*		Serial.print("Temperatures = ");
		Serial.print(Tcelsius);   Serial.print(" C, ");
		Serial.print(Tfahrenheit); Serial.println(" F");
	//}

}
*/ 

