#pragma once

#include "../_Shared/ProtocolSettings.h"



/**  
 * \brief 
 */
class CApplication
{
public:
	CApplication();
	virtual ~CApplication();

	int Run();

private:
	CProtocolSettings m_cSettings;
};