#include "NetvarManager.hpp"

using namespace std;

namespace CSGO
{
	CNetVars::CNetVars()
	{
		m_tables.clear();

		ClientClass *clientClass = g_pClient->GetClientClasses();
		if (!clientClass)
			return;

		while (clientClass)
		{
			RecvTable *recvTable = clientClass->GetTable();
			m_tables.push_back(recvTable);

			clientClass = clientClass->NextClass();
		}
	}

	// calls GetProp wrapper to get the absolute offset of the prop
	DWORD CNetVars::GetOffset(const char *tableName, const char *propName)
	{
		DWORD dwOffset = GetProperty(tableName, propName);
		return dwOffset;
	}

	// calls GetProp wrapper to get prop and sets the proxy of the prop
	bool CNetVars::HookProp(const char *tableName, const char *propName, RecvVarProxyFn fun)
	{
		RecvProp *recvProp = 0;
		GetProperty(tableName, propName, &recvProp);
		if (!recvProp)
			return false;

		recvProp->SetProxyFn(fun);

		return true;
	}

	// wrapper so we can use recursion without too much performance loss
	DWORD CNetVars::GetProperty(const char *tableName, const char *propName, RecvProp **prop)
	{
		RecvTable *recvTable = GetTable(tableName);
		if (!recvTable)
			return 0;

		int offset = GetProperty(recvTable, propName, prop);
		if (!offset)
			return 0;

		return offset;
	}

	// uses recursion to return a the relative offset to the given prop and sets the prop param
	DWORD CNetVars::GetProperty(RecvTable *recvTable, const char *propName, RecvProp **prop)
	{
		int extraOffset = 0;
		for (int i = 0; i < recvTable->m_nProps; ++i)
		{
			RecvProp *recvProp = &recvTable->m_pProps[i];
			RecvTable *child = recvProp->GetDataTable();

			if (child && (child->m_nProps > 0))
			{
				int tmp = GetProperty(child, propName, prop);
				if (tmp)
					extraOffset += (recvProp->GetOffset() + tmp);
			}

			if (_strcmpi(recvProp->m_pVarName, propName))
				continue;

			if (prop)
				*prop = recvProp;

			return (recvProp->GetOffset() + extraOffset);
		}

		return extraOffset;
	}

	RecvTable *CNetVars::GetTable(const char *tableName)
	{
		if (m_tables.empty())
			return 0;

		for each (RecvTable *table in m_tables)
		{
			if (!table)
				continue;

			if (_strcmpi(table->m_pNetTableName, tableName) == 0)
				return table;
		}

		return 0;
	}
}