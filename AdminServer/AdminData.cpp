#include "AdminData.h"

AdminData * AdminData::m_instance = NULL;

AdminData * AdminData::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new AdminData();
	}

	return m_instance;
}
