/* Структура проекта FMVI-MS
FMVI-MS.ino	-	основной файл проекта
	Определение глобальных констант и переменных по секциям-подсистемам
	Содержит в себе объекты:
			CEEPROM:			хранение параметров в EEPROM
			CSerialPort:		прием/передача данных по последовательным портам
			CEMFM:			подсчет импульсов расходомера EMFM QTLD-15 / внешнего генератора
			CTemperatureSensor:	получение температуры воды из датчика температуры DS18B20 Maxim/Dallas ICs
			CInputPower:		получение и контроль уровня зарадя АКБ
			CEMFMAlarm:			конрроль аварийных сигналов расходомера
___________________________________________________________________________________________________
CSerialPort.cpp -	прием/передача данных по последовательным портам, 
	class CSerialPort {}
	Определение структур данных обмена между FMVI-MS и FMVI-CP
	ISR_Timer()	-	обработчик прерываний аппаратного таймера: передача пакета данных в FMVI-CP
___________________________________________________________________________________________________
CEMFM.cpp	-	подсчет импульсов расходомера EMFM QTLD-15 / внешнего генератора,
	class CEMFM {}
	ISR_InputImp	-	обработчик прерываний от внешнего источника импульсов: посчет импульсов расходомера
	Предусмотреть возможность определять импульс при переходах 0-1 и 1-0.
___________________________________________________________________________________________________
CTemperatureSensor.cpp	-	получение температуры воды из датчика температуры DS18B20 Maxim/Dallas ICs
	class CTemperatureSensor {}
___________________________________________________________________________________________________
CInputPower.cpp	-	получение и контроль уровня зарадя АКБ,
	class CInputPower {}
___________________________________________________________________________________________________
CEMFMAlarm.cpp	-	конрроль аварийных сигналов расходомера,
	class CEMFMAlarm {}
___________________________________________________________________________________________________
CEEPROM.cpp	-	подсистема хранения параметров в EEPROM,
class CEEPROM {}
___________________________________________________________________________________________________

*/
// Объявления глобальных констант и переменных по секциям-подсистемам:

// Global Flags
INT isInitMS = 0;			// Is there a initialisation of FMVI-MS
INT isContactCP = 0;		// Is there a contact FMVI-MS with FMVI-CP
INT isTemperatureScan = 0;	
INT is Command = 0;



///////////////////////////////////////////////////////////////
// Working loop of FMVI-MS
///////////////////////////////////////////////////////////////

void loop()
{
	// Обработка нажатия клавиш:
	if (Serial.available()) {
		char ch = Serial.read();
		switch (ch) {
		case '0'...'9':
			// v = v * 10 + ch - '0';
			break;
		case 'p':
			FrequencyTimer2::setPeriod(v);
			Serial.print("set ");
			Serial.print((long)v, DEC);
			Serial.print(" = ");
			Serial.print((long)FrequencyTimer2::getPeriod(), DEC);
			Serial.println();
			v = 0;
			break;
		case 'r':
			Serial.print("period is ");
			Serial.println(FrequencyTimer2::getPeriod());
			break;
		case 'e':
			FrequencyTimer2::enable();
			break;
		case 'd':
			FrequencyTimer2::disable();
			break;
		case 'o':
			FrequencyTimer2::setOnOverflow(Burp);
			break;
		case 'n':
			FrequencyTimer2::setOnOverflow(0);
			break;
		case 'b':
			unsigned long count;
			noInterrupts();     // disable interrupts while reading the count
			count = burpCount;  // so we don't accidentally read it while the
			interrupts();       // Burp() function is changing the value!
			Serial.println(count, DEC);
			break;
		}
	}
	
	if (!isInit) {
		// Чтение данных из EEPROM
		// Инициализация объектов

		// Первоначальное измерение температуры
		if (!isTemperatureScan) {
		
			isTemperatureScan = 1;
		}
		
		isInit = 1;

	}

	if (!isContactCP) {
		// Ожидание контакта
		isContactCP = 1;
	}	

	

	// Измерение температуры
	if (!isTemperatureScan) {

		isTemperatureScan = 1;
	}
	// Чтение ALARM EMFM
	// Чтение состяния датчика температуры
	// Чтение уровня выходного напржения АКБ
	
}




