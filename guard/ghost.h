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

#ifndef GHOST_H
#define GHOST_H

#include "includes.h"

//
// CGHost
//

class CUDPSocket;
class CCRC32;
class CSHA1;
class CBNET;
class CGHostDB;
class CBaseCallable;
class CLanguage;
class CConfig;
//UDPCommandSocket patch
class CUDPServer;
class CDBGames;

class CGHost
{
public:
	CUDPSocket *m_UDPSocket;				// a UDP socket for sending broadcasts and other junk (used with !sendlan)
	CCRC32 *m_CRC;							// for calculating CRC's
	CSHA1 *m_SHA;							// for calculating SHA1's
	vector<CBNET *> m_BNETs;				// all our battle.net connections (there can be more than one)
	CGHostDB *m_DB;							// database
	vector<CBaseCallable *> m_Callables;	// vector of orphaned callables waiting to die
	vector<BYTEARRAY> m_LocalAddresses;		// vector of local IP addresses
	CLanguage *m_Language;					// language
	bool m_Exiting;							// set to true to force ghost to shutdown next update (used by SignalCatcher)
	bool m_ExitingNice;						// set to true to force ghost to disconnect from all battle.net connections and wait for all games to finish before shutting down
	string m_Version;						// GHost++ version string
	string m_Versionb;
	uint32_t m_HostCounter;					// the current host counter (a unique number to identify a game, incremented each time a game is created)
	string m_LanguageFile;					// config value: language file
	string m_Warcraft3Path;					// config value: Warcraft 3 path
	bool m_TFT;								// config value: TFT enabled or not
	char m_CommandTrigger;					// config value: the command trigger inside games
	unsigned char m_LANWar3Version;			// config value: LAN warcraft 3 version
	uint32_t m_ReplayWar3Version;			// config value: replay warcraft 3 version (for saving replays)
	string m_BindAddress;
	string m_MapPath;
	string m_defaultMap;
	
	bool m_PrintPubLadder;
	vector<string> m_UDPUsers;				// our udp users
	vector<string> m_IPUsers;
	string m_LastIp;						// our last udp command is comming from this ip
	bool m_inconsole;	

	//inform section
	vector<CDBGames *> m_Games;

	//php site page system
	bool m_SiteUpdateSupported;
	uint32_t m_SiteLastUpdateTimer;
	uint32_t m_SiteUpdatedTime;
	string gPhpFile;

	//activation section
	uint32_t m_UDPGuardPort;

	string m_ForumUrl;
	string m_StatsUrl;
	bool m_IsOperator;
	bool m_AutoCreate;
	bool commands[100];						//table with commands enable or not.
	string gFlamesFile;
	vector<string> m_FlameList;
	string gIconsFile;
	uint32_t m_IconTimer;

	//mail variables
	bool m_MailEnabled;
	uint32_t m_MailMaxSended;
	uint32_t m_MailMaxReceived;

	uint32_t m_FlameWordsNumber;
	uint32_t m_ChallengeSafeRange;
	string m_UnlockBot;
	string m_FreeHostFromCountry;
	string m_FirstWelcomeChannel;
	string m_News;
	uint16_t m_DefaultAutoLvl;				// config value: autolevel when player join channel
	string m_CommunityName;						// config value: name of community
	string m_CommunityNameLower;
	bool m_AllowTopaz;
	string m_ExternalIP;
	bool m_IconSupport;						//config if icon system supported or not.
	string m_CountrySign;					//if not empty only some countries will be able to sign for challenge
	

	uint32_t m_PoolLimit;
	bool m_OldLadderSystem;
	uint32_t m_AddPoints;
	uint32_t m_RemPoints;
	bool m_CountOnly5v5;

	bool m_GoPubByWhisper;
	bool m_ResAutostartHostPriv;

	//Game inform system
	void AddGame( uint32_t gameid, string hostbot, string owner, uint32_t starttime, uint32_t slots, uint32_t gamestate, string gamename );
	void DelGame( uint32_t gameid );
	void AddNames( uint32_t gameid, string names, uint32_t slot );
	void UpdateGameSlots( uint32_t gameid, uint32_t slots, string name);
	void UpdateGameState( uint32_t gameid, uint32_t state);
	void UpdateGameName( uint32_t gameid, string gamename);
	void UpdateGameTime( uint32_t gameid, uint32_t gamestarttime, bool creepspawn);
	void UpdateGameOwner( uint32_t gameid, string owner);	
	void GetGames( string user );
	
	bool IsGameId( uint32_t gameid );
	uint32_t GetIdFromName( string name );
	void GetGame( string user, uint32_t gameid );
	string GetHostbotName( uint32_t gameid );
	
	uint32_t GetUsersNumber( );
	uint32_t GetChallenges( );
	uint32_t GetPubs( );
	uint32_t GetPrivs( );
	uint32_t GetLobby( );	
	
	CGHost( CConfig *CFG );
	~CGHost( );
	
	void UDPChatDel(string ip);
	void UDPChatAdd(string ip);
	bool UDPChatAuth(string ip);
//	void UDPChatSendBack(string s);
//	void UDPChatSend(string s);
//	void UDPChatSend(BYTEARRAY b);
	string UDPChatWhoIs(string c, string s);
	void UDPCommands(string Message);
	// processing functions

	bool Update( long usecBlock );

	// events
	void EventBNETConnecting( CBNET *bnet );
	void EventBNETConnected( CBNET *bnet );
	void EventBNETDisconnected( CBNET *bnet );
	void EventBNETLoggedIn( CBNET *bnet );
	void EventBNETGameRefreshed( CBNET *bnet );
	void EventBNETGameRefreshFailed( CBNET *bnet );
	void EventBNETConnectTimedOut( CBNET *bnet );
	void EventBNETWhisper( CBNET *bnet, string user, string message );
	void EventBNETChat( CBNET *bnet, string user, string message );
	void EventBNETEmote( CBNET *bnet, string user, string message );

	// other functions
	string GetDay( );
	void UDPSend( string ip, uint16_t port, string message );
	void ReloadConfigs( );
	void SetConfigs( CConfig *CFG );
	
	void LoadIPToCountryData( );
	// UDPCommandSocket patch
	CUDPServer *m_UDPCommandSocket;		// a UDP socket for receiving commands
	string m_UDPCommandSpoofTarget;     // the realm to send udp received commands to
};

#endif
