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

#ifndef GHOSTDBMYSQL_H
#define GHOSTDBMYSQL_H

//
// CGHostDBMySQL
//
class CGHostDBMySQL : public CGHostDB
{
private:
	string m_Server;
	string m_Database;
	string m_User;
	string m_Password;
	uint16_t m_Port;
	uint32_t m_BotID;
	queue<void *> m_IdleConnections;
	uint32_t m_NumConnections;
	uint32_t m_OutstandingCallables;

public:
	CGHostDBMySQL( CConfig *CFG );
	virtual ~CGHostDBMySQL( );

	virtual string GetStatus( );
	virtual void RecoverCallable( CBaseCallable *callable );

	//Threaded database functions
	virtual void CreateThread( CBaseCallable *callable );
	
	virtual CCallableMailAdd *ThreadedMailAdd( uint32_t id, string server, string sender, string receiver, string message );
	virtual CCallableMailRemove *ThreadedMailRemove(string server, string user, uint32_t id );
	virtual CCallableMailReaded *ThreadedMailReaded(string server, string user );
	virtual CCallableMailList *ThreadedMailList( string server );

	virtual CCallableInfoCount *ThreadedInfoCount( string server );
	virtual CCallableInfoCheck *ThreadedInfoCheck( string server, string user );
	virtual CCallableInfoAdd *ThreadedInfoAdd( string server, string user, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t pubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message );
	virtual CCallableInfoPubCalculate *ThreadedInfoPubCalculate( string server);
	virtual CCallableInfoPrivCalculate *ThreadedInfoPrivCalculate( string server);
	virtual CCallableInfoPrivPoints *ThreadedInfoPrivPoints( string server, string user, uint32_t privpoints );
	virtual CCallableInfoPubPoints *ThreadedInfoPubPoints( string server, string user, uint32_t pubpoints );
	virtual CCallableInfoChallwins *ThreadedInfoChallwins( string server, string user, uint32_t challwins );
	virtual CCallableInfoChallloses *ThreadedInfoChallloses( string server, string user, uint32_t challloses );
	virtual CCallableInfoRemove *ThreadedInfoRemove( string server, string user );
	virtual CCallableInfoList *ThreadedInfoList( string server );
	virtual CCallableInfoUpdateCountry *ThreadedInfoUpdateCountry( string server, string user, string country );
	virtual CCallableInfoUpdateLvl *ThreadedInfoUpdateLvl( string server, string user, uint32_t lvl, string admin );
	virtual CCallableInfoUpdateMessage *ThreadedInfoUpdateMessage( string server, string user, uint32_t message );
	virtual CCallableInfoUpdateGinfo *ThreadedInfoUpdateGinfo( string server, string user, string admin, string ginfo );

	virtual CCallableWarnCount *ThreadedWarnCount( string server );
	virtual CCallableWarnCheck *ThreadedWarnCheck( string server, string user );
	virtual CCallableWarnAdd *ThreadedWarnAdd( string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin );
	virtual CCallableWarnUpdateAdd *ThreadedWarnUpdateAdd( string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, string admin );
	virtual CCallableWarnChannelBan *ThreadedWarnChannelBan( string server, string user, uint32_t daysban, string admin );
	virtual CCallableWarnRemove *ThreadedWarnRemove( string server, string user );
	virtual CCallableWarnList *ThreadedWarnList( string server );
	
	virtual CCallableRunQuery *ThreadedRunQuery( string query );
	
	virtual CCallableBanCount *ThreadedBanCount( string server );
	virtual CCallableBanCheck *ThreadedBanCheck( string server, string user, string ip );
	virtual CCallableBanAdd *ThreadedBanAdd( string server, string user, string ip, string gamename, string admin, string reason );
	virtual CCallableBanRemove *ThreadedBanRemove( string server, string user );
	virtual CCallableBanRemove *ThreadedBanRemove( string user );
	virtual CCallableBanList *ThreadedBanList( string server );
	virtual CCallableGameAdd *ThreadedGameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver );
	virtual CCallableGamePlayerAdd *ThreadedGamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour );
	virtual CCallableGamePlayerSummaryCheck *ThreadedGamePlayerSummaryCheck( string name );
	virtual CCallableDotAGameAdd *ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
	virtual CCallableDotAPlayerAdd *ThreadedDotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills );
	virtual CCallableDotAPlayerSummaryCheck *ThreadedDotAPlayerSummaryCheck( string name );
	virtual CCallableDotAPlayerKillerCheck *ThreadedDotAPlayerKillerCheck( string name, uint32_t rank );
	virtual CCallableDotAPlayerFarmerCheck *ThreadedDotAPlayerFarmerCheck( string name, uint32_t rank );
	virtual CCallableDownloadAdd *ThreadedDownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime );
	virtual CCallableScoreCheck *ThreadedScoreCheck( string category, string name, string server );
	virtual CCallableW3MMDPlayerAdd *ThreadedW3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings );

	// other database functions
	virtual void *GetIdleConnection( );
};

//
// global helper functions
//
bool MySQLMailAdd(void *conn, string *error, uint32_t botid, uint32_t id, string server, string sender, string receiver, string message );
bool MySQLMailRemove(void *conn, string *error, uint32_t botid, string server, string user, uint32_t id );
bool MySQLMailReaded(void *conn, string *error, uint32_t botid, string server, string user );
vector<CDBMail *> MySQLMailList( void *conn, string *error, uint32_t botid, string server );

uint32_t MySQLInfoCount( void *conn, string *error, uint32_t botid, string server );
CDBInfo *MySQLInfoCheck( void *conn, string *error, uint32_t botid, string server, string user );
bool MySQLInfoAdd( void *conn, string *error, uint32_t botid, string server, string user, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t pubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message );
bool MySQLInfoPrivCalculate( void *conn, string *error, uint32_t botid, string server);
bool MySQLInfoPubCalculate( void *conn, string *error, uint32_t botid, string server);
bool MySQLInfoPrivPoints( void *conn, string *error, uint32_t botid, string server, string user, uint32_t privpoints );
bool MySQLInfoPubPoints( void *conn, string *error, uint32_t botid, string server, string user, uint32_t pubpoints );
bool MySQLInfoChallwins( void *conn, string *error, uint32_t botid, string server, string user, uint32_t challwins );
bool MySQLInfoChallloses( void *conn, string *error, uint32_t botid, string server, string user, uint32_t challloses );
bool MySQLInfoRemove( void *conn, string *error, uint32_t botid, string server, string user );
vector<CDBInfo *> MySQLInfoList( void *conn, string *error, uint32_t botid, string server );
bool MySQLInfoUpdateCountry( void *conn, string *error, uint32_t botid, string server, string user, string country );
bool MySQLInfoUpdateLvl( void *conn, string *error, uint32_t botid, string server, string user, uint32_t lvl, string admin );
bool MySQLInfoUpdateMessage( void *conn, string *error, uint32_t botid, string server, string user, uint32_t message );
bool MySQLInfoUpdateGinfo( void *conn, string *error, uint32_t botid, string server, string user, string ginfo, string admin );

uint32_t MySQLWarnCount( void *conn, string *error, uint32_t botid, string server );
CDBWarn *MySQLWarnCheck( void *conn, string *error, uint32_t botid, string server, string user );
bool MySQLWarnAdd( void *conn, string *error, uint32_t botid, string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin );
bool MySQLWarnUpdateAdd( void *conn, string *error, uint32_t botid, string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, string admin );
bool MySQLWarnChannelBan( void *conn, string *error, uint32_t botid, string server, string user, uint32_t daysban, string admin );
bool MySQLWarnRemove( void *conn, string *error, uint32_t botid, string server, string user );
vector<CDBWarn *> MySQLWarnList( void *conn, string *error, uint32_t botid, string server );

bool MySQLRunQuery( void *conn, string *error, uint32_t botid, string query );

uint32_t MySQLBanCount( void *conn, string *error, uint32_t botid, string server );
CDBBan *MySQLBanCheck( void *conn, string *error, uint32_t botid, string server, string user, string ip );
bool MySQLBanAdd( void *conn, string *error, uint32_t botid, string server, string user, string ip, string gamename, string admin, string reason );
bool MySQLBanRemove( void *conn, string *error, uint32_t botid, string server, string user );
bool MySQLBanRemove( void *conn, string *error, uint32_t botid, string user );
vector<CDBBan *> MySQLBanList( void *conn, string *error, uint32_t botid, string server );
uint32_t MySQLGameAdd( void *conn, string *error, uint32_t botid, string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver );
uint32_t MySQLGamePlayerAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour );
CDBGamePlayerSummary *MySQLGamePlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name );
uint32_t MySQLDotAGameAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
uint32_t MySQLDotAPlayerAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills );
CDBDotAPlayerSummary *MySQLDotAPlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name );
CDBDotAPlayerKiller *MySQLDotAPlayerKillerCheck( void *conn, string *error, uint32_t botid, string name, uint32_t rank );
CDBDotAPlayerFarmer *MySQLDotAPlayerFarmerCheck( void *conn, string *error, uint32_t botid, string name, uint32_t rank );
bool MySQLDownloadAdd( void *conn, string *error, uint32_t botid, string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime );
double MySQLScoreCheck( void *conn, string *error, uint32_t botid, string category, string name, string server );
uint32_t MySQLW3MMDPlayerAdd( void *conn, string *error, uint32_t botid, string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing );
bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,int32_t> var_ints );
bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,double> var_reals );
bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,string> var_strings );

//
// MySQL Callables
//
class CMySQLCallable : virtual public CBaseCallable
{
protected:
	void *m_Connection;
	string m_SQLServer;
	string m_SQLDatabase;
	string m_SQLUser;
	string m_SQLPassword;
	uint16_t m_SQLPort;
	uint32_t m_SQLBotID;

public:
	CMySQLCallable( void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), m_Connection( nConnection ), m_SQLBotID( nSQLBotID ), m_SQLServer( nSQLServer ), m_SQLDatabase( nSQLDatabase ), m_SQLUser( nSQLUser ), m_SQLPassword( nSQLPassword ), m_SQLPort( nSQLPort ) { }
	virtual ~CMySQLCallable( ) { }

	virtual void *GetConnection( )	{ return m_Connection; }

	virtual void Init( );
	virtual void Close( );
};

class CMySQLCallableMailReaded : public CCallableMailReaded, public CMySQLCallable
{
public:
	CMySQLCallableMailReaded( string nServer, string nUser, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableMailReaded( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableMailReaded( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableMailRemove : public CCallableMailRemove, public CMySQLCallable
{
public:
	CMySQLCallableMailRemove( string nServer, string nUser, uint32_t nId, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableMailRemove( nServer, nUser, nId ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableMailRemove( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableMailList : public CCallableMailList, public CMySQLCallable
{
public:
	CMySQLCallableMailList( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableMailList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableMailList( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableMailAdd : public CCallableMailAdd, public CMySQLCallable
{
public:
	CMySQLCallableMailAdd( uint32_t nId, string nServer, string nSender, string nReceiver, string nMessage, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableMailAdd( nId, nServer, nSender, nReceiver, nMessage ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableMailAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};


class CMySQLCallableInfoCount : public CCallableInfoCount, public CMySQLCallable
{
public:
	CMySQLCallableInfoCount( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoCount( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoCount( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoCheck : public CCallableInfoCheck, public CMySQLCallable
{
public:
	CMySQLCallableInfoCheck( string nServer, string nUser, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoCheck( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoAdd : public CCallableInfoAdd, public CMySQLCallable
{
public:
	CMySQLCallableInfoAdd( string nServer, string nUser, uint32_t nLvl, uint32_t nPrivRank, uint32_t nPubRank, uint32_t nPrivPoints, uint32_t nPubPoints, string nAdmin, string nCountry, uint32_t nChallwins, uint32_t nChallloses, string nGinfo, uint32_t nMessage, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoAdd( nServer, nUser, nLvl, nPrivRank, nPubRank, nPrivPoints, nPubPoints, nAdmin, nCountry, nChallwins, nChallloses, nGinfo, nMessage ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoPrivCalculate : public CCallableInfoPrivCalculate, public CMySQLCallable
{
public:
	CMySQLCallableInfoPrivCalculate( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoPrivCalculate( nServer), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoPrivCalculate( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoPubCalculate : public CCallableInfoPubCalculate, public CMySQLCallable
{
public:
	CMySQLCallableInfoPubCalculate( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoPubCalculate( nServer), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoPubCalculate( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoPrivPoints : public CCallableInfoPrivPoints, public CMySQLCallable
{
public:
	CMySQLCallableInfoPrivPoints( string nServer, string nUser, uint32_t nPrivPoints, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoPrivPoints( nServer, nUser, nPrivPoints ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoPrivPoints( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoPubPoints : public CCallableInfoPubPoints, public CMySQLCallable
{
public:
	CMySQLCallableInfoPubPoints( string nServer, string nUser, uint32_t nPubPoints, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoPubPoints( nServer, nUser, nPubPoints ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoPubPoints( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoChallwins : public CCallableInfoChallwins, public CMySQLCallable
{
public:
	CMySQLCallableInfoChallwins( string nServer, string nUser, uint32_t nChallwins, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoChallwins( nServer, nUser, nChallwins ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoChallwins( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoChallloses : public CCallableInfoChallloses, public CMySQLCallable
{
public:
	CMySQLCallableInfoChallloses( string nServer, string nUser, uint32_t nChallloses, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoChallloses( nServer, nUser, nChallloses ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoChallloses( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoRemove : public CCallableInfoRemove, public CMySQLCallable
{
public:
	CMySQLCallableInfoRemove( string nServer, string nUser, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoRemove( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoRemove( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoList : public CCallableInfoList, public CMySQLCallable
{
public:
	CMySQLCallableInfoList( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoList( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableInfoUpdateCountry : public CCallableInfoUpdateCountry, public CMySQLCallable
{
public:
	CMySQLCallableInfoUpdateCountry( string nServer, string nUser, string nCountry, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoUpdateCountry( nServer, nUser, nCountry ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoUpdateCountry( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};
class CMySQLCallableInfoUpdateLvl : public CCallableInfoUpdateLvl, public CMySQLCallable
{
public:
	CMySQLCallableInfoUpdateLvl( string nServer, string nUser, uint32_t nLvl, string nAdmin, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoUpdateLvl( nServer, nUser, nLvl, nAdmin ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoUpdateLvl( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};
class CMySQLCallableInfoUpdateMessage : public CCallableInfoUpdateMessage, public CMySQLCallable
{
public:
	CMySQLCallableInfoUpdateMessage( string nServer, string nUser, uint32_t nMessage, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoUpdateMessage( nServer, nUser, nMessage), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoUpdateMessage( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};
class CMySQLCallableInfoUpdateGinfo : public CCallableInfoUpdateGinfo, public CMySQLCallable
{
public:
	CMySQLCallableInfoUpdateGinfo( string nServer, string nUser, string nGinfo, string nAdmin, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableInfoUpdateGinfo( nServer, nUser, nGinfo, nAdmin ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableInfoUpdateGinfo( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableWarnCount : public CCallableWarnCount, public CMySQLCallable
{
public:
	CMySQLCallableWarnCount( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableWarnCount( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableWarnCount( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableWarnCheck : public CCallableWarnCheck, public CMySQLCallable
{
public:
	CMySQLCallableWarnCheck( string nServer, string nUser, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableWarnCheck( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableWarnCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableWarnAdd : public CCallableWarnAdd, public CMySQLCallable
{
public:
	CMySQLCallableWarnAdd( string nServer, string nUser, uint32_t nWarnings, string nWarning, uint32_t nTotalwarn, uint32_t nDaysban, string nAdmin , void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableWarnAdd( nServer, nUser, nWarnings, nWarning, nTotalwarn, nDaysban, nAdmin ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableWarnAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};


class CMySQLCallableWarnUpdateAdd : public CCallableWarnUpdateAdd, public CMySQLCallable
{
public:
	CMySQLCallableWarnUpdateAdd( string nServer, string nUser, uint32_t nWarnings, string nWarning, uint32_t nTotalwarn, string nAdmin , void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableWarnUpdateAdd( nServer, nUser, nWarnings, nWarning, nTotalwarn, nAdmin ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableWarnUpdateAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};


class CMySQLCallableWarnChannelBan : public CCallableWarnChannelBan, public CMySQLCallable
{
public:
	CMySQLCallableWarnChannelBan( string nServer, string nUser, uint32_t nDaysban, string nAdmin , void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableWarnChannelBan( nServer, nUser, nDaysban, nAdmin ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableWarnChannelBan( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};


class CMySQLCallableWarnRemove : public CCallableWarnRemove, public CMySQLCallable
{
public:
	CMySQLCallableWarnRemove( string nServer, string nUser, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableWarnRemove( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableWarnRemove( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableWarnList : public CCallableWarnList, public CMySQLCallable
{
public:
	CMySQLCallableWarnList( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableWarnList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableWarnList( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableRunQuery : public CCallableRunQuery, public CMySQLCallable
{
public:
	CMySQLCallableRunQuery( string nQuery, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableRunQuery( nQuery ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableRunQuery( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanCount : public CCallableBanCount, public CMySQLCallable
{
public:
	CMySQLCallableBanCount( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanCount( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanCount( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanCheck : public CCallableBanCheck, public CMySQLCallable
{
public:
	CMySQLCallableBanCheck( string nServer, string nUser, string nIP, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanCheck( nServer, nUser, nIP ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanAdd : public CCallableBanAdd, public CMySQLCallable
{
public:
	CMySQLCallableBanAdd( string nServer, string nUser, string nIP, string nGameName, string nAdmin, string nReason, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanAdd( nServer, nUser, nIP, nGameName, nAdmin, nReason ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanRemove : public CCallableBanRemove, public CMySQLCallable
{
public:
	CMySQLCallableBanRemove( string nServer, string nUser, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanRemove( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanRemove( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanList : public CCallableBanList, public CMySQLCallable
{
public:
	CMySQLCallableBanList( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanList( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableGameAdd : public CCallableGameAdd, public CMySQLCallable
{
public:
	CMySQLCallableGameAdd( string nServer, string nMap, string nGameName, string nOwnerName, uint32_t nDuration, uint32_t nGameState, string nCreatorName, string nCreatorServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableGameAdd( nServer, nMap, nGameName, nOwnerName, nDuration, nGameState, nCreatorName, nCreatorServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableGameAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableGamePlayerAdd : public CCallableGamePlayerAdd, public CMySQLCallable
{
public:
	CMySQLCallableGamePlayerAdd( uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableGamePlayerAdd( nGameID, nName, nIP, nSpoofed, nSpoofedRealm, nReserved, nLoadingTime, nLeft, nLeftReason, nTeam, nColour ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableGamePlayerAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableGamePlayerSummaryCheck : public CCallableGamePlayerSummaryCheck, public CMySQLCallable
{
public:
	CMySQLCallableGamePlayerSummaryCheck( string nName, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableGamePlayerSummaryCheck( nName ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableGamePlayerSummaryCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAGameAdd : public CCallableDotAGameAdd, public CMySQLCallable
{
public:
	CMySQLCallableDotAGameAdd( uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDotAGameAdd( nGameID, nWinner, nMin, nSec ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAGameAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAPlayerAdd : public CCallableDotAPlayerAdd, public CMySQLCallable
{
public:
	CMySQLCallableDotAPlayerAdd( uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, string nItem1, string nItem2, string nItem3, string nItem4, string nItem5, string nItem6, string nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDotAPlayerAdd( nGameID, nColour, nKills, nDeaths, nCreepKills, nCreepDenies, nAssists, nGold, nNeutralKills, nItem1, nItem2, nItem3, nItem4, nItem5, nItem6, nHero, nNewColour, nTowerKills, nRaxKills, nCourierKills ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAPlayerAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAPlayerSummaryCheck : public CCallableDotAPlayerSummaryCheck, public CMySQLCallable
{
public:
	CMySQLCallableDotAPlayerSummaryCheck( string nName, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDotAPlayerSummaryCheck( nName ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAPlayerSummaryCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAPlayerKillerCheck : public CCallableDotAPlayerKillerCheck, public CMySQLCallable
{
public:
	CMySQLCallableDotAPlayerKillerCheck( string nName, uint32_t nRank, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDotAPlayerKillerCheck( nName, nRank ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAPlayerKillerCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAPlayerFarmerCheck : public CCallableDotAPlayerFarmerCheck, public CMySQLCallable
{
public:
	CMySQLCallableDotAPlayerFarmerCheck( string nName, uint32_t nRank, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDotAPlayerFarmerCheck( nName, nRank ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAPlayerFarmerCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDownloadAdd : public CCallableDownloadAdd, public CMySQLCallable
{
public:
	CMySQLCallableDownloadAdd( string nMap, uint32_t nMapSize, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nDownloadTime, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDownloadAdd( nMap, nMapSize, nName, nIP, nSpoofed, nSpoofedRealm, nDownloadTime ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDownloadAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableScoreCheck : public CCallableScoreCheck, public CMySQLCallable
{
public:
	CMySQLCallableScoreCheck( string nCategory, string nName, string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableScoreCheck( nCategory, nName, nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableScoreCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableW3MMDPlayerAdd : public CCallableW3MMDPlayerAdd, public CMySQLCallable
{
public:
	CMySQLCallableW3MMDPlayerAdd( string nCategory, uint32_t nGameID, uint32_t nPID, string nName, string nFlag, uint32_t nLeaver, uint32_t nPracticing, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableW3MMDPlayerAdd( nCategory, nGameID, nPID, nName, nFlag, nLeaver, nPracticing ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableW3MMDPlayerAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableW3MMDVarAdd : public CCallableW3MMDVarAdd, public CMySQLCallable
{
public:
	CMySQLCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,int32_t> nVarInts, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarInts ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	CMySQLCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,double> nVarReals, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarReals ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	CMySQLCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,string> nVarStrings, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarStrings ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableW3MMDVarAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

#endif