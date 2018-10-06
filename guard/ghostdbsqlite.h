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

#ifndef GHOSTDBSQLITE_H
#define GHOSTDBSQLITE_H

//
// CSQLITE3 (wrapper class)
//
class CSQLITE3
{
private:
	void *m_DB;
	bool m_Ready;
	vector<string> m_Row;

public:
	CSQLITE3( string filename );
	~CSQLITE3( );

	bool GetReady( )			{ return m_Ready; }
	vector<string> *GetRow( )	{ return &m_Row; }
	string GetError( );

	int Prepare( string query, void **Statement );
	int Step( void *Statement );
	int Finalize( void *Statement );
	int Reset( void *Statement );
	int ClearBindings( void *Statement );
	int Exec( string query );
	uint32_t LastRowID( );
};

//
// CGHostDBSQLite
//
class CGHostDBSQLite
{
private:
	string m_File;
	CSQLITE3 *m_DB;

	// we keep some prepared statements in memory rather than recreating them each function call
	// this is an optimization because preparing statements takes time
	// however it only pays off if you're going to be using the statement extremely often

	void *FromAddStmt;
	bool m_HasError;
	string m_Error;

public:
	CGHostDBSQLite( CConfig *CFG );
	virtual ~CGHostDBSQLite( );

	virtual bool Begin( );
	virtual bool Commit( );
	
	virtual bool RunQuery( string query );
	
	virtual bool FromAdd( uint32_t ip1, uint32_t ip2, string country, string fullcountry );
};

#endif
