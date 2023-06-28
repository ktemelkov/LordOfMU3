#ifndef __ProxyBuilder_H
#define __ProxyBuilder_H

#include "Proxy.h"
#include "..\_Shared\LoaderInterface.h"



/**
 * \brief 
 */
class CProxyBuilder
{
public:
	static IConnectionProxy* CreateProxy(IConnectionHandler* pHandler);
};

#endif // __ProxyBuilder_H