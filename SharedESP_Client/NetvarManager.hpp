#pragma once

#include "Main.hpp"
#include "dt_recv2.h"

// Credits to http://marcusthorman.blogspot.com/2012/06/networked-variables-in-source-engine.html

namespace CSGO
{
	class CNetVars
	{
	public:
		CNetVars();
		DWORD GetOffset(const char *tableName, const char *propName);
		bool HookProp(const char *tableName, const char *propName, RecvVarProxyFn fun);
	private:
		DWORD GetProperty(const char *tableName, const char *propName, RecvProp **prop = 0);
		DWORD GetProperty(RecvTable *recvTable, const char *propName, RecvProp **prop = 0);
		RecvTable *GetTable(const char *tableName);
		std::vector<RecvTable*> m_tables;
	};
}