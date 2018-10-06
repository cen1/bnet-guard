/*

   Copyright [2008] [Trevor Hogan]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   CODE PORTED FROM THE ORIGINAL GHOST PROJECT: http://ghost.pwner.org/

*/

#include "ghost.h"
#include "util.h"
#include "config.h"
#include "ghostdb.h"
#include "ghostdbsqlite.h"
#include "sqlite3.h"

//
// CQSLITE3 (wrapper class)
//

CSQLITE3 :: CSQLITE3(string filename)
{
	m_Ready = true;

	if (sqlite3_open_v2(filename.c_str(), (sqlite3 **)&m_DB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
		m_Ready = false;
}

CSQLITE3 :: ~CSQLITE3()
{
	sqlite3_close((sqlite3 *)m_DB);
}

string CSQLITE3 :: GetError()
{
	return sqlite3_errmsg((sqlite3 *)m_DB);
}

int CSQLITE3 :: Prepare(string query, void **Statement)
{
	return sqlite3_prepare_v2((sqlite3 *)m_DB, query.c_str(), -1, (sqlite3_stmt **)Statement, NULL);
}

int CSQLITE3 :: Step(void *Statement)
{
	int RC = sqlite3_step((sqlite3_stmt *)Statement);

	if (RC == SQLITE_ROW)
	{
		m_Row.clear();

		for (int i = 0; i < sqlite3_column_count((sqlite3_stmt *)Statement); i++)
		{
			char *ColumnText = (char *)sqlite3_column_text((sqlite3_stmt *)Statement, i);

			if (ColumnText)
				m_Row.push_back(ColumnText);
			else
				m_Row.push_back(string());
		}
	}

	return RC;
}

int CSQLITE3 :: Finalize(void *Statement)
{
	return sqlite3_finalize((sqlite3_stmt *)Statement);
}

int CSQLITE3 :: Reset(void *Statement)
{
	return sqlite3_reset((sqlite3_stmt *)Statement);
}

int CSQLITE3 :: ClearBindings(void *Statement)
{
	return sqlite3_clear_bindings((sqlite3_stmt *)Statement);
}

int CSQLITE3 :: Exec(string query)
{
	return sqlite3_exec((sqlite3 *)m_DB, query.c_str(), NULL, NULL, NULL);
}

uint32_t CSQLITE3 :: LastRowID()
{
	return (uint32_t)sqlite3_last_insert_rowid((sqlite3 *)m_DB);
}

//
// CGHostDBSQLite
//
CGHostDBSQLite :: CGHostDBSQLite(CConfig *CFG)
{
	m_File = CFG->GetString("db_sqlite3_file", "ghost.dbs");
	CONSOLE_Print("[SQLITE3] version " + string(SQLITE_VERSION), true);
	CONSOLE_Print("[SQLITE3] opening database [" + m_File + "]" , true);
	m_DB = new CSQLITE3(m_File);

	if (!m_DB->GetReady()) {
		// setting m_HasError to true indicates there's been a critical error and we want GHost to shutdown
		// this is okay here because we're in the constructor so we're not dropping any games or players
		CONSOLE_Print(string("[SQLITE3] error opening database [" + m_File + "] - ") + m_DB->GetError(), true);
		m_HasError = true;
		m_Error = "error opening database";
		return;
	}

	if (m_DB->Exec("CREATE TEMPORARY TABLE iptocountry (ip1 INTEGER NOT NULL, ip2 INTEGER NOT NULL, country TEXT NOT NULL, fullcountry TEXT NOT NULL, PRIMARY KEY (ip1, ip2))") != SQLITE_OK) {
		CONSOLE_Print("[SQLITE3] error creating temporary iptocountry table - " + m_DB->GetError(), true);
	}

	FromAddStmt = NULL;
}

CGHostDBSQLite :: ~CGHostDBSQLite()
{
	if (FromAddStmt)
		m_DB->Finalize(FromAddStmt);

	CONSOLE_Print("[SQLITE3] closing database [" + m_File + "]" , true);
	delete m_DB;
}

bool CGHostDBSQLite :: Begin()
{
	return m_DB->Exec("BEGIN TRANSACTION") == SQLITE_OK;
}

bool CGHostDBSQLite :: Commit()
{
	return m_DB->Exec("COMMIT TRANSACTION") == SQLITE_OK;
}

bool CGHostDBSQLite :: RunQuery(string query)
{
	bool Success = false;
	sqlite3_stmt *Statement;
	m_DB->Prepare(query, (void **)&Statement);

	if (Statement)
	{
		int RC = m_DB->Step(Statement);

		if (RC == SQLITE_DONE)
			Success = true;
		else if (RC == SQLITE_ERROR)
			CONSOLE_Print("[SQLITE3] error run query - " + m_DB->GetError(), true);

		m_DB->Finalize(Statement);
	}
	else
		CONSOLE_Print("[SQLITE3] prepare error run query - " + m_DB->GetError() , true);

	return Success;
}

bool CGHostDBSQLite :: FromAdd(uint32_t ip1, uint32_t ip2, string country, string fullcountry)
{
	// a big thank you to tjado for help with the iptocountry feature

	bool Success = false;

	if (!FromAddStmt)
		m_DB->Prepare("INSERT INTO iptocountry VALUES (?, ?, ?, ?)", (void **)&FromAddStmt);

	if (FromAddStmt)
	{
		// we bind the ip as an int64 because SQLite treats it as signed

		sqlite3_bind_int64((sqlite3_stmt *)FromAddStmt, 1, ip1);
		sqlite3_bind_int64((sqlite3_stmt *)FromAddStmt, 2, ip2);
		sqlite3_bind_text((sqlite3_stmt *)FromAddStmt, 3, country.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text((sqlite3_stmt *)FromAddStmt, 4, fullcountry.c_str(), -1, SQLITE_TRANSIENT);

		int RC = m_DB->Step(FromAddStmt);

		if (RC == SQLITE_DONE)
			Success = true;
		else if (RC == SQLITE_ERROR)
			CONSOLE_Print("[SQLITE3] error adding iptocountry [" + UTIL_ToString(ip1) + " : " + UTIL_ToString(ip2) + " : " + country + "] - " + m_DB->GetError(), true);

		m_DB->Reset(FromAddStmt);
	}
	else
		CONSOLE_Print("[SQLITE3] prepare error adding iptocountry [" + UTIL_ToString(ip1) + " : " + UTIL_ToString(ip2) + " : " + country + "] - " + m_DB->GetError() , true);

	return Success;
}