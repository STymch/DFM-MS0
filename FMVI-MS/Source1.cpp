/* Структура проекта FMVI-MS
FMVI-MS.ino	-	основной файл проекта
	Определение глобальных констант и переменных по секциям-подсистемам
	Содержит в себе объекты:
			CEEPROM:			хранение параметров в EEPROM
			CSerialPort:		прием/передача данных по последовательным портам
			CInputImp:			подсчет импульсов расходомера EMFM QTLD-15 / внешнего генератора
			CTemperatureSensor:	получение температуры воды из датчика температуры DS18B20 Maxim/Dallas ICs
			CInputPower:		получение и контроль уровня зарадя АКБ
			CEMFMAlarm:			конрроль аварийных сигналов расходомера
___________________________________________________________________________________________________
CSerialPort.cpp -	прием/передача данных по последовательным портам, 
	class CSerialPort {}
	Определение структур данных обмена между FMVI-MS и FMVI-CP
	ISR_Timer()	-	обработчик прерываний аппаратного таймера: передача пакета данных в FMVI-CP
___________________________________________________________________________________________________
CInputImp.cpp	-	подсчет импульсов расходомера EMFM QTLD-15 / внешнего генератора,
	class CInputImp {}
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
// Initialisation FMVI-MS
///////////////////////////////////////////////////////////////
void setup()
{
// Создание объекта FMVI-MS
// Инициализация объекта
	



}


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

	if (!pSSerial->Read(pbBuffer)) {
		// В последовательном порту есть данные от FMVI-CP
		// Разбор команды и ее выполнение
		switch (код_команды) {



		}
		// Запись данных в пакет для передачи в FMVI-CP 
	}

	// Измерение температуры
	if (!isTemperatureScan) {

		isTemperatureScan = 1;
	}
	// Чтение ALARM EMFM
	// Чтение состяния датчика температуры
	// Чтение уровня выходного напржения АКБ
	
}




// Обработчик аппаратных прерываний от внешнего источника импульсов
// Глобальные параметры: nImp_Ltr - кол-во импульсов на литр, nImpThresold - минимальна длительность импульса
void ISR_InputImp() {
	
	if (есть_импульс_нужной_длительности) {
		dwAllImpCount++;							// Общий счетчик импульсов
		
		if (dwCurrImpCount > 0) dwCurrImpCount--;	// Счетчик импульсов для заданного пролива

		dwImpTime = millis();						// Запоминаем отсчет времени импульса
	}
	else // дребезг


}




///////////////////////////////////////////////////////
// <<< CCmndMS - class for structure of commands of FMVI-MS
///////////////////////////////////////////////////////
const INT		CMND_LEN = 5;				// Size of command in bytes
class CCmndMS
{
protected:
	union									// Union for access to data
	{
		BYTE		m_pCmndMS[CMND_LEN + 1];// Array of bytes for command
		struct								// Structure of data
		{
			BYTE	m_bLen;					// Длина пакета информации <= CMND_LEN				1
			BYTE	m_bCode;				// Код команды										1
			union							// Аргументы команды								1-4
			{
				DWORD	m_dwArg;			// Аргумент DWORD (4)		
				BYTE	m_bArg;				// Аргумент BYTE (1)
				UINT	m_nArg;				// Аргумент UINT (2)
				FLOAT	m_fArg;				// Аргумент FLOAT (4)
			};			
		};
	};
public:
	// Constructor, destructor
	CCmndMS(INT nDataLen = CMND_LEN) : m_bLen(nDataLen) {}
	~CCmndMS() {}

	// Get pointer to data
	BYTE*	GetDataMS() { return m_pDataMS; }
	BYTE	GetLen() { return m_bLen; }
	BYTE	GetStatus() { return m_bStatus; }
	FLOAT	GetTempr() { return m_fTempr; }
	UINT	GetPowerU() { return m_nPowerU; }
	FLOAT	GetQ() { return m_fQ; }
	DWORD	GetCountF() { return m_dwCountFull; }
	DWORD	GetCountC() { return m_dwCountCurr; }


	// Set data from buffer of bytes
	void	SetDataMS(BYTE b) { for (int i = 1; i <= m_bLen; m_pDataMS[i++] = b); }
	void	SetDataMS(BYTE *pBuff) { for (int i = 1; i <= m_bLen; m_pDataMS[i++] = (*pBuff)++); }
	void	SetLen(BYTE bLen) { m_bLen = bLen; }
	void	SetStatus(BYTE bStatus) { m_bStatus = bStatus; }
	void	SetTempr(FLOAT fT) { m_fTempr = fT; }
	void	SetPowerU(UINT nU) { m_nPowerU = nU; }
	void	SetQ(FLOAT fQ) { m_fQ = fQ; }
	void	SetCountF(DWORD dwC) { m_dwCountFull = dwC; }
	void	SetCountC(DWORD dwC) { m_dwCountCurr = dwC; }

};
