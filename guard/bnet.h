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

#ifndef BNET_H
#define BNET_H

//
// CBNET
//

class CTCPClient;
class CCommandPacket;
class CBNCSUtilInterface;
class CBNETProtocol;
class CBNLSClient;
class CIncomingChatEvent;

class CCallableGamePlayerSummaryCheck;
class CCallableDotAPlayerSummaryCheck;
class CCallableDotAPlayerKillerCheck;
class CCallableDotAPlayerFarmerCheck;

class CDBInfo;
class CCallableInfoUpdateCountry;
class CCallableInfoUpdateLvl;
class CCallableInfoUpdateMessage;
class CCallableInfoUpdateGinfo;
class CCallableInfoCount;
class CCallableInfoAdd;
class CCallableInfoPrivCalculate;
class CCallableInfoPubCalculate;
class CCallableInfoPrivPoints;
class CCallableInfoPubPoints;
class CCallableInfoChallwins;
class CCallableInfoChallloses;
class CCallableInfoRemove;
class CCallableInfoList;

class CCallableMailAdd;
class CCallableMailRemove;
class CCallableMailList;
class CCallableMailReaded;
class CDBMail;

class CCallableWarnCount;
class CCallableWarnAdd;
class CCallableWarnUpdateAdd;
class CCallableWarnChannelBan;
class CCallableWarnRemove;
class CCallableWarnList;
class CDBWarn;

class CCallableRunQuery;

class CDBHostBot;
class CDBChannel;
class CDBIcons;
class CDBMaps;

class CCallableBanCount;
class CCallableBanAdd;
class CCallableBanRemove;
class CCallableBanList;
class CDBBan;

typedef pair<string,CCallableMailAdd *> PairedMailAdd;
typedef pair<string,CCallableMailRemove *> PairedMailRemove;
typedef pair<string,CCallableMailReaded *> PairedMailReaded;

typedef pair<string,CCallableInfoCount *> PairedInfoCount;
typedef pair<string,CCallableInfoAdd *> PairedInfoAdd;
typedef pair<string,CCallableInfoPrivCalculate *> PairedInfoPrivCalculate;
typedef pair<string,CCallableInfoPubCalculate *> PairedInfoPubCalculate;
typedef pair<string,CCallableInfoPrivPoints *> PairedInfoPrivPoints;
typedef pair<string,CCallableInfoPubPoints *> PairedInfoPubPoints;
typedef pair<string,CCallableInfoChallwins *> PairedInfoChallwins;
typedef pair<string,CCallableInfoChallloses *> PairedInfoChallloses;
typedef pair<string,CCallableInfoRemove *> PairedInfoRemove;
typedef pair<string,CCallableInfoUpdateCountry *> PairedInfoUpdateCountry;
typedef pair<string,CCallableInfoUpdateLvl *> PairedInfoUpdateLvl;
typedef pair<string,CCallableInfoUpdateMessage *> PairedInfoUpdateMessage;
typedef pair<string,CCallableInfoUpdateGinfo *> PairedInfoUpdateGinfo;

typedef pair<string,CCallableWarnCount *> PairedWarnCount;
typedef pair<string,CCallableWarnAdd *> PairedWarnAdd;
typedef pair<string,CCallableWarnUpdateAdd *> PairedWarnUpdateAdd;
typedef pair<string,CCallableWarnChannelBan *> PairedWarnChannelBan;
typedef pair<string,CCallableWarnRemove *> PairedWarnRemove;

typedef pair<string,CCallableRunQuery *> PairedRunQuery;

typedef pair<string,CCallableBanCount *> PairedBanCount;
typedef pair<string,CCallableBanAdd *> PairedBanAdd;
typedef pair<string,CCallableBanRemove *> PairedBanRemove;
typedef pair<string,CCallableGamePlayerSummaryCheck *> PairedGPSCheck;
typedef pair<string,CCallableDotAPlayerSummaryCheck *> PairedDPSCheck;
typedef pair<string,CCallableDotAPlayerKillerCheck *> PairedKillerCheck;
typedef pair<string,CCallableDotAPlayerFarmerCheck *> PairedFarmerCheck;

class CBNET
{
public:
	CGHost *m_GHost;

private:
	CTCPClient *m_Socket;							// the connection to battle.net
	CBNETProtocol *m_Protocol;						// battle.net protocol
	CBNLSClient *m_BNLSClient;						// the BNLS client (for external warden handling)
	queue<CCommandPacket *> m_Packets;				// queue of incoming packets
	CBNCSUtilInterface *m_BNCSUtil;					// the interface to the bncsutil library (used for logging into battle.net)
	queue<BYTEARRAY> m_OutPackets;					// queue of outgoing packets to be sent (to prevent getting kicked for flooding)

	vector<PairedMailAdd> m_PairedMailAdds;
	vector<PairedMailRemove> m_PairedMailRemoves;
	vector<PairedMailReaded> m_PairedMailReadeds;

	vector<PairedInfoCount> m_PairedInfoCounts;		
	vector<PairedInfoAdd> m_PairedInfoAdds;			
	vector<PairedInfoPrivCalculate> m_PairedInfoPrivCalculates;
	vector<PairedInfoPubCalculate> m_PairedInfoPubCalculates;
	vector<PairedInfoPrivPoints> m_PairedInfoPrivPointss;
	vector<PairedInfoPubPoints> m_PairedInfoPubPointss;
	vector<PairedInfoChallwins> m_PairedInfoChallwinss;
	vector<PairedInfoChallloses> m_PairedInfoChalllosess;
	vector<PairedInfoRemove> m_PairedInfoRemoves;	
	vector<PairedInfoUpdateCountry> m_PairedInfoUpdateCountrys;
	vector<PairedInfoUpdateLvl> m_PairedInfoUpdateLvls;
	vector<PairedInfoUpdateMessage> m_PairedInfoUpdateMessages;
	vector<PairedInfoUpdateGinfo> m_PairedInfoUpdateGinfos;

	vector<PairedWarnCount> m_PairedWarnCounts;		
	vector<PairedWarnAdd> m_PairedWarnAdds;
	vector<PairedWarnUpdateAdd> m_PairedWarnUpdateAdds;
	vector<PairedWarnChannelBan> m_PairedWarnChannelBans;
	vector<PairedWarnRemove> m_PairedWarnRemoves;

	vector<PairedRunQuery> m_PairedRunQuerys;	
	
	vector<PairedBanCount> m_PairedBanCounts;		// vector of paired threaded database ban counts in progress
	vector<PairedBanAdd> m_PairedBanAdds;			// vector of paired threaded database ban adds in progress
	vector<PairedBanRemove> m_PairedBanRemoves;		// vector of paired threaded database ban removes in progress
	vector<PairedGPSCheck> m_PairedGPSChecks;		// vector of paired threaded database game player summary checks in progress
	vector<PairedDPSCheck> m_PairedDPSChecks;		// vector of paired threaded database DotA player summary checks in progress
	vector<PairedKillerCheck> m_PairedKillerChecks;		// vector of paired threaded database DotA player summary checks in progress
	vector<PairedFarmerCheck> m_PairedFarmerChecks;		// vector of paired threaded database DotA player summary checks in progress

	CCallableBanList *m_CallableBanList;
	CCallableMailList *m_CallableMailList;
	CCallableInfoList *m_CallableInfoList;
	CCallableWarnList *m_CallableWarnList;
	
	vector<CDBMail *> m_Mails;
	vector<CDBInfo *> m_Infos;
	vector<CDBWarn *> m_Warns;
	
	vector<CDBMaps *> m_Maps;
	vector<CDBChannel *> m_Users;					// vector of users in channel
	vector<CDBHostBot *> m_HostBots;
	set<string> m_OnlineUsers;
	vector<string> m_InChannelUsers;
	vector<string> m_MuteUsers;
	
	vector<CDBBan *> m_Bans;						// vector of cached bans
	bool m_Exiting;									// set to true and this class will be deleted next update
	string m_Server;								// battle.net server to connect to
	string m_ServerIP;								// battle.net server to connect to (the IP address so we don't have to resolve it every time we connect)
	string m_ServerAlias;							// battle.net server alias (short name, e.g. "USEast")
	string m_BNLSServer;							// BNLS server to connect to (for warden handling)
	uint16_t m_BNLSPort;							// BNLS port
	uint32_t m_BNLSWardenCookie;					// BNLS warden cookie
	string m_CDKeyROC;								// ROC CD key
	string m_CDKeyTFT;								// TFT CD key
	string m_CountryAbbrev;							// country abbreviation
	string m_Country;								// country
	string m_UserName;								// battle.net username
	string m_UserNameLower;
	string m_UserPassword;							// battle.net password
	string m_FirstChannel;							// the first chat channel to join upon entering chat (note: we hijack this to store the last channel when entering a game)
	string m_CurrentChannel;						// the current chat channel
	string m_RootAdmin;								// the root admin
	char m_CommandTrigger;							// the character prefix to identify commands
	unsigned char m_War3Version;					// custom warcraft 3 version for PvPGN users
	BYTEARRAY m_EXEVersion;							// custom exe version for PvPGN users
	BYTEARRAY m_EXEVersionHash;						// custom exe version hash for PvPGN users
	string m_PasswordHashType;						// password hash type for PvPGN users
	string m_PVPGNRealmName;						// realm name for PvPGN users (for mutual friend spoofchecks)
	uint32_t m_MaxMessageLength;					// maximum message length for PvPGN users
	uint32_t m_HostCounterID;						// the host counter ID to identify players from this realm
	uint32_t m_LastDisconnectedTime;				// GetTime when we were last disconnected from battle.net
	uint32_t m_LastConnectionAttemptTime;			// GetTime when we last attempted to connect to battle.net
	uint32_t m_LastNullTime;						// GetTime when the last null packet was sent for detecting disconnects
	uint32_t m_LastOutPacketTicks;					// GetTicks when the last packet was sent for the m_OutPackets queue
	uint32_t m_LastOutPacketSize;
	uint32_t m_LastBanRefreshTime;					// GetTime when the ban list was last refreshed from the database
	bool m_FirstConnect;							// if we haven't tried to connect to battle.net yet
	bool m_WaitingToConnect;						// if we're waiting to reconnect to battle.net after being disconnected
	bool m_LoggedIn;								// if we've logged into battle.net or not
	bool m_InChat;									// if we've entered chat or not (but we're not necessarily in a chat channel yet)
	bool m_HoldFriends;								// whether to auto hold friends when creating a game or not
	bool m_HoldClan;								// whether to auto hold clan members when creating a game or not
	bool m_PublicCommands;							// whether to allow public commands or not

	bool m_HostbotsChecked;
	bool m_PrintNews;
	uint32_t m_SlotsLastSpam;
	uint32_t m_SlotsSpamTimer;
	bool m_SlotsSpamEnable;
	uint32_t m_StartTime;
	uint32_t m_GameId;
	
	bool m_RootOnly;
	bool m_OnlineChecking;
	uint32_t m_OnlineCheckingTime;

	string m_MassMessageCalledBy;
	bool m_ssList;
	bool m_CountingStart;							
	uint32_t m_CounderStart;
	uint32_t m_UsersOffBnet; //counting users that recived mass message

	uint32_t m_LastMailRefreshTime;
	uint32_t m_LastInfoRefreshTime;					
	uint32_t m_LastWarnRefreshTime;
	uint32_t m_LastGoPressed;
	string m_TempTopic;
	uint32_t m_LastAnnounceTime;				
	uint32_t m_AnnounceInterval;
	string m_AnnounceMessage;
	uint32_t m_SpamTime;
	uint32_t m_LastChannelBanCheck;

	uint32_t m_GamedThatBotCreate;

	//RMK section
	string m_RMK[12];
	uint32_t m_RMKstate;  //0 = disabled 1 = just rmk 2 = challenge rmk
	
	//Challenge and meplay
	bool m_SignLock;
	string m_meplay[12][2];
	uint32_t m_meplayppl;
	bool m_meplayon;		
	bool m_holdslots;
	bool m_FreeReady;
	uint32_t m_ReservesNeeded;
	bool m_ChallengeTimerExtended;
	uint32_t m_ChallengeTimersRemainSpam;
	bool m_ChallengeTimers;
	uint32_t m_ChallengeTime;
	uint32_t m_ChallengeStep;
	bool m_Challenge;
	string m_VotedMode;
	string m_Challenger;
	string m_Challenged;

	bool m_ChannelModerate;
	uint32_t m_PickStep;
	uint32_t m_holdnumb1;
	uint32_t m_holdnumb2;
	uint32_t m_PickUser;
	
	//Icon system
	vector<CDBIcons *> m_IconList;
	uint32_t m_IconRefreshTimer;
	bool m_IconLoadedFromFile;
	bool m_IconForceRefresh;
	
public:
	CBNET( CGHost *nGHost, string nServer, string nServerAlias, string nBNLSServer, uint16_t nBNLSPort, uint32_t nBNLSWardenCookie, string nCDKeyROC, string nCDKeyTFT, string nCountryAbbrev, string nCountry, string nUserName, string nUserPassword, string nFirstChannel, string nRootAdmin, char nCommandTrigger, bool nHoldFriends, bool nHoldClan, bool nPublicCommands, unsigned char nWar3Version, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType, string nPVPGNRealmName, uint32_t nMaxMessageLength, uint32_t nHostCounterID );
	~CBNET( );

	bool GetExiting( )					{ return m_Exiting; }
	string GetServer( )					{ return m_Server; }
	string GetServerAlias( )			{ return m_ServerAlias; }
	string GetCDKeyROC( )				{ return m_CDKeyROC; }
	string GetCDKeyTFT( )				{ return m_CDKeyTFT; }
	string GetUserName( )				{ return m_UserName; }
	string GetUserPassword( )			{ return m_UserPassword; }
	string GetFirstChannel( )			{ return m_FirstChannel; }
	string GetCurrentChannel( )			{ return m_CurrentChannel; }
	string GetRootAdmin( )				{ return m_RootAdmin; }
	void UpdateRoot( string nRoot )		{ m_RootAdmin+= " "+nRoot; }
	char GetCommandTrigger( )			{ return m_CommandTrigger; }
	BYTEARRAY GetEXEVersion( )			{ return m_EXEVersion; }
	BYTEARRAY GetEXEVersionHash( )		{ return m_EXEVersionHash; }
	string GetPasswordHashType( )		{ return m_PasswordHashType; }
	string GetPVPGNRealmName( )			{ return m_PVPGNRealmName; }
	uint32_t GetHostCounterID( )		{ return m_HostCounterID; }
	bool GetLoggedIn( )					{ return m_LoggedIn; }
	bool GetInChat( )					{ return m_InChat; }
	bool GetHoldFriends( )				{ return m_HoldFriends; }
	bool GetHoldClan( )					{ return m_HoldClan; }
	bool GetPublicCommands( )			{ return m_PublicCommands; }
	uint32_t GetOutPacketsQueued( )		{ return m_OutPackets.size( ); }
	BYTEARRAY GetUniqueName( );

	//Processing functions
	unsigned int SetFD( void *fd, void *send_fd, int *nfds );
	bool Update( void *fd, void *send_fd );
	void ExtractPackets( );
	void ProcessPackets( );
	void ProcessChatEvent( CIncomingChatEvent *chatEvent );

	//Functions to send packets to battle.net
	void SendJoinChannel( string channel );
	void SendGetFriendsList( );
	void SendGetClanList( );
	void QueueEnterChat( );
	void SendChatCommand(string user, string chatCommand );
	void SendChatCommand(string chatCommand );
	void QueueChatCommand( string chatCommand );
	void QueueChatCommand( string chatCommand, string user, bool whisper );
	
	void UnqueuePackets( unsigned char type );
	void UnqueueChatCommand( string chatCommand );
	
	//Site functions
	void UpdateSite( );

	//Inform functions
	void GetNames( string user, uint32_t gameid );
	void GetGameFromBots( string user, string botname );
	void GetInfo( string user );
	void GetAllInfo( string user, string payload );
	void FindUser( string user, string victim );
	uint32_t GetUsersNumberInChannel( );
	
	//Hostbot functions
	void ResetHostbots( );
	void UnhostHostbots( );
	uint32_t GetMaxGames( );
	void UpdateHostbot( string hostbot, uint32_t state, uint32_t hostedgames, uint32_t maxgames );
	uint32_t GetHostbotGames( string hostbot );
	uint32_t GetHostbotState( string hostbot );
	void GetHostbots( string name );
	void DelHostbot( string hostbot );

	//Map functions
	void SetMap( string User, string Map );
	void SetObs( string User, uint32_t obs);
	string IsMap( string User );
	uint32_t IsObs( string User );
	void DelMap( string User );
	void PrintMaps( string User );

	//Other functions
	void UnhostAllGames( );
	uint32_t GetSafelisted( );
	string LocalTime( );
	void PrintMuted( string name );
	bool IsMuted( string name );
	bool IsRootAdmin( string name );
	bool IsChannelBan( string name );
	bool IsHostBan( string name );
	
	//Autohost system
	string GetAvailableHostbot( );

	//Ladder system
	uint32_t GetUserChallWins( string user );
	uint32_t GetUserChallLoses(string user);
	uint32_t GetUserPrivPoints(string user);
	uint32_t GetUserPubPoints(string user);
	void AddGameInLadder(string state, string win1, string win2, string win3, string win4, string win5, string loser1, string loser2, string loser3, string loser4, string loser5, uint32_t slots );

	uint32_t GetPrivRank(string user);
	uint32_t GetPubRank(string user);
	bool ChallengeRankProtectPass(string User1, string User2);
	void FlameCheck(string User, string msg);
	
	//Challenge and meplay section
	string GetRandomUser( );
	uint32_t GetPoolHoldedSlots( );
	uint32_t GetHoldedSlots( );
	bool IsInPool( string name );
	void PrintPool( string name );
	void Sign( string name, string mode );
	void UnSign( string name );
	void Pick( string picker, string name, bool rnd );
	string GetUserMode( string name );
	string GetMode( );
	void ChangeMode( string name, string mode );

	//Mail functions
	bool MailSpamAlreadyReaded( string user, string message );
	void PrintSendedMails( string user );
	void PrintMails( string user, string receiver, bool admin, bool command );
	bool MailsSendedMaxLimit( string name );
	bool MailsReceivedMaxLimit( string name );

	//Virtual db stuff
	void AddMail( uint32_t id, string sender, string receiver, string message );
	void RemoveMail( string name, uint32_t id );
	void ReadedMail( string name );
	bool IsMail( string name );
	uint32_t GetMailId( );
	uint32_t GetWarnId( );
	uint32_t GetInfoId( );
	void Cban(string name, uint32_t hoursban, string admin);
	uint32_t GetHoursSince1970( );
	bool AlreadyBanned( string name, string gamename, string hostbot );
	void StartGame( );
	bool IsHostbot ( string name );
	CDBInfo *IsInfo( string name );
	CDBWarn *IsWarn(string name);
	void AddInfo( string name, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t pubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message );
	void PrivPointsInfo( string name, uint32_t privpoints );
	void PubPointsInfo( string name, uint32_t pubpoints );
	void ChallWinsInfo( string name, uint32_t challwins );
	void ChallLosesInfo( string name, uint32_t challloses );
	void CalculatePrivInfo( );
	void CalculatePubInfo( );
	void AddWarn( string name, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin );
	void UpdateAddWarn( string name, uint32_t warnings, string warning, uint32_t totalwarn, string admin );
	void ChannelBanWarn( string name, uint32_t daysban, string admin );
	void RemoveInfo( string name );
	void RemoveWarn( string name );
	void UpdateCountryInfo(string name, string country);
	void UpdateLvlInfo(string name, uint32_t lvl, string admin);
	void UpdateMessageInfo(string name, uint32_t message);
	void UpdateGinfoInfo(string name, string ginfo, string admin);	

	void MassMessage(string name, string message, bool PassTimer,bool TestUsers);	
	void ResetChallenge( bool resetobs, bool holdpicked, bool challenge );
	void ChannelModerate( );
	void RefreshVectors( );
	void TableSwap( string& a, string& b );
	void AddChannelUser(string User,uint32_t Userlvl, bool banned, bool topaz);
	void SAnnounce( uint32_t interval, string message );
	
	//Ban section
	CDBBan *IsBannedName( string name );
	CDBBan *IsBannedIP( string ip );
	void AddBan( string name, string ip, string gamename, string admin, string reason );
	void RemoveBan( string name );

	//General functions
	string GetHostbotFromOwner( string user );
	void DelayMs(uint32_t msec);

	//Icon system
	string GetIconFromRank( uint32_t rank );
	string GetNameFromPrivRank( uint32_t rank );
	string GetNameFromPubRank( uint32_t rank );
	void ResetChannelIcons( string channel );
};

#endif
