/* ��������� ������� FMVI-MS
FMVI-MS.ino	-	�������� ���� �������
	����������� ���������� �������� � ���������� �� �������-�����������
	�������� � ���� �������:
			CEEPROM:			�������� ���������� � EEPROM
			CSerialPort:		�����/�������� ������ �� ���������������� ������
			CEMFM:			������� ��������� ����������� EMFM QTLD-15 / �������� ����������
			CTemperatureSensor:	��������� ����������� ���� �� ������� ����������� DS18B20 Maxim/Dallas ICs
			CInputPower:		��������� � �������� ������ ������ ���
			CEMFMAlarm:			�������� ��������� �������� �����������
___________________________________________________________________________________________________
CSerialPort.cpp -	�����/�������� ������ �� ���������������� ������, 
	class CSerialPort {}
	����������� �������� ������ ������ ����� FMVI-MS � FMVI-CP
	ISR_Timer()	-	���������� ���������� ����������� �������: �������� ������ ������ � FMVI-CP
___________________________________________________________________________________________________
CEMFM.cpp	-	������� ��������� ����������� EMFM QTLD-15 / �������� ����������,
	class CEMFM {}
	ISR_InputImp	-	���������� ���������� �� �������� ��������� ���������: ������ ��������� �����������
	������������� ����������� ���������� ������� ��� ��������� 0-1 � 1-0.
___________________________________________________________________________________________________
CTemperatureSensor.cpp	-	��������� ����������� ���� �� ������� ����������� DS18B20 Maxim/Dallas ICs
	class CTemperatureSensor {}
___________________________________________________________________________________________________
CInputPower.cpp	-	��������� � �������� ������ ������ ���,
	class CInputPower {}
___________________________________________________________________________________________________
CEMFMAlarm.cpp	-	�������� ��������� �������� �����������,
	class CEMFMAlarm {}
___________________________________________________________________________________________________
CEEPROM.cpp	-	���������� �������� ���������� � EEPROM,
class CEEPROM {}
___________________________________________________________________________________________________

*/
// ���������� ���������� �������� � ���������� �� �������-�����������:

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
	// ��������� ������� ������:
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
		// ������ ������ �� EEPROM
		// ������������� ��������

		// �������������� ��������� �����������
		if (!isTemperatureScan) {
		
			isTemperatureScan = 1;
		}
		
		isInit = 1;

	}

	if (!isContactCP) {
		// �������� ��������
		isContactCP = 1;
	}	

	

	// ��������� �����������
	if (!isTemperatureScan) {

		isTemperatureScan = 1;
	}
	// ������ ALARM EMFM
	// ������ �������� ������� �����������
	// ������ ������ ��������� ��������� ���
	
}




