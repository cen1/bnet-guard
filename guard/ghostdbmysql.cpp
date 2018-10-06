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
#include "ghostdbmysql.h"

#include <signal.h>

#ifdef WIN32
 #include <winsock.h>
#endif

#include <mysql.h>
#include <boost/thread.hpp>

//
// CGHostDBMySQL
//

CGHostDBMySQL :: CGHostDBMySQL( CConfig *CFG ) : CGHostDB( CFG )
{
	m_Server = CFG->GetString( "db_mysql_server", string( ) );
	m_Database = CFG->GetString( "db_mysql_database", "ghost" );
	m_User = CFG->GetString( "db_mysql_user", string( ) );
	m_Password = CFG->GetString( "db_mysql_password", string( ) );
	m_Port = CFG->GetInt( "db_mysql_port", 0 );
	m_BotID = CFG->GetInt( "db_mysql_botid", 0 );
	m_NumConnections = 1;
	m_OutstandingCallables = 0;

	mysql_library_init( 0, NULL, NULL );

	// create the first connection

	CONSOLE_Print( "[MYSQL] connecting to database server" , true);
	MYSQL *Connection = NULL;

	if( !( Connection = mysql_init( NULL ) ) )
	{
		CONSOLE_Print( string( "[MYSQL] " ) + mysql_error( Connection ), true );
		m_HasError = true;
		m_Error = "error initializing MySQL connection";
		return;
	}

	my_bool Reconnect = true;
	mysql_options( Connection, MYSQL_OPT_RECONNECT, &Reconnect );

	if( !( mysql_real_connect( Connection, m_Server.c_str( ), m_User.c_str( ), m_Password.c_str( ), m_Database.c_str( ), m_Port, NULL, 0 ) ) )
	{
		CONSOLE_Print( string( "[MYSQL] " ) + mysql_error( Connection ) , true);
		m_HasError = true;
		m_Error = "error connecting to MySQL server";
		return;
	}

	m_IdleConnections.push( Connection );
}


CGHostDBMySQL :: ~CGHostDBMySQL( )
{
	CONSOLE_Print( "[MYSQL] closing " + UTIL_ToString( m_IdleConnections.size( ) ) + "/" + UTIL_ToString( m_NumConnections ) + " idle MySQL connections" , true);

	while( !m_IdleConnections.empty( ) )
	{
		mysql_close( (MYSQL *)m_IdleConnections.front( ) );
		m_IdleConnections.pop( );
	}

	if( m_OutstandingCallables > 0 )
		CONSOLE_Print( "[MYSQL] " + UTIL_ToString( m_OutstandingCallables ) + " outstanding callables were never recovered", true );

	mysql_library_end( );
}

string CGHostDBMySQL :: GetStatus( )
{
	return "DB STATUS --- Connections: " + UTIL_ToString( m_IdleConnections.size( ) ) + "/" + UTIL_ToString( m_NumConnections ) + " idle. Outstanding callables: " + UTIL_ToString( m_OutstandingCallables ) + ".";
}

void CGHostDBMySQL :: RecoverCallable( CBaseCallable *callable )
{
	CMySQLCallable *MySQLCallable = dynamic_cast<CMySQLCallable *>( callable );

	if( MySQLCallable )
	{
		if( m_IdleConnections.size( ) > 10 )
		{
			mysql_close( (MYSQL *)MySQLCallable->GetConnection( ) );
			m_NumConnections--;
		}
		else
			m_IdleConnections.push( MySQLCallable->GetConnection( ) );

		if( m_OutstandingCallables == 0 )
			CONSOLE_Print( "[MYSQL] recovered a mysql callable with zero outstanding", true );
		else
			m_OutstandingCallables--;

		if( !MySQLCallable->GetError( ).empty( ) )
			CONSOLE_Print( "[MYSQL] error --- " + MySQLCallable->GetError( ) , true);
	}
	else
		CONSOLE_Print( "[MYSQL] tried to recover a non-mysql callable" , true);
}

void CGHostDBMySQL :: CreateThread( CBaseCallable *callable )
{
	try
	{
		boost :: thread Thread( boost :: ref( *callable ) );
	}
	catch( boost :: thread_resource_error tre )
	{
		CONSOLE_Print( "[MYSQL] error spawning thread on attempt #1 [" + string( tre.what( ) ) + "], pausing execution and trying again in 50ms" , true);
		MILLISLEEP( 50 );

		try
		{
			boost :: thread Thread( boost :: ref( *callable ) );
		}
		catch( boost :: thread_resource_error tre2 )
		{
			CONSOLE_Print( "[MYSQL] error spawning thread on attempt #2 [" + string( tre2.what( ) ) + "], giving up", true );
			callable->SetReady( true );
		}
	}
}

CCallableMailAdd *CGHostDBMySQL :: ThreadedMailAdd( uint32_t id, string server, string sender, string receiver, string message )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableMailAdd *Callable = new CMySQLCallableMailAdd( id, server, sender, receiver, message, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableMailReaded *CGHostDBMySQL :: ThreadedMailReaded( string server, string user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableMailReaded *Callable = new CMySQLCallableMailReaded( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableMailRemove *CGHostDBMySQL :: ThreadedMailRemove( string server, string user, uint32_t id )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableMailRemove *Callable = new CMySQLCallableMailRemove( server, user, id, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableMailList *CGHostDBMySQL :: ThreadedMailList( string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableMailList *Callable = new CMySQLCallableMailList( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}


CCallableInfoCount *CGHostDBMySQL :: ThreadedInfoCount( string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoCount *Callable = new CMySQLCallableInfoCount( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoCheck *CGHostDBMySQL :: ThreadedInfoCheck( string server, string user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoCheck *Callable = new CMySQLCallableInfoCheck( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoAdd *CGHostDBMySQL :: ThreadedInfoAdd( string server, string user, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t pubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message  )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoAdd *Callable = new CMySQLCallableInfoAdd( server, user, lvl, privrank, pubrank, privpoints, pubpoints, admin, country, challwins, challloses, ginfo, message, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoPrivCalculate *CGHostDBMySQL :: ThreadedInfoPrivCalculate( string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoPrivCalculate *Callable = new CMySQLCallableInfoPrivCalculate( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoPubCalculate *CGHostDBMySQL :: ThreadedInfoPubCalculate( string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoPubCalculate *Callable = new CMySQLCallableInfoPubCalculate( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoPrivPoints *CGHostDBMySQL :: ThreadedInfoPrivPoints( string server, string user, uint32_t privpoints )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoPrivPoints *Callable = new CMySQLCallableInfoPrivPoints( server, user, privpoints, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoPubPoints *CGHostDBMySQL :: ThreadedInfoPubPoints( string server, string user, uint32_t pubpoints )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoPubPoints *Callable = new CMySQLCallableInfoPubPoints( server, user, pubpoints, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoChallwins *CGHostDBMySQL :: ThreadedInfoChallwins( string server, string user, uint32_t challwins )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoChallwins *Callable = new CMySQLCallableInfoChallwins( server, user, challwins, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoChallloses *CGHostDBMySQL :: ThreadedInfoChallloses( string server, string user, uint32_t challloses )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoChallloses *Callable = new CMySQLCallableInfoChallloses( server, user, challloses, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoRemove *CGHostDBMySQL :: ThreadedInfoRemove( string server, string user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoRemove *Callable = new CMySQLCallableInfoRemove( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableInfoList *CGHostDBMySQL :: ThreadedInfoList( string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoList *Callable = new CMySQLCallableInfoList( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}
CCallableInfoUpdateCountry *CGHostDBMySQL :: ThreadedInfoUpdateCountry( string server, string user, string country )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoUpdateCountry *Callable = new CMySQLCallableInfoUpdateCountry( server, user, country, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}
CCallableInfoUpdateLvl *CGHostDBMySQL :: ThreadedInfoUpdateLvl( string server, string user, uint32_t lvl, string admin )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoUpdateLvl *Callable = new CMySQLCallableInfoUpdateLvl( server, user, lvl, admin, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}
CCallableInfoUpdateMessage *CGHostDBMySQL :: ThreadedInfoUpdateMessage( string server, string user, uint32_t message  )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoUpdateMessage *Callable = new CMySQLCallableInfoUpdateMessage( server, user, message, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}
CCallableInfoUpdateGinfo *CGHostDBMySQL :: ThreadedInfoUpdateGinfo( string server, string user, string ginfo, string admin )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableInfoUpdateGinfo *Callable = new CMySQLCallableInfoUpdateGinfo( server, user, ginfo, admin, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableWarnCount *CGHostDBMySQL :: ThreadedWarnCount( string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableWarnCount *Callable = new CMySQLCallableWarnCount( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableWarnCheck *CGHostDBMySQL :: ThreadedWarnCheck( string server, string user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableWarnCheck *Callable = new CMySQLCallableWarnCheck( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableWarnAdd *CGHostDBMySQL :: ThreadedWarnAdd( string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableWarnAdd *Callable = new CMySQLCallableWarnAdd( server, user, warnings, warning, totalwarn, daysban, admin, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableWarnUpdateAdd *CGHostDBMySQL :: ThreadedWarnUpdateAdd( string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, string admin )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableWarnUpdateAdd *Callable = new CMySQLCallableWarnUpdateAdd( server, user, warnings, warning, totalwarn, admin, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableWarnChannelBan *CGHostDBMySQL :: ThreadedWarnChannelBan( string server, string user, uint32_t daysban, string admin )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableWarnChannelBan *Callable = new CMySQLCallableWarnChannelBan( server, user, daysban, admin, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableWarnRemove *CGHostDBMySQL :: ThreadedWarnRemove( string server, string user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableWarnRemove *Callable = new CMySQLCallableWarnRemove( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableWarnList *CGHostDBMySQL :: ThreadedWarnList( string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableWarnList *Callable = new CMySQLCallableWarnList( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableRunQuery *CGHostDBMySQL :: ThreadedRunQuery( string query )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableRunQuery *Callable = new CMySQLCallableRunQuery( query, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}
///////////////////////////////////////////////
CCallableBanCount *CGHostDBMySQL :: ThreadedBanCount( string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanCount *Callable = new CMySQLCallableBanCount( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanCheck *CGHostDBMySQL :: ThreadedBanCheck( string server, string user, string ip )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanCheck *Callable = new CMySQLCallableBanCheck( server, user, ip, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanAdd *CGHostDBMySQL :: ThreadedBanAdd( string server, string user, string ip, string gamename, string admin, string reason )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanAdd *Callable = new CMySQLCallableBanAdd( server, user, ip, gamename, admin, reason, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanRemove *CGHostDBMySQL :: ThreadedBanRemove( string server, string user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanRemove *Callable = new CMySQLCallableBanRemove( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanRemove *CGHostDBMySQL :: ThreadedBanRemove( string user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanRemove *Callable = new CMySQLCallableBanRemove( string( ), user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanList *CGHostDBMySQL :: ThreadedBanList( string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanList *Callable = new CMySQLCallableBanList( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableGameAdd *CGHostDBMySQL :: ThreadedGameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableGameAdd *Callable = new CMySQLCallableGameAdd( server, map, gamename, ownername, duration, gamestate, creatorname, creatorserver, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableGamePlayerAdd *CGHostDBMySQL :: ThreadedGamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableGamePlayerAdd *Callable = new CMySQLCallableGamePlayerAdd( gameid, name, ip, spoofed, spoofedrealm, reserved, loadingtime, left, leftreason, team, colour, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableGamePlayerSummaryCheck *CGHostDBMySQL :: ThreadedGamePlayerSummaryCheck( string name )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableGamePlayerSummaryCheck *Callable = new CMySQLCallableGamePlayerSummaryCheck( name, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDotAGameAdd *CGHostDBMySQL :: ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDotAGameAdd *Callable = new CMySQLCallableDotAGameAdd( gameid, winner, min, sec, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDotAPlayerAdd *CGHostDBMySQL :: ThreadedDotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDotAPlayerAdd *Callable = new CMySQLCallableDotAPlayerAdd( gameid, colour, kills, deaths, creepkills, creepdenies, assists, gold, neutralkills, item1, item2, item3, item4, item5, item6, hero, newcolour, towerkills, raxkills, courierkills, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDotAPlayerSummaryCheck *CGHostDBMySQL :: ThreadedDotAPlayerSummaryCheck( string name )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDotAPlayerSummaryCheck *Callable = new CMySQLCallableDotAPlayerSummaryCheck( name, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDotAPlayerKillerCheck *CGHostDBMySQL :: ThreadedDotAPlayerKillerCheck( string name, uint32_t rank)
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDotAPlayerKillerCheck *Callable = new CMySQLCallableDotAPlayerKillerCheck( name, rank, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDotAPlayerFarmerCheck *CGHostDBMySQL :: ThreadedDotAPlayerFarmerCheck( string name, uint32_t rank )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDotAPlayerFarmerCheck *Callable = new CMySQLCallableDotAPlayerFarmerCheck( name, rank, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDownloadAdd *CGHostDBMySQL :: ThreadedDownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDownloadAdd *Callable = new CMySQLCallableDownloadAdd( map, mapsize, name, ip, spoofed, spoofedrealm, downloadtime, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableScoreCheck *CGHostDBMySQL :: ThreadedScoreCheck( string category, string name, string server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableScoreCheck *Callable = new CMySQLCallableScoreCheck( category, name, server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableW3MMDPlayerAdd *CGHostDBMySQL :: ThreadedW3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableW3MMDPlayerAdd *Callable = new CMySQLCallableW3MMDPlayerAdd( category, gameid, pid, name, flag, leaver, practicing, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableW3MMDVarAdd *CGHostDBMySQL :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableW3MMDVarAdd *Callable = new CMySQLCallableW3MMDVarAdd( gameid, var_ints, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableW3MMDVarAdd *CGHostDBMySQL :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableW3MMDVarAdd *Callable = new CMySQLCallableW3MMDVarAdd( gameid, var_reals, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableW3MMDVarAdd *CGHostDBMySQL :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableW3MMDVarAdd *Callable = new CMySQLCallableW3MMDVarAdd( gameid, var_strings, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

void *CGHostDBMySQL :: GetIdleConnection( )
{
	void *Connection = NULL;

	if( !m_IdleConnections.empty( ) )
	{
		Connection = m_IdleConnections.front( );
		m_IdleConnections.pop( );
	}

	return Connection;
}

//
// unprototyped global helper functions
//

string MySQLEscapeString( void *conn, string str )
{
	char *to = new char[str.size( ) * 2 + 1];
	unsigned long size = mysql_real_escape_string( (MYSQL *)conn, to, str.c_str( ), str.size( ) );
	string result( to, size );
	delete [] to;
	return result;
}

vector<string> MySQLFetchRow( MYSQL_RES *res )
{
	vector<string> Result;

	MYSQL_ROW Row = mysql_fetch_row( res );

	if( Row )
	{
		unsigned long *Lengths;
		Lengths = mysql_fetch_lengths( res );

		for( unsigned int i = 0; i < mysql_num_fields( res ); i++ )
		{
			if( Row[i] )
				Result.push_back( string( Row[i], Lengths[i] ) );
			else
				Result.push_back( string( ) );
		}
	}

	return Result;
}

//
// global helper functions
//
bool MySQLMailAdd( void *conn, string *error, uint32_t botid, uint32_t id, string server, string sender, string receiver, string message )
{
	transform( sender.begin( ), sender.end( ), sender.begin( ), (int(*)(int))tolower );
	transform( receiver.begin( ), receiver.end( ), receiver.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscSender = MySQLEscapeString( conn, sender );
	string EscReceiver = MySQLEscapeString( conn, receiver );
	string EscMessage = MySQLEscapeString( conn, message );
	bool Success = false;//NOW( )
	string Query = "INSERT INTO mails ( id, server, sender, receiver, message, readed, date ) VALUES ( "+UTIL_ToString(id)+", '" + EscServer + "', '" + EscSender + "', '" + EscReceiver + "', '" + EscMessage + "', 0, NOW( ) )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLMailReaded( void *conn, string *error, uint32_t botid, string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	string Query = "UPDATE mails SET readed = 1 WHERE receiver = '" + EscUser + "' AND server = '"+EscServer+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}
bool MySQLMailRemove( void *conn, string *error, uint32_t botid, string server, string user, uint32_t id )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	string Query = "DELETE FROM mails WHERE server='" + EscServer + "' AND (receiver='" + EscUser + "' OR sender= '" + EscUser + "') AND id="+UTIL_ToString(id)+"";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

vector<CDBMail *> MySQLMailList( void *conn, string *error, uint32_t botid,  string server )
{
	string EscServer = MySQLEscapeString( conn, server );
	vector<CDBMail *> MailList;
	string Query = "SELECT id, server, sender, receiver, message, readed, date FROM mails WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			while( Row.size( ) == 7 )
			{//id, server, sender, receiver, message, readed, date
				MailList.push_back( new CDBMail( UTIL_ToUInt32(Row[0]), Row[1], Row[2], Row[3], Row[4], UTIL_ToUInt32(Row[5]), Row[6] ) );
				Row = MySQLFetchRow( Result );
			}

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return MailList;
}


uint32_t MySQLInfoCount( void *conn, string *error, uint32_t botid, string server )
{
	string EscServer = MySQLEscapeString( conn, server );
	uint32_t Count = 0;
	string Query = "SELECT COUNT(*) FROM infos WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 1 )
				Count = UTIL_ToUInt32( Row[0] );
			else
				*error = "error counting infos [" + server + "] - row doesn't have 1 column";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Count;
}

CDBInfo *MySQLInfoCheck( void *conn, string *error, uint32_t botid, string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	CDBInfo *Info = NULL;
	string Query;

	Query = "SELECT id, lvl, privrank, pubrank, privpoints, pubpoints, admin, country, challwins, challloses, ginfo, message, date FROM infos WHERE server='" + EscServer + "' AND name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 13 )
				Info = new CDBInfo( UTIL_ToUInt32(Row[0]), server, user, UTIL_ToUInt32(Row[1]), UTIL_ToUInt32(Row[2]), UTIL_ToUInt32(Row[3]),  UTIL_ToUInt32(Row[4]), UTIL_ToUInt32(Row[5]), Row[6], Row[7], UTIL_ToUInt32(Row[8]), UTIL_ToUInt32(Row[9]), Row[10], UTIL_ToUInt32(Row[11]), Row[12] );
			/* else
				*error = "error checking info [" + server + " : " + user + "] - row doesn't have 6 columns"; */

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Info;
}

bool MySQLInfoAdd( void *conn, string *error, uint32_t botid, string server, string user, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t pubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	string EscAdmin = MySQLEscapeString( conn, admin );
	string EscCountry = MySQLEscapeString( conn, country );
	string EscGinfo = MySQLEscapeString( conn, ginfo );	
	bool Success = false;

	string Query = "INSERT INTO infos ( botid, server, name, lvl, privrank, pubrank, privpoints, pubpoints, admin, country, challwins, challloses, ginfo, message, date ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscServer + "', '" + EscUser + "', "+UTIL_ToString(lvl)+", "+UTIL_ToString(privrank)+", "+UTIL_ToString(pubrank)+", "+UTIL_ToString(privpoints)+", "+UTIL_ToString(pubpoints)+", '"+EscAdmin+"', '"+EscCountry+"', "+ UTIL_ToString(challwins) +", "+UTIL_ToString(challloses)+", '"+EscGinfo+"', "+UTIL_ToString(message)+", CURDATE( ) )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLInfoPrivCalculate( void *conn, string *error, uint32_t botid, string server )
{
	string EscServer = MySQLEscapeString( conn, server );
	bool Success = false;
	string QueryCalc="SET @var_name = 0";
	if( mysql_real_query( (MYSQL *)conn, QueryCalc.c_str( ), QueryCalc.size( ) ) != 0 )
	*error = mysql_error( (MYSQL *)conn );
						
	string QueryCalc2="UPDATE infos SET privrank=@var_name := @var_name +1 WHERE server='"+EscServer+"' ORDER BY privpoints DESC";
	if( mysql_real_query( (MYSQL *)conn, QueryCalc2.c_str( ), QueryCalc2.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else 
	{
		CONSOLE_Print("[MySQL]: Priv Rank Culculated", true);
		Success = true;
	}
	return Success;
}

bool MySQLInfoPubCalculate( void *conn, string *error, uint32_t botid, string server )
{
	string EscServer = MySQLEscapeString( conn, server );
	bool Success = false;
	string QueryCalc="SET @var_name = 0";
	if( mysql_real_query( (MYSQL *)conn, QueryCalc.c_str( ), QueryCalc.size( ) ) != 0 )
	*error = mysql_error( (MYSQL *)conn );
						
	string QueryCalc2="UPDATE infos SET pubrank=@var_name := @var_name +1 WHERE server='"+EscServer+"' ORDER BY pubpoints DESC";
	if( mysql_real_query( (MYSQL *)conn, QueryCalc2.c_str( ), QueryCalc2.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else 
	{
		CONSOLE_Print("[MySQL]: Pub Rank Culculated", true);
		Success = true;
	}
	return Success;
}

bool MySQLInfoPrivPoints( void *conn, string *error, uint32_t botid, string server, string user, uint32_t privpoints )
{
		bool Success = false;

	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	transform( server.begin( ), server.end( ), server.begin( ), (int(*)(int))tolower );
	string EscName = MySQLEscapeString( conn, user );
	string EscServer = MySQLEscapeString( conn, server );
	string Query = "UPDATE infos SET privpoints = "+UTIL_ToString(privpoints)+" WHERE name = '" + EscName + "' AND server = '"+EscServer+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		//CONSOLE_Print("[MySQL]: Points added for "+user+" in "+server+".");
		Success = true;
	}

	return Success;
}

bool MySQLInfoPubPoints( void *conn, string *error, uint32_t botid, string server, string user, uint32_t pubpoints )
{
		bool Success = false;

	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	transform( server.begin( ), server.end( ), server.begin( ), (int(*)(int))tolower );
	string EscName = MySQLEscapeString( conn, user );
	string EscServer = MySQLEscapeString( conn, server );
	string Query = "UPDATE infos SET pubpoints = "+UTIL_ToString(pubpoints)+" WHERE name = '" + EscName + "' AND server = '"+EscServer+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		//CONSOLE_Print("[MySQL]: Points added for "+user+" in "+server+".");
		Success = true;
	}

	return Success;
}

bool MySQLInfoChallwins( void *conn, string *error, uint32_t botid, string server, string user, uint32_t challwins )
{
	bool Success = false;

	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	transform( server.begin( ), server.end( ), server.begin( ), (int(*)(int))tolower );
	string EscName = MySQLEscapeString( conn, user );
	string EscServer = MySQLEscapeString( conn, server );
	string Query = "UPDATE infos SET challwins = "+UTIL_ToString(challwins)+" WHERE name = '" + EscName + "' AND server = '"+EscServer+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		CONSOLE_Print("[MySQL]: Channel wins added for "+user+" in "+server+".", true);
		Success = true;
	}

	return Success;
}

bool MySQLInfoChallloses( void *conn, string *error, uint32_t botid, string server, string user, uint32_t challloses )
{
	bool Success = false;

	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	transform( server.begin( ), server.end( ), server.begin( ), (int(*)(int))tolower );
	string EscName = MySQLEscapeString( conn, user );
	string EscServer = MySQLEscapeString( conn, server );
	string Query = "UPDATE infos SET challloses = "+UTIL_ToString(challloses)+" WHERE name = '" + EscName + "' AND server = '"+EscServer+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		CONSOLE_Print("[MySQL]: Channel loses added for "+user+" in "+server+".", true);
		Success = true;
	}

	return Success;
}

bool MySQLInfoRemove( void *conn, string *error, uint32_t botid, string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	string Query = "DELETE FROM infos WHERE server='" + EscServer + "' AND name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

vector<CDBInfo *> MySQLInfoList( void *conn, string *error, uint32_t botid, string server )
{
	string EscServer = MySQLEscapeString( conn, server );
	vector<CDBInfo *> InfoList;
	string Query = "SELECT id, name, lvl, privrank, pubrank, privpoints, pubpoints, admin, country, challwins, challloses, ginfo, message, DATE(date) FROM infos WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			while( Row.size( ) == 14 )
			{
				InfoList.push_back( new CDBInfo( UTIL_ToUInt32(Row[0]),server, Row[1], UTIL_ToUInt32(Row[2]), UTIL_ToUInt32(Row[3]), UTIL_ToUInt32(Row[4]), UTIL_ToUInt32(Row[5]), UTIL_ToUInt32(Row[6]), Row[7], Row[8], UTIL_ToUInt32(Row[9]), UTIL_ToUInt32(Row[10]), Row[11], UTIL_ToUInt32(Row[12]), Row[13] ) );
				Row = MySQLFetchRow( Result );
			}

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return InfoList;
}

bool MySQLInfoUpdateCountry( void *conn, string *error, uint32_t botid, string server, string user, string country )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	string EscCountry = MySQLEscapeString( conn, country );	
	bool Success = false;
	string Query = "UPDATE infos SET country='"+EscCountry+"' WHERE name='" + EscUser + "' AND server='"+EscServer+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLInfoUpdateLvl( void *conn, string *error, uint32_t botid, string server, string user, uint32_t lvl, string admin )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	string EscAdmin = MySQLEscapeString( conn, admin );

	bool Success = false;
	string Query = "UPDATE infos SET lvl="+UTIL_ToString(lvl)+", admin='"+EscAdmin+"' WHERE name='" + EscUser + "' AND server='"+EscServer+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLInfoUpdateMessage( void *conn, string *error, uint32_t botid, string server, string user, uint32_t message )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	string Query = "UPDATE infos SET message="+UTIL_ToString(message)+" WHERE name='" + EscUser + "' AND server='"+EscServer+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLInfoUpdateGinfo( void *conn, string *error, uint32_t botid, string server, string user, string ginfo, string admin )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	string EscGinfo = MySQLEscapeString( conn, ginfo );	
	string EscAdmin = MySQLEscapeString( conn, admin );
	bool Success = false;
	//UPDATE info SET message="+ UTIL_ToString(message) +" WHERE name='" + EscUser + "' AND server='"+EscServer+"'
	string Query = "UPDATE infos SET ginfo='"+EscGinfo+"', admin='"+EscAdmin+"' WHERE name='" + EscUser + "' AND server='"+EscServer+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}


uint32_t MySQLWarnCount( void *conn, string *error, uint32_t botid, string server )
{
	string EscServer = MySQLEscapeString( conn, server );
	uint32_t Count = 0;
	string Query = "SELECT COUNT(*) FROM warns WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 1 )
				Count = UTIL_ToUInt32( Row[0] );
			else
				*error = "error counting warns [" + server + "] - row doesn't have 1 column";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Count;
}

CDBWarn *MySQLWarnCheck( void *conn, string *error, uint32_t botid, string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	CDBWarn *Warn = NULL;
	string Query;

	Query = "SELECT id, warnings, warning, totalwarn, daysban, admin, DATE(date) FROM warns WHERE server='" + EscServer + "' AND name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 7 )
				Warn = new CDBWarn( UTIL_ToUInt32(Row[0]), server, user, UTIL_ToUInt32(Row[1]), Row[2], UTIL_ToUInt32(Row[3]), UTIL_ToUInt32(Row[4]), Row[5], Row[6] );
			/* else
				*error = "error checking warn [" + server + " : " + user + "] - row doesn't have 6 columns"; */

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Warn;
}

bool MySQLWarnAdd( void *conn, string *error, uint32_t botid,  string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	string EscWarning = MySQLEscapeString( conn, warning );
	string EscAdmin = MySQLEscapeString( conn, admin );
	bool Success = false;
	string Query = "INSERT INTO warns ( botid, server, name, warnings, warning, totalwarn, daysban, admin, date) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscServer + "', '" + EscUser + "', " + UTIL_ToString(warnings) + ", '"+EscWarning+"', "+UTIL_ToString(totalwarn)+", "+UTIL_ToString(daysban)+", '"+EscAdmin+"', CURDATE( ))";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLWarnUpdateAdd( void *conn, string *error, uint32_t botid,  string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, string admin )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	string EscWarning = MySQLEscapeString( conn, warning );
	string EscAdmin = MySQLEscapeString( conn, admin );
	bool Success = false;
	//UPDATE infos SET country='"+EscCountry+"' WHERE name='" + EscUser + "' AND server='"+E
	string Query = "UPDATE warns SET warnings = "+UTIL_ToString(warnings)+", warning = '"+EscWarning+"', totalwarn = "+UTIL_ToString(totalwarn)+", admin = '"+EscAdmin+"' WHERE server = '"+EscServer+"' AND name = '"+EscUser+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLWarnChannelBan( void *conn, string *error, uint32_t botid,  string server, string user, uint32_t daysban, string admin )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	string EscAdmin = MySQLEscapeString( conn, admin );
	bool Success = false;
	//UPDATE infos SET country='"+EscCountry+"' WHERE name='" + EscUser + "' AND server='"+E
	string Query = "UPDATE warns SET daysban = "+UTIL_ToString(daysban)+", admin ='"+EscAdmin+"', date = CURDATE( ) WHERE server = '"+EscServer+"' AND name = '"+EscUser+"'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLWarnRemove( void *conn, string *error, uint32_t botid, string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	string Query = "DELETE FROM warns WHERE server='" + EscServer + "' AND name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

vector<CDBWarn *> MySQLWarnList( void *conn, string *error, uint32_t botid, string server )
{
	string EscServer = MySQLEscapeString( conn, server );
	vector<CDBWarn *> WarnList;
	string Query = "SELECT id, name, warnings, warning, totalwarn, daysban, admin, DATE(date) FROM warns WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			while( Row.size( ) == 8 )
			{
				WarnList.push_back( new CDBWarn( UTIL_ToUInt32(Row[0]), server, Row[1], UTIL_ToUInt32(Row[2]), Row[3], UTIL_ToUInt32(Row[4]), UTIL_ToUInt32(Row[5]), Row[6], Row[7] ) );
				Row = MySQLFetchRow( Result );
			}

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return WarnList;
}

bool MySQLRunQuery( void *conn, string *error, uint32_t botid, string query )
{
	//string EscQuery = MySQLEscapeString( conn, query );
	bool Success = false;
	string Query = query;
	//CONSOLE_Print(Query, true);

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}
////////////////////////////////////////////////////////////////
uint32_t MySQLBanCount( void *conn, string *error, uint32_t botid, string server )
{
	string EscServer = MySQLEscapeString( conn, server );
	uint32_t Count = 0;
	string Query = "SELECT COUNT(*) FROM bans WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 1 )
				Count = UTIL_ToUInt32( Row[0] );
			else
				*error = "error counting bans [" + server + "] - row doesn't have 1 column";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Count;
}

CDBBan *MySQLBanCheck( void *conn, string *error, uint32_t botid, string server, string user, string ip )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	string EscIP = MySQLEscapeString( conn, ip );
	CDBBan *Ban = NULL;
	string Query;

	if( ip.empty( ) )
		Query = "SELECT name, ip, DATE(date), gamename, admin, reason FROM bans WHERE server='" + EscServer + "' AND name='" + EscUser + "'";
	else
		Query = "SELECT name, ip, DATE(date), gamename, admin, reason FROM bans WHERE (server='" + EscServer + "' AND name='" + EscUser + "') OR ip='" + EscIP + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 6 )
				Ban = new CDBBan( server, Row[0], Row[1], Row[2], Row[3], Row[4], Row[5] );
			/* else
				*error = "error checking ban [" + server + " : " + user + "] - row doesn't have 6 columns"; */

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Ban;
}

bool MySQLBanAdd( void *conn, string *error, uint32_t botid, string server, string user, string ip, string gamename, string admin, string reason )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	string EscIP = MySQLEscapeString( conn, ip );
	string EscGameName = MySQLEscapeString( conn, gamename );
	string EscAdmin = MySQLEscapeString( conn, admin );
	string EscReason = MySQLEscapeString( conn, reason );
	bool Success = false;
	string Query = "INSERT INTO bans ( botid, server, name, ip, date, gamename, admin, reason ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscServer + "', '" + EscUser + "', '" + EscIP + "', NOW( ), '" + EscGameName + "', '" + EscAdmin + "', '" + EscReason + "' )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLBanRemove( void *conn, string *error, uint32_t botid, string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscServer = MySQLEscapeString( conn, server );
	string EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	string Query = "DELETE FROM bans WHERE server='" + EscServer + "' AND name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLBanRemove( void *conn, string *error, uint32_t botid, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	string EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	string Query = "DELETE FROM bans WHERE name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

vector<CDBBan *> MySQLBanList( void *conn, string *error, uint32_t botid, string server )
{
	string EscServer = MySQLEscapeString( conn, server );
	vector<CDBBan *> BanList;
	string Query = "SELECT name, ip, DATE(date), gamename, admin, reason FROM bans WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			while( Row.size( ) == 6 )
			{
				BanList.push_back( new CDBBan( server, Row[0], Row[1], Row[2], Row[3], Row[4], Row[5] ) );
				Row = MySQLFetchRow( Result );
			}

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return BanList;
}

uint32_t MySQLGameAdd( void *conn, string *error, uint32_t botid, string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver )
{
	uint32_t RowID = 0;
	string EscServer = MySQLEscapeString( conn, server );
	string EscMap = MySQLEscapeString( conn, map );
	string EscGameName = MySQLEscapeString( conn, gamename );
	string EscOwnerName = MySQLEscapeString( conn, ownername );
	string EscCreatorName = MySQLEscapeString( conn, creatorname );
	string EscCreatorServer = MySQLEscapeString( conn, creatorserver );
	string Query = "INSERT INTO games ( botid, server, map, datetime, gamename, ownername, duration, gamestate, creatorname, creatorserver ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscServer + "', '" + EscMap + "', NOW( ), '" + EscGameName + "', '" + EscOwnerName + "', " + UTIL_ToString( duration ) + ", " + UTIL_ToString( gamestate ) + ", '" + EscCreatorName + "', '" + EscCreatorServer + "' )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = (uint32_t)mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

uint32_t MySQLGamePlayerAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	uint32_t RowID = 0;
	string EscName = MySQLEscapeString( conn, name );
	string EscIP = MySQLEscapeString( conn, ip );
	string EscSpoofedRealm = MySQLEscapeString( conn, spoofedrealm );
	string EscLeftReason = MySQLEscapeString( conn, leftreason );
	string Query = "INSERT INTO gameplayers ( botid, gameid, name, ip, spoofed, reserved, loadingtime, `left`, leftreason, team, colour, spoofedrealm ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", '" + EscName + "', '" + EscIP + "', " + UTIL_ToString( spoofed ) + ", " + UTIL_ToString( reserved ) + ", " + UTIL_ToString( loadingtime ) + ", " + UTIL_ToString( left ) + ", '" + EscLeftReason + "', " + UTIL_ToString( team ) + ", " + UTIL_ToString( colour ) + ", '" + EscSpoofedRealm + "' )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = (uint32_t)mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

CDBGamePlayerSummary *MySQLGamePlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	string EscName = MySQLEscapeString( conn, name );
	CDBGamePlayerSummary *GamePlayerSummary = NULL;
	string Query = "SELECT MIN(DATE(datetime)), MAX(DATE(datetime)), COUNT(*), MIN(loadingtime), AVG(loadingtime), MAX(loadingtime), MIN(`left`/duration)*100, AVG(`left`/duration)*100, MAX(`left`/duration)*100, MIN(duration), AVG(duration), MAX(duration) FROM gameplayers LEFT JOIN games ON games.id=gameid WHERE LOWER(name)='" + EscName + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 12 )
			{
				string FirstGameDateTime = Row[0];
				string LastGameDateTime = Row[1];
				uint32_t TotalGames = UTIL_ToUInt32( Row[2] );
				uint32_t MinLoadingTime = UTIL_ToUInt32( Row[3] );
				uint32_t AvgLoadingTime = UTIL_ToUInt32( Row[4] );
				uint32_t MaxLoadingTime = UTIL_ToUInt32( Row[5] );
				uint32_t MinLeftPercent = UTIL_ToUInt32( Row[6] );
				uint32_t AvgLeftPercent = UTIL_ToUInt32( Row[7] );
				uint32_t MaxLeftPercent = UTIL_ToUInt32( Row[8] );
				uint32_t MinDuration = UTIL_ToUInt32( Row[9] );
				uint32_t AvgDuration = UTIL_ToUInt32( Row[10] );
				uint32_t MaxDuration = UTIL_ToUInt32( Row[11] );
				GamePlayerSummary = new CDBGamePlayerSummary( string( ), name, FirstGameDateTime, LastGameDateTime, TotalGames, MinLoadingTime, AvgLoadingTime, MaxLoadingTime, MinLeftPercent, AvgLeftPercent, MaxLeftPercent, MinDuration, AvgDuration, MaxDuration );
			}
			else
				*error = "error checking gameplayersummary [" + name + "] - row doesn't have 12 columns";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return GamePlayerSummary;
}

uint32_t MySQLDotAGameAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
	uint32_t RowID = 0;
	string Query = "INSERT INTO dotagames ( botid, gameid, winner, min, sec ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( winner ) + ", " + UTIL_ToString( min ) + ", " + UTIL_ToString( sec ) + " )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = (uint32_t)mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

uint32_t MySQLDotAPlayerAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills )
{
	uint32_t RowID = 0;
	string EscItem1 = MySQLEscapeString( conn, item1 );
	string EscItem2 = MySQLEscapeString( conn, item2 );
	string EscItem3 = MySQLEscapeString( conn, item3 );
	string EscItem4 = MySQLEscapeString( conn, item4 );
	string EscItem5 = MySQLEscapeString( conn, item5 );
	string EscItem6 = MySQLEscapeString( conn, item6 );
	string EscHero = MySQLEscapeString( conn, hero );
	string Query = "INSERT INTO dotaplayers ( botid, gameid, colour, kills, deaths, creepkills, creepdenies, assists, gold, neutralkills, item1, item2, item3, item4, item5, item6, hero, newcolour, towerkills, raxkills, courierkills ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( colour ) + ", " + UTIL_ToString( kills ) + ", " + UTIL_ToString( deaths ) + ", " + UTIL_ToString( creepkills ) + ", " + UTIL_ToString( creepdenies ) + ", " + UTIL_ToString( assists ) + ", " + UTIL_ToString( gold ) + ", " + UTIL_ToString( neutralkills ) + ", '" + EscItem1 + "', '" + EscItem2 + "', '" + EscItem3 + "', '" + EscItem4 + "', '" + EscItem5 + "', '" + EscItem6 + "', '" + EscHero + "', " + UTIL_ToString( newcolour ) + ", " + UTIL_ToString( towerkills ) + ", " + UTIL_ToString( raxkills ) + ", " + UTIL_ToString( courierkills ) + " )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = (uint32_t)mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

CDBDotAPlayerSummary *MySQLDotAPlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	string EscName = MySQLEscapeString( conn, name );
	CDBDotAPlayerSummary *DotAPlayerSummary = NULL;
	string Query = "SELECT COUNT(dotaplayers.id), SUM(kills), SUM(deaths), SUM(creepkills), SUM(creepdenies), SUM(assists), SUM(neutralkills), SUM(towerkills), SUM(raxkills), SUM(courierkills) FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour WHERE LOWER(name)='" + EscName + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 10 )
			{
				uint32_t TotalGames = UTIL_ToUInt32( Row[0] );

				if( TotalGames > 0 )
				{
					uint32_t TotalWins = 0;
					uint32_t TotalLosses = 0;
					uint32_t TotalKills = UTIL_ToUInt32( Row[1] );
					uint32_t TotalDeaths = UTIL_ToUInt32( Row[2] );
					uint32_t TotalCreepKills = UTIL_ToUInt32( Row[3] );
					uint32_t TotalCreepDenies = UTIL_ToUInt32( Row[4] );
					uint32_t TotalAssists = UTIL_ToUInt32( Row[5] );
					uint32_t TotalNeutralKills = UTIL_ToUInt32( Row[6] );
					uint32_t TotalTowerKills = UTIL_ToUInt32( Row[7] );
					uint32_t TotalRaxKills = UTIL_ToUInt32( Row[8] );
					uint32_t TotalCourierKills = UTIL_ToUInt32( Row[9] );

					// calculate total wins

					string Query2 = "SELECT COUNT(*) FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour LEFT JOIN dotagames ON games.id=dotagames.gameid WHERE name='" + EscName + "' AND ((winner=1 AND dotaplayers.newcolour>=1 AND dotaplayers.newcolour<=5) OR (winner=2 AND dotaplayers.newcolour>=7 AND dotaplayers.newcolour<=11))";

					if( mysql_real_query( (MYSQL *)conn, Query2.c_str( ), Query2.size( ) ) != 0 )
						*error = mysql_error( (MYSQL *)conn );
					else
					{
						MYSQL_RES *Result2 = mysql_store_result( (MYSQL *)conn );

						if( Result2 )
						{
							vector<string> Row2 = MySQLFetchRow( Result2 );

							if( Row2.size( ) == 1 )
								TotalWins = UTIL_ToUInt32( Row2[0] );
							else
								*error = "error checking dotaplayersummary wins [" + name + "] - row doesn't have 1 column";

							mysql_free_result( Result2 );
						}
						else
							*error = mysql_error( (MYSQL *)conn );
					}

					// calculate total losses

					string Query3 = "SELECT COUNT(*) FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour LEFT JOIN dotagames ON games.id=dotagames.gameid WHERE name='" + EscName + "' AND ((winner=2 AND dotaplayers.newcolour>=1 AND dotaplayers.newcolour<=5) OR (winner=1 AND dotaplayers.newcolour>=7 AND dotaplayers.newcolour<=11))";

					if( mysql_real_query( (MYSQL *)conn, Query3.c_str( ), Query3.size( ) ) != 0 )
						*error = mysql_error( (MYSQL *)conn );
					else
					{
						MYSQL_RES *Result3 = mysql_store_result( (MYSQL *)conn );

						if( Result3 )
						{
							vector<string> Row3 = MySQLFetchRow( Result3 );

							if( Row3.size( ) == 1 )
								TotalLosses = UTIL_ToUInt32( Row3[0] );
							else
								*error = "error checking dotaplayersummary losses [" + name + "] - row doesn't have 1 column";

							mysql_free_result( Result3 );
						}
						else
							*error = mysql_error( (MYSQL *)conn );
					}

					// done

					DotAPlayerSummary = new CDBDotAPlayerSummary( string( ), name, TotalGames, TotalWins, TotalLosses, TotalKills, TotalDeaths, TotalCreepKills, TotalCreepDenies, TotalAssists, TotalNeutralKills, TotalTowerKills, TotalRaxKills, TotalCourierKills );
				}
			}
			else
				*error = "error checking dotaplayersummary [" + name + "] - row doesn't have 10 columns";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return DotAPlayerSummary;
}

CDBDotAPlayerKiller *MySQLDotAPlayerKillerCheck( void *conn, string *error, uint32_t botid, string name, uint32_t rank )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	string EscName = MySQLEscapeString( conn, name );
	CDBDotAPlayerKiller *DotAPlayerKiller = NULL;
	string Query="";
	if(name.empty( ))
		Query = "SELECT games.id, name, kills, deaths,assists FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour WHERE dotaplayers.colour<12 GROUP BY name, games.id ORDER BY MAX(kills + assists - (deaths *2)) DESC LIMIT 0,1";
	else if(!name.empty( ) && rank == 0)
		Query = "SELECT games.id, name,kills, deaths,assists FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour WHERE dotaplayers.colour<12 AND name='"+EscName+"' GROUP BY name, games.id ORDER BY MAX(kills + assists - (deaths *2))  DESC LIMIT 0,1";
	else
		Query = "SELECT games.id, name,kills, deaths,assists FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour WHERE dotaplayers.colour<12 AND name='"+EscName+"' GROUP BY name, games.id ORDER BY MAX(kills + assists - (deaths *2))  DESC LIMIT 0,1";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 5 )
			{		
				uint32_t Gameid = UTIL_ToUInt32( Row[0] );
				string Name = Row[1];
				uint32_t Kills = UTIL_ToUInt32( Row[2] );
				uint32_t Deaths = UTIL_ToUInt32( Row[3] );				
				uint32_t Assists = UTIL_ToUInt32( Row[4] );				
				DotAPlayerKiller = new CDBDotAPlayerKiller( string( ), Gameid, Name, Kills, Deaths, Assists );
			}
			else
				*error = "error checking dotaplayerkiller [" + name + "] - row doesn't have 5 columns";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return DotAPlayerKiller;
}

CDBDotAPlayerFarmer *MySQLDotAPlayerFarmerCheck( void *conn, string *error, uint32_t botid, string name, uint32_t rank )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	string EscName = MySQLEscapeString( conn, name );
	CDBDotAPlayerFarmer *DotAPlayerFarmer = NULL;
	string Query="";
	if(name.empty())
		Query = "SELECT games.id, name,creepkills, creepdenies, neutralkills FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour WHERE dotaplayers.colour<12 GROUP BY name, games.id ORDER BY MAX(creepkills+ creepdenies+ neutralkills) DESC LIMIT 0,1";
	else if(!name.empty( ) && rank == 0)
		Query = "SELECT games.id, name,creepkills, creepdenies, neutralkills FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour WHERE dotaplayers.colour<12 AND name='"+EscName+"' GROUP BY name, games.id ORDER BY MAX(creepkills+ creepdenies+ neutralkills) DESC LIMIT 0,1";
	else
		Query = "SELECT games.id, name,creepkills, creepdenies, neutralkills FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour WHERE dotaplayers.colour<12 AND name='"+EscName+"' GROUP BY name, games.id ORDER BY MAX(creepkills+ creepdenies+ neutralkills) DESC LIMIT 0,1";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );
			if( Row.size( ) == 5 )
			{	
				uint32_t Gameid = UTIL_ToUInt32( Row[0] );
				string Name =  Row[1];
				uint32_t CreepKills = UTIL_ToUInt32( Row[2] );
				uint32_t CreepDenies = UTIL_ToUInt32( Row[3] );
				uint32_t NeutralKills = UTIL_ToUInt32( Row[4] );
				DotAPlayerFarmer = new CDBDotAPlayerFarmer( string( ), Gameid, Name, CreepKills, CreepDenies, NeutralKills );
			}
			else
				*error = "error checking dotaplayerfarmer [" + name + "] - row doesn't have 5 columns";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return DotAPlayerFarmer;
}

bool MySQLDownloadAdd( void *conn, string *error, uint32_t botid, string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime )
{
	bool Success = false;
	string EscMap = MySQLEscapeString( conn, map );
	string EscName = MySQLEscapeString( conn, name );
	string EscIP = MySQLEscapeString( conn, ip );
	string EscSpoofedRealm = MySQLEscapeString( conn, spoofedrealm );
	string Query = "INSERT INTO downloads ( botid, map, mapsize, datetime, name, ip, spoofed, spoofedrealm, downloadtime ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscMap + "', " + UTIL_ToString( mapsize ) + ", NOW( ), '" + EscName + "', '" + EscIP + "', " + UTIL_ToString( spoofed ) + ", '" + EscSpoofedRealm + "', " + UTIL_ToString( downloadtime ) + " )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

double MySQLScoreCheck( void *conn, string *error, uint32_t botid, string category, string name, string server )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	string EscCategory = MySQLEscapeString( conn, category );
	string EscName = MySQLEscapeString( conn, name );
	string EscServer = MySQLEscapeString( conn, server );
	double Score = -100000.0;
	string Query = "SELECT score FROM scores WHERE category='" + EscCategory + "' AND name='" + EscName + "' AND server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 1 )
				Score = UTIL_ToDouble( Row[0] );
			/* else
				*error = "error checking score [" + category + " : " + name + " : " + server + "] - row doesn't have 1 column"; */

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Score;
}

uint32_t MySQLW3MMDPlayerAdd( void *conn, string *error, uint32_t botid, string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	uint32_t RowID = 0;
	string EscCategory = MySQLEscapeString( conn, category );
	string EscName = MySQLEscapeString( conn, name );
	string EscFlag = MySQLEscapeString( conn, flag );
	string Query = "INSERT INTO w3mmdplayers ( botid, category, gameid, pid, name, flag, leaver, practicing ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscCategory + "', " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( pid ) + ", '" + EscName + "', '" + EscFlag + "', " + UTIL_ToString( leaver ) + ", " + UTIL_ToString( practicing ) + " )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = (uint32_t)mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,int32_t> var_ints )
{
	if( var_ints.empty( ) )
		return false;

	bool Success = false;
	string Query;

	for( map<VarP,int32_t> :: iterator i = var_ints.begin( ); i != var_ints.end( ); i++ )
	{
		string EscVarName = MySQLEscapeString( conn, i->first.second );

		if( Query.empty( ) )
			Query = "INSERT INTO w3mmdvars ( botid, gameid, pid, varname, value_int ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second ) + " )";
		else
			Query += ", ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second ) + " )";
	}

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,double> var_reals )
{
	if( var_reals.empty( ) )
		return false;

	bool Success = false;
	string Query;

	for( map<VarP,double> :: iterator i = var_reals.begin( ); i != var_reals.end( ); i++ )
	{
		string EscVarName = MySQLEscapeString( conn, i->first.second );

		if( Query.empty( ) )
			Query = "INSERT INTO w3mmdvars ( botid, gameid, pid, varname, value_real ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second, 10 ) + " )";
		else
			Query += ", ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second, 10 ) + " )";
	}

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,string> var_strings )
{
	if( var_strings.empty( ) )
		return false;

	bool Success = false;
	string Query;

	for( map<VarP,string> :: iterator i = var_strings.begin( ); i != var_strings.end( ); i++ )
	{
		string EscVarName = MySQLEscapeString( conn, i->first.second );
		string EscValueString = MySQLEscapeString( conn, i->second );

		if( Query.empty( ) )
			Query = "INSERT INTO w3mmdvars ( botid, gameid, pid, varname, value_string ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', '" + EscValueString + "' )";
		else
			Query += ", ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', '" + EscValueString + "' )";
	}

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

//
// MySQL Callables
//

void CMySQLCallable :: Init( )
{
	CBaseCallable :: Init( );

#ifndef WIN32
	// disable SIGPIPE since this is (or should be) a new thread and it doesn't inherit the spawning thread's signal handlers
	// MySQL should automatically disable SIGPIPE when we initialize it but we do so anyway here

	signal( SIGPIPE, SIG_IGN );
#endif

	mysql_thread_init( );

	if( !m_Connection )
	{
		if( !( m_Connection = mysql_init( NULL ) ) )
			m_Error = mysql_error( (MYSQL *)m_Connection );

		my_bool Reconnect = true;
		mysql_options( (MYSQL *)m_Connection, MYSQL_OPT_RECONNECT, &Reconnect );

		if( !( mysql_real_connect( (MYSQL *)m_Connection, m_SQLServer.c_str( ), m_SQLUser.c_str( ), m_SQLPassword.c_str( ), m_SQLDatabase.c_str( ), m_SQLPort, NULL, 0 ) ) )
			m_Error = mysql_error( (MYSQL *)m_Connection );
	}
	else if( mysql_ping( (MYSQL *)m_Connection ) != 0 )
		m_Error = mysql_error( (MYSQL *)m_Connection );
}

void CMySQLCallable :: Close( )
{
	mysql_thread_end( );

	CBaseCallable :: Close( );
}

void CMySQLCallableMailAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		m_Result = MySQLMailAdd( m_Connection, &m_Error, m_SQLBotID, m_Id, m_Server, m_Sender, m_Receiver, m_Message);
	}

	Close( );
}
void CMySQLCallableMailReaded :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		m_Result = MySQLMailReaded( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );
	}

	Close( );
}
void CMySQLCallableMailRemove :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		m_Result = MySQLMailRemove( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Id );
	}

	Close( );
}

void CMySQLCallableMailList :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLMailList( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableInfoCount :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoCount( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableInfoCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoCheck( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );

	Close( );
}

void CMySQLCallableInfoAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Lvl, m_PrivRank, m_PubRank, m_PrivPoints, m_PubPoints, m_Admin, m_Country, m_Challwins, m_Challloses, m_Ginfo, m_Message);

	Close( );
}

void CMySQLCallableInfoPrivCalculate :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoPrivCalculate( m_Connection, &m_Error, m_SQLBotID, m_Server);

	Close( );
}

void CMySQLCallableInfoPubCalculate :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoPubCalculate( m_Connection, &m_Error, m_SQLBotID, m_Server);

	Close( );
}

void CMySQLCallableInfoPrivPoints :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoPrivPoints( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_PrivPoints);

	Close( );
}

void CMySQLCallableInfoPubPoints :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoPubPoints( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_PubPoints);

	Close( );
}

void CMySQLCallableInfoChallwins :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoChallwins( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Challwins);

	Close( );
}

void CMySQLCallableInfoChallloses :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoChallloses( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Challloses);

	Close( );
}

void CMySQLCallableInfoRemove :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		m_Result = MySQLInfoRemove( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );
	}

	Close( );
}

void CMySQLCallableInfoList :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoList( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableInfoUpdateCountry :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoUpdateCountry( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Country);

	Close( );
}
void CMySQLCallableInfoUpdateLvl :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoUpdateLvl( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Lvl, m_Admin);

	Close( );
}
void CMySQLCallableInfoUpdateMessage :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoUpdateMessage( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Message);

	Close( );
}
void CMySQLCallableInfoUpdateGinfo :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLInfoUpdateGinfo( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Ginfo, m_Admin);

	Close( );
}

void CMySQLCallableWarnCount :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLWarnCount( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableWarnCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLWarnCheck( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );

	Close( );
}

void CMySQLCallableWarnAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLWarnAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Warnings, m_Warning, m_Totalwarn, m_Daysban, m_Admin);

	Close( );
}

void CMySQLCallableWarnUpdateAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLWarnUpdateAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Warnings, m_Warning, m_Totalwarn, m_Admin);

	Close( );
}

void CMySQLCallableWarnChannelBan :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLWarnChannelBan( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Daysban, m_Admin);

	Close( );
}

void CMySQLCallableWarnRemove :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		m_Result = MySQLWarnRemove( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );
	}

	Close( );
}

void CMySQLCallableWarnList :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLWarnList( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableRunQuery :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		m_Result = MySQLRunQuery( m_Connection, &m_Error, m_SQLBotID, m_Query );
	}

	Close( );
}
///////////////////////////////////////
void CMySQLCallableBanCount :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLBanCount( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableBanCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLBanCheck( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_IP );

	Close( );
}

void CMySQLCallableBanAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLBanAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_IP, m_GameName, m_Admin, m_Reason );

	Close( );
}

void CMySQLCallableBanRemove :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		if( m_Server.empty( ) )
			m_Result = MySQLBanRemove( m_Connection, &m_Error, m_SQLBotID, m_User );
		else
			m_Result = MySQLBanRemove( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );
	}

	Close( );
}

void CMySQLCallableBanList :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLBanList( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableGameAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLGameAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_Map, m_GameName, m_OwnerName, m_Duration, m_GameState, m_CreatorName, m_CreatorServer );

	Close( );
}

void CMySQLCallableGamePlayerAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLGamePlayerAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_Name, m_IP, m_Spoofed, m_SpoofedRealm, m_Reserved, m_LoadingTime, m_Left, m_LeftReason, m_Team, m_Colour );

	Close( );
}

void CMySQLCallableGamePlayerSummaryCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLGamePlayerSummaryCheck( m_Connection, &m_Error, m_SQLBotID, m_Name );

	Close( );
}

void CMySQLCallableDotAGameAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDotAGameAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_Winner, m_Min, m_Sec );

	Close( );
}

void CMySQLCallableDotAPlayerAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDotAPlayerAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_Colour, m_Kills, m_Deaths, m_CreepKills, m_CreepDenies, m_Assists, m_Gold, m_NeutralKills, m_Item1, m_Item2, m_Item3, m_Item4, m_Item5, m_Item6, m_Hero, m_NewColour, m_TowerKills, m_RaxKills, m_CourierKills );

	Close( );
}

void CMySQLCallableDotAPlayerSummaryCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDotAPlayerSummaryCheck( m_Connection, &m_Error, m_SQLBotID, m_Name );

	Close( );
}

void CMySQLCallableDotAPlayerKillerCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDotAPlayerKillerCheck( m_Connection, &m_Error, m_SQLBotID, m_Name, m_Rank );

	Close( );
}

void CMySQLCallableDotAPlayerFarmerCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDotAPlayerFarmerCheck( m_Connection, &m_Error, m_SQLBotID, m_Name, m_Rank );

	Close( );
}

void CMySQLCallableDownloadAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDownloadAdd( m_Connection, &m_Error, m_SQLBotID, m_Map, m_MapSize, m_Name, m_IP, m_Spoofed, m_SpoofedRealm, m_DownloadTime );

	Close( );
}

void CMySQLCallableScoreCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLScoreCheck( m_Connection, &m_Error, m_SQLBotID, m_Category, m_Name, m_Server );

	Close( );
}

void CMySQLCallableW3MMDPlayerAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLW3MMDPlayerAdd( m_Connection, &m_Error, m_SQLBotID, m_Category, m_GameID, m_PID, m_Name, m_Flag, m_Leaver, m_Practicing );

	Close( );
}

void CMySQLCallableW3MMDVarAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		if( m_ValueType == VALUETYPE_INT )
			m_Result = MySQLW3MMDVarAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_VarInts );
		else if( m_ValueType == VALUETYPE_REAL )
			m_Result = MySQLW3MMDVarAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_VarReals );
		else
			m_Result = MySQLW3MMDVarAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_VarStrings );
	}

	Close( );
}
