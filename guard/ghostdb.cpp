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

//
// CGHostDB
//

CGHostDB :: CGHostDB( CConfig *CFG )
{
	m_HasError = false;
}

CGHostDB :: ~CGHostDB( )
{

}

void CGHostDB :: RecoverCallable( CBaseCallable *callable )
{

}

bool CGHostDB :: Begin( )
{
	return true;
}

bool CGHostDB :: Commit( )
{
	return true;
}

bool CGHostDB :: MailAdd( uint32_t id, string server, string sender, string receiver, string message )
{
	return false;
}

bool CGHostDB :: MailRemove( string server, string user, uint32_t id )
{
	return false;
}

bool CGHostDB :: MailReaded( string server, string user )
{
	return false;
}

vector<CDBMail *> CGHostDB :: MailList( string server )
{
	return vector<CDBMail *>( );
}


uint32_t CGHostDB :: InfoCount( string server )
{
	return 0;
}

CDBInfo *CGHostDB :: InfoCheck( string server, string user)
{
	return NULL;
}

bool CGHostDB :: InfoAdd( string server, string user, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t pubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message)
{
	return false;
}

bool CGHostDB :: InfoPrivCalculate( string server)
{
	return false;
}

bool CGHostDB :: InfoPubCalculate( string server)
{
	return false;
}

bool CGHostDB :: InfoPrivPoints( string server, string user, uint32_t privpoints )
{
	return false;
}

bool CGHostDB :: InfoPubPoints( string server, string user, uint32_t pubpoints )
{
	return false;
}

bool CGHostDB :: InfoChallwins( string server, string user, uint32_t challwins )
{
	return false;
}

bool CGHostDB :: InfoChallloses( string server, string user, uint32_t challloses )
{
	return false;
}

bool CGHostDB :: InfoRemove( string server, string user )
{
	return false;
}
vector<CDBInfo *> CGHostDB :: InfoList( string server )
{
	return vector<CDBInfo *>( );
}

bool CGHostDB :: InfoUpdateCountry( string server, string user, string country)
{
	return false;
}

bool CGHostDB :: InfoUpdateLvl( string server, string user, uint32_t lvl, string admin)
{
	return false;
}

bool CGHostDB :: InfoUpdateMessage( string server, string user, uint32_t message)
{
	return false;
}

bool CGHostDB :: InfoUpdateGinfo( string server, string user, string ginfo, string admin)
{
	return false;
}



uint32_t CGHostDB :: WarnCount( string server )
{
	return 0;
}

CDBWarn *CGHostDB :: WarnCheck( string server, string user)
{
	return NULL;
}
bool CGHostDB :: WarnAdd(string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin)
{
	return false;
}
bool CGHostDB :: WarnUpdateAdd(string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, string admin)
{
	return false;
}
bool CGHostDB :: WarnChannelBan(string server, string user, uint32_t daysban, string admin)
{
	return false;
}

bool CGHostDB :: WarnRemove( string server, string user )
{
	return false;
}

vector<CDBWarn *> CGHostDB :: WarnList( string server )
{
	return vector<CDBWarn *>( );
}

bool CGHostDB :: RunQuery( string query )
{
	return false;
}
/////////////////////////////////////////////////////

uint32_t CGHostDB :: BanCount( string server )
{
	return 0;
}

CDBBan *CGHostDB :: BanCheck( string server, string user, string ip )
{
	return NULL;
}

bool CGHostDB :: BanAdd( string server, string user, string ip, string gamename, string admin, string reason )
{
	return false;
}

bool CGHostDB :: BanRemove( string server, string user )
{
	return false;
}

bool CGHostDB :: BanRemove( string user )
{
	return false;
}

vector<CDBBan *> CGHostDB :: BanList( string server )
{
	return vector<CDBBan *>( );
}

uint32_t CGHostDB :: GameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver )
{
	return 0;
}

uint32_t CGHostDB :: GamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour )
{
	return 0;
}

uint32_t CGHostDB :: GamePlayerCount( string name )
{
	return 0;
}

CDBGamePlayerSummary *CGHostDB :: GamePlayerSummaryCheck( string name )
{
	return NULL;
}

uint32_t CGHostDB :: DotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
	return 0;
}

uint32_t CGHostDB :: DotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills )
{
	return 0;
}

uint32_t CGHostDB :: DotAPlayerCount( string name )
{
	return 0;
}

CDBDotAPlayerSummary *CGHostDB :: DotAPlayerSummaryCheck( string name )
{
	return NULL;
}

CDBDotAPlayerKiller *CGHostDB :: DotAPlayerKillerCheck( string name, uint32_t rank )
{
	return NULL;
}

CDBDotAPlayerFarmer *CGHostDB :: DotAPlayerFarmerCheck( string name, uint32_t rank )
{
	return NULL;
}

string CGHostDB :: FromCheck( uint32_t ip )
{
	return "??";
}

string CGHostDB :: FullFromCheck( uint32_t ip )
{
	return "??";
}

bool CGHostDB :: FromAdd(  uint32_t ip1, uint32_t ip2, string country, string fullcountry )
{
	return false;
}

bool CGHostDB :: DownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime )
{
	return false;
}

uint32_t CGHostDB :: W3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing )
{
	return 0;
}

bool CGHostDB :: W3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints )
{
	return false;
}

bool CGHostDB :: W3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals )
{
	return false;
}

bool CGHostDB :: W3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings )
{
	return false;
}

void CGHostDB :: CreateThread( CBaseCallable *callable )
{
	callable->SetReady( true );
}

CCallableMailAdd *CGHostDB :: ThreadedMailAdd( uint32_t id, string server, string sender, string receiver, string message )
{
	return NULL;
}
CCallableMailReaded *CGHostDB :: ThreadedMailReaded( string server, string user )
{
	return NULL;
}
CCallableMailRemove *CGHostDB :: ThreadedMailRemove( string server, string user, uint32_t id )
{
	return NULL;
}

CCallableMailList *CGHostDB :: ThreadedMailList( string server )
{
	return NULL;
}

CCallableInfoCount *CGHostDB :: ThreadedInfoCount( string server )
{
	return NULL;
}

CCallableInfoCheck *CGHostDB :: ThreadedInfoCheck( string server, string user )
{
	return NULL;
}

CCallableInfoAdd *CGHostDB :: ThreadedInfoAdd( string server, string user, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t pubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message )
{
	return NULL;
}

CCallableInfoPrivCalculate *CGHostDB :: ThreadedInfoPrivCalculate( string server)
{
	return NULL;
}

CCallableInfoPubCalculate *CGHostDB :: ThreadedInfoPubCalculate( string server)
{
	return NULL;
}

CCallableInfoPrivPoints *CGHostDB :: ThreadedInfoPrivPoints( string server, string user, uint32_t privpoints )
{
	return NULL;
}

CCallableInfoPubPoints *CGHostDB :: ThreadedInfoPubPoints( string server, string user, uint32_t pubpoints )
{
	return NULL;
}

CCallableInfoChallwins *CGHostDB :: ThreadedInfoChallwins( string server, string user, uint32_t challwins )
{
	return NULL;
}

CCallableInfoChallloses *CGHostDB :: ThreadedInfoChallloses( string server, string user, uint32_t challloses )
{
	return NULL;
}

CCallableInfoRemove *CGHostDB :: ThreadedInfoRemove( string server, string user )
{
	return NULL;
}

CCallableInfoList *CGHostDB :: ThreadedInfoList( string server )
{
	return NULL;
}

CCallableInfoUpdateCountry *CGHostDB :: ThreadedInfoUpdateCountry( string server, string user, string country )
{
	return NULL;
}

CCallableInfoUpdateLvl *CGHostDB :: ThreadedInfoUpdateLvl( string server, string user, uint32_t lvl, string admin )
{
	return NULL;
}

CCallableInfoUpdateMessage *CGHostDB :: ThreadedInfoUpdateMessage( string server, string user, uint32_t message )
{
	return NULL;
}

CCallableInfoUpdateGinfo *CGHostDB :: ThreadedInfoUpdateGinfo( string server, string user, string ginfo, string admin )
{
	return NULL;
}

CCallableWarnCount *CGHostDB :: ThreadedWarnCount( string server )
{
	return NULL;
}

CCallableWarnCheck *CGHostDB :: ThreadedWarnCheck( string server, string user )
{
	return NULL;
}

CCallableWarnAdd *CGHostDB :: ThreadedWarnAdd(string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin )
{
	return NULL;
}
CCallableWarnUpdateAdd *CGHostDB :: ThreadedWarnUpdateAdd(string server, string user, uint32_t warnings, string warning, uint32_t totalwarn, string admin )
{
	return NULL;
}
CCallableWarnChannelBan *CGHostDB :: ThreadedWarnChannelBan(string server, string user, uint32_t daysban, string admin )
{
	return NULL;
}

CCallableWarnRemove *CGHostDB :: ThreadedWarnRemove( string server, string user )
{
	return NULL;
}

CCallableWarnList *CGHostDB :: ThreadedWarnList( string server )
{
	return NULL;
}

CCallableRunQuery *CGHostDB :: ThreadedRunQuery( string query )
{
	return NULL;
}
/////////////////////////////////////
CCallableBanCount *CGHostDB :: ThreadedBanCount( string server )
{
	return NULL;
}

CCallableBanCheck *CGHostDB :: ThreadedBanCheck( string server, string user, string ip )
{
	return NULL;
}

CCallableBanAdd *CGHostDB :: ThreadedBanAdd( string server, string user, string ip, string gamename, string admin, string reason )
{
	return NULL;
}

CCallableBanRemove *CGHostDB :: ThreadedBanRemove( string server, string user )
{
	return NULL;
}

CCallableBanRemove *CGHostDB :: ThreadedBanRemove( string user )
{
	return NULL;
}

CCallableBanList *CGHostDB :: ThreadedBanList( string server )
{
	return NULL;
}

CCallableGameAdd *CGHostDB :: ThreadedGameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver )
{
	return NULL;
}

CCallableGamePlayerAdd *CGHostDB :: ThreadedGamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour )
{
	return NULL;
}

CCallableGamePlayerSummaryCheck *CGHostDB :: ThreadedGamePlayerSummaryCheck( string name )
{
	return NULL;
}

CCallableDotAGameAdd *CGHostDB :: ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
	return NULL;
}

CCallableDotAPlayerAdd *CGHostDB :: ThreadedDotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills )
{
	return NULL;
}

CCallableDotAPlayerSummaryCheck *CGHostDB :: ThreadedDotAPlayerSummaryCheck( string name )
{
	return NULL;
}

CCallableDotAPlayerKillerCheck *CGHostDB :: ThreadedDotAPlayerKillerCheck( string name, uint32_t rank )
{
	return NULL;
}

CCallableDotAPlayerFarmerCheck *CGHostDB :: ThreadedDotAPlayerFarmerCheck( string name, uint32_t rank )
{
	return NULL;
}

CCallableDownloadAdd *CGHostDB :: ThreadedDownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime )
{
	return NULL;
}

CCallableScoreCheck *CGHostDB :: ThreadedScoreCheck( string category, string name, string server )
{
	return NULL;
}

CCallableW3MMDPlayerAdd *CGHostDB :: ThreadedW3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings )
{
	return NULL;
}

//
// Callables
//

void CBaseCallable :: Init( )
{
	m_StartTicks = GetTicks( );
}

void CBaseCallable :: Close( )
{
	m_EndTicks = GetTicks( );
	m_Ready = true;
}

CCallableMailAdd :: ~CCallableMailAdd( )
{

}

CCallableMailReaded :: ~CCallableMailReaded( )
{

}

CCallableMailRemove :: ~CCallableMailRemove( )
{

}

CCallableMailList :: ~CCallableMailList( )
{
	// don't delete anything in m_Result here, it's the caller's responsibility
}


CCallableInfoCount :: ~CCallableInfoCount( )
{

}

CCallableInfoCheck :: ~CCallableInfoCheck( )
{
	delete m_Result;
}

CCallableInfoAdd :: ~CCallableInfoAdd( )
{

}

CCallableInfoPrivCalculate :: ~CCallableInfoPrivCalculate( )
{

}

CCallableInfoPubCalculate :: ~CCallableInfoPubCalculate( )
{

}

CCallableInfoPrivPoints :: ~CCallableInfoPrivPoints( )
{

}

CCallableInfoPubPoints :: ~CCallableInfoPubPoints( )
{

}

CCallableInfoChallwins :: ~CCallableInfoChallwins( )
{

}

CCallableInfoChallloses :: ~CCallableInfoChallloses( )
{

}

CCallableInfoRemove :: ~CCallableInfoRemove( )
{

}

CCallableInfoList :: ~CCallableInfoList( )
{
	// don't delete anything in m_Result here, it's the caller's responsibility
}

CCallableInfoUpdateCountry :: ~CCallableInfoUpdateCountry( )
{

}

CCallableInfoUpdateLvl :: ~CCallableInfoUpdateLvl( )
{

}

CCallableInfoUpdateMessage :: ~CCallableInfoUpdateMessage( )
{

}

CCallableInfoUpdateGinfo :: ~CCallableInfoUpdateGinfo( )
{

}

CCallableWarnCount :: ~CCallableWarnCount( )
{

}

CCallableWarnCheck :: ~CCallableWarnCheck( )
{
	delete m_Result;
}

CCallableWarnAdd :: ~CCallableWarnAdd( )
{

}
CCallableWarnUpdateAdd :: ~CCallableWarnUpdateAdd( )
{

}
CCallableWarnChannelBan :: ~CCallableWarnChannelBan( )
{

}

CCallableWarnRemove :: ~CCallableWarnRemove( )
{

}

CCallableWarnList :: ~CCallableWarnList( )
{
	// don't delete anything in m_Result here, it's the caller's responsibility
}

CCallableRunQuery :: ~CCallableRunQuery( )
{

}

CCallableBanCount :: ~CCallableBanCount( )
{

}

CCallableBanCheck :: ~CCallableBanCheck( )
{
	delete m_Result;
}

CCallableBanAdd :: ~CCallableBanAdd( )
{

}

CCallableBanRemove :: ~CCallableBanRemove( )
{

}

CCallableBanList :: ~CCallableBanList( )
{
	// don't delete anything in m_Result here, it's the caller's responsibility
}

CCallableGameAdd :: ~CCallableGameAdd( )
{

}

CCallableGamePlayerAdd :: ~CCallableGamePlayerAdd( )
{

}

CCallableGamePlayerSummaryCheck :: ~CCallableGamePlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableDotAGameAdd :: ~CCallableDotAGameAdd( )
{

}

CCallableDotAPlayerAdd :: ~CCallableDotAPlayerAdd( )
{

}

CCallableDotAPlayerSummaryCheck :: ~CCallableDotAPlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableDotAPlayerKillerCheck :: ~CCallableDotAPlayerKillerCheck( )
{
	delete m_Result;
}

CCallableDotAPlayerFarmerCheck :: ~CCallableDotAPlayerFarmerCheck( )
{
	delete m_Result;
}

CCallableDownloadAdd :: ~CCallableDownloadAdd( )
{

}

CCallableScoreCheck :: ~CCallableScoreCheck( )
{

}

CCallableW3MMDPlayerAdd :: ~CCallableW3MMDPlayerAdd( )
{

}

CCallableW3MMDVarAdd :: ~CCallableW3MMDVarAdd( )
{

}

//
// CDBBan
//
CDBBan :: CDBBan( string nServer, string nName, string nIP, string nDate, string nGameName, string nAdmin, string nReason )
{
	m_Server = nServer;
	m_Name = nName;
	m_IP = nIP;
	m_Date = nDate;
	m_GameName = nGameName;
	m_Admin = nAdmin;
	m_Reason = nReason;
}

CDBBan :: ~CDBBan( )
{

}

//
// CDBMail
//
CDBMail :: CDBMail( uint32_t nId, string nServer, string nSender, string nReceiver, string nMessage, uint32_t nReaded, string nDate)
{
	m_Id = nId;
	m_Server = nServer;
	m_Sender = nSender;
	m_Receiver = nReceiver;
	m_Message = nMessage;
	m_Readed = nReaded;
	m_Date = nDate;
}

CDBMail :: ~CDBMail( )
{

}

//
// CDBInfo
//
CDBInfo :: CDBInfo( uint32_t nId, string nServer, string nName,uint32_t nLvl, uint32_t nPrivRank, uint32_t nPubRank, uint32_t nPrivPoints, uint32_t nPubPoints, string nAdmin, string nCountry, uint32_t nChallwins, uint32_t nChallloses, string nGinfo,uint32_t nMessage, string nDate)
{
	m_Id = nId;
	m_Server = nServer;
	m_Name = nName;
	m_Lvl = nLvl;
	m_PrivRank = nPrivRank;
	m_PubRank = nPubRank;
	m_PrivPoints = nPrivPoints;
	m_PubPoints = nPubPoints;
	m_Admin = nAdmin;
	m_Country = nCountry;
	m_Challwins = nChallwins;
	m_Challloses = nChallloses;
	m_Ginfo = nGinfo;
	m_Message = nMessage;
	m_Date = nDate;
}

CDBInfo :: ~CDBInfo( )
{

}

//
// CDBWarn
//
CDBWarn :: CDBWarn( uint32_t nId, string nServer, string nName, uint32_t nWarnings, string nWarning, uint32_t nTotalwarn, uint32_t nDaysban, string nAdmin, string nDate)
{
	m_Id = nId;
	m_Server = nServer;
	m_Name = nName;
	m_Warnings = nWarnings;
	m_Warning = nWarning;
	m_Totalwarn = nTotalwarn;
	m_Daysban = nDaysban;
	m_Admin = nAdmin;
	m_Date = nDate;
}

CDBWarn :: ~CDBWarn( )
{

}

CDBGames :: CDBGames( uint32_t nGameId, string nHostbot, string nOwner, uint32_t nStartTime, uint32_t nUsersSlots, uint32_t nGameState, string nGameName )
{
	m_GameId = nGameId;
	m_Hostbot = nHostbot;
	m_Owner = nOwner;
	m_StartTime = nStartTime;
	m_UsersSlots = nUsersSlots;
	m_GameState = nGameState;
	m_CreepsSpawn = false;
	m_GameName = nGameName;
}

CDBGames :: ~CDBGames( )
{

}

CDBChannel :: CDBChannel( string nUser, uint32_t nTime, uint32_t nLvl, bool nBanned, bool nTopaz )
{
	m_User = nUser;
	m_Time = nTime;
	m_Lvl = nLvl;
	m_Banned = nBanned;
	m_Topaz =nTopaz;
	m_Signed = false;
	m_SignedTime = GetTime( );
	m_Mode = "-";
}

CDBChannel :: ~CDBChannel( )
{

}

CDBMaps :: CDBMaps( string nUser, string nMap )
{
	m_User = nUser;
	m_Map = nMap;
	m_Obs = 0;
}

CDBMaps :: ~CDBMaps( )
{

}


CDBHostBot :: CDBHostBot( string nHostBot, uint32_t nState, uint32_t nHostedGames, uint32_t nMaxGames )
{
	m_HostBot = nHostBot;
	m_State = nState;
	m_HostedGames = nHostedGames;
	m_MaxGames = nMaxGames;
}

CDBHostBot :: ~CDBHostBot( )
{

}

CDBIcons :: CDBIcons( string nUser, string nFrom, string nIcon, string nReason )
{
	m_User = nUser;
	m_From = nFrom;
	m_Icon = nIcon;
	m_Reason = nReason;
}

CDBIcons :: ~CDBIcons( )
{

}

//
// CDBGame
//
CDBGame :: CDBGame( uint32_t nID, string nServer, string nMap, string nDateTime, string nGameName, string nOwnerName, uint32_t nDuration )
{
	m_ID = nID;
	m_Server = nServer;
	m_Map = nMap;
	m_DateTime = nDateTime;
	m_GameName = nGameName;
	m_OwnerName = nOwnerName;
	m_Duration = nDuration;
}

CDBGame :: ~CDBGame( )
{

}

//
// CDBGamePlayer
//
CDBGamePlayer :: CDBGamePlayer( uint32_t nID, uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour )
{
	m_ID = nID;
	m_GameID = nGameID;
	m_Name = nName;
	m_IP = nIP;
	m_Spoofed = nSpoofed;
	m_SpoofedRealm = nSpoofedRealm;
	m_Reserved = nReserved;
	m_LoadingTime = nLoadingTime;
	m_Left = nLeft;
	m_LeftReason = nLeftReason;
	m_Team = nTeam;
	m_Colour = nColour;
}

CDBGamePlayer :: ~CDBGamePlayer( )
{

}

//
// CDBGamePlayerSummary
//
CDBGamePlayerSummary :: CDBGamePlayerSummary( string nServer, string nName, string nFirstGameDateTime, string nLastGameDateTime, uint32_t nTotalGames, uint32_t nMinLoadingTime, uint32_t nAvgLoadingTime, uint32_t nMaxLoadingTime, uint32_t nMinLeftPercent, uint32_t nAvgLeftPercent, uint32_t nMaxLeftPercent, uint32_t nMinDuration, uint32_t nAvgDuration, uint32_t nMaxDuration )
{
	m_Server = nServer;
	m_Name = nName;
	m_FirstGameDateTime = nFirstGameDateTime;
	m_LastGameDateTime = nLastGameDateTime;
	m_TotalGames = nTotalGames;
	m_MinLoadingTime = nMinLoadingTime;
	m_AvgLoadingTime = nAvgLoadingTime;
	m_MaxLoadingTime = nMaxLoadingTime;
	m_MinLeftPercent = nMinLeftPercent;
	m_AvgLeftPercent = nAvgLeftPercent;
	m_MaxLeftPercent = nMaxLeftPercent;
	m_MinDuration = nMinDuration;
	m_AvgDuration = nAvgDuration;
	m_MaxDuration = nMaxDuration;
}

CDBGamePlayerSummary :: ~CDBGamePlayerSummary( )
{

}
//
// CDBDotAGame
//
CDBDotAGame :: CDBDotAGame( uint32_t nID, uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec )
{
	m_ID = nID;
	m_GameID = nGameID;
	m_Winner = nWinner;
	m_Min = nMin;
	m_Sec = nSec;
}

CDBDotAGame :: ~CDBDotAGame( )
{

}

//
// CDBDotAPlayer
//
CDBDotAPlayer :: CDBDotAPlayer( )
{
	m_ID = 0;
	m_GameID = 0;
	m_Colour = 0;
	m_Kills = 0;
	m_Deaths = 0;
	m_CreepKills = 0;
	m_CreepDenies = 0;
	m_Assists = 0;
	m_Gold = 0;
	m_NeutralKills = 0;
	m_NewColour = 0;
	m_TowerKills = 0;
	m_RaxKills = 0;
	m_CourierKills = 0;
}

CDBDotAPlayer :: CDBDotAPlayer( uint32_t nID, uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, string nItem1, string nItem2, string nItem3, string nItem4, string nItem5, string nItem6, string nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills )
{
	m_ID = nID;
	m_GameID = nGameID;
	m_Colour = nColour;
	m_Kills = nKills;
	m_Deaths = nDeaths;
	m_CreepKills = nCreepKills;
	m_CreepDenies = nCreepDenies;
	m_Assists = nAssists;
	m_Gold = nGold;
	m_NeutralKills = nNeutralKills;
	m_Items[0] = nItem1;
	m_Items[1] = nItem2;
	m_Items[2] = nItem3;
	m_Items[3] = nItem4;
	m_Items[4] = nItem5;
	m_Items[5] = nItem6;
	m_Hero = nHero;
	m_NewColour = nNewColour;
	m_TowerKills = nTowerKills;
	m_RaxKills = nRaxKills;
	m_CourierKills = nCourierKills;
}

CDBDotAPlayer :: ~CDBDotAPlayer( )
{

}

string CDBDotAPlayer :: GetItem( unsigned int i )
{
	if( i < 6 )
		return m_Items[i];

	return string( );
}

void CDBDotAPlayer :: SetItem( unsigned int i, string item )
{
	if( i < 6 )
		m_Items[i] = item;
}

//
// CDBDotAPlayerSummary
//
CDBDotAPlayerSummary :: CDBDotAPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, uint32_t nTotalCreepKills, uint32_t nTotalCreepDenies, uint32_t nTotalAssists, uint32_t nTotalNeutralKills, uint32_t nTotalTowerKills, uint32_t nTotalRaxKills, uint32_t nTotalCourierKills )
{
	m_Server = nServer;
	m_Name = nName;
	m_TotalGames = nTotalGames;
	m_TotalWins = nTotalWins;
	m_TotalLosses = nTotalLosses;
	m_TotalKills = nTotalKills;
	m_TotalDeaths = nTotalDeaths;
	m_TotalCreepKills = nTotalCreepKills;
	m_TotalCreepDenies = nTotalCreepDenies;
	m_TotalAssists = nTotalAssists;
	m_TotalNeutralKills = nTotalNeutralKills;
	m_TotalTowerKills = nTotalTowerKills;
	m_TotalRaxKills = nTotalRaxKills;
	m_TotalCourierKills = nTotalCourierKills;
}

CDBDotAPlayerSummary :: ~CDBDotAPlayerSummary( )
{

}


//
// CDBDotAPlayerKiller
//
CDBDotAPlayerKiller :: CDBDotAPlayerKiller( string nServer, uint32_t nGameid, string nName, uint32_t nTotalKills, uint32_t nTotalDeaths, uint32_t nTotalAssists )
{
	m_Server = nServer;
	m_Gameid = nGameid;
	m_Name = nName;
	m_TotalKills = nTotalKills;
	m_TotalDeaths = nTotalDeaths;
	m_TotalAssists = nTotalAssists;
}

CDBDotAPlayerKiller :: ~CDBDotAPlayerKiller( )
{

}

//
// CDBDotAPlayerFarmer
//
CDBDotAPlayerFarmer :: CDBDotAPlayerFarmer( string nServer, uint32_t nGameid, string nName, uint32_t nTotalCreepKills, uint32_t nTotalCreepDenies, uint32_t nTotalNeutralKills )
{
	m_Server = nServer;
	m_Gameid = nGameid;
	m_Name = nName;
	m_TotalCreepKills = nTotalCreepKills;
	m_TotalCreepDenies = nTotalCreepDenies;
	m_TotalNeutralKills = nTotalNeutralKills;
}

CDBDotAPlayerFarmer :: ~CDBDotAPlayerFarmer( )
{

}
