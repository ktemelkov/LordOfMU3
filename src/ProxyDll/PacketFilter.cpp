#include "stdafx.h"
#include "PacketFilter.h"

#include "Proxy.h"


/**
 *
 */
CPacketFilter::CPacketFilter(CProxy* pProxy)
	: m_pProxy(pProxy)
{
}

/**
 *
 */
CPacketFilter::~CPacketFilter()
{
}
