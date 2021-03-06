//////////////////////////////////////////////////////
// FILE  : MibAccess.h
//
//

#ifndef _SNMP_ACCESS_H_
#define _SNMP_ACCESS_H_

#include <snmp.h>

#define SNMP_FUNC_TYPE                          WINAPI

//////////////////////////////////////////////////////////////
// Defenitoin of pointers to the four functions in the Mib Dll
//
typedef BOOL (WINAPI *pSnmpExtensionInit)(
										  IN  DWORD               dwTimeZeroReference,
										  OUT HANDLE              *hPollForTrapEvent,
										  OUT AsnObjectIdentifier *supportedView);

typedef BOOL (WINAPI *pSnmpExtensionTrap)(
										  OUT AsnObjectIdentifier *enterprise,
										  OUT AsnInteger          *genericTrap,
										  OUT AsnInteger          *specificTrap,
										  OUT AsnTimeticks        *timeStamp,
										  OUT RFC1157VarBindList  *variableBindings);

typedef BOOL (WINAPI *pSnmpExtensionQuery)(
										   IN BYTE                   requestType,
										   IN OUT RFC1157VarBindList *variableBindings,
										   OUT AsnInteger            *errorStatus,
										   OUT AsnInteger            *errorIndex);

typedef BOOL (WINAPI *pSnmpExtensionInitEx)(OUT AsnObjectIdentifier *supportedView);

typedef VOID (SNMP_FUNC_TYPE *pSnmpUtilVarBindFree)
					(SnmpVarBind * pVb);
typedef SNMPAPI (SNMP_FUNC_TYPE *pSnmpUtilOidNCmp)
					(AsnObjectIdentifier * pOid1,
					AsnObjectIdentifier * pOid2,
					UINT nSubIds);
typedef SNMPAPI (SNMP_FUNC_TYPE *pSnmpUtilOidCpy)
					(AsnObjectIdentifier * pOidDst,
					AsnObjectIdentifier * pOidSrc);

class MibExtLoad
{
public:
	MibExtLoad(LPSTR MibDllName, LPSTR SnmpDllName);
	~MibExtLoad();
	BOOL Init(DWORD dwTimeZeroReference,HANDLE *hPollForTrapEvent,AsnObjectIdentifier *supportedView);
	BOOL InitEx(AsnObjectIdentifier *supportedView);
	BOOL Query(BYTE requestType,OUT RFC1157VarBindList *variableBindings,
			AsnInteger *errorStatus,AsnInteger *errorIndex);

	BOOL Trap(AsnObjectIdentifier *enterprise, AsnInteger *genericTrap, 
		AsnInteger *specificTrap, AsnTimeticks *timeStamp, 
		RFC1157VarBindList  *variableBindings);

private:	
	HINSTANCE				m_hInst;
	pSnmpExtensionInit		m_Init;
	pSnmpExtensionInitEx	m_InitEx;
	pSnmpExtensionQuery		m_Query;
	pSnmpExtensionTrap		m_Trap;

protected:
	HINSTANCE				m_hSnmp;
	pSnmpUtilVarBindFree	m_SnmpUtilVarBindFree;
	pSnmpUtilOidNCmp		m_SnmpUtilOidNCmp;
	pSnmpUtilOidCpy			m_SnmpUtilOidCpy;
};


class MibII : public MibExtLoad
{
public:
	MibII();
	~MibII();
	BOOL Init();
	BOOL GetIPAddress(UINT IpArray[],UINT &IpArraySize);
	BOOL GetIPMask(UINT IpArray[],UINT &IpArraySize);
};


#endif
