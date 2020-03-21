/************************************************
Copyright(c) 2006, AsiaControl Company
All rights reserved.
File Name:   DevVDR.h
Class Name:  CDevVDR
Brief:       Device sub class
History: 
Date	         Author  	       Remarks
Aug. 2006	     Develop Dep.1     
************************************************/
#pragma once

class CDevVDR: public CDevBase
{
	DECLARE_DYNCREATE(CDevVDR)
public:
	CDevVDR(void);
	~CDevVDR(void);
	
	virtual BOOL GetRegisters(LPVOID *ppReg, int *pRegNum ); 
	virtual BOOL GetDevices(LPVOID *ppDevice, int *pDeviceNum);
	virtual WORD ConvertUserConfigToVar( LPVOID lpDbItemItem, LPVOID lpVar);
	virtual BOOL AddVarToPacket( LPVOID lpVar, int nVarAccessType, LPVOID lpPacket);
	virtual BOOL ProcessPacket2( LPVOID lpPacket );
	virtual BOOL TryConnect( const TCHAR*  szDeviceName, int nUnitAddr, LPVOID lpDevAddr );
		
private:
	int GetSendTimes(PPACKET  pPac);
	BOOL GetSendString(PPACKET pPac, int& nLen, int& nRecLen, int nTimes); 
	BOOL SendDataToKingView(PPACKET pPac, int nLen); 
	BOOL TryConnectOfSelfDefined(const TCHAR*  szDeviceName, int nUnitAddr, PDEVADDR pDevAddr); 
	BOOL GetTryConnectPacketParam(PPACKET pPacket, IdNo2* pIdNo, const TCHAR* szDeviceName);
	BOOL AssertPacketProperty(PPACKET pPac);
	BOOL Transmission(int nLen, int nExpectedLen, int& nRecLen); 
	BOOL PreProcessData (PPACKET pPac, int nbyteRead,int nExpectedLen, int nTimes);
	BOOL GetFirstFrame(int nbyteRead, int nExpectedLen, BYTE byFrameHead);
	
	BOOL GetPacRule(char* buffer,int data, int& nLen);
	BOOL getTimeStr(char* tm, int len);

	BOOL GetAnaylogPacRule(char* buffer,long data, int& nLen);
	BOOL GetCompleLenBuffer(char* buffer, long data, int len);
	BOOL GetCompleLenBuffer2(char* buffer, int data, int len);
};
