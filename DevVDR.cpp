/************************************************
Copyright(c) 2006, AsiaControl Company
All rights reserved.
File Name:   DevVDR.cpp
Class Name:  CDevVDR
Brief:       Device sub class
History: 
Date	         Author  	       Remarks
Aug. 2006	     Develop Dep.1     
************************************************/
#include "StdAfx.h"
#include "DevVDR.h"
#include "VDRPro.h"

//add or modify devices  here  
static DEVICE_INFO gsDeviceInfo[]=
{
	//add your code  here, the following for your reference
	{_T("VDR")  },	//name1
};

#define		DEVICE_TYPE_NUM		sizeof(gsDeviceInfo)/sizeof(DEVICE_INFO)  

#define		DEVICE_NAME1			_T("VDR")


static REG_INFO gsRegInfo[]=
{
	{_T("ALARM"), 0, 1, INT_DATATYPE, PT_WRITE },
	{_T("ANALOG"), 0, 1, LONG_DATATYPE, PT_WRITE}
};

#define		REG_TYPE_NUM		sizeof(gsRegInfo)/sizeof(REG_INFO)  

#define     VDR_REG   0

IMPLEMENT_DYNAMIC(CDevVDR, CDevBase)

CDevVDR::CDevVDR(void)
{
	m_nPreRecLen = 0; 
}


CDevVDR::~CDevVDR(void)
{
}

/****************************************************************************
*   Name
		GetRegisters
*	Type
		public
*	Function
		The GetRegister retrieves a register list for the specified szDeviceName
*	Return Value
		return true if successful;
*	Parameters
		ppReg
			[out] Pointer to a variable that receives the address of register list
		pRegNum
			[out] Pointer to a variable that receives the size of register list
*****************************************************************************/
BOOL CDevVDR::GetRegisters( LPVOID *ppReg, int *pRegNum )
{
	CDebug::ShowFunMessage(_T("CDevVDR::GetRegisters"));
	ASSERT(ppReg != NULL);
	ASSERT(pRegNum != NULL);
	*ppReg = gsRegInfo ;
	*pRegNum = REG_TYPE_NUM ;
	return TRUE ;
}

/****************************************************************************
*   Name
		GetDevices
*	Type
		public
*	Function
		The GetRegister retrieves a devices list for the specified device kind class
*	Return Value
		return true if successful; 
*	Parameters
		ppDevice
			[out] Pointer to a variable that receives the address of device list
		pDeviceNum
			[out] Pointer to a variable that receives the size of device list
*****************************************************************************/
BOOL CDevVDR::GetDevices(LPVOID *ppDevice, int *pDeviceNum) //
{
	CDebug::ShowFunMessage(_T("CDevVDR::GetDevices"));
	ASSERT(ppDevice != NULL);
	ASSERT(pDeviceNum != NULL);
	*ppDevice = gsDeviceInfo ;
	*pDeviceNum = DEVICE_TYPE_NUM ;
	return TRUE ;
}

/****************************************************************************
*   Name
		ConvertUserConfigToVar
*	Type
		public
*	Function
		Convert the variable string to KINGVIEW's structural variables (PLCVAR, see "DATATYPE.h").
*	Return Value
		return 0 if successful; otherwise retrun 1 if failed. 
*	Parameters
		lpDbItem
			[in]  Pointer to a MiniDbItem variant
			[in,out] Pointer to a PLCVAR variant
*****************************************************************************/
WORD CDevVDR::ConvertUserConfigToVar( LPVOID lpDbItemItem, LPVOID lpVar)  //
{
	
	ASSERT(lpDbItemItem != NULL);
	ASSERT(lpVar != NULL);
	
	MiniDbItem * pDbItem = (MiniDbItem*)lpDbItemItem;
	PPLCVAR pPlcVar = (PPLCVAR) lpVar;
	/*
	Notes:
	This function is to convert the variable string to KINGVIEW's structural variables
	we advise you not to modify for common drivers.
	*/

	REG_INFO * pRegInfo = NULL;
	int nRegNum = 0 ;
	GetRegisters( (LPVOID*)&pRegInfo, &nRegNum );
	//Initialization 
	CString szRegister(pDbItem->szRegister);
	//Delete blank characters 
	szRegister.TrimLeft();
	szRegister.TrimRight();
	if (szRegister.IsEmpty())
	{		
		m_pPro->m_nLastErrorCode = ERR_REG_NAME;
		return 1;
	}
	//Not to access the Maximum length supported.
	if (szRegister.GetLength() > MAX_CONFIG_STRING_LENGTH)
	{
		//m_pPro->m_nLastErrorCode = ERR_CONFIG_MAXLEN;
		m_pPro->m_nLastErrorCode = ERR_REG_NAME;
		return 1;
	}	
	
	//if found
	BOOL bMatched = FALSE;
	//get the index number of register
	int  nRegIndex = 0;
	do
	{	
		//if found the register string
		if (szRegister.Find(pRegInfo[nRegIndex].sRegName) == 0 )
		{
			bMatched = TRUE;
		}
		
	}while (!bMatched && ++nRegIndex < nRegNum );
	
	if (!bMatched)
	{
		m_pPro->m_nLastErrorCode = ERR_REG_NAME;
		return 1;
	}	
	
	
	//get channel string
	CString szChannelConfig = szRegister.Right(szRegister.GetLength() \
		- (int)_tcslen(pRegInfo[nRegIndex].sRegName));
	
	//Allowed characters : 0123456789ABCDEF.:
	CString szAllow(_T("0123456789ABCDEF.:"));
	
	//Check all characters.
	for (int nIndex = 0; nIndex < szChannelConfig.GetLength(); nIndex++)
	{
		
		if (szAllow.Find(szChannelConfig[nIndex]) == -1)
		{
			m_pPro->m_nLastErrorCode = ERR_REG_NAME;
			return 1;
		}
	}
	
	
	//check the string format
	if(pRegInfo[nRegIndex].nLowIndex == pRegInfo[nRegIndex].nUpperIndex)
	{		
		if (pRegInfo[nRegIndex].nLowIndex != 0)
		{
			if(szChannelConfig.IsEmpty())
			{
				m_pPro->m_nLastErrorCode = ERR_REG_CH;
				return 1;
			}
		}
	}	

	//get the channel(offset) number
	int nNo = _ttoi(szChannelConfig);

	//if low the index number less than the high index number
	if(pRegInfo[nRegIndex].nLowIndex < pRegInfo[nRegIndex].nUpperIndex)
	{		

		if(nNo < pRegInfo[nRegIndex].nLowIndex) 
		{
			//if not in the range
			m_pPro->m_nLastErrorCode = ERR_REG_CH;
			return 1; 					
		}	
		else if(nNo >pRegInfo[nRegIndex].nUpperIndex)
		{

			m_pPro->m_nLastErrorCode = ERR_REG_CH;
			return 1; 
		}
		else
		{
			//success
			pPlcVar -> nNo = nNo;								
		}			
	}

	else if (pRegInfo[nRegIndex].nLowIndex ==  0 && pRegInfo[nRegIndex].nUpperIndex == 0)
	{
		if(nNo > 0)  
		{
			m_pPro->m_nLastErrorCode = ERR_REG_CH;
			return 1;
		}
		else if(nNo < 0)
		{
			m_pPro->m_nLastErrorCode = ERR_REG_CH;
			return 1;
		}
		//default
		pPlcVar->nNo = 0;		
	}
	else
	{
		m_pPro->m_nLastErrorCode = ERR_REG_FORMAT;
		return 1;
	}
	
	pPlcVar->nRegType = nRegIndex;						
	pPlcVar->pszRegName = pRegInfo[nRegIndex].sRegName;
	
	if(pDbItem->nDataType & pRegInfo[nRegIndex].wDataType)
	{
		pPlcVar->nDataType = pDbItem->nDataType;		
		return 0;
	}			
	else
	{				
		m_pPro->m_nLastErrorCode =  ERR_REG_DATATYPE;
		return 1;				
	}
	return 0;
}

/****************************************************************************
*   Name
		AddVarToPacket
*	Type
		public
*	Function
		Whether or not the var can add to the lpPacket
*	Return Value
		return true if the lpVar can be added into the lpPacket;otherwise false.
*	Parameters
		lpVar 
			[in]  Pointer to a PLCVAR variant whether or not add into the packet
		nVarAccessType
			[in]  Specifies the lpvar Access Type.(NO USED)
		lpPacket
			[int] Pointer to a PACKET variant whether or not contain the lpVar
*****************************************************************************/
BOOL CDevVDR::AddVarToPacket( LPVOID lpVar, int nVarAccessType, LPVOID lpPacket)
{
	CDebug::ShowFunMessage(_T("CDevVDR::AddVarToPacket"));
	PPACKET pPac = (PACKET *)lpPacket;
	PPLCVAR pVar = (PLCVAR *)lpVar;
	/*
	Add you code here
	the following code for your reference.
	*/

	//total vars in the packet
	int nTotalNo;
	//only if var type, address, data types are all the same
	if((nVarAccessType == pPac->nPacketType)&&(pPac->nUnitNo == pVar->nUnitNo)
		&& (pPac->nRegType == pVar->nRegType))
	{
		
		if ( nVarAccessType == PT_WRITE)
		{
			if ( pVar->nNo == pPac->nStartNo)  
			{
				return TRUE;
			}
			else  
			{
				return FALSE;
			}
		}
		//read
		//if var channel less than the packet start No
		if ( pVar->nNo < pPac->nStartNo )
		{//chart:		-----pVar->nNo-------pPac->Start================pPac->End----------
			//get the channel rangle
			nTotalNo = pPac->nEndNo - pVar->nNo + 1;
			//MAX_PACKET_NUM: the allowed Max var numbers in a packet
			
			if ( nTotalNo <= MAX_PACKET_NUM )
			{
				pPac->nStartNo = pVar->nNo;	
				return TRUE;
			}
		}
		//if var channel larger than the packet start No
		else if ( pVar->nNo > pPac->nEndNo )
		{//chart:------------pPac->Start================pPac->End----pVar->nNo------
				//get the channel rangle
			nTotalNo = pVar->nNo - pPac->nStartNo + 1;
			
			if ( nTotalNo <= MAX_PACKET_NUM )
			{
				pPac->nEndNo = pVar->nNo;
				return TRUE;
			}
		}
		else
		{//chart:------------pPac->Start====pVar->nNo============pPac->End----------
			return TRUE;
		}
		//
		//chart:------------pPac->Start====pVar->nNo============pPac->End----------
	}	
	return FALSE;
}

/****************************************************************************
*   Name
		ProcessPacket2
*	Type
		public
*	Function
		Processing of data packets in 6.0 interface (new interface in brief), must be implemented.
*	Return Value
		return true if successful; otherwise false.
*	Parameters
		lpPacket
			[in,out] Pointer a PACKET variable.
*****************************************************************************/
BOOL CDevVDR::ProcessPacket2( LPVOID lpPacket )
{
	CDebug::ShowFunMessage(_T("CDevVDR::ProcessPacket2"));
	
	ASSERT(lpPacket != NULL);
	PPACKET pPac = (PPACKET)lpPacket;
	ZeroMemory(m_bySndBuf, sizeof(m_bySndBuf));
	ZeroMemory(m_byRecBuf, sizeof(m_byRecBuf));
	if( !AssertPacketProperty(pPac) )
	{
		return FALSE;
	}

    int nSendLen = 0;                                //send length
	int nExpectedLen = 0;                            //expect length to receive
	int nRecLen = 0;                                 //received lenght
	
	int nSendTimes = GetSendTimes(pPac);
	int  nDealedTime = 1;
	do
	{
		if (!GetSendString(pPac, nSendLen, nExpectedLen, nDealedTime))
		{
			CDebug::ShowErroMessage(_T("Get send string error!"));
			return FALSE;
		}
		
		if (!Transmission(nSendLen, nExpectedLen,  nRecLen))
		{
			CDebug::ShowErroMessage(_T("Transmission error!"));
			return FALSE;
		}
		
		nDealedTime++; 
		
	}while (nDealedTime <= nSendTimes);
	//send correct data to KingView
	if (pPac->nPacketType == PT_READ) 
	{
		return SendDataToKingView(pPac,nExpectedLen);
	}

	return TRUE;
}


/****************************************************************************
*   Name
		TryConnect
*	Type
		public
*	Function
		Try to connect with the Device when the communication failed.
*	Return Value
		return true if successful; otherwise false.
*	Parameters
		pDeviceName
			[in] Pointer to a null-terminated string that identifies the 
			specified pDeviceName.
		nUnitAddr
			[in]  Specifies the Unit Address 
		lpDevAddr
			[in]  Point to a DEVADDR variant 
*	Remarks
		When "processpacket" failed, tryconnect twice,
		if failed again, the touchvew will connect with setted time. 
*****************************************************************************/
BOOL CDevVDR::TryConnect( const TCHAR*  szDeviceName, int nUnitAddr, LPVOID lpDevAddr )
{
	CDebug::ShowFunMessage(_T("CDevVDR::TryConnect"));
	ASSERT( m_pSerialCom);
	ASSERT(szDeviceName != NULL);
	ASSERT(lpDevAddr != NULL);
	
		
#if defined(_TRYCONNECT_TRUE)
		//not to try connect, return true directly
	return TRUE;	
#elif defined(_TRYCONNECT_USERDEFINE)
	//try connect by youself 
	DEVADDR *pRealDevAddr = (DEVADDR*)lpDevAddr;
#if defined (_REOPENCOM)
	//re-open the serial port
	m_pSerialCom->ReOpenCom(); 
#endif
	return TryConnectOfSelfDefined(szDeviceName, nUnitAddr, pRealDevAddr);	
#elif defined(_TRYCONNECT_USEPACKET)	
	//try connect by process packet

#if defined (_REOPENCOM)
	m_pSerialCom->ReOpenCom(); 
#endif
	PPACKET   pPac = new PACKET;
	ID_NO2    *pTempIdNo = new ID_NO2;	
	pPac->nUnitNo = nUnitAddr;	
	pTempIdNo->wNo = pPac->nStartNo;
	//return value
	BOOL bRet = FALSE;
	//get the packet paramters
	bRet = GetTryConnectPacketParam(pPac, pTempIdNo, szDeviceName);
	
	if(bRet)
	{
		//added to tail of the vallist 
		pPac->varList.AddTail(pTempIdNo);
		//excute
		bRet = ProcessPacket2(pPac);
	}
	delete   pPac;
	delete   pTempIdNo;
	return   bRet;
#endif
	return TRUE;
}


//
BOOL CDevVDR::TryConnectOfSelfDefined(const TCHAR*  szDeviceName, int iUnitAddr, PDEVADDR pDevAddr)
{
	CDebug::ShowFunMessage(_T("CDevVDR::VTTryConnect"));
	//This function is to realize  try connect by yourself.
	//Add your  handler code here, but a recommend you to use the Transmission function

	return TRUE;	
}
//
BOOL CDevVDR::GetTryConnectPacketParam(PPACKET pPacket, IdNo2* pIdNo, const TCHAR* szDeviceName)
{
	CDebug::ShowFunMessage(_T("CDevVDR::VTGetTryConnectPacketParam"));
	 
	//This function is to build your packet.
	//Add your  handler code here, the following code for your reference
	pPacket->nPacketType = PT_READ;
	pPacket->nRegType = 0;	
	pPacket->nStartNo = 0;
	pPacket->nEndNo = 0;
	
	pIdNo->wDataType = UINT_DATATYPE;
	return TRUE;
}

/****************************************************************************
*   Name
AssertPacketProperty
*	Type
private
*	Function
check if the packet property of R/W conflict with the register property of R/W
*	Return Value
return true if correct; otherwise wrong.
*	Parameters
pPac
[in] Pointer a PACKET variant
*****************************************************************************/
BOOL CDevVDR::AssertPacketProperty(PPACKET pPac)
{
	CDebug::ShowFunMessage(_T("CDevVDR::AssertPacketProperty"));

	ASSERT(pPac != NULL);
	int nRegIndex = pPac->nRegType;
	SHORT nPacType = pPac->nPacketType;
	if(gsRegInfo[nRegIndex].nData == PT_READWRITE )
	{
		return TRUE;
	}
	else if(gsRegInfo[nRegIndex].nData != nPacType)
	{
		CString szErrMes;
		if(gsRegInfo[nRegIndex].nData == PT_WRITE)
		{
			szErrMes.Format(_T("Read %s Register Wrong : %s Register is WRITE_ONLY"), gsRegInfo[nRegIndex].sRegName, gsRegInfo[nRegIndex].sRegName);
		}
		else
		{
			szErrMes.Format(_T("Write %s Register Wrong : %s Register is READ_ONLY"), gsRegInfo[nRegIndex].sRegName, gsRegInfo[nRegIndex].sRegName);
		}
		CDebug::ShowDetailMessage((LPTSTR)(LPCTSTR)szErrMes);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/****************************************************************************
*   Name
		GetSendString
*	Type
		public
*	Function
		Build sending frame according the packet information.
*	Return Value
		return true if successful; otherwise false.
*	Parameters
		pPac
			[in,out] Pointer a PACKET variant
		iLen
			[out] the data's length to send
		iRecLen
			[out] the expect length to receive
		iTimes
			[in] The number to deal packet in a process
			 used for some devices which get the data through query-ack for a few times 
*	Remarks
		When "processpacket" failed, tryconnect twice,
		if failed again, the touchvew will connect with setted time. 
*****************************************************************************/
BOOL CDevVDR::GetSendString(PPACKET pPac,int& nLen, int& nRecLen, int nTimes)
{
	CDebug::ShowFunMessage(_T("CDevVDR::GetSendString"));
	ASSERT(pPac != NULL);

	POSITION pos =  pPac->varList.GetHeadPosition();

	CString szErrMes;

	while(pos)
	{
		ID_NO2 * pIdNo = static_cast<ID_NO2*>(pPac->varList.GetNext(pos));
		char buffer[100] = {0};

		int nRegIndex = pPac->nRegType;

		//发报警 XYALR
		if (nRegIndex == 0) 
		{
			long data = pIdNo->plcValue.intVal;
			GetPacRule(buffer, data, nLen);
		} 
		//发模拟量 XYALV
		else if (nRegIndex == 1) 
		{
			long data = pIdNo->plcValue.longVal;
			GetAnaylogPacRule(buffer, data, nLen);
		}
		else 
		{
			return FALSE;
		}
		
		memset(m_bySndBuf, 0, sizeof(m_bySndBuf));
		memcpy(m_bySndBuf, buffer, nLen);
	}

	return TRUE;
}

/****************************************************************************
*   Name
		SendDataToKingView
*	Type
		public
*	Function
		Send Data To KingView
*	Return Value
		return true if successful; otherwise false.
*	Parameters
		pPac
			[in,out] Pointer a PACKET variant
		iLen
			[out] the received length 
*****************************************************************************/
BOOL CDevVDR::SendDataToKingView(PPACKET pPac, int nLen)
{
	CDebug::ShowFunMessage(_T("CDevVDR::SendDataToKingView"));
	ASSERT(pPac != NULL);
	//
	POSITION pos =  pPac->varList.GetHeadPosition();
	
	while(pos)
	{
		ID_NO2 * pIdNo = static_cast<ID_NO2*>(pPac->varList.GetNext(pos));

/*
Add your  handler code here,
The pIdNo is the pointer to  KingView variables in the packet.
About the variable's data types, see "datatype.h" 
You can also call the SendDataToIdNo function to realize this,
you can see the demo example for reference.
*/
	}

	return TRUE;
}


/****************************************************************************
*   Name
		Transmission
*	Type
		public
*	Function
		Send Data to serial port and receive data from serial port
*	Return Value
		return true if successful; otherwise false.
*	Parameters
		iLen
			[in] The length to send
		iExpectedLen
			[in,out] the expected length to receive
		iRecLen
			[in,out] the received length 
*****************************************************************************/
BOOL CDevVDR::Transmission(int nLen, int nExpectedLen, int& nRecLen)
{
	CDebug::ShowFunMessage(_T("CDevVDR::Transmission"));	
	/*
	Notes:
	This function is to realize to send data to serial port and receive data from serial port.
	This is a important function so we advise you not to modify strongly.
	*/
	//Clear the serial com
	PurgeComm(m_pSerialCom->m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	//send data to serial com
	if (!m_pSerialCom->PhysicalSend(m_bySndBuf,nLen))
	{
		return FALSE;
	}	
	
	return TRUE;
	
}

/****************************************************************************
*   Name
		GetFirstFrame
*	Type
		public
*	Function
		Get a first frame from the received data
*	Return Value
		return true if successful; otherwise false.
*	Parameters
		nbyteRead
			[in] The length received
		iExpectedLen
			[in] the expected length to receive
		chFrameHead
			[in] the first byte (the flag) of the frame 
*****************************************************************************/
BOOL CDevVDR::GetFirstFrame(int nbyteRead, int nExpectedLen, BYTE byFrameHead)
{
	CDebug::ShowFunMessage(_T("CDevVDR::GetFirstFrame"));
	ASSERT(nbyteRead >= nExpectedLen);
	//Add your modification  handler code here,
	//the following code for reference
	int i = 0;
	do
	{
		if(m_byRecBuf[i] == byFrameHead)
		{
			memcpy(m_byRecBuf, m_byRecBuf+i, nExpectedLen);
			return TRUE;
		}
		i++;
	}while(i < nbyteRead);
	return FALSE;
}

/****************************************************************************
*   Name
		PreProcessData
*	Type
		public
*	Function
		Pre-Process data when received
*	Return Value
		return true if successful; otherwise false.
*	Parameters
		pPac
			[in,out] Pointer a PACKET variant
		nbyteRead
			[in] The length received
		iExpectedLen
			[in] the expected length to receive
*****************************************************************************/
BOOL CDevVDR::PreProcessData (PPACKET pPac, int nbyteRead ,int nExpectedLen, int nTimes)
{
	CDebug::ShowFunMessage(_T("CDevVDR::PreProcessData"));
	ASSERT(pPac != NULL);
	if (pPac == NULL)
	{
		return FALSE;
	}
/*
Add your  handler code here,
the received frame stored in the received buffer----m_byRecBuf
you can see the demo example for reference.
*/

	return TRUE;
}


/****************************************************************************
*   Name
		GetSendTimes
*	Type
		public
*	Function
		According the regsiter information,
		set the query-acknowledge times in a data process.
*	Return Value
		a integer
*	Parameters
		pPac
			[in,out] Pointer a PACKET variant
*****************************************************************************/
int CDevVDR::GetSendTimes(PPACKET  pPac)//
{
	ASSERT(pPac != NULL);
	int nReturnTimes = 1;	
	//Add your  handler code here
	//the default return value is 1

	return nReturnTimes;
	
}

/****************************************************************************
*   Name

*	Type

*	Function
		
*	Return Value
     e.g. $XYALR,150257,013,A*6D
*	Parameters


*****************************************************************************/
BOOL CDevVDR::GetPacRule(char* buffer,int data, int& nLen)
{
	CDebug::ShowFunMessage(_T("CDevVDR::GetPacRule"));
	
	char tm[64] = {0};

	//head
	char* head = "$XYALR";

	//time
	if (!getTimeStr(tm,64)) {
		return FALSE;
	}

	strcpy(buffer, head);
	strcat(buffer,",");
	strcat(buffer, tm);
	strcat(buffer,",");
	
	//number
	int almNo = data % 1000;
	char alm[10] = {0};

	if (!GetCompleLenBuffer2(alm, almNo, 3)) {
		return FALSE;
	}

	strcat(buffer, alm);
	strcat(buffer,",");

	//state
	if (data % 1000 == 0) {
		strcat(buffer, "B");//心跳
	} else {
		if (data / 1000 == 1) {
			strcat(buffer, "A");
		} else {
			strcat(buffer, "V");
		}
	}
	
	//checksum
	BYTE checkSum = CCheck::Xor_Check((BYTE*)(&buffer[1]), 
		strlen(buffer) - sizeof(char));
	BYTE CRC[10] = {0};
	CHelper::ByteToASC(CRC, checkSum);
	
	strcat(buffer, "*");
	strcat(buffer, (char*)CRC);
	strcat(buffer, "\r\n");

	nLen = strlen(buffer);

	return TRUE;
}

/****************************************************************************
*   Name

*	Type

*	Function
		
*	Return Value
		
*	Parameters

*****************************************************************************/
BOOL CDevVDR::getTimeStr(char* tm, int len)
{
	time_t t = time(0);
	strftime(tm, len, "%H%M%S", localtime(&t));//时分秒

	return TRUE;
}

/****************************************************************************
*   Name

*	Type

*	Function
		
*	Return Value
     e.g. $XYALV,150257,013,12345*6D
*	Parameters


*****************************************************************************/
BOOL CDevVDR::GetAnaylogPacRule(char* buffer,long data, int& nLen)
{
	CDebug::ShowFunMessage(_T("CDevVDR::GetAnaylogPacRule"));
	
	char tm[64] = {0};

	//head
	char* head = "$XYALV";

	//time
	if (!getTimeStr(tm,64)) {
		return FALSE;
	}

	strcpy(buffer, head);
	strcat(buffer,",");
	strcat(buffer, tm);
	strcat(buffer,",");

	//number
	long almNo = data / 100000;
	char alm[10] = {0};

	if (!GetCompleLenBuffer(alm, almNo, 3)) {
		return FALSE;
	}

	strcat(buffer, alm);
	strcat(buffer,",");

	//value
	char value[10] = {0};
	if (!GetCompleLenBuffer(value, data%100000, 5)) {
		return FALSE;
	}

	strcat(buffer, value);
	
	//checksum
	BYTE checkSum = CCheck::Xor_Check((BYTE*)(&buffer[1]), 
		strlen(buffer) - sizeof(char));
	BYTE CRC[10] = {0};
	CHelper::ByteToASC(CRC, checkSum);
	
	strcat(buffer, "*");
	strcat(buffer, (char*)CRC);
	strcat(buffer, "\r\n");

	nLen = strlen(buffer);

	return TRUE;
}

/****************************************************************************
*   Name

*	Type

*	Function
		
*	Return Value
		
*	Parameters

*****************************************************************************/
BOOL CDevVDR::GetCompleLenBuffer(char* buffer, long data, int len)
{
	if (len >= 10) {
		return FALSE;
	}

	char temp[10] = {0}, result[10] = {0};
	ltoa(data, temp, 10);

	int templen = len - strlen(temp);
	
	char* zero = "0\0";
	for (int i = 0; i < templen; i++) {
		strcat(result, zero);
	}

	strcat(result, temp);
	memcpy(buffer, result, len);

	return TRUE;
}

/****************************************************************************
*   Name

*	Type

*	Function
		
*	Return Value
		
*	Parameters

*****************************************************************************/
BOOL CDevVDR::GetCompleLenBuffer2(char* buffer, int data, int len)
{
	if (len >= 10) {
		return FALSE;
	}

	char temp[10] = {0}, result[10] = {0};
	itoa(data, temp, 10);

	int templen = len - strlen(temp);
	
	char* zero = "0\0";
	for (int i = 0; i < templen; i++) {
		strcat(result, zero);
	}

	strcat(result, temp);
	memcpy(buffer, result, len);

	return TRUE;
}
















