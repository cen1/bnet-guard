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

#ifndef GHOSTDB_H
#define GHOSTDB_H

//
// CGHostDB
//
class CBaseCallable;

class CCallableMailAdd;
class CCallableMailRemove;
class CCallableMailList;
class CCallableMailReaded;
class CDBMail;

class CCallableInfoCount;
class CCallableInfoCheck;
class CCallableInfoAdd;
class CCallableInfoRemove;
class CCallableInfoList;
class CCallableInfoUpdateCountry;
class CCallableInfoUpdateLvl;
class CCallableInfoUpdateMessage;
class CCallableInfoUpdateGinfo;
class CCallableInfoPrivCalculate;
class CCallableInfoPubCalculate;
class CCallableInfoPrivPoints;
class CCallableInfoPubPoints;
class CCallableInfoChallwins;
class CCallableInfoChallloses;
class CDBInfo;

class CCallableWarnCount;
class CCallableWarnCheck;
class CCallableWarnAdd;
class CCallableWarnUpdateAdd;
class CCallableWarnChannelBan;
class CCallableWarnRemove;
class CCallableWarnList;
class CDBWarn;

class CCallableRunQuery;

class CDBGames;
class CDBChannel;
class CDBHostBots;
class CDBIcons;
class CDBMaps;

class CCallableBanCount;
class CCallableBanCheck;
class CCallableBanAdd;
class CCallableBanRemove;
class CCallableBanList;
class CCallableGameAdd;
class CCallableGamePlayerAdd;
class CCallableGamePlayerSummaryCheck;
class CCallableDotAGameAdd;
class CCallableDotAPlayerAdd;
class CCallableDotAPlayerSummaryCheck;
class CCallableDotAPlayerKillerCheck;
class CCallableDotAPlayerFarmerCheck;

class CCallableDownloadAdd;
class CCallableScoreCheck;
class CCallableW3MMDPlayerAdd;
class CCallableW3MMDVarAdd;
class CDBBan;
class CDBGame;
class CDBGamePlayer;
class CDBGamePlayerSummary;
class CDBDotAPlayerSummary;
class CDBDotAPlayerKiller;
class CDBDotAPlayerFarmer;

typedef pair<uint32_t,string> VarP;

class CGHostDB
{
protected:
	bool m_HasError;
	string m_Error;

public:
	CGHostDB( CConfig *CFG );
	virtual ~CGHostDB( );

	bool HasError( )			{ return m_HasError; }
	string GetError( )			{ return m_Error; }
	virtual string GetStatus( )	{ return "DB STATUS --- OK"; }

	virtual void RecoverCallable( CBaseCallable *callable );

	//Standard (non-threaded) database functions

	virtual bool Begin( );
	virtual bool Commit( );

	virtual bool MailAdd(uint32_t id, string server, string sender, string receiver, string message );
	virtual bool MailRemove(string server, string user, uint32_t id );
	virtual bool MailReaded(string server, string user );
	virtual vector<CDBMail *> MailList( string server );

	virtual uint32_t InfoCount( string server );
	virtual bool InfoAdd( string server, string user, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t npubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message );
	virtual bool InfoPrivCalculate( string server);
	virtual bool InfoPubCalculate( string server);
	virtual bool InfoPrivPoints( string server, string user, uint32_t privpoints );
	virtual bool InfoPubPoints( string server, string user, uint32_t pubpoints );
	virtual bool InfoChallwins( string server, string user, uint32_t challwins );
	virtual bool InfoChallloses( string server, string user, uint32_t challloses );
	virtual CDBInfo *InfoCheck( string server, string user);
	virtual bool InfoRemove( string server, string user );
	virtual vector<CDBInfo *> InfoList( string server );
	virtual bool InfoUpdateCountry( string server, string user, string country);
	virtual bool InfoUpdateLvl( string server, string user, uint32_t lvl, string admin);
	virtual bool InfoUpdateMessage( string server, string user, uint32_t message);
	virtual bool InfoUpdateGinfo( string server, string user, string ginfo, string admin);

	virtual uint32_t WarnCount( string server );
	virtual bool WarnAdd(string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin);
	virtual bool WarnUpdateAdd (string server, string user, uint32_t warnings, string warning, uint32_t totalwarn,string admin);
	virtual bool WarnChannelBan (string server, string user, uint32_t daysban, string admin);
	virtual CDBWarn *WarnCheck( string server, string user);
	virtual bool WarnRemove( string server, string user );
	virtual vector<CDBWarn *> WarnList( string server );

	virtual bool RunQuery( string query );
	
	virtual uint32_t BanCount( string server );
	virtual CDBBan *BanCheck( string server, string user, string ip );
	virtual bool BanAdd( string server, string user, string ip, string gamename, string admin, string reason );
	virtual bool BanRemove( string server, string user );
	virtual bool BanRemove( string user );
	virtual vector<CDBBan *> BanList( string server );
	virtual uint32_t GameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver );
	virtual uint32_t GamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour );
	virtual uint32_t GamePlayerCount( string name );
	virtual CDBGamePlayerSummary *GamePlayerSummaryCheck( string name );
	virtual uint32_t DotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
	virtual uint32_t DotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills );
	virtual uint32_t DotAPlayerCount( string name );
	virtual CDBDotAPlayerSummary *DotAPlayerSummaryCheck( string name );
	virtual CDBDotAPlayerKiller *DotAPlayerKillerCheck( string name, uint32_t rank );
	virtual CDBDotAPlayerFarmer *DotAPlayerFarmerCheck( string name, uint32_t rank );
	virtual string FromCheck( uint32_t ip );
	virtual string FullFromCheck( uint32_t ip );//add by me
	virtual bool FromAdd(  uint32_t ip1, uint32_t ip2, string country, string fullcountry );
	virtual bool DownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime );
	virtual uint32_t W3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing );
	virtual bool W3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints );
	virtual bool W3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals );
	virtual bool W3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings );

	//Threaded database functions
	virtual void CreateThread( CBaseCallable *callable );

	virtual CCallableMailAdd *ThreadedMailAdd(uint32_t id, string server, string sender, string receiver, string message );
	virtual CCallableMailRemove *ThreadedMailRemove(string server, string user, uint32_t id );
	virtual CCallableMailReaded *ThreadedMailReaded(string server, string user );
	virtual CCallableMailList *ThreadedMailList( string server );

	virtual CCallableInfoCount *ThreadedInfoCount( string server );
	virtual CCallableInfoAdd *ThreadedInfoAdd( string server, string user, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t pubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message);
	virtual CCallableInfoPrivCalculate *ThreadedInfoPrivCalculate( string server);
	virtual CCallableInfoPubCalculate *ThreadedInfoPubCalculate( string server);
	virtual CCallableInfoPrivPoints *ThreadedInfoPrivPoints( string server, string user, uint32_t privpoints );
	virtual CCallableInfoPubPoints *ThreadedInfoPubPoints( string server, string user, uint32_t pubpoints );
	virtual CCallableInfoChallwins *ThreadedInfoChallwins( string server, string user, uint32_t challwins );
	virtual CCallableInfoChallloses *ThreadedInfoChallloses( string server, string users, uint32_t challloses );
	virtual CCallableInfoCheck *ThreadedInfoCheck( string server, string user);
	virtual CCallableInfoRemove *ThreadedInfoRemove( string server, string user );
	virtual CCallableInfoList *ThreadedInfoList( string server );
	virtual CCallableInfoUpdateCountry *ThreadedInfoUpdateCountry( string server, string user, string country);
	virtual CCallableInfoUpdateLvl *ThreadedInfoUpdateLvl( string server, string user, uint32_t lvl, string admin);
	virtual CCallableInfoUpdateMessage *ThreadedInfoUpdateMessage( string server, string user, uint32_t message);
	virtual CCallableInfoUpdateGinfo *ThreadedInfoUpdateGinfo( string server, string user, string ginfo, string admin);

	virtual CCallableWarnCount *ThreadedWarnCount( string server );
	virtual CCallableWarnAdd *ThreadedWarnAdd(string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin);
	virtual CCallableWarnUpdateAdd *ThreadedWarnUpdateAdd(string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, string admin);
	virtual CCallableWarnChannelBan *ThreadedWarnChannelBan(string server, string user, uint32_t daysban, string admin);
	virtual CCallableWarnCheck *ThreadedWarnCheck( string server, string user);
	virtual CCallableWarnRemove *ThreadedWarnRemove( string server, string user );
	virtual CCallableWarnList *ThreadedWarnList( string server );

	virtual CCallableRunQuery *ThreadedRunQuery(string query);
	
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
};

//
// Callables
//

// life cycle of a callable:
//  - the callable is created in one of the database's ThreadedXXX functions
//  - initially the callable is NOT ready (i.e. m_Ready = false)
//  - the ThreadedXXX function normally creates a thread to perform some query and (potentially) store some result in the callable
//  - at the time of this writing all threads are immediately detached, the code does not join any threads (the callable's "readiness" is used for this purpose instead)
//  - when the thread completes it will set m_Ready = true
//  - DO NOT DO *ANYTHING* TO THE CALLABLE UNTIL IT'S READY OR YOU WILL CREATE A CONCURRENCY MESS
//  - THE ONLY SAFE FUNCTION IN THE CALLABLE IS GetReady
//  - when the callable is ready you may access the callable's result which will have been set within the (now terminated) thread

// example usage:
//  - normally you will call a ThreadedXXX function, store the callable in a vector, and periodically check if the callable is ready
//  - when the callable is ready you will consume the result then you will pass the callable back to the database via the RecoverCallable function
//  - the RecoverCallable function allows the database to recover some of the callable's resources to be reused later (e.g. MySQL connections)
//  - note that this will NOT free the callable's memory, you must do that yourself after calling the RecoverCallable function
//  - be careful not to leak any callables, it's NOT safe to delete a callable even if you decide that you don't want the result anymore
//  - you should deliver any to-be-orphaned callables to the main vector in CGHost so they can be properly deleted when ready even if you don't care about the result anymore
//  - e.g. if a player does a stats check immediately before a game is deleted you can't just delete the callable on game deletion unless it's ready

class CBaseCallable
{
protected:
	string m_Error;
	volatile bool m_Ready;
	uint32_t m_StartTicks;
	uint32_t m_EndTicks;

public:
	CBaseCallable( ) : m_Error( ), m_Ready( false ), m_StartTicks( 0 ), m_EndTicks( 0 ) { }
	virtual ~CBaseCallable( ) { }

	virtual void operator( )( ) { }

	virtual void Init( );
	virtual void Close( );

	virtual string GetError( )				{ return m_Error; }
	virtual bool GetReady( )				{ return m_Ready; }
	virtual void SetReady( bool nReady )	{ m_Ready = nReady; }
	virtual uint32_t GetElapsed( )			{ return m_Ready ? m_EndTicks - m_StartTicks : 0; }
};

class CCallableMailAdd : virtual public CBaseCallable
{
protected:
	uint32_t m_Id;
	string m_Server;
	string m_Sender;
	string m_Receiver;
	string m_Message;
	bool m_Result;

public:
	CCallableMailAdd( uint32_t nId, string nServer, string nSender, string nReceiver, string nMessage ) : CBaseCallable( ), m_Id( nId ), m_Server( nServer ), m_Sender( nSender ), m_Receiver( nReceiver ), m_Message( nMessage ), m_Result( false ) { }
	virtual ~CCallableMailAdd( );
	virtual uint32_t GetId( )				{ return m_Id; }
	virtual string GetServer( )				{ return m_Server; }
	virtual string GetSender( )				{ return m_Sender; }
	virtual string GetReceiver( )			{ return m_Receiver; }
	virtual string GetMessage( )			{ return m_Message; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableMailReaded : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	bool m_Result;

public:
	CCallableMailReaded( string nServer, string nUser) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableMailReaded( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableMailRemove : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_Id;
	bool m_Result;

public:
	CCallableMailRemove( string nServer, string nUser, uint32_t nId ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Id( nId ), m_Result( false ) { }
	virtual ~CCallableMailRemove( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetId( )				{ return m_Id; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableMailList : virtual public CBaseCallable
{
protected:
	string m_Server;
	vector<CDBMail *> m_Result;

public:
	CCallableMailList( string nServer ) : CBaseCallable( ), m_Server( nServer ) { }
	virtual ~CCallableMailList( );

	virtual vector<CDBMail *> GetResult( )				{ return m_Result; }
	virtual void SetResult( vector<CDBMail *> nResult )	{ m_Result = nResult; }
};

class CCallableInfoCount : virtual public CBaseCallable
{
protected:
	string m_Server;
	uint32_t m_Result;

public:
	CCallableInfoCount( string nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( 0 ) { }
	virtual ~CCallableInfoCount( );

	virtual string GetServer( )					{ return m_Server; }
	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableInfoCheck : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	CDBInfo *m_Result;

public:
	CCallableInfoCheck( string nServer, string nUser) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( NULL ) { }
	virtual ~CCallableInfoCheck( );

	virtual string GetServer( )					{ return m_Server; }
	virtual string GetUser( )					{ return m_User; }
	virtual CDBInfo *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBInfo *nResult )	{ m_Result = nResult; }
};

class CCallableInfoAdd : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_Lvl;
	uint32_t m_PrivRank;
	uint32_t m_PubRank;
	uint32_t m_PrivPoints;
	uint32_t m_PubPoints;
	string m_Admin;
	string m_Country;
	uint32_t m_Challwins;
	uint32_t m_Challloses;
	string m_Ginfo;
	uint32_t m_Message;
	bool m_Result;
public:// string admin, string country, string ginfo, uint32_t message);
	CCallableInfoAdd( string nServer, string nUser, uint32_t nLvl, uint32_t nPrivRank, uint32_t nPubRank, uint32_t nPrivPoints, uint32_t nPubPoints, string nAdmin, string nCountry, uint32_t nChallwins, uint32_t nChallloses, string nGinfo, uint32_t nMessage ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Lvl(nLvl), m_PrivRank(nPrivRank), m_PubRank(nPubRank), m_PrivPoints(nPrivPoints), m_PubPoints(nPubPoints), m_Admin(nAdmin), m_Country(nCountry), m_Challwins(nChallwins), m_Challloses(nChallloses), m_Ginfo(nGinfo),m_Message(nMessage), m_Result( false ) { }
	virtual ~CCallableInfoAdd( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetLvl( )				{ return m_Lvl; }
	virtual uint32_t GetPrivRank( )			{ return m_PrivRank; }
	virtual uint32_t GetPubRank( )			{ return m_PubRank; }
	virtual uint32_t GetPrivPoints( )		{ return m_PrivPoints; }
	virtual uint32_t GetPubPoints( )		{ return m_PubPoints; }
	virtual string GetAdmin( )				{ return m_Admin; }
	virtual string GetCountry( )			{ return m_Country; }
	virtual uint32_t GetChallwins( )		{ return m_Challwins; }
	virtual uint32_t GetChallloses( )		{ return m_Challloses; }
	virtual string GetGinfo( )				{ return m_Ginfo; }
	virtual uint32_t GetMessage( )			{ return m_Message; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoPrivCalculate : virtual public CBaseCallable
{
protected:
	string m_Server;
	bool m_Result;
public:
	CCallableInfoPrivCalculate( string nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( false ) { }
	virtual ~CCallableInfoPrivCalculate( );

	virtual string GetServer( )				{ return m_Server; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoPubCalculate : virtual public CBaseCallable
{
protected:
	string m_Server;
	bool m_Result;
public:
	CCallableInfoPubCalculate( string nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( false ) { }
	virtual ~CCallableInfoPubCalculate( );

	virtual string GetServer( )				{ return m_Server; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoPrivPoints : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_PrivPoints;
	bool m_Result;
public:
	CCallableInfoPrivPoints( string nServer, string nUser, uint32_t nPrivPoints ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_PrivPoints( nPrivPoints ), m_Result( false ) { }
	virtual ~CCallableInfoPrivPoints( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetPrivPoints( )		{ return m_PrivPoints; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoPubPoints : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_PubPoints;
	bool m_Result;
public:
	CCallableInfoPubPoints( string nServer, string nUser, uint32_t nPubPoints ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_PubPoints( nPubPoints ), m_Result( false ) { }
	virtual ~CCallableInfoPubPoints( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetPubPoints( )		{ return m_PubPoints; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoChallwins : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_Challwins;
	bool m_Result;
public:
	CCallableInfoChallwins( string nServer, string nUser, uint32_t nChallwins ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Challwins( nChallwins ), m_Result( false ) { }
	virtual ~CCallableInfoChallwins( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetChallwins( )		{ return m_Challwins; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoChallloses : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_Challloses;
	bool m_Result;
public:
	CCallableInfoChallloses( string nServer, string nUser, uint32_t nChallloses ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Challloses( nChallloses ), m_Result( false ) { }
	virtual ~CCallableInfoChallloses( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetChallloses( )		{ return m_Challloses; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoRemove : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	bool m_Result;

public:
	CCallableInfoRemove( string nServer, string nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableInfoRemove( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoList : virtual public CBaseCallable
{
protected:
	string m_Server;
	vector<CDBInfo *> m_Result;

public:
	CCallableInfoList( string nServer ) : CBaseCallable( ), m_Server( nServer ) { }
	virtual ~CCallableInfoList( );

	virtual vector<CDBInfo *> GetResult( )				{ return m_Result; }
	virtual void SetResult( vector<CDBInfo *> nResult )	{ m_Result = nResult; }
};

class CCallableInfoUpdateCountry : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	string m_Country;
	bool m_Result;
public:
	CCallableInfoUpdateCountry( string nServer, string nUser, string nCountry) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ),m_Country(nCountry), m_Result( false ) { }
	virtual ~CCallableInfoUpdateCountry( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual string GetCountry( )			{ return m_Country; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoUpdateLvl : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_Lvl;
	string m_Admin;
	bool m_Result;
public:
	CCallableInfoUpdateLvl( string nServer, string nUser, uint32_t nLvl, string nAdmin) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ),m_Lvl(nLvl), m_Admin(nAdmin), m_Result( false ) { }
	virtual ~CCallableInfoUpdateLvl( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetLvl( )				{ return m_Lvl; }
	virtual string GetAdmin( )				{ return m_Admin; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoUpdateMessage : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_Message;
	bool m_Result;
public:
	CCallableInfoUpdateMessage( string nServer, string nUser, uint32_t nMessage) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ),m_Message(nMessage), m_Result( false ) { }
	virtual ~CCallableInfoUpdateMessage( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetMessage( )			{ return m_Message; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableInfoUpdateGinfo : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	string m_Ginfo;
	string m_Admin;
	bool m_Result;
public:
	CCallableInfoUpdateGinfo( string nServer, string nUser, string nGinfo, string nAdmin) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ),m_Ginfo(nGinfo), m_Admin(nAdmin), m_Result( false ) { }
	virtual ~CCallableInfoUpdateGinfo( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual string GetGinfo( )				{ return m_Ginfo; }
	virtual string GetAdmin( )				{ return m_Admin; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableWarnCount : virtual public CBaseCallable
{
protected:
	string m_Server;
	uint32_t m_Result;

public:
	CCallableWarnCount( string nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( 0 ) { }
	virtual ~CCallableWarnCount( );

	virtual string GetServer( )					{ return m_Server; }
	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableWarnCheck : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	CDBWarn *m_Result;

public:
	CCallableWarnCheck( string nServer, string nUser) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( NULL ) { }
	virtual ~CCallableWarnCheck( );

	virtual string GetServer( )					{ return m_Server; }
	virtual string GetUser( )					{ return m_User; }
	virtual CDBWarn *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBWarn *nResult )	{ m_Result = nResult; }
};

class CCallableWarnAdd : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_Warnings;
	string m_Warning;
	uint32_t m_Totalwarn;
	uint32_t m_Daysban;
	string m_Admin;
	bool m_Result;
public:
	CCallableWarnAdd( string nServer, string nUser, uint32_t nWarnings, string nWarning, uint32_t nTotalwarn, uint32_t nDaysban, string nAdmin) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Warnings(nWarnings), m_Warning(nWarning), m_Totalwarn(nTotalwarn), m_Daysban(nDaysban), m_Admin(nAdmin), m_Result( false ) { }
	virtual ~CCallableWarnAdd( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetWarnings( )			{ return m_Warnings; }
	virtual string GetWarning( )			{ return m_Warning; }
	virtual uint32_t GetTotalwarn( )		{ return m_Totalwarn; }
	virtual uint32_t GetDaysban( )			{ return m_Daysban; }
	virtual string GetAdmin( )				{ return m_Admin; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableWarnUpdateAdd : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_Warnings;
	string m_Warning;
	uint32_t m_Totalwarn;
	string m_Admin;
	bool m_Result;
public:
	CCallableWarnUpdateAdd( string nServer, string nUser, uint32_t nWarnings, string nWarning, uint32_t nTotalwarn, string nAdmin) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Warnings(nWarnings), m_Warning(nWarning), m_Totalwarn(nTotalwarn), m_Admin(nAdmin), m_Result( false ) { }
	virtual ~CCallableWarnUpdateAdd( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetWarnings( )			{ return m_Warnings; }
	virtual string GetWarning( )			{ return m_Warning; }
	virtual uint32_t GetTotalwarn( )		{ return m_Totalwarn; }
	virtual string GetAdmin( )				{ return m_Admin; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableWarnChannelBan : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	uint32_t m_Daysban;
	string m_Admin;
	bool m_Result;
public:
	CCallableWarnChannelBan( string nServer, string nUser, uint32_t nDaysban, string nAdmin) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Daysban(nDaysban), m_Admin(nAdmin), m_Result( false ) { }
	virtual ~CCallableWarnChannelBan( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual uint32_t GetDaysban( )			{ return m_Daysban; }
	virtual string GetAdmin( )				{ return m_Admin; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableWarnRemove : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	bool m_Result;

public:
	CCallableWarnRemove( string nServer, string nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableWarnRemove( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableWarnList : virtual public CBaseCallable
{
protected:
	string m_Server;
	vector<CDBWarn *> m_Result;

public:
	CCallableWarnList( string nServer ) : CBaseCallable( ), m_Server( nServer ) { }
	virtual ~CCallableWarnList( );

	virtual vector<CDBWarn *> GetResult( )				{ return m_Result; }
	virtual void SetResult( vector<CDBWarn *> nResult )	{ m_Result = nResult; }
};

class CCallableRunQuery : virtual public CBaseCallable
{
protected:
	string m_Query;
	bool m_Result;

public:
	CCallableRunQuery( string nQuery ) : CBaseCallable( ), m_Query( nQuery ), m_Result( false ) { }
	virtual ~CCallableRunQuery( );

	virtual string GetQuery( )				{ return m_Query; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableBanCount : virtual public CBaseCallable
{
protected:
	string m_Server;
	uint32_t m_Result;

public:
	CCallableBanCount( string nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( 0 ) { }
	virtual ~CCallableBanCount( );

	virtual string GetServer( )					{ return m_Server; }
	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableBanCheck : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	string m_IP;
	CDBBan *m_Result;

public:
	CCallableBanCheck( string nServer, string nUser, string nIP ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_IP( nIP ), m_Result( NULL ) { }
	virtual ~CCallableBanCheck( );

	virtual string GetServer( )					{ return m_Server; }
	virtual string GetUser( )					{ return m_User; }
	virtual string GetIP( )						{ return m_IP; }
	virtual CDBBan *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBBan *nResult )	{ m_Result = nResult; }
};

class CCallableBanAdd : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	string m_IP;
	string m_GameName;
	string m_Admin;
	string m_Reason;
	bool m_Result;

public:
	CCallableBanAdd( string nServer, string nUser, string nIP, string nGameName, string nAdmin, string nReason ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_IP( nIP ), m_GameName( nGameName ), m_Admin( nAdmin ), m_Reason( nReason ), m_Result( false ) { }
	virtual ~CCallableBanAdd( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual string GetIP( )					{ return m_IP; }
	virtual string GetGameName( )			{ return m_GameName; }
	virtual string GetAdmin( )				{ return m_Admin; }
	virtual string GetReason( )				{ return m_Reason; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableBanRemove : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	bool m_Result;

public:
	CCallableBanRemove( string nServer, string nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableBanRemove( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableBanList : virtual public CBaseCallable
{
protected:
	string m_Server;
	vector<CDBBan *> m_Result;

public:
	CCallableBanList( string nServer ) : CBaseCallable( ), m_Server( nServer ) { }
	virtual ~CCallableBanList( );

	virtual vector<CDBBan *> GetResult( )				{ return m_Result; }
	virtual void SetResult( vector<CDBBan *> nResult )	{ m_Result = nResult; }
};

class CCallableGameAdd : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_Map;
	string m_GameName;
	string m_OwnerName;
	uint32_t m_Duration;
	uint32_t m_GameState;
	string m_CreatorName;
	string m_CreatorServer;
	uint32_t m_Result;

public:
	CCallableGameAdd( string nServer, string nMap, string nGameName, string nOwnerName, uint32_t nDuration, uint32_t nGameState, string nCreatorName, string nCreatorServer ) : CBaseCallable( ), m_Server( nServer ), m_Map( nMap ), m_GameName( nGameName ), m_OwnerName( nOwnerName ), m_Duration( nDuration ), m_GameState( nGameState ), m_CreatorName( nCreatorName ), m_CreatorServer( nCreatorServer ), m_Result( 0 ) { }
	virtual ~CCallableGameAdd( );

	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableGamePlayerAdd : virtual public CBaseCallable
{
protected:
	uint32_t m_GameID;
	string m_Name;
	string m_IP;
	uint32_t m_Spoofed;
	string m_SpoofedRealm;
	uint32_t m_Reserved;
	uint32_t m_LoadingTime;
	uint32_t m_Left;
	string m_LeftReason;
	uint32_t m_Team;
	uint32_t m_Colour;
	uint32_t m_Result;

public:
	CCallableGamePlayerAdd( uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour ) : CBaseCallable( ), m_GameID( nGameID ), m_Name( nName ), m_IP( nIP ), m_Spoofed( nSpoofed ), m_SpoofedRealm( nSpoofedRealm ), m_Reserved( nReserved ), m_LoadingTime( nLoadingTime ), m_Left( nLeft ), m_LeftReason( nLeftReason ), m_Team( nTeam ), m_Colour( nColour ), m_Result( 0 ) { }
	virtual ~CCallableGamePlayerAdd( );

	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableGamePlayerSummaryCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	CDBGamePlayerSummary *m_Result;

public:
	CCallableGamePlayerSummaryCheck( string nName ) : CBaseCallable( ), m_Name( nName ), m_Result( NULL ) { }
	virtual ~CCallableGamePlayerSummaryCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual CDBGamePlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBGamePlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableDotAGameAdd : virtual public CBaseCallable
{
protected:
	uint32_t m_GameID;
	uint32_t m_Winner;
	uint32_t m_Min;
	uint32_t m_Sec;
	uint32_t m_Result;

public:
	CCallableDotAGameAdd( uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec ) : CBaseCallable( ), m_GameID( nGameID ), m_Winner( nWinner ), m_Min( nMin ), m_Sec( nSec ), m_Result( 0 ) { }
	virtual ~CCallableDotAGameAdd( );

	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableDotAPlayerAdd : virtual public CBaseCallable
{
protected:
	uint32_t m_GameID;
	uint32_t m_Colour;
	uint32_t m_Kills;
	uint32_t m_Deaths;
	uint32_t m_CreepKills;
	uint32_t m_CreepDenies;
	uint32_t m_Assists;
	uint32_t m_Gold;
	uint32_t m_NeutralKills;
	string m_Item1;
	string m_Item2;
	string m_Item3;
	string m_Item4;
	string m_Item5;
	string m_Item6;
	string m_Hero;
	uint32_t m_NewColour;
	uint32_t m_TowerKills;
	uint32_t m_RaxKills;
	uint32_t m_CourierKills;
	uint32_t m_Result;

public:
	CCallableDotAPlayerAdd( uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, string nItem1, string nItem2, string nItem3, string nItem4, string nItem5, string nItem6, string nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills ) : CBaseCallable( ), m_GameID( nGameID ), m_Colour( nColour ), m_Kills( nKills ), m_Deaths( nDeaths ), m_CreepKills( nCreepKills ), m_CreepDenies( nCreepDenies ), m_Assists( nAssists ), m_Gold( nGold ), m_NeutralKills( nNeutralKills ), m_Item1( nItem1 ), m_Item2( nItem2 ), m_Item3( nItem3 ), m_Item4( nItem4 ), m_Item5( nItem5 ), m_Item6( nItem6 ), m_Hero( nHero ), m_NewColour( nNewColour ), m_TowerKills( nTowerKills ), m_RaxKills( nRaxKills ), m_CourierKills( nCourierKills ), m_Result( 0 ) { }
	virtual ~CCallableDotAPlayerAdd( );

	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableDotAPlayerSummaryCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	CDBDotAPlayerSummary *m_Result;

public:
	CCallableDotAPlayerSummaryCheck( string nName ) : CBaseCallable( ), m_Name( nName ), m_Result( NULL ) { }
	virtual ~CCallableDotAPlayerSummaryCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual CDBDotAPlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBDotAPlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableDotAPlayerKillerCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	uint32_t m_Rank;
	CDBDotAPlayerKiller *m_Result;

public:
	CCallableDotAPlayerKillerCheck( string nName, uint32_t nRank ) : CBaseCallable( ), m_Name( nName ), m_Rank( nRank ), m_Result( NULL ) { }
	virtual ~CCallableDotAPlayerKillerCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual uint32_t GetRank( )								{ return m_Rank; }
	virtual CDBDotAPlayerKiller *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBDotAPlayerKiller *nResult )	{ m_Result = nResult; }
};

class CCallableDotAPlayerFarmerCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	uint32_t m_Rank;
	CDBDotAPlayerFarmer *m_Result;

public:
	CCallableDotAPlayerFarmerCheck( string nName, uint32_t nRank ) : CBaseCallable( ), m_Name( nName ), m_Rank( nRank ), m_Result( NULL ) { }
	virtual ~CCallableDotAPlayerFarmerCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual uint32_t GetRank( )								{ return m_Rank; }
	virtual CDBDotAPlayerFarmer *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBDotAPlayerFarmer *nResult )	{ m_Result = nResult; }
};

class CCallableDownloadAdd : virtual public CBaseCallable
{
protected:
	string m_Map;
	uint32_t m_MapSize;
	string m_Name;
	string m_IP;
	uint32_t m_Spoofed;
	string m_SpoofedRealm;
	uint32_t m_DownloadTime;
	bool m_Result;

public:
	CCallableDownloadAdd( string nMap, uint32_t nMapSize, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nDownloadTime ) : CBaseCallable( ), m_Map( nMap ), m_MapSize( nMapSize ), m_Name( nName ), m_IP( nIP ), m_Spoofed( nSpoofed ), m_SpoofedRealm( nSpoofedRealm ), m_DownloadTime( nDownloadTime ), m_Result( false ) { }
	virtual ~CCallableDownloadAdd( );

	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableScoreCheck : virtual public CBaseCallable
{
protected:
	string m_Category;
	string m_Name;
	string m_Server;
	double m_Result;

public:
	CCallableScoreCheck( string nCategory, string nName, string nServer ) : CBaseCallable( ), m_Category( nCategory ), m_Name( nName ), m_Server( nServer ), m_Result( 0.0 ) { }
	virtual ~CCallableScoreCheck( );

	virtual string GetName( )					{ return m_Name; }
	virtual double GetResult( )					{ return m_Result; }
	virtual void SetResult( double nResult )	{ m_Result = nResult; }
};

class CCallableW3MMDPlayerAdd : virtual public CBaseCallable
{
protected:
	string m_Category;
	uint32_t m_GameID;
	uint32_t m_PID;
	string m_Name;
	string m_Flag;
	uint32_t m_Leaver;
	uint32_t m_Practicing;
	uint32_t m_Result;

public:
	CCallableW3MMDPlayerAdd( string nCategory, uint32_t nGameID, uint32_t nPID, string nName, string nFlag, uint32_t nLeaver, uint32_t nPracticing ) : CBaseCallable( ), m_Category( nCategory ), m_GameID( nGameID ), m_PID( nPID ), m_Name( nName ), m_Flag( nFlag ), m_Leaver( nLeaver ), m_Practicing( nPracticing ), m_Result( 0 ) { }
	virtual ~CCallableW3MMDPlayerAdd( );

	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableW3MMDVarAdd : virtual public CBaseCallable
{
protected:
	uint32_t m_GameID;
	map<VarP,int32_t> m_VarInts;
	map<VarP,double> m_VarReals;
	map<VarP,string> m_VarStrings;

	enum ValueType {
		VALUETYPE_INT = 1,
		VALUETYPE_REAL = 2,
		VALUETYPE_STRING = 3
	};

	ValueType m_ValueType;
	bool m_Result;

public:
	CCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,int32_t> nVarInts ) : CBaseCallable( ), m_GameID( nGameID ), m_VarInts( nVarInts ), m_ValueType( VALUETYPE_INT ), m_Result( false ) { }
	CCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,double> nVarReals ) : CBaseCallable( ), m_GameID( nGameID ), m_VarReals( nVarReals ), m_ValueType( VALUETYPE_REAL ), m_Result( false ) { }
	CCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,string> nVarStrings ) : CBaseCallable( ), m_GameID( nGameID ), m_VarStrings( nVarStrings ), m_ValueType( VALUETYPE_STRING ), m_Result( false ) { }
	virtual ~CCallableW3MMDVarAdd( );

	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

//
// CDBBan
//
class CDBBan
{
private:
	string m_Server;
	string m_Name;
	string m_IP;
	string m_Date;
	string m_GameName;
	string m_Admin;
	string m_Reason;

public:
	CDBBan( string nServer, string nName, string nIP, string nDate, string nGameName, string nAdmin, string nReason );
	~CDBBan( );

	string GetServer( )		{ return m_Server; }
	string GetName( )		{ return m_Name; }
	string GetIP( )			{ return m_IP; }
	string GetDate( )		{ return m_Date; }
	string GetGameName( )	{ return m_GameName; }
	string GetAdmin( )		{ return m_Admin; }
	string GetReason( )		{ return m_Reason; }
};

//
// CDBMail
//
class CDBMail
{
private:
	uint32_t m_Id;
	string m_Server;
	string m_Sender;
	string m_Receiver;
	string m_Message;
	uint32_t m_Readed;
	string m_Date;
public:
	CDBMail( uint32_t nId, string nServer, string nSender, string nReceiver, string nMessage, uint32_t nReaded, string nDate );
	~CDBMail( );

	uint32_t GetId( )			{ return m_Id; }
	string GetServer( )			{ return m_Server; }
	string GetSender( )			{ return m_Sender; }
	string GetReceiver( )		{ return m_Receiver; }
	string GetMessage( )		{ return m_Message; }
	uint32_t GetReaded( )		{ return m_Readed; }
	string GetDate( )			{ return m_Date; }
	void SetReaded( )			{ m_Readed = 1; }
};

//
// CDBInfo
//
class CDBInfo
{
private:
	uint32_t m_Id;
	string m_Server;
	string m_Name;
	uint32_t m_Lvl;
	uint32_t m_PrivRank;
	uint32_t m_PubRank;
	uint32_t m_PrivPoints;
	uint32_t m_PubPoints;
	string m_Admin;
	string m_Country;
	uint32_t m_Challwins;
	uint32_t m_Challloses;
	string m_Ginfo;
	uint32_t m_Message;
	string m_Date;

public:
	CDBInfo( uint32_t nId, string nServer, string nName, uint32_t nLvl, uint32_t nPrivRank, uint32_t nPubRank, uint32_t nPrivPoints, uint32_t nPubPoints, string nAdmin, string nCountry, uint32_t nChallwins, uint32_t nChallloses, string nGinfo,uint32_t nMessage, string nDate );
	~CDBInfo( );

	uint32_t GetId( )			{ return m_Id; }
	string GetServer( )			{ return m_Server; }
	string GetUser( )			{ return m_Name; }
	uint32_t GetLvl( )			{ return m_Lvl; }
	uint32_t GetPrivRank( )		{ return m_PrivRank; }
	uint32_t GetPubRank( )		{ return m_PubRank; }
	uint32_t GetPrivPoints( )	{ return m_PrivPoints; }
	uint32_t GetPubPoints( )	{ return m_PubPoints; }
	string GetAdmin( )			{ return m_Admin; }
	string GetCountry( )		{ return m_Country; }
	uint32_t GetChallwins( )	{ return m_Challwins; }
	uint32_t GetChallloses( )	{ return m_Challloses; }
	string GetGinfo( )			{ return m_Ginfo; }
	uint32_t GetMessage( )		{ return m_Message; }
	string GetDate( )			{ return m_Date; }
	void UpdatePrivRank( uint32_t nRank ) { m_PrivRank = nRank; }
	void UpdatePubRank( uint32_t nRank ) { m_PubRank = nRank; }

};

class CDBWarn
{
private:
	uint32_t m_Id;
	string m_Server;
	string m_Name;
	uint32_t m_Warnings;
	string m_Warning;
	uint32_t m_Totalwarn;
	uint32_t m_Daysban;
	string m_Admin;
	string m_Date;

public:
	CDBWarn( uint32_t nId, string nServer, string nName, uint32_t nWarnings, string nWarning, uint32_t nTotalwarn, uint32_t nDaysban, string nAdmin, string nDate );
	~CDBWarn( );

	uint32_t GetId( )		{ return m_Id; }
	string GetServer( )		{ return m_Server; }
	string GetName( )		{ return m_Name; }
	uint32_t GetWarnings( )	{ return m_Warnings; }
	string GetWarning( )	{ return m_Warning; }
	uint32_t GetTotalwarn( ){ return m_Totalwarn; }
	uint32_t GetDaysban( )	{ return m_Daysban; }
	string GetAdmin( )		{ return m_Admin; }
	string GetDate( )		{ return m_Date; }
};

class CDBGames
{
private:
	uint32_t m_GameId;
	string m_Hostbot;
	string m_Owner;
	uint32_t m_StartTime;
	uint32_t m_UsersSlots;
	uint32_t m_GameState;
	bool m_CreepsSpawn;
	string m_GameName;
	string m_Names[12];
	string m_Obs;

public:
	CDBGames( uint32_t nGameId, string nHostbot, string nOwner, uint32_t nStartTime, uint32_t nUsersNumber, uint32_t nGameState, string nGameName );
	~CDBGames( );
	uint32_t GetGameId( )		{ return m_GameId; }
	string GetHostbot( )		{ return m_Hostbot; }
	string GetOwner( )			{ return m_Owner; }
	uint32_t GetStartTime( )	{ return m_StartTime; }
	uint32_t GetUsersSlots( )	{ return m_UsersSlots; }
	uint32_t GetGameState( )	{ return m_GameState; }
	bool GetCreepsSpawn( )		{ return m_CreepsSpawn; }
	string GetGameName( )		{ return m_GameName; }	
	string GetTeam1( )			
	{  
		string nNames;
		for (int i=0;i<5;i++)		
			if(!m_Names[i].empty( ))
				nNames += UTIL_ToString(i+1)+". "+m_Names[i]+" ";
		
		return nNames;
	}
	string GetTeam2( )
	{  
		string nNames;
		for (int i=5;i<10;i++)
			if(!m_Names[i].empty( ))
				nNames += UTIL_ToString(i+1)+". "+m_Names[i]+" ";
		return nNames;
	}
	string GetObs( )
	{  
		string nNames="";
		for (int i=10;i<12;i++)
		{
			if(!m_Names[i].empty())
				nNames += UTIL_ToString(i+1)+". "+m_Names[i]+" ";
		}
		return nNames;
	}
	string GetName( uint32_t nSlot )	{ return m_Names[nSlot]; }
	void DelName( uint32_t nSlot ) {m_Names[nSlot] = "*"+m_Names[nSlot];}
	void PutNames(string nName, uint32_t nSlot)	{ m_Names[nSlot] = nName; }
	void PutObs(string nObs)		{ m_Obs = nObs; }
	void UpdateGameSlots(uint32_t nGameSlots) { m_UsersSlots = nGameSlots; }
	void UpdateGameState(uint32_t nGameState) { m_GameState = nGameState; }
	void UpdateCreepsSpawn( )	{ m_CreepsSpawn = true; }
	void UpdateGameName(string nGameName) { m_GameName = nGameName; }
	void UpdateGameTime(uint32_t nStartTime) { m_StartTime = nStartTime; }
	void UpdateGameOwner(string nGameOwner) { m_Owner = nGameOwner; }
};

class CDBChannel
{
private:
	string m_User;
	uint32_t m_Time;
	uint32_t m_Lvl;
	bool m_Banned;
	bool m_Topaz;
	bool m_Signed;
	uint32_t m_SignedTime;
	string m_Mode;
public:
	CDBChannel( string nUser, uint32_t nTime, uint32_t nLvl, bool nBanned, bool nTopaz );
	~CDBChannel( );
	string GetUser( )			{ return m_User; }
	uint32_t GetSec( )			{ return m_Time; }
	uint32_t GetLvl( )			{ return m_Lvl; }
	bool GetBanned( )			{ return m_Banned; }
	bool GetTopaz( )			{ return m_Topaz; }
	void ResetTime( )			{ m_Time = GetTime( ); }
	bool GetSigned( )			{ return m_Signed; }
	uint32_t GetSignedTime( )	{ return m_SignedTime; }
	string GetMode( )			{ return m_Mode; }
	
	void UpdateSign( string mode ) { m_Signed = true; m_Mode = mode; }
	void UpdateSignTime( )			{ m_SignedTime = GetTime( ); }
	void ChangeMode( string mode ) { m_Mode = mode; }
	void ResetSign( )		{m_Signed = false; m_Mode = "-"; }
	
};

class CDBMaps
{
private:
	string m_User;
	string m_Map;
	uint32_t m_Obs;
public:
	CDBMaps( string nUser, string nMap );
	~CDBMaps( );
	string GetUser( )		{ return m_User; }
	string GetMap( )		{ return m_Map; }
	uint32_t GetObs( )		{ return m_Obs; }
	void UpdateMap( string nMap ) { m_Map = nMap; }
	void UpdateObs( uint32_t nObs) { m_Obs = nObs; }
};

class CDBHostBot
{
private:
	string m_HostBot;
	uint32_t m_State;//0 dead 1 ready 2 lobby
	uint32_t m_HostedGames;
	uint32_t m_MaxGames;
public:
	CDBHostBot( string nHostBot, uint32_t nState, uint32_t m_HostedGames, uint32_t MaxGames );
	~CDBHostBot( );
	string GetHostBot( )								{ return m_HostBot; }
	uint32_t GetState( )								{ return m_State; }	
	uint32_t GetGames( )								{ return m_HostedGames; }
	uint32_t GetMaxGames( )								{ return m_MaxGames; }
	void UpdateState( uint32_t nState )					{ m_State = nState; }
	void UpdateGames( uint32_t nHostedGames )			{ m_HostedGames = nHostedGames; }
	void UpdateMaxGames( uint32_t nMaxGames )			{ m_MaxGames = nMaxGames; }
	
};

class CDBIcons
{
private:
	string m_User;
	string m_From;
	string m_Icon;
	string m_Reason;	
public:
	CDBIcons( string nUser, string nFrom, string nIcon, string nReason );
	~CDBIcons( );
	string GetUser( )		{ return m_User; }	
	string GetFrom( )		{ return m_From; }
	string GetIcon( )		{ return m_Icon; }
	string GetReason( )		{ return m_Reason; }
	

};

//
// CDBGame
//
class CDBGame
{
private:
	uint32_t m_ID;
	string m_Server;
	string m_Map;
	string m_DateTime;
	string m_GameName;
	string m_OwnerName;
	uint32_t m_Duration;

public:
	CDBGame( uint32_t nID, string nServer, string nMap, string nDateTime, string nGameName, string nOwnerName, uint32_t nDuration );
	~CDBGame( );

	uint32_t GetID( )		{ return m_ID; }
	string GetServer( )		{ return m_Server; }
	string GetMap( )		{ return m_Map; }
	string GetDateTime( )	{ return m_DateTime; }
	string GetGameName( )	{ return m_GameName; }
	string GetOwnerName( )	{ return m_OwnerName; }
	uint32_t GetDuration( )	{ return m_Duration; }

	void SetDuration( uint32_t nDuration )	{ m_Duration = nDuration; }
};

//
// CDBGamePlayer
//
class CDBGamePlayer
{
private:
	uint32_t m_ID;
	uint32_t m_GameID;
	string m_Name;
	string m_IP;
	uint32_t m_Spoofed;
	string m_SpoofedRealm;
	uint32_t m_Reserved;
	uint32_t m_LoadingTime;
	uint32_t m_Left;
	string m_LeftReason;
	uint32_t m_Team;
	uint32_t m_Colour;

public:
	CDBGamePlayer( uint32_t nID, uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour );
	~CDBGamePlayer( );

	uint32_t GetID( )			{ return m_ID; }
	uint32_t GetGameID( )		{ return m_GameID; }
	string GetName( )			{ return m_Name; }
	string GetIP( )				{ return m_IP; }
	uint32_t GetSpoofed( )		{ return m_Spoofed; }
	string GetSpoofedRealm( )	{ return m_SpoofedRealm; }
	uint32_t GetReserved( )		{ return m_Reserved; }
	uint32_t GetLoadingTime( )	{ return m_LoadingTime; }
	uint32_t GetLeft( )			{ return m_Left; }
	string GetLeftReason( )		{ return m_LeftReason; }
	uint32_t GetTeam( )			{ return m_Team; }
	uint32_t GetColour( )		{ return m_Colour; }

	void SetLoadingTime( uint32_t nLoadingTime )	{ m_LoadingTime = nLoadingTime; }
	void SetLeft( uint32_t nLeft )					{ m_Left = nLeft; }
	void SetLeftReason( string nLeftReason )		{ m_LeftReason = nLeftReason; }
};

//
// CDBGamePlayerSummary
//
class CDBGamePlayerSummary
{
private:
	string m_Server;
	string m_Name;
	string m_FirstGameDateTime;		// datetime of first game played
	string m_LastGameDateTime;		// datetime of last game played
	uint32_t m_TotalGames;			// total number of games played
	uint32_t m_MinLoadingTime;		// minimum loading time in milliseconds (this could be skewed because different maps have different load times)
	uint32_t m_AvgLoadingTime;		// average loading time in milliseconds (this could be skewed because different maps have different load times)
	uint32_t m_MaxLoadingTime;		// maximum loading time in milliseconds (this could be skewed because different maps have different load times)
	uint32_t m_MinLeftPercent;		// minimum time at which the player left the game expressed as a percentage of the game duration (0-100)
	uint32_t m_AvgLeftPercent;		// average time at which the player left the game expressed as a percentage of the game duration (0-100)
	uint32_t m_MaxLeftPercent;		// maximum time at which the player left the game expressed as a percentage of the game duration (0-100)
	uint32_t m_MinDuration;			// minimum game duration in seconds
	uint32_t m_AvgDuration;			// average game duration in seconds
	uint32_t m_MaxDuration;			// maximum game duration in seconds

public:
	CDBGamePlayerSummary( string nServer, string nName, string nFirstGameDateTime, string nLastGameDateTime, uint32_t nTotalGames, uint32_t nMinLoadingTime, uint32_t nAvgLoadingTime, uint32_t nMaxLoadingTime, uint32_t nMinLeftPercent, uint32_t nAvgLeftPercent, uint32_t nMaxLeftPercent, uint32_t nMinDuration, uint32_t nAvgDuration, uint32_t nMaxDuration );
	~CDBGamePlayerSummary( );

	string GetServer( )					{ return m_Server; }
	string GetName( )					{ return m_Name; }
	string GetFirstGameDateTime( )		{ return m_FirstGameDateTime; }
	string GetLastGameDateTime( )		{ return m_LastGameDateTime; }
	uint32_t GetTotalGames( )			{ return m_TotalGames; }
	uint32_t GetMinLoadingTime( )		{ return m_MinLoadingTime; }
	uint32_t GetAvgLoadingTime( )		{ return m_AvgLoadingTime; }
	uint32_t GetMaxLoadingTime( )		{ return m_MaxLoadingTime; }
	uint32_t GetMinLeftPercent( )		{ return m_MinLeftPercent; }
	uint32_t GetAvgLeftPercent( )		{ return m_AvgLeftPercent; }
	uint32_t GetMaxLeftPercent( )		{ return m_MaxLeftPercent; }
	uint32_t GetMinDuration( )			{ return m_MinDuration; }
	uint32_t GetAvgDuration( )			{ return m_AvgDuration; }
	uint32_t GetMaxDuration( )			{ return m_MaxDuration; }
};

//
// CDBDotAGame
//
class CDBDotAGame
{
private:
	uint32_t m_ID;
	uint32_t m_GameID;
	uint32_t m_Winner;
	uint32_t m_Min;
	uint32_t m_Sec;

public:
	CDBDotAGame( uint32_t nID, uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec );
	~CDBDotAGame( );

	uint32_t GetID( )		{ return m_ID; }
	uint32_t GetGameID( )	{ return m_GameID; }
	uint32_t GetWinner( )	{ return m_Winner; }
	uint32_t GetMin( )		{ return m_Min; }
	uint32_t GetSec( )		{ return m_Sec; }
};

//
// CDBDotAPlayer
//
class CDBDotAPlayer
{
private:
	uint32_t m_ID;
	uint32_t m_GameID;
	uint32_t m_Colour;
	uint32_t m_Kills;
	uint32_t m_Deaths;
	uint32_t m_CreepKills;
	uint32_t m_CreepDenies;
	uint32_t m_Assists;
	uint32_t m_Gold;
	uint32_t m_NeutralKills;
	string m_Items[6];
	string m_Hero;
	uint32_t m_NewColour;
	uint32_t m_TowerKills;
	uint32_t m_RaxKills;
	uint32_t m_CourierKills;

public:
	CDBDotAPlayer( );
	CDBDotAPlayer( uint32_t nID, uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, string nItem1, string nItem2, string nItem3, string nItem4, string nItem5, string nItem6, string nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills );
	~CDBDotAPlayer( );

	uint32_t GetID( )			{ return m_ID; }
	uint32_t GetGameID( )		{ return m_GameID; }
	uint32_t GetColour( )		{ return m_Colour; }
	uint32_t GetKills( )		{ return m_Kills; }
	uint32_t GetDeaths( )		{ return m_Deaths; }
	uint32_t GetCreepKills( )	{ return m_CreepKills; }
	uint32_t GetCreepDenies( )	{ return m_CreepDenies; }
	uint32_t GetAssists( )		{ return m_Assists; }
	uint32_t GetGold( )			{ return m_Gold; }
	uint32_t GetNeutralKills( )	{ return m_NeutralKills; }
	string GetItem( unsigned int i );
	string GetHero( )			{ return m_Hero; }
	uint32_t GetNewColour( )	{ return m_NewColour; }
	uint32_t GetTowerKills( )	{ return m_TowerKills; }
	uint32_t GetRaxKills( )		{ return m_RaxKills; }
	uint32_t GetCourierKills( )	{ return m_CourierKills; }

	void SetColour( uint32_t nColour )				{ m_Colour = nColour; }
	void SetKills( uint32_t nKills )				{ m_Kills = nKills; }
	void SetDeaths( uint32_t nDeaths )				{ m_Deaths = nDeaths; }
	void SetCreepKills( uint32_t nCreepKills )		{ m_CreepKills = nCreepKills; }
	void SetCreepDenies( uint32_t nCreepDenies )	{ m_CreepDenies = nCreepDenies; }
	void SetAssists( uint32_t nAssists )			{ m_Assists = nAssists; }
	void SetGold( uint32_t nGold )					{ m_Gold = nGold; }
	void SetNeutralKills( uint32_t nNeutralKills )	{ m_NeutralKills = nNeutralKills; }
	void SetItem( unsigned int i, string item );
	void SetHero( string nHero )					{ m_Hero = nHero; }
	void SetNewColour( uint32_t nNewColour )		{ m_NewColour = nNewColour; }
	void SetTowerKills( uint32_t nTowerKills )		{ m_TowerKills = nTowerKills; }
	void SetRaxKills( uint32_t nRaxKills )			{ m_RaxKills = nRaxKills; }
	void SetCourierKills( uint32_t nCourierKills )	{ m_CourierKills = nCourierKills; }
};

//
// CDBDotAPlayerSummary
//
class CDBDotAPlayerSummary
{
private:
	string m_Server;
	string m_Name;
	uint32_t m_TotalGames;			// total number of dota games played
	uint32_t m_TotalWins;			// total number of dota games won
	uint32_t m_TotalLosses;			// total number of dota games lost
	uint32_t m_TotalKills;			// total number of hero kills
	uint32_t m_TotalDeaths;			// total number of deaths
	uint32_t m_TotalCreepKills;		// total number of creep kills
	uint32_t m_TotalCreepDenies;	// total number of creep denies
	uint32_t m_TotalAssists;		// total number of assists
	uint32_t m_TotalNeutralKills;	// total number of neutral kills
	uint32_t m_TotalTowerKills;		// total number of tower kills
	uint32_t m_TotalRaxKills;		// total number of rax kills
	uint32_t m_TotalCourierKills;	// total number of courier kills

public:
	CDBDotAPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, uint32_t nTotalCreepKills, uint32_t nTotalCreepDenies, uint32_t nTotalAssists, uint32_t nTotalNeutralKills, uint32_t nTotalTowerKills, uint32_t nTotalRaxKills, uint32_t nTotalCourierKills );
	~CDBDotAPlayerSummary( );

	string GetServer( )					{ return m_Server; }
	string GetName( )					{ return m_Name; }
	uint32_t GetTotalGames( )			{ return m_TotalGames; }
	uint32_t GetTotalWins( )			{ return m_TotalWins; }
	uint32_t GetTotalLosses( )			{ return m_TotalLosses; }
	uint32_t GetTotalKills( )			{ return m_TotalKills; }
	uint32_t GetTotalDeaths( )			{ return m_TotalDeaths; }
	uint32_t GetTotalCreepKills( )		{ return m_TotalCreepKills; }
	uint32_t GetTotalCreepDenies( )		{ return m_TotalCreepDenies; }
	uint32_t GetTotalAssists( )			{ return m_TotalAssists; }
	uint32_t GetTotalNeutralKills( )	{ return m_TotalNeutralKills; }
	uint32_t GetTotalTowerKills( )		{ return m_TotalTowerKills; }
	uint32_t GetTotalRaxKills( )		{ return m_TotalRaxKills; }
	uint32_t GetTotalCourierKills( )	{ return m_TotalCourierKills; }

	float GetAvgKills( )				{ return m_TotalGames > 0 ? (float)m_TotalKills / m_TotalGames : 0; }
	float GetAvgDeaths( )				{ return m_TotalGames > 0 ? (float)m_TotalDeaths / m_TotalGames : 0; }
	float GetAvgCreepKills( )			{ return m_TotalGames > 0 ? (float)m_TotalCreepKills / m_TotalGames : 0; }
	float GetAvgCreepDenies( )			{ return m_TotalGames > 0 ? (float)m_TotalCreepDenies / m_TotalGames : 0; }
	float GetAvgAssists( )				{ return m_TotalGames > 0 ? (float)m_TotalAssists / m_TotalGames : 0; }
	float GetAvgNeutralKills( )			{ return m_TotalGames > 0 ? (float)m_TotalNeutralKills / m_TotalGames : 0; }
	float GetAvgTowerKills( )			{ return m_TotalGames > 0 ? (float)m_TotalTowerKills / m_TotalGames : 0; }
	float GetAvgRaxKills( )				{ return m_TotalGames > 0 ? (float)m_TotalRaxKills / m_TotalGames : 0; }
	float GetAvgCourierKills( )			{ return m_TotalGames > 0 ? (float)m_TotalCourierKills / m_TotalGames : 0; }
};

//
// CDBDotAPlayerKiller
//
class CDBDotAPlayerKiller
{
private:
	string m_Server;
	uint32_t m_Gameid;
	string m_Name;
	uint32_t m_TotalKills;			// total number of hero kills
	uint32_t m_TotalDeaths;			// total number of deaths
	uint32_t m_TotalAssists;		// total number of assists
public:
	CDBDotAPlayerKiller( string nServer, uint32_t nGameid, string nName, uint32_t nTotalKills, uint32_t nTotalDeaths, uint32_t nTotalAssists );
	~CDBDotAPlayerKiller( );

	string GetServer( )					{ return m_Server; }
	uint32_t GetGameid( )				{ return m_Gameid; }
	string GetName( )					{ return m_Name; }
	uint32_t GetTotalKills( )			{ return m_TotalKills; }
	uint32_t GetTotalDeaths( )			{ return m_TotalDeaths; }
	uint32_t GetTotalAssists( )			{ return m_TotalAssists; }
};

class CDBDotAPlayerFarmer
{
private:
	string m_Server;
	uint32_t m_Gameid;
	string m_Name;
	uint32_t m_TotalCreepKills;		// total number of creep kills
	uint32_t m_TotalCreepDenies;	// total number of creep denies
	uint32_t m_TotalNeutralKills;	// total number of neutral kills

public:
	CDBDotAPlayerFarmer( string nServer, uint32_t nGameid, string nName, uint32_t nTotalCreepKills, uint32_t nTotalCreepDenies, uint32_t nTotalNeutralKills );
	~CDBDotAPlayerFarmer( );

	string GetServer( )					{ return m_Server; }
	uint32_t GetGameid( )				{ return m_Gameid; }
	string GetName( )					{ return m_Name; }
	uint32_t GetTotalCreepKills( )		{ return m_TotalCreepKills; }
	uint32_t GetTotalCreepDenies( )		{ return m_TotalCreepDenies; }
	uint32_t GetTotalNeutralKills( )	{ return m_TotalNeutralKills; }
};
#endif
