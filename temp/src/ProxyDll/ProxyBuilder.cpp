#include "stdafx.h"
#include "ProxyBuilder.h"
#include "GameProxy.h"
#include "PacketLogger.h"
#include "PacketDecryptFilter.h"
#include "PacketEncryptFilter.h"
#include "GameCommands.h"
#include "DebugOut.h"
#include "AutoPickupFilter.h"
#include "AutosayFilter.h"
#include "CharInfoFilter.h"
#include "MultihitPacketFilter.h"
#include "FastMoveFilter.h"
#include "AutoKillFilter.h"
#include "ScriptProcessorFilter.h"
#include "ReflectPacketFilter.h"
#include "RecordRoutePacketFilter.h"
#include "ProtocolDetectFilter.h"
#include "AutoLoginFilter.h"
#include "AutoPotFilter.h"
#include "InventoryManagerFilter.h"




/**
 * \brief 
 */
IConnectionProxy* CProxyBuilder::CreateProxy(IConnectionHandler* pHandler)
{
	CGameProxy* pProxy = new CGameProxy(pHandler);

//	pProxy->AddFilter(new CAutoLoginFilter(pProxy));
	pProxy->AddFilter(new CProtocolDetectFilter(pProxy));
	pProxy->AddFilter(new CPacketDecryptFilter(pProxy));
	pProxy->AddFilter(new CPacketLogger(pProxy));


	pProxy->AddFilter(new CCharInfoFilter(pProxy));
	pProxy->AddFilter(new CGameCommands(pProxy));
	pProxy->AddFilter(new CAutoPickupFilter(pProxy));
	pProxy->AddFilter(new CAutosayFilter(pProxy));
	pProxy->AddFilter(new CScriptProcessorFilter(pProxy));


	pProxy->AddFilter(new CAutoKillFilter(pProxy));
	pProxy->AddFilter(new CMultihitPacketFilter(pProxy));
	pProxy->AddFilter(new CFastMoveFilter(pProxy));
	pProxy->AddFilter(new CReflectPacketFilter(pProxy));
	pProxy->AddFilter(new CRecordRoutePacketFilter(pProxy));
	pProxy->AddFilter(new CInventoryManagerFilter(pProxy));
	pProxy->AddFilter(new CAutoPotFilter(pProxy));

	pProxy->AddFilter(new CPacketEncryptFilter(pProxy));

	return (IConnectionProxy*)pProxy;
}

