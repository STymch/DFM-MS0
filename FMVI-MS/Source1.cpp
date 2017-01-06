/* ��������� ������� FMVI-MS
FMVI-MS.ino	-	�������� ���� �������
	����������� ���������� �������� � ���������� �� �������-�����������
	�������� � ���� �������:
			CEEPROM:			�������� ���������� � EEPROM
			CSerialPort:		�����/�������� ������ �� ���������������� ������
			CInputImp:			������� ��������� ����������� EMFM QTLD-15 / �������� ����������
			CTemperatureSensor:	��������� ����������� ���� �� ������� ����������� DS18B20 Maxim/Dallas ICs
			CInputPower:		��������� � �������� ������ ������ ���
			CEMFMAlarm:			�������� ��������� �������� �����������
___________________________________________________________________________________________________

CEEPROM.cpp	-	���������� �������� ���������� � EEPROM, 
	class CEEPROM {}
___________________________________________________________________________________________________
CSerialPort.cpp -	�����/�������� ������ �� ���������������� ������, 
	class CSerialPort {}
	����������� �������� ������ ������ ����� FMVI-MS � FMVI-CP
	ISR_Timer()	-	���������� ���������� ����������� �������: �������� ������ ������ � FMVI-CP
___________________________________________________________________________________________________
CInputImp.cpp	-	������� ��������� ����������� EMFM QTLD-15 / �������� ����������,
	class CInputImp {}
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
*/
// ���������� ���������� �������� � ���������� �� �������-�����������:

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
// �������� ������� FMVI-MS
// ������������� �������
	



}


///////////////////////////////////////////////////////////////
// Working loop of FMVI-MS
///////////////////////////////////////////////////////////////

void loop()
{
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

	if (!pSSerial->Read(pbBuffer)) {
		// � ���������������� ����� ���� ������ �� FMVI-CP
		// ������ ������� � �� ����������
		switch (���_�������) {



		}
		// ������ ������ � ����� ��� �������� � FMVI-CP 
	}

	// ��������� �����������
	if (!isTemperatureScan) {

		isTemperatureScan = 1;
	}
	// ������ ALARM EMFM
	// ������ �������� ������� �����������
	// ������ ������ ��������� ��������� ���
	
}



// ���������� ���������� ����������� �������
// ���������� ���������: FLOAT fTimerFreq - ������� ���������� �������
void ISR_Timer() {
	// �������� � ���������������� ���� ��������� ����� ������
	pSSerial->Write(pbOutBuff);

	// ���������� ������� ����� �������
	dwTimerTick++;

}

// ���������� ���������� ���������� �� �������� ��������� ���������
// ���������� ���������: nImp_Ltr - ���-�� ��������� �� ����, nImpThresold - ���������� ������������ ��������
void ISR_InputImp() {
	
	if (����_�������_������_������������) {
		dwAllImpCount++;							// ����� ������� ���������
		
		if (dwCurrImpCount > 0) dwCurrImpCount--;	// ������� ��������� ��� ��������� �������

		dwImpTime = millis();						// ���������� ������ ������� ��������
	}
	else // �������


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
			BYTE	m_bLen;					// ����� ������ ���������� <= CMND_LEN				1
			BYTE	m_bCode;				// ��� �������										1
			union							// ��������� �������								1-4
			{
				DWORD	m_dwArg;			// �������� DWORD (4)		
				BYTE	m_bArg;				// �������� BYTE (1)
				UINT	m_nArg;				// �������� UINT (2)
				FLOAT	m_fArg;				// �������� FLOAT (4)
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
