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
#include "language.h"
#include "socket.h"
#include "commandpacket.h"
#include "ghostdb.h"
#include "bncsutilinterface.h"
#include "bnlsclient.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include "md5.h"
#include "mysql.h"
#include <complex>

#include <boost/filesystem.hpp>

//
// CBNET
//

CBNET::CBNET( CGHost *nGHost, string nServer, string nServerAlias, string nBNLSServer, uint16_t nBNLSPort, uint32_t nBNLSWardenCookie, string nCDKeyROC, string nCDKeyTFT, string nCountryAbbrev, string nCountry, string nUserName, string nUserPassword, string nFirstChannel, string nRootAdmin, char nCommandTrigger, bool nHoldFriends, bool nHoldClan, bool nPublicCommands, unsigned char nWar3Version, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType, string nPVPGNRealmName, uint32_t nMaxMessageLength, uint32_t nHostCounterID ) {
	m_GHost = nGHost;
	m_Socket = new CTCPClient();
	m_Protocol = new CBNETProtocol();
	m_BNLSClient = NULL;
	m_BNCSUtil = new CBNCSUtilInterface( nUserName, nUserPassword );
	
	m_CallableBanList = m_GHost->m_DB->ThreadedBanList( nServer );
	
	m_Exiting = false;
	m_Server = nServer;
	string LowerServer = UTIL_ToLower(m_Server);

	if (!nServerAlias.empty()) {
		m_ServerAlias = nServerAlias;
	}
	else if (LowerServer == "europe.battle.net") {
		m_ServerAlias = "Europe";
	}
	else {
		m_ServerAlias = m_Server;
	}

	if (nPasswordHashType == "pvpgn" && !nBNLSServer.empty( ) ) {
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] pvpgn connection found with a configured BNLS server, ignoring BNLS server" , true);
		nBNLSServer.clear();
		nBNLSPort = 0;
		nBNLSWardenCookie = 0;
	}

	m_BNLSServer = nBNLSServer;
	m_BNLSPort = nBNLSPort;
	m_BNLSWardenCookie = nBNLSWardenCookie;
	m_CDKeyROC = nCDKeyROC;
	m_CDKeyTFT = nCDKeyTFT;

	// remove dashes from CD keys and convert to uppercase

	m_CDKeyROC.erase( remove( m_CDKeyROC.begin( ), m_CDKeyROC.end( ), '-' ), m_CDKeyROC.end( ) );
	m_CDKeyTFT.erase( remove( m_CDKeyTFT.begin( ), m_CDKeyTFT.end( ), '-' ), m_CDKeyTFT.end( ) );
	transform( m_CDKeyROC.begin( ), m_CDKeyROC.end( ), m_CDKeyROC.begin( ), (int(*)(int))toupper );
	transform( m_CDKeyTFT.begin( ), m_CDKeyTFT.end( ), m_CDKeyTFT.begin( ), (int(*)(int))toupper );

	if (m_CDKeyROC.size() != 26) {
		CONSOLE_Print("[BNET: " + m_ServerAlias + "] warning - your ROC CD key is not 26 characters long and is probably invalid", true);
	}

	if (m_GHost->m_TFT && m_CDKeyTFT.size() != 26) {
		CONSOLE_Print("[BNET: " + m_ServerAlias + "] warning - your TFT CD key is not 26 characters long and is probably invalid", true);
	}

	m_CountryAbbrev = nCountryAbbrev;
	m_Country = nCountry;
	m_UserName = nUserName;
	m_UserNameLower = UTIL_ToLower(m_UserName);
	m_UserPassword = nUserPassword;
	m_FirstChannel = nFirstChannel;
	m_RootAdmin = UTIL_ToLower(nRootAdmin);
	m_CommandTrigger = nCommandTrigger;
	m_War3Version = nWar3Version;
	m_EXEVersion = nEXEVersion;
	m_EXEVersionHash = nEXEVersionHash;
	m_PasswordHashType = nPasswordHashType;
	m_PVPGNRealmName = nPVPGNRealmName;
	m_MaxMessageLength = nMaxMessageLength;
	m_HostCounterID = nHostCounterID;
	m_LastDisconnectedTime = 0;
	m_LastConnectionAttemptTime = 0;
	m_LastNullTime = 0;
	m_LastOutPacketTicks = 0;
	m_LastOutPacketSize = 0;
	m_LastBanRefreshTime = GetTime();
	m_FirstConnect = true;
	m_WaitingToConnect = true;
	m_LoggedIn = false;
	m_HostbotsChecked = false;
	m_InChat = false;
	m_HoldFriends = nHoldFriends;
	m_HoldClan = nHoldClan;
	m_PublicCommands = nPublicCommands;

	m_CallableMailList = m_GHost->m_DB->ThreadedMailList( nServer );
	m_CallableInfoList = m_GHost->m_DB->ThreadedInfoList( nServer );
	m_CallableWarnList = m_GHost->m_DB->ThreadedWarnList( nServer );

	m_PrintNews = true;
	m_SlotsLastSpam = 0;
	m_SlotsSpamTimer = 0;
	m_SlotsSpamEnable = false;

	//Icon system
	m_IconRefreshTimer = GetTime();
	m_IconForceRefresh = false;
	m_IconLoadedFromFile = false;

	m_GameId = 0;

	m_RootOnly = false;
	m_OnlineCheckingTime=0;
	m_OnlineChecking = false;
	m_MassMessageCalledBy="";
	m_ssList = true;
	m_CountingStart = false;
	m_CounderStart = 0;
	m_UsersOffBnet = 0;
	m_StartTime = GetTime();
	m_LastMailRefreshTime = GetTime();
	m_LastChannelBanCheck = GetTime();
	m_LastInfoRefreshTime = GetTime();
	m_LastWarnRefreshTime = GetTime();
	m_LastAnnounceTime = 0;
	m_AnnounceInterval = 0;
	m_SpamTime=0;
	m_TempTopic="";
	m_GamedThatBotCreate=1;
	m_LastGoPressed=0;

	m_Users.push_back(new CDBChannel(m_UserName,GetTime(),7, false, false));
	m_OnlineUsers.insert(m_UserName);	

	//meplay command
	m_SignLock = false;
	m_ReservesNeeded=5;
	m_meplayppl=0;
	m_PickUser=0;
	m_PickStep=1;
	m_holdnumb1 = 1;
	m_holdnumb2 = 6;
	m_FreeReady = false;
	m_meplayon=true;

	//Rmk
	m_RMKstate = 0;

	//reset slots
	for (int i=0;i<12;i++)
	{
		m_RMK[i] = "";
		m_meplay[i][0]="";
		m_meplay[i][1]="-";
	}
	m_holdslots=false;

	//challenge command
	m_VotedMode = "";
	m_ChallengeTimersRemainSpam=0;
	m_ChallengeTimers=false;
	m_ChallengeStep=0;
	m_Challenge=false;
	m_Challenger="";
	m_Challenged="";
	m_ChallengeTime=0;
	m_ChannelModerate=false;
	
	//load flames from flames.txt
	ifstream in;
	in.open( m_GHost->gFlamesFile.c_str( ) );

	if (in.fail( ) ) {
		CONSOLE_Print("Fail load flame text", true);		
	}
	else {		
		uint32_t Count = 0;
		string Line;

		while( !in.eof( )  ) {
			getline( in, Line );			
			if (in.eof( ) )
				break;

			if (!Line.empty( ) )
			{
				m_GHost->m_FlameList.push_back(Line);
				m_GHost->m_FlameWordsNumber++;
			}	
		}
		in.close();
	}

	//load icons if icon system is enabled.
	if (m_GHost->m_IconSupport==1 ) {
		ifstream in2;
		in2.open( m_GHost->gIconsFile.c_str( ) );

		if (in2.fail( ) ) {
			CONSOLE_Print("[Icon]: Fail load icons file", true);		
		}
		else {	
			CONSOLE_Print("[Icon]: Loading icon file from disk.", true);
			uint32_t Count = 0;
			string Line;
			//here we must seperate each line to take from it 4 variables.
			//username iconname whoaddicon reason
			while( !in2.eof( ) && Count < 500 ) {
				getline( in2, Line );

				stringstream SS;
				SS << Line;
				string tbl[4];
				uint32_t counter=0;
				while( !SS.eof( ) ) {
					string words;
					SS >> words;

					if (SS.fail( ) ) {
						CONSOLE_Print("[Icon]: system error #1", true);
						break;
					}
					else {
						tbl[counter]=words;
						counter++;
					}
				}

				if (!Line.empty( ) && counter == 4 ) {
					m_IconList.push_back(new CDBIcons(tbl[0],tbl[1],tbl[2],tbl[3]));
					Count++;
				}
				else {
					CONSOLE_Print("[Icon]: system error #2", true);
				}

				if (in2.eof()) {
					break;
				}
			}
			in2.close();
			m_IconLoadedFromFile = true;
		}
		
	}
}

CBNET::~CBNET( )
{
	delete m_Socket;
	delete m_Protocol;
	delete m_BNLSClient;

	while( !m_Packets.empty( ) ) {
		delete m_Packets.front();
		m_Packets.pop();
	}

	delete m_BNCSUtil;

	for (vector<PairedMailAdd>::iterator i = m_PairedMailAdds.begin(); i != m_PairedMailAdds.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedMailRemove>::iterator i = m_PairedMailRemoves.begin(); i != m_PairedMailRemoves.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedMailReaded>::iterator i = m_PairedMailReadeds.begin(); i != m_PairedMailReadeds.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoCount>::iterator i = m_PairedInfoCounts.begin(); i != m_PairedInfoCounts.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoAdd>::iterator i = m_PairedInfoAdds.begin(); i != m_PairedInfoAdds.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );
	
	for (vector<PairedInfoPrivCalculate>::iterator i = m_PairedInfoPrivCalculates.begin(); i != m_PairedInfoPrivCalculates.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoPubCalculate>::iterator i = m_PairedInfoPubCalculates.begin(); i != m_PairedInfoPubCalculates.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoPrivPoints>::iterator i = m_PairedInfoPrivPointss.begin(); i != m_PairedInfoPrivPointss.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoPubPoints>::iterator i = m_PairedInfoPubPointss.begin(); i != m_PairedInfoPubPointss.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoChallwins>::iterator i = m_PairedInfoChallwinss.begin(); i != m_PairedInfoChallwinss.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoChallloses>::iterator i = m_PairedInfoChalllosess.begin(); i != m_PairedInfoChalllosess.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoRemove>::iterator i = m_PairedInfoRemoves.begin(); i != m_PairedInfoRemoves.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoUpdateCountry>::iterator i = m_PairedInfoUpdateCountrys.begin(); i != m_PairedInfoUpdateCountrys.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoUpdateLvl>::iterator i = m_PairedInfoUpdateLvls.begin(); i != m_PairedInfoUpdateLvls.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoUpdateMessage>::iterator i = m_PairedInfoUpdateMessages.begin(); i != m_PairedInfoUpdateMessages.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedInfoUpdateGinfo>::iterator i = m_PairedInfoUpdateGinfos.begin(); i != m_PairedInfoUpdateGinfos.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedWarnCount>::iterator i = m_PairedWarnCounts.begin(); i != m_PairedWarnCounts.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedWarnAdd>::iterator i = m_PairedWarnAdds.begin(); i != m_PairedWarnAdds.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedWarnUpdateAdd>::iterator i = m_PairedWarnUpdateAdds.begin(); i != m_PairedWarnUpdateAdds.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedWarnChannelBan>::iterator i = m_PairedWarnChannelBans.begin(); i != m_PairedWarnChannelBans.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedWarnRemove>::iterator i = m_PairedWarnRemoves.begin(); i != m_PairedWarnRemoves.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedRunQuery>::iterator i = m_PairedRunQuerys.begin(); i != m_PairedRunQuerys.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );
	
	for (vector<PairedBanCount>::iterator i = m_PairedBanCounts.begin(); i != m_PairedBanCounts.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedBanAdd>::iterator i = m_PairedBanAdds.begin(); i != m_PairedBanAdds.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedBanRemove>::iterator i = m_PairedBanRemoves.begin(); i != m_PairedBanRemoves.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedGPSCheck>::iterator i = m_PairedGPSChecks.begin(); i != m_PairedGPSChecks.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedDPSCheck>::iterator i = m_PairedDPSChecks.begin(); i != m_PairedDPSChecks.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );
	
	for (vector<PairedKillerCheck>::iterator i = m_PairedKillerChecks.begin(); i != m_PairedKillerChecks.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for (vector<PairedFarmerCheck>::iterator i = m_PairedFarmerChecks.begin(); i != m_PairedFarmerChecks.end(); i++ )
		m_GHost->m_Callables.push_back( i->second );

	if (m_CallableBanList )
		m_GHost->m_Callables.push_back( m_CallableBanList );

	if (m_CallableMailList )
		m_GHost->m_Callables.push_back( m_CallableMailList );

	if (m_CallableInfoList )
		m_GHost->m_Callables.push_back( m_CallableInfoList );

	if (m_CallableWarnList )
		m_GHost->m_Callables.push_back( m_CallableWarnList );

	for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++ )
		delete *i;

	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++ )
		delete *i;

	for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); i++ )
		delete *i;

	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ )
		delete *i;

	for (vector<CDBMaps *>::iterator i = m_Maps.begin(); i != m_Maps.end(); i++ )
		delete *i;

	for (vector<CDBIcons *>::iterator i = m_IconList.begin(); i != m_IconList.end(); i++ )
		delete *i;

	for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end(); i++ )
		delete *i;
}

BYTEARRAY CBNET::GetUniqueName() {
	return m_Protocol->GetUniqueName();
}

unsigned int CBNET::SetFD(void *fd, void *send_fd, int *nfds) {
	unsigned int NumFDs = 0;

	if (!m_Socket->HasError( ) && m_Socket->GetConnected( ) ) {
		m_Socket->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
		NumFDs++;

		if (m_BNLSClient) {
			NumFDs += m_BNLSClient->SetFD(fd, send_fd, nfds);
		}
	}

	return NumFDs;
}

bool CBNET::Update(void *fd, void *send_fd) {

	if (GetTime() < m_StartTime) {
		m_StartTime = GetTime();
	}

	//Run this one time after you logged on server
	if (!m_HostbotsChecked && m_LoggedIn && m_Infos.size( )>0 ) {
		m_HostbotsChecked = true;
		string tmp= "";
		string tmp2= "";
		bool isnotinlist = true;
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++ ) {
			//Add all lvl 6 in hostbot pool if they aren't in it already
			if ((*i)->GetLvl( )==6) {
				tmp = (*i)->GetUser();
				transform( tmp.begin( ), tmp.end( ), tmp.begin( ), (int(*)(int))tolower );
				for (vector<CDBHostBot *>::iterator j = m_HostBots.begin(); j != m_HostBots.end();j++ ) {
					tmp2 = (*j)->GetHostBot();
					transform( tmp2.begin( ), tmp2.end( ), tmp2.begin( ), (int(*)(int))tolower );
					if (tmp2 == tmp )			
						isnotinlist = false;
				}

				if (isnotinlist) {
					m_HostBots.push_back(new CDBHostBot(tmp, 0, 0, 10));
				}
			}
		}

		for (vector<CDBHostBot *> ::iterator i = m_HostBots.begin(); i != m_HostBots.end(); i++) {
			SendChatCommand((*i)->GetHostBot(), UTIL_ToString(m_CommandTrigger) + "gethb");
		}
	}

	if (( m_GHost->m_SiteLastUpdateTimer + m_GHost->m_SiteUpdatedTime < GetTime( ))&& m_GHost->m_SiteUpdateSupported ) {
		CONSOLE_Print("Updating php file.");
		m_GHost->m_SiteLastUpdateTimer = GetTime();
		UpdateSite();
	}

	//Icons add if icon system support from bnet
	if (((GetTime( ) > m_IconRefreshTimer + m_GHost->m_IconTimer ) || m_IconForceRefresh) && m_LoggedIn && m_GHost->m_IconSupport && m_IconLoadedFromFile) {
		m_IconRefreshTimer = GetTime();
		m_IconForceRefresh = false;
		CONSOLE_Print("[Icon]: Daily refreshing icon list from all users.", true);

		uint32_t maxrankadd=25;

		m_IconRefreshTimer = GetTime();
		m_IconForceRefresh = false;
		CONSOLE_Print("[Icon]: Daily refreshing icon list from all users.", true);
		CONSOLE_Print("[Icon]: Deleting icons from bnet from file.", true);
		ifstream in3;
		in3.open(m_GHost->gIconsFile.c_str());
		if (in3.fail()) {
			CONSOLE_Print("[Icon]: Fail load icons file", true);
		}
		else {
			CONSOLE_Print("[Icon]: Loading icon file from disk to delete icons from bnet.", true);
			uint32_t Count = 0;
			string Line;
			while (!in3.eof() && Count < 500) {
				getline(in3, Line);
				stringstream SS;
				SS << Line;
				string tbl[4];
				uint32_t counter = 0;
				while (!SS.eof()) {
					string words;
					SS >> words;
					if (SS.fail()) {
						CONSOLE_Print("[Icon]: system error #6", true);
						break;
					}
					else {
						tbl[counter] = words;
						counter++;
					}
				}

				if (!Line.empty() && counter == 4) {
					SendChatCommand("/seticon del " + tbl[0] + " " + tbl[2]);
					Count++;
				}
				else {
					CONSOLE_Print("[Icon]: system error #7", true);
				}

				if (in3.eof()) {
					break;
				}
			}
			in3.close();
		}

		CONSOLE_Print("[Icon]: Clearing file", true);
		ifstream a_file(m_GHost->gIconsFile.c_str());
		if (!a_file.is_open()) {
			CONSOLE_Print("[Icon]: system error #3. Can't open icon file.", true);
		}
		else {
			ofstream a_file(m_GHost->gIconsFile.c_str(), ios::trunc);
			CONSOLE_Print("[Icon]: File cleared.", true);
		}

		m_IconList.clear();

		for (vector<CDBInfo *> ::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
			if ((*i)->GetLvl() < 6 && (*i)->GetPrivPoints() > 0 && (*i)->GetPrivRank() <= maxrankadd) {
				//Write log file
				CONSOLE_Print("[Icon]: Writing new icon file.", true);
				
				m_IconList.push_back(new CDBIcons((*i)->GetUser(), m_UserName, GetIconFromRank((*i)->GetPrivRank()), "rank_icon" + UTIL_ToString((*i)->GetPrivRank())));
				
				fstream filestr;
				filestr.open(m_GHost->gIconsFile.c_str(), fstream::in | fstream::out | fstream::app);
				filestr << (*i)->GetUser() + " " + GetIconFromRank((*i)->GetPrivRank()) + " rank_icon" + UTIL_ToString((*i)->GetPrivRank()) << endl;
				filestr.close();
			}
		}

		CONSOLE_Print("[Icon]: Giving new icons to bnet and writing in file also for backup", true);
		for (vector<CDBIcons *>::iterator i = m_IconList.begin(); i != m_IconList.end(); i++ ) {
			fstream filestr;
			filestr.open (m_GHost->gIconsFile.c_str( ), fstream::in | fstream::out | fstream::app);
			filestr<<(*i)->GetUser( )+" "+(*i)->GetFrom( )+" "+(*i)->GetIcon( )+" "+(*i)->GetReason( )<<endl;
			SendChatCommand("/seticon add "+(*i)->GetUser( )+" "+(*i)->GetIcon( ));
			filestr.close();
		}
	}
	
	if (m_LastChannelBanCheck + 900<GetTime( ))
	{
		m_LastChannelBanCheck=GetTime();
		
		uint32_t Hours = GetHoursSince1970();
		for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); i++ ) {
			if ((*i)->GetDaysban( )>0) {
				if (Hours>=(*i)->GetDaysban()) {
					SendChatCommand("/unban "+(*i)->GetName());
					m_PairedWarnChannelBans.push_back( PairedWarnChannelBan((*i)->GetName(), m_GHost->m_DB->ThreadedWarnChannelBan( m_Server,(*i)->GetName(), 0, "Bot" ) ) );					
				}
			}
		}
		CONSOLE_Print("[Console: " + m_ServerAlias + "] Channel bans checked", true);
	}

	if (m_OnlineChecking && m_OnlineCheckingTime + 2 < GetTime( )) {
		uint32_t UsersNumber=0;
		string temp;
		uint32_t UsersInTemp=0;
		SendChatCommand(m_MassMessageCalledBy,"Users in games.");
		for (set<string>::iterator j = m_OnlineUsers.begin(); j != m_OnlineUsers.end(); j++ ) {
			if (*j!=m_UserName) {
				temp+=*j+"   ";
				UsersNumber++;
				UsersInTemp++;
			}
			if (UsersInTemp==4) {
				SendChatCommand(m_MassMessageCalledBy,temp);
				temp="";
				UsersInTemp=0;
			}
		}
		if (UsersInTemp > 0) {
			SendChatCommand(m_MassMessageCalledBy, temp);
		}
		SendChatCommand(m_MassMessageCalledBy,UTIL_ToString(UsersNumber)+" users.");

		SendChatCommand(m_MassMessageCalledBy,"-----------------------------------------------------------");
		SendChatCommand(m_MassMessageCalledBy,"Users in channels.");
		temp="";
		UsersNumber=0;
		UsersInTemp = 0;
		for (vector<string>::iterator j = m_InChannelUsers.begin(); j != m_InChannelUsers.end(); j++ ) {
			if (*j!=m_UserName) {
				temp+=*j+"   ";
				UsersNumber++;
				UsersInTemp++;
			}
			if (UsersInTemp==4) {
				SendChatCommand(m_MassMessageCalledBy,temp);
				temp="";
				UsersInTemp=0;
			}
		}
		if (UsersInTemp > 0) {
			SendChatCommand(m_MassMessageCalledBy, temp);
		}

		SendChatCommand(m_MassMessageCalledBy,UTIL_ToString(UsersNumber)+" users.");

		m_OnlineChecking=false;
	}


	if (m_CountingStart && m_CounderStart + 2 < GetTime( )) {

		uint32_t ss=0;
		uint32_t nss=0;
		for (vector<CDBInfo *>::iterator j = m_Infos.begin(); j != m_Infos.end(); j++ ) {
			if ((*j)->GetLvl() < 6 && (*j)->GetLvl() > 0 && (*j)->GetMessage() == 1) {
				ss++;
			}
			if ((*j)->GetMessage() == 1 && (*j)->GetLvl() == 0 && (*j)->GetAdmin() == m_GHost->m_CommunityNameLower) {
				nss++;
			}
		}	
		uint32_t recivers = 0;
		if (m_ssList) {
			recivers = ss - m_UsersOffBnet;
		}
		else {
			recivers = nss - m_UsersOffBnet;
		}

		SendChatCommand(m_MassMessageCalledBy,UTIL_ToString(recivers)+" users have received the message.");
		m_UsersOffBnet= 0;
		m_CountingStart=false;
	}

	if (m_SlotsSpamEnable && GetTime( ) > m_SlotsSpamTimer + m_SlotsLastSpam ) {
		m_SlotsLastSpam = GetTime();
		uint32_t count=0;
		for (uint32_t i = 0; i < 12; i++) {
			if (!m_meplay[i][0].empty()) {
				count++;
			}
		}

		if ((count!=0 && !m_Challenge))	{
			string slots;
			for (int i = 1; i < 6; i++) {
				if (!m_meplay[i - 1][0].empty()) {
					slots += UTIL_ToString(i) + "." + m_meplay[i - 1][0] + " ";
				}
			}
			
			string slotsb;
			for (int i = 6; i < 11; i++) {
				if (!m_meplay[i - 1][0].empty()) {
					slotsb += UTIL_ToString(i) + "." + m_meplay[i - 1][0] + " ";
				}
			}
						
			QueueChatCommand("Sentinel: "+ slots );	
			if (slotsb != "") {
				QueueChatCommand("Scourge: " + slotsb);
			}
		}
		else {							
			QueueChatCommand("No slots held");							
		}
	}

	if (!m_AnnounceMessage.empty( ) && GetTime( ) >= m_LastAnnounceTime + m_AnnounceInterval ) {
		QueueChatCommand( m_AnnounceMessage );	
		m_LastAnnounceTime = GetTime();
		
		if (m_Challenge && m_AnnounceInterval>=60) {
			MassMessage(m_Challenger, "Challenge started. /join "+m_CurrentChannel+" and "+UTIL_ToString(m_CommandTrigger)+"sign", true, false);
		}
	}

	if (m_Challenge && m_ChallengeTimers ) {
		if (GetTime( ) >m_ChallengeTime + 600) {				
			m_ChallengeTimerExtended = false;
			m_ChallengeTime+=300;
			m_ChallengeTimersRemainSpam = 0;
		}
		else if (GetTime( ) >m_ChallengeTime + 540&& m_ChallengeTimersRemainSpam==1) {
			m_ChallengeTimerExtended = true;
			m_ChallengeTimersRemainSpam++;
		}
		else if (GetTime( ) >m_ChallengeTime + 300 && m_ChallengeTimersRemainSpam==0) {
			m_ChallengeTimersRemainSpam++;
		}
	}

	//
	// update callables
	//
	for (vector<PairedMailRemove>::iterator i = m_PairedMailRemoves.begin(); i != m_PairedMailRemoves.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				RemoveMail( i->second->GetUser( ), i->second->GetId( ) );
				SendChatCommand(i->first,i->second->GetUser( )+" mail with id "+UTIL_ToString( i->second->GetId( ) )+" is deleted.");
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedMailRemoves.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedMailReaded>::iterator i = m_PairedMailReadeds.begin(); i != m_PairedMailReadeds.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				ReadedMail( i->second->GetUser( ));
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedMailReadeds.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedMailAdd>::iterator i = m_PairedMailAdds.begin(); i != m_PairedMailAdds.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				AddMail( i->second->GetId( ), i->second->GetSender( ), i->second->GetReceiver( ), i->second->GetMessage( ));
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedMailAdds.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoCount>::iterator i = m_PairedInfoCounts.begin(); i != m_PairedInfoCounts.end(); ) {
		if (i->second->GetReady( ) ) {
			uint32_t Count = i->second->GetResult();

			if (Count == 0) {
				SendChatCommand(i->first, "There isn 't any user with info in db at " + m_Server + " server.");
			}
			else if (Count == 1) {
				SendChatCommand(i->first, "There is only one user with info in db at " + m_Server + " server.");
			}
			else {
				SendChatCommand(i->first, "There are " + UTIL_ToString(Count) + " users with info in db at " + m_Server + " server.");
			}
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoCounts.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoAdd>::iterator i = m_PairedInfoAdds.begin(); i != m_PairedInfoAdds.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				AddInfo( i->second->GetUser( ), i->second->GetLvl( ), i->second->GetPrivRank( ),i->second->GetPubRank( ), i->second->GetPrivPoints( ), i->second->GetPubPoints( ), i->second->GetAdmin( ),i->second->GetCountry( ), i->second->GetChallwins( ), i->second->GetChallloses( ) ,i->second->GetGinfo( ),i->second->GetMessage( ) );
			}
			else {
				QueueChatCommand("Error adding info in database.");
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoAdds.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoPrivCalculate>::iterator i = m_PairedInfoPrivCalculates.begin(); i != m_PairedInfoPrivCalculates.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				CalculatePrivInfo(  );
			}
		
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoPrivCalculates.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoPubCalculate>::iterator i = m_PairedInfoPubCalculates.begin(); i != m_PairedInfoPubCalculates.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				CalculatePubInfo(  );
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoPubCalculates.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoPrivPoints>::iterator i = m_PairedInfoPrivPointss.begin(); i != m_PairedInfoPrivPointss.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				PrivPointsInfo( i->second->GetUser( ), i->second->GetPrivPoints( ) );
				
				if (!i->first.empty()) {
					SendChatCommand(i->first, "Your priv points are now " + UTIL_ToString(i->second->GetPrivPoints()) + ".");
				}
			}
			else {
				QueueChatCommand("Error adding points in " + i->second->GetServer() + " for " + i->second->GetUser(), i->first, !i->first.empty());
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoPrivPointss.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoPubPoints>::iterator i = m_PairedInfoPubPointss.begin(); i != m_PairedInfoPubPointss.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				PubPointsInfo( i->second->GetUser( ), i->second->GetPubPoints( ) );
				
				if (!i->first.empty()) {
					SendChatCommand(i->first, "Your pub points are now " + UTIL_ToString(i->second->GetPubPoints()) + ".");
				}
			}
			else {
				QueueChatCommand("Error adding points in " + i->second->GetServer() + " for " + i->second->GetUser(), i->first, !i->first.empty());
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoPubPointss.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoChallwins>::iterator i = m_PairedInfoChallwinss.begin(); i != m_PairedInfoChallwinss.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				ChallWinsInfo( i->second->GetUser( ), i->second->GetChallwins( ) );
			}
			else {
				QueueChatCommand("Error adding points in " + i->second->GetServer() + " for " + i->second->GetUser(), i->first, !i->first.empty());
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoChallwinss.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoChallloses>::iterator i = m_PairedInfoChalllosess.begin(); i != m_PairedInfoChalllosess.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				ChallLosesInfo( i->second->GetUser( ), i->second->GetChallloses( ) );
				
			}
			else {
				QueueChatCommand("Error adding points in " + i->second->GetServer() + " for " + i->second->GetUser(), i->first, !i->first.empty());
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoChalllosess.erase( i );
		}
		else {
			i++;
		}
	}
	
	for (vector<PairedInfoRemove>::iterator i = m_PairedInfoRemoves.begin(); i != m_PairedInfoRemoves.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				RemoveInfo( i->second->GetUser( ) );
				QueueChatCommand( "Info from "+ i->second->GetUser( )+" removed." , i->first, !i->first.empty( ) );
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoRemoves.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoUpdateCountry>::iterator i = m_PairedInfoUpdateCountrys.begin(); i != m_PairedInfoUpdateCountrys.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				UpdateCountryInfo( i->second->GetUser( ), i->second->GetCountry( ) );
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoUpdateCountrys.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoUpdateLvl>::iterator i = m_PairedInfoUpdateLvls.begin(); i != m_PairedInfoUpdateLvls.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				UpdateLvlInfo( i->second->GetUser( ), i->second->GetLvl( ), i->second->GetAdmin( ) );
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoUpdateLvls.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoUpdateMessage>::iterator i = m_PairedInfoUpdateMessages.begin(); i != m_PairedInfoUpdateMessages.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				UpdateMessageInfo( i->second->GetUser( ) ,i->second->GetMessage( ) );
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoUpdateMessages.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedInfoUpdateGinfo>::iterator i = m_PairedInfoUpdateGinfos.begin(); i != m_PairedInfoUpdateGinfos.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				UpdateGinfoInfo( i->second->GetUser( ), i->second->GetGinfo( ),i->second->GetAdmin( ) );
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedInfoUpdateGinfos.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedWarnCount>::iterator i = m_PairedWarnCounts.begin(); i != m_PairedWarnCounts.end(); ) {
		if (i->second->GetReady( ) ) {
			uint32_t Count = i->second->GetResult();

			if (Count == 0) {
				QueueChatCommand("There isn 't a warning in db at " + m_Server + " server.", i->first, !i->first.empty());
			}
			else if (Count == 1) {
				QueueChatCommand("There is only 1 warning in db at " + m_Server + " server.", i->first, !i->first.empty());
			}
			else {
				QueueChatCommand("There are " + UTIL_ToString(Count) + " warnings in db at " + m_Server + " server.", i->first, !i->first.empty());
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedWarnCounts.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedWarnAdd>::iterator i = m_PairedWarnAdds.begin(); i != m_PairedWarnAdds.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				AddWarn( i->second->GetUser( ), i->second->GetWarnings( ), i->second->GetWarning( ), i->second->GetTotalwarn( ), i->second->GetDaysban( ),i->second->GetAdmin( ) );
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedWarnAdds.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedWarnUpdateAdd>::iterator i = m_PairedWarnUpdateAdds.begin(); i != m_PairedWarnUpdateAdds.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				UpdateAddWarn( i->second->GetUser( ), i->second->GetWarnings( ), i->second->GetWarning( ), i->second->GetTotalwarn( ), i->second->GetAdmin( ) );
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedWarnUpdateAdds.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedWarnChannelBan>::iterator i = m_PairedWarnChannelBans.begin(); i != m_PairedWarnChannelBans.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				ChannelBanWarn( i->second->GetUser( ), i->second->GetDaysban( ),i->second->GetAdmin( ) );	
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedWarnChannelBans.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedWarnRemove>::iterator i = m_PairedWarnRemoves.begin(); i != m_PairedWarnRemoves.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				RemoveWarn( i->second->GetUser( ) );
				QueueChatCommand( "Warnings deleted from "+i->second->GetUser( )+"." , i->first, !i->first.empty( ) );
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedWarnRemoves.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedRunQuery>::iterator i = m_PairedRunQuerys.begin(); i != m_PairedRunQuerys.end(); ) {
		if (i->second->GetReady( ) ) {
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedRunQuerys.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedBanCount>::iterator i = m_PairedBanCounts.begin(); i != m_PairedBanCounts.end(); ) {
		if (i->second->GetReady( ) ) {
			uint32_t Count = i->second->GetResult();

			if (Count == 0) {
				QueueChatCommand(m_GHost->m_Language->ThereAreNoBannedUsers(m_Server), i->first, !i->first.empty());
			}
			else if (Count == 1) {
				QueueChatCommand(m_GHost->m_Language->ThereIsBannedUser(m_Server), i->first, !i->first.empty());
			}
			else {
				QueueChatCommand(m_GHost->m_Language->ThereAreBannedUsers(m_Server, UTIL_ToString(Count)), i->first, !i->first.empty());
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanCounts.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedBanAdd>::iterator i = m_PairedBanAdds.begin(); i != m_PairedBanAdds.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				AddBan( i->second->GetUser( ), i->second->GetIP( ), i->second->GetGameName( ), i->second->GetAdmin( ), i->second->GetReason( ) );
				SendChatCommand(i->first, m_GHost->m_Language->BannedUser( i->second->GetServer( ), i->second->GetUser( ) ) );
			}
			else {
				SendChatCommand(i->first, m_GHost->m_Language->ErrorBanningUser(i->second->GetServer(), i->second->GetUser()));
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanAdds.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedBanRemove>::iterator i = m_PairedBanRemoves.begin(); i != m_PairedBanRemoves.end(); ) {
		if (i->second->GetReady( ) ) {
			if (i->second->GetResult( ) ) {
				RemoveBan( i->second->GetUser( ) );
				SendChatCommand(i->first, m_GHost->m_Language->UnbannedUser( i->second->GetUser( ) ));
			}
			else {
				SendChatCommand(i->first, m_GHost->m_Language->ErrorUnbanningUser(i->second->GetUser()));
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanRemoves.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedGPSCheck>::iterator i = m_PairedGPSChecks.begin(); i != m_PairedGPSChecks.end(); ) {
		if (i->second->GetReady( ) ) {
			CDBGamePlayerSummary *GamePlayerSummary = i->second->GetResult();

			if (!i->first.empty( )) {//if whisper
				if (GamePlayerSummary) {
					SendChatCommand(i->first, m_GHost->m_Language->HasPlayedGamesWithThisBot(i->second->GetName(), GamePlayerSummary->GetFirstGameDateTime(), GamePlayerSummary->GetLastGameDateTime(), UTIL_ToString(GamePlayerSummary->GetTotalGames()), UTIL_ToString((float)GamePlayerSummary->GetAvgLoadingTime() / 1000, 2), UTIL_ToString(GamePlayerSummary->GetAvgLeftPercent())));
				}
				else {
					SendChatCommand(i->first, m_GHost->m_Language->HasntPlayedGamesWithThisBot(i->second->GetName()));
				}
			}
			else {
				if (GamePlayerSummary )
					QueueChatCommand( m_GHost->m_Language->HasPlayedGamesWithThisBot( i->second->GetName( ), GamePlayerSummary->GetFirstGameDateTime( ), GamePlayerSummary->GetLastGameDateTime( ), UTIL_ToString( GamePlayerSummary->GetTotalGames( ) ), UTIL_ToString( (float)GamePlayerSummary->GetAvgLoadingTime( ) / 1000, 2 ), UTIL_ToString( GamePlayerSummary->GetAvgLeftPercent( ) ) ), i->first, !i->first.empty( ) );
				else
					QueueChatCommand( m_GHost->m_Language->HasntPlayedGamesWithThisBot( i->second->GetName( ) ), i->first, !i->first.empty( ) );
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedGPSChecks.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedDPSCheck>::iterator i = m_PairedDPSChecks.begin(); i != m_PairedDPSChecks.end(); ) {
		if (i->second->GetReady( ) ) {
			CDBDotAPlayerSummary *DotAPlayerSummary = i->second->GetResult();

			if (DotAPlayerSummary ) {
				if (!i->first.empty( )) {//if whisper
					SendChatCommand( i->first,"---------- "+i->second->GetName( )+"'s stats ----------" );
					uint32_t temp2 = (100 * DotAPlayerSummary->GetTotalWins( ) )/ DotAPlayerSummary->GetTotalGames( ) ;
					SendChatCommand(i->first, "Wins/Loses stats: ("+UTIL_ToString( DotAPlayerSummary->GetTotalWins( ))+"/"+UTIL_ToString( DotAPlayerSummary->GetTotalLosses( ))+") --- Win procent: "+UTIL_ToString(temp2)+"%");
					
					SendChatCommand( i->first,"Average cs: "+UTIL_ToString( DotAPlayerSummary->GetAvgCreepKills( ), 2 )+"/"+UTIL_ToString( DotAPlayerSummary->GetAvgCreepDenies( ), 2 )+"/"+UTIL_ToString( DotAPlayerSummary->GetAvgNeutralKills( ), 2 )+
						" --- Total cs : "+UTIL_ToString( DotAPlayerSummary->GetTotalCreepKills( ) )+"/"+UTIL_ToString( DotAPlayerSummary->GetTotalCreepDenies( ) )+"/"+UTIL_ToString( DotAPlayerSummary->GetTotalNeutralKills( )));
					
					SendChatCommand( i->first,"Average score: "+UTIL_ToString( DotAPlayerSummary->GetAvgKills( ), 2 )+"/"+UTIL_ToString( DotAPlayerSummary->GetAvgDeaths( ), 2 )+"/"+UTIL_ToString( DotAPlayerSummary->GetAvgAssists( ), 2 )+
						" --- Total score: "+UTIL_ToString( DotAPlayerSummary->GetTotalKills( ))+"/"+UTIL_ToString( DotAPlayerSummary->GetTotalDeaths( ))+"/"+UTIL_ToString( DotAPlayerSummary->GetTotalAssists( ) ));
					
					SendChatCommand( i->first,"Total tower/rax/courier kill: "+UTIL_ToString( DotAPlayerSummary->GetTotalTowerKills( ))+"/"+
						UTIL_ToString( DotAPlayerSummary->GetTotalRaxKills( ) )+"/"+
						UTIL_ToString( DotAPlayerSummary->GetTotalCourierKills( ) ));
				}
				else {
					QueueChatCommand("Error...");	
				}
			}
			else {
				QueueChatCommand(m_GHost->m_Language->HasntPlayedDotAGamesWithThisBot(i->second->GetName()), i->first, !i->first.empty());
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedDPSChecks.erase( i );
		}
		else {
			i++;
		}
	}

	for (vector<PairedKillerCheck>::iterator i = m_PairedKillerChecks.begin(); i != m_PairedKillerChecks.end(); ) {
		if (i->second->GetReady( ) ) {
			CDBDotAPlayerKiller *DotAPlayerKiller = i->second->GetResult();

			if (DotAPlayerKiller ) 	{
				if (!i->first.empty( )) {//if whisper
	
					if (i->second->GetName( ).empty( )) {
						SendChatCommand( i->first,"---------- Best killer stats ----------" );
						SendChatCommand(i->first, "Gameid / Name / Kills / Death / Assists --- "+UTIL_ToString( DotAPlayerKiller->GetGameid( ))+"/"+DotAPlayerKiller->GetName( )+"/"+UTIL_ToString(DotAPlayerKiller->GetTotalKills( ))+"/"+UTIL_ToString(DotAPlayerKiller->GetTotalDeaths( ))+"/"+UTIL_ToString(DotAPlayerKiller->GetTotalAssists( ))+".");
					}
					else if (!i->second->GetName( ).empty( ) && i->second->GetRank( )==0) {
						SendChatCommand(i->first, "Best killer: Gameid / Name / Kills / Death / Assists --- "+UTIL_ToString( DotAPlayerKiller->GetGameid( ))+"/"+UTIL_ToString(DotAPlayerKiller->GetTotalKills( ))+"/"+UTIL_ToString(DotAPlayerKiller->GetTotalDeaths( ))+"/"+UTIL_ToString(DotAPlayerKiller->GetTotalAssists( ))+".");
					}
					else {
						CONSOLE_Print("[icons] Checking icon killer rank "+UTIL_ToString(i->second->GetRank( ))+".", true);
					}

				}
				else {
					QueueChatCommand("Error...");	
				}
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedKillerChecks.erase( i );
		}
		else {
			i++;
		}
	}


	for (vector<PairedFarmerCheck>::iterator i = m_PairedFarmerChecks.begin(); i != m_PairedFarmerChecks.end(); ) {
		if (i->second->GetReady( ) ) {
			CDBDotAPlayerFarmer *DotAPlayerFarmer = i->second->GetResult();

			if (DotAPlayerFarmer ) {
				if (!i->first.empty( )) {
					if (i->second->GetName( ).empty( )) {
						SendChatCommand( i->first,"---------- Best farmer stats ----------" );
						SendChatCommand(i->first, "Gameid / Name / CKill / Deny / NKill --- "+UTIL_ToString( DotAPlayerFarmer->GetGameid( ))+"/"+DotAPlayerFarmer->GetName( )+"/"+UTIL_ToString(DotAPlayerFarmer->GetTotalCreepKills( ))+"/"+UTIL_ToString(DotAPlayerFarmer->GetTotalCreepDenies( ))+"/"+UTIL_ToString(DotAPlayerFarmer->GetTotalNeutralKills( ))+".");
					}
					else if (!i->second->GetName( ).empty( ) && i->second->GetRank( )==0) {
						SendChatCommand(i->first, "Best farmer: Gameid / Name / CKill / Deny / NKill --- "+UTIL_ToString( DotAPlayerFarmer->GetGameid( ))+"/"+UTIL_ToString(DotAPlayerFarmer->GetTotalCreepKills( ))+"/"+UTIL_ToString(DotAPlayerFarmer->GetTotalCreepDenies( ))+"/"+UTIL_ToString(DotAPlayerFarmer->GetTotalNeutralKills( ))+".");
					}
					else {
						CONSOLE_Print("[icons] Checking icon farmer rank "+UTIL_ToString(i->second->GetRank( ))+".", true);
					}
				}
				else {
					QueueChatCommand("Error...");	
				}
			}
			
			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedFarmerChecks.erase( i );
		}
		else {
			i++;
		}
	}

	//Refresh the mail list every day
	if (!m_CallableMailList && GetTime() - m_LastMailRefreshTime >= 86400) {
		m_CallableMailList = m_GHost->m_DB->ThreadedMailList(m_Server);
	}

	if (m_CallableMailList && m_CallableMailList->GetReady( ) ) {
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] refreshed mail list (" + UTIL_ToString( m_Mails.size( ) ) + " -> " + UTIL_ToString( m_CallableMailList->GetResult( ).size( ) ) + " mails)" , true);

		for (vector<CDBMail *> ::iterator i = m_Mails.begin(); i != m_Mails.end(); i++) {
			delete *i;
		}

		m_Mails = m_CallableMailList->GetResult();
		m_GHost->m_DB->RecoverCallable( m_CallableMailList );
		delete m_CallableMailList;
		m_CallableMailList = NULL;
		m_LastMailRefreshTime = GetTime();
	}

	//Refresh the info list every day
	if (!m_CallableInfoList && GetTime() - m_LastInfoRefreshTime >= 86400) {
		m_CallableInfoList = m_GHost->m_DB->ThreadedInfoList(m_Server);
	}

	if (m_CallableInfoList && m_CallableInfoList->GetReady( ) ) {
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] refreshed info list (" + UTIL_ToString( m_Infos.size( ) ) + " -> " + UTIL_ToString( m_CallableInfoList->GetResult( ).size( ) ) + " infos)" , true);

		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++ )
			delete *i;
		m_Infos = m_CallableInfoList->GetResult();
		m_GHost->m_DB->RecoverCallable( m_CallableInfoList );
		delete m_CallableInfoList;
		m_CallableInfoList = NULL;	
		m_LastInfoRefreshTime = GetTime();	
	}
	
	//Refresh the warn list every day
	if (!m_CallableWarnList && GetTime( ) - m_LastWarnRefreshTime >= 86400 ) {
		m_CallableWarnList = m_GHost->m_DB->ThreadedWarnList( m_Server );
		CONSOLE_Print("[BNET: " + m_ServerAlias + "] Refreshed warn list daily");
	}
	
	if (m_CallableWarnList && m_CallableWarnList->GetReady( ) ) {
		 CONSOLE_Print( "[BNET: " + m_ServerAlias + "] refreshed warn list (" + UTIL_ToString( m_Warns.size( ) ) + " -> " + UTIL_ToString( m_CallableWarnList->GetResult( ).size( ) ) + " warns)" , true);

		 for (vector<CDBWarn *> ::iterator i = m_Warns.begin(); i != m_Warns.end(); i++) {
			 delete *i;
		 }

		m_Warns = m_CallableWarnList->GetResult();
		m_GHost->m_DB->RecoverCallable( m_CallableWarnList );
		delete m_CallableWarnList;
		m_CallableWarnList = NULL;
		m_LastWarnRefreshTime = GetTime();
	}
	
	//Refresh the ban list every day
	if (!m_CallableBanList && GetTime() - m_LastBanRefreshTime >= 86400) {
		m_CallableBanList = m_GHost->m_DB->ThreadedBanList(m_Server);
	}

	if (m_CallableBanList && m_CallableBanList->GetReady( ) ) {

		for (vector<CDBBan *> ::iterator i = m_Bans.begin(); i != m_Bans.end(); i++) {
			delete *i;
		}

		m_Bans = m_CallableBanList->GetResult();
		m_GHost->m_DB->RecoverCallable( m_CallableBanList );
		delete m_CallableBanList;
		m_CallableBanList = NULL;
		m_LastBanRefreshTime = GetTime();
	}

	// we return at the end of each if statement so we don't have to deal with errors related to the order of the if statements
	// that means it might take a few ms longer to complete a task involving multiple steps (in this case, reconnecting) due to blocking or sleeping
	// but it's not a big deal at all, maybe 100ms in the worst possible case (based on a 50ms blocking time)
	
	if (m_Socket->HasError( ) ) {
		// the socket has an error
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] disconnected from battle.net due to socket error" , true);

		if (m_Socket->GetError() == ECONNRESET && GetTime() - m_LastConnectionAttemptTime <= 15) {
			CONSOLE_Print("[BNET: " + m_ServerAlias + "] warning - you are probably temporarily IP banned from battle.net", true);
		}

		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect" , true);
		m_GHost->EventBNETDisconnected( this );
		delete m_BNLSClient;
		m_BNLSClient = NULL;
		m_BNCSUtil->Reset( m_UserName, m_UserPassword );
		m_Socket->Reset();
		m_LastDisconnectedTime = GetTime();
		m_LoggedIn = false;
		if (m_HostbotsChecked) {	
			m_HostbotsChecked = false;
			ResetHostbots();
		}

		m_InChat = false;
		m_WaitingToConnect = true;
		return m_Exiting;
	}

	if (!m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && !m_WaitingToConnect ) {
		// the socket was disconnected
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] disconnected from battle.net" , true);
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] waiting 30 seconds to reconnect" , true);
		m_GHost->EventBNETDisconnected( this );
		delete m_BNLSClient;
		m_BNLSClient = NULL;
		m_BNCSUtil->Reset( m_UserName, m_UserPassword );
		m_Socket->Reset();
		m_LastDisconnectedTime = GetTime();
		m_LoggedIn = false;

		m_HostbotsChecked = false;
		ResetHostbots();

		m_InChat = false;
		m_WaitingToConnect = true;
		return m_Exiting;
	}

	if (m_Socket->GetConnected( ) ) {
		// the socket is connected and everything appears to be working properly

		m_Socket->DoRecv( (fd_set *)fd );
		ExtractPackets();
		ProcessPackets();

		// update the BNLS client

		if (m_BNLSClient ) {
			if (m_BNLSClient->Update( fd, send_fd ) ) {
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] deleting BNLS client" , true);
				delete m_BNLSClient;
				m_BNLSClient = NULL;
			}
			else {
				BYTEARRAY WardenResponse = m_BNLSClient->GetWardenResponse();

				if (!WardenResponse.empty( ) )
					m_Socket->PutBytes( m_Protocol->SEND_SID_WARDEN( WardenResponse ) );
			}
		}

		// check if at least one packet is waiting to be sent and if we've waited long enough to prevent flooding
		// this formula has changed many times but currently we wait 1 second if the last packet was "small", 3.5 seconds if it was "medium", and 4 seconds if it was "big"

		uint32_t WaitTicks = 0;

		if (m_LastOutPacketSize < 10) {
			WaitTicks = 1000;
		}
		else if (m_LastOutPacketSize < 100) {
			WaitTicks = 3500;
		}
		else {
			WaitTicks = 4000;
		}

		if (!m_OutPackets.empty( ) && GetTicks( ) - m_LastOutPacketTicks >= WaitTicks ) {
			if (m_OutPackets.size() > 7) {
				CONSOLE_Print("[BNET: " + m_ServerAlias + "] packet queue warning - there are " + UTIL_ToString(m_OutPackets.size()) + " packets waiting to be sent", true);
			}
			m_Socket->PutBytes( m_OutPackets.front( ) );
			m_LastOutPacketSize = m_OutPackets.front( ).size();
			m_OutPackets.pop();
			m_LastOutPacketTicks = GetTicks();
		}

		// send a null packet every 60 seconds to detect disconnects
		if (GetTime( ) - m_LastNullTime >= 60 && GetTicks( ) - m_LastOutPacketTicks >= 60000 ) {
			m_Socket->PutBytes( m_Protocol->SEND_SID_NULL( ) );
			m_LastNullTime = GetTime();
		}

		m_Socket->DoSend( (fd_set *)send_fd );
		return m_Exiting;
	}

	if (m_Socket->GetConnecting( ) ) {
		// we are currently attempting to connect to battle.net

		if (m_Socket->CheckConnect( ) ) {
			// the connection attempt completed

			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] connected" , true);
			m_GHost->EventBNETConnected( this );
			m_Socket->PutBytes( m_Protocol->SEND_PROTOCOL_INITIALIZE_SELECTOR( ) );
			m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_INFO( m_War3Version, m_GHost->m_TFT, m_CountryAbbrev, m_Country ) );
			
			m_Socket->DoSend( (fd_set *)send_fd );
			m_LastNullTime = GetTime();
			m_LastOutPacketTicks = GetTicks();

			while( !m_OutPackets.empty( ) )
				m_OutPackets.pop();

			return m_Exiting;
		}
		else if (GetTime( ) - m_LastConnectionAttemptTime >= 15 ) {
			// the connection attempt timed out (15 seconds)

			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] connect timed out" , true);
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] waiting 30 seconds to reconnect" , true);
			m_GHost->EventBNETConnectTimedOut( this );
			m_Socket->Reset();
			m_LastDisconnectedTime = GetTime();
			m_WaitingToConnect = true;
			return m_Exiting;
		}
	}

	if (!m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && ( m_FirstConnect || GetTime( ) - m_LastDisconnectedTime >= 30 ) ) {
		// attempt to connect to battle.net

		m_FirstConnect = false;
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] connecting to server [" + m_Server + "] on port 6112" , true);
		m_GHost->EventBNETConnecting( this );

		if (!m_GHost->m_BindAddress.empty()) {
			CONSOLE_Print("[BNET: " + m_ServerAlias + "] attempting to bind to address [" + m_GHost->m_BindAddress + "]", true);
		}

		if (m_ServerIP.empty( ) ) {
			m_Socket->Connect( m_GHost->m_BindAddress, m_Server, 6112 );

			if (!m_Socket->HasError( ) ) {
				m_ServerIP = m_Socket->GetIPString();
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] resolved and cached server IP address " + m_ServerIP, true );
			}
		}
		else {
			// use cached server IP address since resolving takes time and is blocking
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using cached server IP address " + m_ServerIP , true);
			m_Socket->Connect( m_GHost->m_BindAddress, m_ServerIP, 6112 );
		}

		m_WaitingToConnect = false;
		m_LastConnectionAttemptTime = GetTime();
		return m_Exiting;
	}

	return m_Exiting;
}

void CBNET::ExtractPackets() {
	// extract as many packets as possible from the socket's receive buffer and put them in the m_Packets queue

	string *RecvBuffer = m_Socket->GetBytes();
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( Bytes.size( ) >= 4 ) {
		// byte 0 is always 255

		if (Bytes[0] == BNET_HEADER_CONSTANT ) {
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if (Length >= 4 ) {
				if (Bytes.size( ) >= Length ) {
					m_Packets.push( new CCommandPacket( BNET_HEADER_CONSTANT, Bytes[1], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ) ) );
					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else {
					return;
				}
			}
			else {
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error - received invalid packet from battle.net (bad length), disconnecting" , true);
				m_Socket->Disconnect();
				return;
			}
		}
		else {
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error - received invalid packet from battle.net (bad header constant), disconnecting" , true);
			m_Socket->Disconnect();
			return;
		}
	}
}

void CBNET::ProcessPackets() {
	CIncomingGameHost *GameHost = NULL;
	CIncomingChatEvent *ChatEvent = NULL;
	BYTEARRAY WardenData;
	vector<CIncomingFriendList *> Friends;
	vector<CIncomingClanList *> Clans;

	// process all the received packets in the m_Packets queue
	// this normally means sending some kind of response

	while( !m_Packets.empty( ) ) {
		CCommandPacket *Packet = m_Packets.front();
		m_Packets.pop();

		if (Packet->GetPacketType( ) == BNET_HEADER_CONSTANT ) {
			switch( Packet->GetID( ) ) 
			{
			case CBNETProtocol::SID_NULL:
				// warning: we do not respond to NULL packets with a NULL packet of our own
				// this is because PVPGN servers are programmed to respond to NULL packets so it will create a vicious cycle of useless traffic
				// official battle.net servers do not respond to NULL packets

				m_Protocol->RECEIVE_SID_NULL( Packet->GetData( ) );
				break;

			case CBNETProtocol::SID_GETADVLISTEX:
				GameHost = m_Protocol->RECEIVE_SID_GETADVLISTEX( Packet->GetData( ) );

				if (GameHost )
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] joining game [" + GameHost->GetGameName( ) + "]" , true);

				delete GameHost;
				GameHost = NULL;
				break;

			case CBNETProtocol::SID_ENTERCHAT:
				if (m_Protocol->RECEIVE_SID_ENTERCHAT( Packet->GetData( ) ) )
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] joining channel [" + m_FirstChannel + "]" , true);
					m_InChat = true;
					m_Socket->PutBytes( m_Protocol->SEND_SID_JOINCHANNEL( m_FirstChannel ) );
				}

				break;

			case CBNETProtocol::SID_CHATEVENT:
				ChatEvent = m_Protocol->RECEIVE_SID_CHATEVENT( Packet->GetData( ) );

				if (ChatEvent )
					ProcessChatEvent( ChatEvent );

				delete ChatEvent;
				ChatEvent = NULL;
				break;

			case CBNETProtocol::SID_CHECKAD:
				m_Protocol->RECEIVE_SID_CHECKAD( Packet->GetData( ) );
				break;

			case CBNETProtocol::SID_STARTADVEX3:
				if (m_Protocol->RECEIVE_SID_STARTADVEX3( Packet->GetData( ) ) ) {
					m_InChat = false;
					m_GHost->EventBNETGameRefreshed( this );
				}
				else {
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] startadvex3 failed", true );
					m_GHost->EventBNETGameRefreshFailed( this );
				}

				break;

			case CBNETProtocol::SID_PING:
				m_Socket->PutBytes( m_Protocol->SEND_SID_PING( m_Protocol->RECEIVE_SID_PING( Packet->GetData( ) ) ) );
				break;

			case CBNETProtocol::SID_AUTH_INFO:
				if (m_Protocol->RECEIVE_SID_AUTH_INFO( Packet->GetData( ) ) ) {
					if (m_BNCSUtil->HELP_SID_AUTH_CHECK( m_GHost->m_TFT, m_GHost->m_Warcraft3Path, m_CDKeyROC, m_CDKeyTFT, m_Protocol->GetValueStringFormulaString( ), m_Protocol->GetIX86VerFileNameString( ), m_Protocol->GetClientToken( ), m_Protocol->GetServerToken( ) ) ) {
						// override the exe information generated by bncsutil if specified in the config file
						// apparently this is useful for pvpgn users

						if (m_EXEVersion.size( ) == 4 ) {
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using custom exe version bnet_custom_exeversion = " + UTIL_ToString( m_EXEVersion[0] ) + " " + UTIL_ToString( m_EXEVersion[1] ) + " " + UTIL_ToString( m_EXEVersion[2] ) + " " + UTIL_ToString( m_EXEVersion[3] ) , true);
							m_BNCSUtil->SetEXEVersion( m_EXEVersion );
						}

						if (m_EXEVersionHash.size( ) == 4 ) {
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using custom exe version hash bnet_custom_exeversionhash = " + UTIL_ToString( m_EXEVersionHash[0] ) + " " + UTIL_ToString( m_EXEVersionHash[1] ) + " " + UTIL_ToString( m_EXEVersionHash[2] ) + " " + UTIL_ToString( m_EXEVersionHash[3] ) , true);
							m_BNCSUtil->SetEXEVersionHash( m_EXEVersionHash );
						}

						if (m_GHost->m_TFT) {
							CONSOLE_Print("[BNET: " + m_ServerAlias + "] attempting to auth as Warcraft III: The Frozen Throne", true);
						}
						else {
							CONSOLE_Print("[BNET: " + m_ServerAlias + "] attempting to auth as Warcraft III: Reign of Chaos", true);
						}

						m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_CHECK( m_GHost->m_TFT, m_Protocol->GetClientToken( ), m_BNCSUtil->GetEXEVersion( ), m_BNCSUtil->GetEXEVersionHash( ), m_BNCSUtil->GetKeyInfoROC( ), m_BNCSUtil->GetKeyInfoTFT( ), m_BNCSUtil->GetEXEInfo( ), "CB By stefos007" ) );

						// the Warden seed is the first 4 bytes of the ROC key hash
						// initialize the Warden handler

						if (!m_BNLSServer.empty( ) ) {
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] creating BNLS client", true );
							delete m_BNLSClient;
							m_BNLSClient = new CBNLSClient( m_BNLSServer, m_BNLSPort, m_BNLSWardenCookie );
							m_BNLSClient->QueueWardenSeed( UTIL_ByteArrayToUInt32( m_BNCSUtil->GetKeyInfoROC( ), false, 16 ) );
						}
					}
					else {
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - bncsutil key hash failed (check your Warcraft 3 path and cd keys), disconnecting" , true);
						m_Socket->Disconnect();
						delete Packet;
						return;
					}
				}

				break;

			case CBNETProtocol::SID_AUTH_CHECK:
				if (m_Protocol->RECEIVE_SID_AUTH_CHECK( Packet->GetData( ) ) ) {
					// cd keys accepted

					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] cd keys accepted" , true);
					m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGON();
					m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGON( m_BNCSUtil->GetClientKey( ), m_UserName ) );
				}
				else {
					// cd keys not accepted

					switch( UTIL_ByteArrayToUInt32( m_Protocol->GetKeyState( ), false ) )
					{
					case CBNETProtocol::KR_ROC_KEY_IN_USE:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - ROC CD key in use by user [" + m_Protocol->GetKeyStateDescription( ) + "], disconnecting" , true);
						break;
					case CBNETProtocol::KR_TFT_KEY_IN_USE:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - TFT CD key in use by user [" + m_Protocol->GetKeyStateDescription( ) + "], disconnecting" , true);
						break;
					case CBNETProtocol::KR_OLD_GAME_VERSION:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - game version is too old, disconnecting" , true);
						break;
					case CBNETProtocol::KR_INVALID_VERSION:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - game version is invalid, disconnecting" , true);
						break;
					default:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - cd keys not accepted, disconnecting" , true);
						break;
					}

					m_Socket->Disconnect();
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol::SID_AUTH_ACCOUNTLOGON:
				if (m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGON( Packet->GetData( ) ) ) {
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] username [" + m_UserName + "] accepted" , true);

					if (m_PasswordHashType == "pvpgn" ) {
						// pvpgn logon

						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using pvpgn logon type (for pvpgn servers only)" , true);
						m_BNCSUtil->HELP_PvPGNPasswordHash( m_UserPassword );
						m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF( m_BNCSUtil->GetPvPGNPasswordHash( ) ) );
					}
					else {
						// battle.net logon

						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using battle.net logon type (for official battle.net servers only)" , true);
						m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGONPROOF( m_Protocol->GetSalt( ), m_Protocol->GetServerPublicKey( ) );
						m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF( m_BNCSUtil->GetM1( ) ) );
					}
				}
				else {
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - invalid username, disconnecting" , true);
					m_Socket->Disconnect();
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol::SID_AUTH_ACCOUNTLOGONPROOF:
				if (m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( Packet->GetData( ) ) ) {
					// logon successful

					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon successful" , true);
					m_LoggedIn = true;
					m_GHost->EventBNETLoggedIn( this );	
					m_Socket->PutBytes( m_Protocol->SEND_SID_ENTERCHAT( ) );					
				}
				else {
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - invalid password, disconnecting" , true);

					// try to figure out if the user might be using the wrong logon type since too many people are confused by this

					string Server = m_Server;
					transform( Server.begin( ), Server.end( ), Server.begin( ), (int(*)(int))tolower );

					if (m_PasswordHashType == "pvpgn" && (Server == "useast.battle.net" || Server == "uswest.battle.net" || Server == "asia.battle.net" || Server == "europe.battle.net")) {
						CONSOLE_Print("[BNET: " + m_ServerAlias + "] it looks like you're trying to connect to a battle.net server using a pvpgn logon type, check your config file's \"battle.net custom data\" section", true);
					}
					else if (m_PasswordHashType != "pvpgn" && (Server != "useast.battle.net" && Server != "uswest.battle.net" && Server != "asia.battle.net" && Server != "europe.battle.net")) {
						CONSOLE_Print("[BNET: " + m_ServerAlias + "] it looks like you're trying to connect to a pvpgn server using a battle.net logon type, check your config file's \"battle.net custom data\" section", true);
					}

					m_Socket->Disconnect();
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol::SID_WARDEN:
				WardenData = m_Protocol->RECEIVE_SID_WARDEN( Packet->GetData( ) );

				if (m_BNLSClient) {
					m_BNLSClient->QueueWardenRaw(WardenData);
				}
				else
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] warning - received warden packet but no BNLS server is available, you will be kicked from battle.net soon" , true);

				break;			
			}
		}

		delete Packet;
	}
}

void CBNET::ProcessChatEvent( CIncomingChatEvent *chatEvent ) {
	CBNETProtocol::IncomingChatEvent Event = chatEvent->GetChatEvent();
	bool Whisper = ( Event == CBNETProtocol::EID_WHISPER );
	string User = chatEvent->GetUser();
	string Message = chatEvent->GetMessage();

	if (Event == CBNETProtocol::EID_WHISPER || Event == CBNETProtocol::EID_TALK ) {
		if (Event == CBNETProtocol::EID_WHISPER ) {
			m_GHost->EventBNETWhisper( this, User, Message );
			CONSOLE_Print( "[WHISPER: " + m_ServerAlias + "] [" + User + "] " + Message , true);
		}
		else {	
			CONSOLE_Print( "[LOCAL: " + m_ServerAlias + "] [" + User + "] " + Message , true);
			m_GHost->EventBNETChat( this, User, Message );
			//kick muted users from channel
			for (vector<string>::iterator j = m_MuteUsers.begin(); j != m_MuteUsers.end(); j++ ) {
				if (*j == User) {
					SendChatCommand("/kick " + *j + " You are muted!!!");
				}
			}
			//Afkers in channel			
			for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end();i++ ) {						
				//reset time from user in channel (time = 0)
				if ((*i)->GetUser() == User) {
					(*i)->ResetTime();
				}
			}
			//Flame check
			FlameCheck(User,Message);			
		}

		// handle bot commands
		if (Message == "?trigger" && (IsRootAdmin(User) || (m_PublicCommands && m_OutPackets.size() <= 3))) {
			SendChatCommand(User, m_GHost->m_Language->CommandTrigger(string(1, m_CommandTrigger)));
		}
		else if (!Message.empty( ) && Message[0] == m_CommandTrigger ) {
			// extract the command trigger, the command, and the payload
			// e.g. "!say hello world" -> command: "say", payload: "hello world"

			string Command;
			string Payload;
			string::size_type PayloadStart = Message.find( " " );

			if (PayloadStart != string::npos ) {
				Command = Message.substr( 1, PayloadStart - 1 );
				Payload = Message.substr( PayloadStart + 1 );
			}
			else {
				Command = Message.substr(1);
			}

			transform( Command.begin( ), Command.end( ), Command.begin( ), (int(*)(int))tolower );
			CDBInfo *Info = IsInfo( User );	

			if (Info) {
				uint32_t Accesslvl=Info->GetLvl();

				//check if username is udp|root that means that we receivce udp from a hostbot
				//so we need to give hostbots abilities.
				bool IsBannedNameUser=false;
				CDBBan *tempBan = IsBannedName( User );
				if (tempBan) {
					IsBannedNameUser=true;
				}

				if (Accesslvl==6) {
					/*********************
					*   Host bot  lvl 6  *
					*********************/
					if (Command=="hbstate" && !Payload.empty( )) { 
						//state 0 = dead(just in list) 1 = ok 2 = lobby 3 = disabled!
						stringstream SS;
						SS << Payload;
						string tbl[3]={"","",""};
						uint32_t counter=0;
						while( !SS.eof( ) ) {
							string words;
							SS >> words;

							if (SS.fail( ) ) {
								SendChatCommand(User,"error");
								break;
							}
							else {
								//userthatbanned fromuser reason gamename tbl[0] is game id to del it from games
								tbl[counter]=words;
								counter++;
							}
						}
						uint32_t maxgames = 0;
						if (!tbl[2].empty()) {
							maxgames = UTIL_ToUInt32(tbl[2]);
						}
						
						if (counter == 3) {
							UpdateHostbot(User, UTIL_ToUInt32(tbl[0]), UTIL_ToUInt32(tbl[1]), maxgames);
						}
					}

					if (Command=="rmk" && !Payload.empty( )) {
						//reset rmk slots just in case
						for (int i = 0; i < 12; i++) {
							m_RMK[i] = "";
						}

						m_RMKstate = 1; //set that bot hold rmk users 2 = challenge rmk
						//hold users in table
						stringstream SS;
						SS << Payload;						
						uint32_t counter=0;
						while( !SS.eof( ) ) {
							string words;
							SS >> words;
							if (SS.fail()) {
								break;
							}
							else {
								if (counter==0) {
									uint32_t hostgameid = UTIL_ToUInt32(words);
									if (hostgameid > 999) {
										m_RMKstate = 2;
									}
								}
								else {
									m_RMK[counter - 1] = words;
								}
								counter++;
							}
						}	
					}
					if (Command == "say" && !Payload.empty()) {
						QueueChatCommand(Payload);
					}

					if (Command=="mail" && m_GHost->m_MailEnabled) {
						for (vector<CDBInfo *> ::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
							if ((*i)->GetLvl() == 5) {
								m_PairedMailAdds.push_back(PairedMailAdd(Whisper ? User : string(), m_GHost->m_DB->ThreadedMailAdd(GetMailId() + 1, m_Server, User, (*i)->GetUser(), Payload)));
							}
						}
					}

					if (Command=="handshake") {
						SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"handshake");
					}
						
					if (( Command == "addinfo") && !Payload.empty( ) ) {
						uint32_t ll=Info->GetLvl();
						string Reason;
						stringstream SS;
						string Victim;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}
						CDBInfo *temp = IsInfo( Victim );
						if (temp ) {
							m_PairedInfoUpdateGinfos.push_back( PairedInfoUpdateGinfo( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateGinfo( m_Server, Victim, Reason, User ) ) );
						}						
					}

					if (Command=="pg" && !Payload.empty()) {						
						stringstream SS;
						SS << Payload;
						string tbl[11];
						uint32_t counter=0;
						while( !SS.eof( ) ) {
							string words;
							SS >> words;

							if (SS.fail( ) ) {
								SendChatCommand(User,"error");
								break;
							}
							else {
								//userthatbanned fromuser reason gamename tbl[0] is game id to del it from games
								if (counter > 3) {
									tbl[3] += " " + words;
								}
								else {
									tbl[counter] = words;
								}

								counter++;
							}
						}
						if (counter >= 4 ) {
							//gameid gameowner state gamename
							uint32_t gameid = UTIL_ToUInt32(tbl[0]);
							uint32_t starttime = GetTime();
							uint32_t gamestate = UTIL_ToUInt32(tbl[2]); //UTIL_ToUInt32(tbl[3]);
							m_GHost->AddGame(gameid,User,tbl[1],starttime,10,gamestate,tbl[3]);
						}
						else {
							QueueChatCommand(User + ": Error adding games in gamelist. " + UTIL_ToString(counter) + " variable sent from " + User + ".");
						}
					
					}
					if (Command=="pu" && !Payload.empty()) {
						//gameid username slotnumber
						stringstream SS;
						SS << Payload;
						string tbl[11];
						uint32_t counter=0;
						while( !SS.eof( ) ) {
							string words;
							SS >> words;

							if (SS.fail( ) ) {
								SendChatCommand(User,"error");
								break;
							}
							else {
								//userthatbanned fromuser reason gamename tbl[0] is game id to del it from games
								tbl[counter]=words;
								counter++;
							}
						}
					}
					if (Command =="cban" && !Payload.empty()) {
						string Reason;
						stringstream SS;
						string Victim;
						SS << Payload;
						SS >> Victim;
						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );
							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}
						uint32_t Hoursban = UTIL_ToUInt32( Reason );
						Cban(Victim, Hoursban, User);						
					}

					if (Command=="ugt" &&!Payload.empty()) {
						uint32_t id = UTIL_ToUInt32(Payload);
						m_GHost->UpdateGameTime(id,GetTime(), true);
					}
					
					if (Command=="ugn" &&!Payload.empty()) {
						string Id;
						string variable; 
						stringstream SS;
						SS << Payload;
						SS >> Id;

						if (!SS.eof( ) ) {
							getline( SS, variable );
							string::size_type Start = variable.find_first_not_of( " " );

							if (Start != string::npos) {
								variable = variable.substr(Start);
							}
						}
						uint32_t id = UTIL_ToUInt32(Id);
						m_GHost->UpdateGameName(id,variable);
						m_GHost->UpdateGameTime(id,GetTime(), false);
					}

					if (Command=="ugo" &&!Payload.empty()) {
						string Id;
						string variable; 
						stringstream SS;
						SS << Payload;
						SS >> Id;

						if (!SS.eof( ) ) {
							getline( SS, variable );
							string::size_type Start = variable.find_first_not_of( " " );

							if (Start != string::npos) {
								variable = variable.substr(Start);
							}
						}
						uint32_t id = UTIL_ToUInt32(Id);
						m_GHost->UpdateGameOwner(id,variable);
					}
					
					if (Command=="ugsl" &&!Payload.empty()) {
						stringstream SS;
						SS << Payload;
						string tbl[3];
						uint32_t counter=0;
						while( !SS.eof( ) ) {
							string words;
							SS >> words;

							if (SS.fail( ) ) {
								SendChatCommand(User,"error");
								break;
							}
							else {
								tbl[counter]=words;
								counter++;
							}
						}
						if (counter == 2 ) {
							uint32_t id = UTIL_ToUInt32(tbl[0]);
							uint32_t slots_num = UTIL_ToUInt32(tbl[1]);
							m_GHost->UpdateGameSlots(id, slots_num,"");	
						}
						else if (counter==3) {
							uint32_t id = UTIL_ToUInt32(tbl[0]);
							uint32_t slots_num = UTIL_ToUInt32(tbl[1]);
							m_GHost->UpdateGameSlots(id, slots_num, tbl[2]);
						}
						else {
							QueueChatCommand(User + ": Error update game slots.");
						}
					}

					if (Command=="ugs" &&!Payload.empty()) {
						string Id;
						string variable; 
						stringstream SS;
						SS << Payload;
						SS >> Id;

						if (!SS.eof( ) ) {
							getline( SS, variable );
							string::size_type Start = variable.find_first_not_of( " " );

							if (Start != string::npos) {
								variable = variable.substr(Start);
							}
						}
						uint32_t state = UTIL_ToUInt32(variable);
						uint32_t id = UTIL_ToUInt32(Id);
						m_GHost->UpdateGameState(id,state);
					}

					if (Command=="delgame" && !Payload.empty()) {
						uint32_t id = UTIL_ToUInt32(Payload);
						m_GHost->DelGame(id);
					}

					if (Command=="ag" && !Payload.empty()) {

						CONSOLE_Print("Received ag command with payload: " + Payload);

						stringstream SS;
						SS << Payload;
						string tbl[11]={"","","","","","","","","","",""};
						uint32_t counter=0;
						while( !SS.eof( ) ) {
							string words;
							SS >> words;

							if (SS.fail( ) ) {
								SendChatCommand(User,"error");
								break;
							}
							else {
								//userthatbanned fromuser reason gamename tbl[0] is game id to del it from games
								tbl[counter]=words;
								counter++;
							}
						}
						
						if (m_GHost->m_CountOnly5v5 && counter < 11) {
							return;
						}

						if (counter == 11) {
							AddGameInLadder(tbl[0], tbl[1], tbl[2], tbl[3], tbl[4], tbl[5], tbl[6], tbl[7], tbl[8], tbl[9], tbl[10], counter);
						}
						else if (counter == 9) {
							AddGameInLadder(tbl[0], tbl[1], tbl[2], tbl[3], tbl[4], "", tbl[5], tbl[6], tbl[7], tbl[8], "", counter);
						}
						else if (counter == 7) {
							AddGameInLadder(tbl[0], tbl[1], tbl[2], tbl[3], "", "", tbl[4], tbl[5], tbl[6], "", "", counter);
						}
						else if (counter == 5) {
							AddGameInLadder(tbl[0], tbl[1], tbl[2], "", "", "", tbl[3], tbl[4], "", "", "", counter);
						}
						else if (counter == 3) {
							AddGameInLadder(tbl[0], tbl[1], "", "", "", "", tbl[2], "", "", "", "", counter);
						}
						else {
							QueueChatCommand(User + ": Error adding game. No id or user in game.");
						}
					}

					if (Command=="addnames" && !Payload.empty()) {
						stringstream SS;
						SS << Payload;
						string tbl[3];
						uint32_t counter=0;
						while( !SS.eof( ) ) {
							string words;
							SS >> words;

							if (SS.fail( ) ) {
								SendChatCommand(User,"error");
								break;
							}
							else {
								//userthatbanned fromuser reason gamename tbl[0] is game id to del it from games
								tbl[counter]=words;
								counter++;
							}
						}
						if (counter == 3 ) {
							uint32_t id = UTIL_ToUInt32(tbl[0]);
							uint32_t slot = UTIL_ToUInt32(tbl[2]);
							if (slot >12) {
								CONSOLE_Print("Debug: !addnames > 12",true);
								return;
							}

							m_GHost->AddNames(id, tbl[1], slot);
						}
						else {
							QueueChatCommand(User + ": Error adding names in game.");
						}
					}

					if (Command=="ban" && !Payload.empty( ) ) {
						// extract the victim and the reason
						// e.g. "Varlock leaver after dying" -> victim: "Varlock", reason: "leaver after dying"
						//userthatbanned fromuser reason gamename
						stringstream SS;
						SS << Payload;
						uint32_t counter=0;
						string victim;
						string bancaller;
						string gamename;
						string reason;
						string ip;
						while( !SS.eof( ) ) {
							string words;
							SS >> words;

							if (SS.fail( ) ) {
								SendChatCommand(User,"error");
								break;
							}
							else {
								if (counter == 0) {
									victim = words;
								}
								else if (counter == 1) {
									bancaller = words;
								}
								else if (counter == 2) {
									ip = words;
								}
								else if (counter == 3) {
									gamename = words;
								}
								else {
									reason += words + " ";
								}
								counter++;
							}
						}

						if (IsBannedName(victim)) {
							QueueChatCommand(m_GHost->m_Language->UserIsAlreadyBanned(m_Server, victim), User, Whisper);
						}
						else {
							m_PairedBanAdds.push_back(PairedBanAdd(Whisper ? User : string(), m_GHost->m_DB->ThreadedBanAdd(m_Server, victim, ip, gamename, bancaller, reason)));
						}
					}

					if (Command=="country" && !Payload.empty()) {						
						string Victim;
						string Reason; 
						stringstream SS;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}
						CDBInfo *Temp = IsInfo( Victim );
						if (Temp) {
							if (Temp->GetCountry() == "??") {
								m_PairedInfoUpdateCountrys.push_back(PairedInfoUpdateCountry(Whisper ? User : string(), m_GHost->m_DB->ThreadedInfoUpdateCountry(m_Server, Victim, Reason)));
							}
						}
					}

					if (Command=="checkobs" && !Payload.empty( )) {
						CDBInfo *Temp = IsInfo( Payload );
						if (Temp) {
							SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"addlvl "+Payload+ " "+ UTIL_ToString(Temp->GetLvl())+" "+UTIL_ToString(Temp->GetPrivRank( ))+" "+UTIL_ToString(Temp->GetPrivPoints( ))+" "+UTIL_ToString(Temp->GetPubRank( ))+" "+UTIL_ToString(Temp->GetPubPoints( ))+ " "+UTIL_ToString(Temp->GetChallwins( )) + " "+UTIL_ToString(Temp->GetChallloses( )) );
						}
						else {
							if (m_GHost->m_DefaultAutoLvl == 0) {
								SendChatCommand(Payload,"Welcome to our community hostbot. You can find us in channel "+m_CurrentChannel+".");
								SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"addlvl "+Payload+ " 0 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 0 0" );
							}
							else {
								SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"addlvl "+Payload+ " 1 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 0 0" );
								SendChatCommand(Payload,"Welcome to our community hostbot. You can find us in channel "+m_CurrentChannel+".");
								SendChatCommand(Payload,"You are added in our database and in ladder system. For any help /r !help 1");
								m_PairedInfoAdds.push_back( PairedInfoAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoAdd( m_Server, Payload, m_GHost->m_DefaultAutoLvl, m_Infos.size( )+1, m_Infos.size( )+1, 0, 0, m_GHost->m_CommunityNameLower, "??", 0, 0, "-",1 ) ) );
							}
						}
					}

					if (Command=="checklvl" && !Payload.empty()) {
						string Victim;
						string Reason; 
						stringstream SS;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}
						if (IsHostBan( Victim )) {
							SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"open "+Reason);				
							SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"say "+Victim +" is banned.");	
						}
						else if (IsChannelBan( Victim )) {
							SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"open "+Reason);				
							SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"say "+Victim +" is channel banned.");	
						}
						else {
							CDBInfo *Temp = IsInfo( Victim );
							if (Temp ) {
								if (Temp->GetLvl() > 0 && Temp->GetLvl() < 7) {
									SendChatCommand(User, UTIL_ToString(m_CommandTrigger) + "addlvl " + Victim + " " + UTIL_ToString(Temp->GetLvl()) + " " + UTIL_ToString(Temp->GetPrivRank()) + " " + UTIL_ToString(Temp->GetPrivPoints()) + " " + UTIL_ToString(Temp->GetPubRank()) + " " + UTIL_ToString(Temp->GetPubPoints()) + " " + UTIL_ToString(Temp->GetChallwins()) + " " + UTIL_ToString(Temp->GetChallloses()));
								}
								else {
									//name lvl privrank , privpoints, pubrank pubpoints, cwins closes
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"open "+Reason);				
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"say "+Victim +" is lvl 0 user.");
								}
							}
							else {
								//add user in database like default eid_join
								if (m_GHost->m_DefaultAutoLvl == 0) {
									SendChatCommand(Victim,"Welcome to our community hostbot. You can find us in channel "+m_CurrentChannel+".");
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"addlvl "+Victim+ " 1 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 0 0" );
								}
								else {
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"addlvl "+Victim+ " 1 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 0 0" );
									//SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"say Adding "+Victim +" in database.");
									SendChatCommand(Victim,"Welcome to our community hostbot. You can find us in channel "+m_CurrentChannel+".");
									SendChatCommand(Victim,"You are added in our database and in ladder system. For any help /r !help 1");
									m_PairedInfoAdds.push_back( PairedInfoAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoAdd( m_Server, Victim, m_GHost->m_DefaultAutoLvl, m_Infos.size( )+1, m_Infos.size( )+1, 0, 0, m_GHost->m_CommunityNameLower, "??", 0, 0, "-",1 ) ) );
								}
							}
						}
					}

					if (Command=="checkjoin" && !Payload.empty()) {
						string Victim;
						string Reason; 
						stringstream SS;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}
						
						if (IsHostBan( Victim )) {
							SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"open "+Reason);				
							SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"say "+Victim +" is banned.");	
						}
						else if (IsChannelBan( Victim )) {
							SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"open "+Reason);				
							SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"say "+Victim +" is channel banned.");	
						}
						else {
							CDBInfo *Temp = IsInfo( Victim );
							if (Temp ) {
								if (Temp->GetLvl()>0 && Temp->GetLvl()< 7) {
									//lvl rank points cwins closes
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"addlvl "+Victim+ " "+ UTIL_ToString(Temp->GetLvl())+" "+UTIL_ToString(Temp->GetPrivRank( ))+" "+UTIL_ToString(Temp->GetPrivPoints( ))+ " "+UTIL_ToString(Temp->GetPubRank( ))+" "+UTIL_ToString(Temp->GetPubPoints( ))+  " "+UTIL_ToString(Temp->GetChallwins( )) + " "+UTIL_ToString(Temp->GetChallloses( )) );
								}
								else {
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"open "+Reason);
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"say "+Victim +" have 0 lvl access!");
								}
							}
							else {
								if (m_GHost->m_DefaultAutoLvl == 0) {
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"open "+Reason);
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"say "+Victim +" doesn't have info in db!");
								}
								else {
									SendChatCommand(User,UTIL_ToString(m_CommandTrigger)+"addlvl "+Victim+ " 1 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 "+UTIL_ToString( m_Infos.size( )+1) +" 1000 0 0" );
									SendChatCommand(Victim,"Welcome to our community hostbot. You can find us in channel "+m_CurrentChannel+".");
									SendChatCommand(Victim,"You are added in our database and in ladder system. For any help /r !help 1");
									SendChatCommand(Victim,"If you don't want receive announce type /r "+UTIL_ToString(m_CommandTrigger)+"m off");
									m_PairedInfoAdds.push_back( PairedInfoAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoAdd( m_Server, Victim, m_GHost->m_DefaultAutoLvl, m_Infos.size( )+1, m_Infos.size( )+1, 0 , 0, m_GHost->m_CommunityNameLower, "??", 0, 0, "-",1 ) ) );
								}
							}																					
						}
					}

					if (Command =="fm" && !Payload.empty( )) {
						string tmp = Payload;
						transform( tmp.begin( ), tmp.end( ), tmp.begin( ), (int(*)(int))tolower );
						if (Payload.find("send-slap") != string::npos) {
							return;
						}

						uint32_t difference = GetTime()-m_SpamTime;
						if (m_SpamTime==0 || (difference < 30)) {	
							m_SpamTime=GetTime();
							for (vector<CDBInfo *> ::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
								if ((*i)->GetMessage() > 0 && (*i)->GetLvl() > 0 && (*i)->GetLvl() < 6) {
									SendChatCommand((*i)->GetUser(), Payload);
								}
							}
						}
						else {
							SendChatCommand(User, UTIL_ToString(m_CommandTrigger) + "say You can use fm command every 30 sec. " + UTIL_ToString(30 - difference) + " sec more.");
						}
					}

					if (Command=="ok") {
						//TODO: fixme
						UpdateSite();
						m_GamedThatBotCreate++;
						DelayMs(1000);
						ResetChallenge( true, false, false );
					}
				}
				
				/*********************
				*      lvl 5         *
				**********************/
				if (Accesslvl==5 || IsRootAdmin( User )) {
					
					if (Command == "refreshicons") {
						//TODO: fixme
						CONSOLE_Print("[Icons]: Manual refresh icons from " + User, true);
						m_IconForceRefresh = true;
					}

					if (Command=="delgamelist" || Command=="dgl") {
						m_GHost->m_Games.clear();
						SendChatCommand(User,"Game list cleared.");
					}

					//TODO: fixme
					if (Command=="fix" && m_Infos.size( ) >0 ) {
						m_CallableBanList = m_GHost->m_DB->ThreadedBanList( m_Server );
						m_CallableInfoList = m_GHost->m_DB->ThreadedInfoList( m_Server );
						m_CallableWarnList = m_GHost->m_DB->ThreadedWarnList( m_Server );
						SendChatCommand(User,"Tring connect 2 db 2 Refresh vectors.");
						DelayMs(4000);
						for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++ ) {
							if ((*i)->GetLvl()>0) {
								SendChatCommand("/unban "+(*i)->GetUser( ));							
							}
							DelayMs(30);
						}	
						m_LastGoPressed = GetTime();
					}
					
					if (Command=="news" ) {
						m_PrintNews = !m_PrintNews;
						if (m_PrintNews) {
							SendChatCommand(User, "News enabled.");
						}
						else {
							SendChatCommand(User, "News diabled.");
						}
					}

					if (Command=="icontimer" || Command=="it" ) {
							SendChatCommand(User,"Time untill auto update icons ("+UTIL_HMSToString(m_IconRefreshTimer + m_GHost->m_IconTimer - GetTime( ))+")");
					}

					if (Command=="delgame" && !Payload.empty()) {
						uint32_t id = UTIL_ToUInt32(Payload);
						m_GHost->DelGame(id);
						SendChatCommand(User,"Deleting game with id number "+Payload);
					}
					
					if (m_GHost->commands[86])
					if (Command=="annmail" && !Payload.empty( ))	{
						if (m_GHost->m_MailEnabled) {
							SendChatCommand(User,"Sending announce mail to all users.");
							m_PairedMailAdds.push_back( PairedMailAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedMailAdd( GetMailId( )+1, m_Server, User, "-", Payload ) ) );
						}
						else {
							SendChatCommand(User, "Mail function is disabled.");
						}
					}


					if (Command=="clricons") {
						ifstream in3;
						in3.open( m_GHost->gIconsFile.c_str( ) );
						if (in3.fail()) {
							CONSOLE_Print("[Icon]: Fail load icons file", true);
						}
						else {	
							CONSOLE_Print("[Icon]: Loading icon file from disk to delete icons from bnet.", true);
							uint32_t Count = 0;
							string Line;				
							while( !in3.eof( ) && Count < 500 ) {
								getline( in3, Line );
								stringstream SS;
								SS << Line;
								string tbl[4];
								uint32_t counter=0;
								while( !SS.eof( ) ) {
									string words;
									SS >> words;
									if (SS.fail( ) ) {
										CONSOLE_Print("[Icon]: system error #6", true);
										break;
									}
									else {
										tbl[counter]=words;
										counter++;
									}
								}

								if (!Line.empty( ) && counter == 4 ) {
									SendChatCommand("/seticon del "+tbl[0]+" "+tbl[2]);
									Count++;
								}
								else {
									CONSOLE_Print("[Icon]: system error #7", true);
								}

								if (in3.eof()) {
									break;
								}
							}
							in3.close();
						}

						CONSOLE_Print("[Icon]: Clearing file", true);
						ifstream a_file ( m_GHost->gIconsFile.c_str( ) );
						if (!a_file.is_open()) {
							CONSOLE_Print("[Icon]: system error #3. Can't open icon file.", true);
						}
						else {
							ofstream a_file ( m_GHost->gIconsFile.c_str( ) , ios::trunc );
							CONSOLE_Print("[Icon]: File cleared.", true);
						}
						SendChatCommand(User,"Removing icons from bnet and clearing log file.");
					}

					// Enable or disable command, !com 75 on
					if (Command=="com" && !Payload.empty( ) ) {
						string Victim;
						string Reason; 
						stringstream SS;
						SS << Payload;
						SS >> Victim;
					
						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}
						uint32_t id = UTIL_ToUInt32( Victim );
						if (id < 100 && (Reason == "on" || Reason=="off" || Reason=="print" )) {
							if (Reason=="on") {
								m_GHost->commands[id] = true;
								SendChatCommand(User,"Command enabled.");
							}
							else if (Reason =="off") {
								m_GHost->commands[id] = false;
								SendChatCommand(User,"Command disabled.");
							}
							else {
								if (m_GHost->commands[id]) {
									SendChatCommand(User, "This command is enabled.");
								}
								else {
									SendChatCommand(User, "This command is disabled.");
								}
							}
						}
						else {
							SendChatCommand(User, "Bad input.");
						}
					}

					if (m_GHost->commands[73])
					if (Command == "maxgames" || Command == "mg") {
						SendChatCommand(User, UTIL_ToString(m_GamedThatBotCreate) + " created");
					}
					
					if (Command=="cal" && Payload.empty()) {
						SendChatCommand(User,"Ranking users in db.");
						CONSOLE_Print( "Ranking users in db.", true);
						m_PairedInfoPrivCalculates.push_back( PairedInfoPrivCalculate( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoPrivCalculate( m_Server) ) );
						m_PairedInfoPubCalculates.push_back( PairedInfoPubCalculate( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoPubCalculate( m_Server) ) );
						//TODO: fixme
						DelayMs(2000);//delay more 2 just in case
						m_CallableInfoList = m_GHost->m_DB->ThreadedInfoList( m_Server );						
					}				

					if (m_GHost->commands[71])
					if (Command=="refresh") {
						//infos warns bans all icons
						if (Payload=="infos") {
							SendChatCommand(User,"Refreshing infos table.");
							m_CallableInfoList = m_GHost->m_DB->ThreadedInfoList( m_Server );
						}
						else if (Payload=="mails") {
							SendChatCommand(User,"Refreshing mail table.");
							m_CallableMailList = m_GHost->m_DB->ThreadedMailList( m_Server );
						}
						else if (Payload=="warns") {
							SendChatCommand(User,"Refreshing warns table.");
							m_CallableWarnList = m_GHost->m_DB->ThreadedWarnList( m_Server );
						}
						else if (Payload=="bans") {
							SendChatCommand(User,"Refreshing bans table.");
							m_CallableBanList = m_GHost->m_DB->ThreadedBanList( m_Server );
						}
						else if (Payload=="all") {
							SendChatCommand(User, "Refreshing all tables, config and php site.");
							m_CallableBanList = m_GHost->m_DB->ThreadedBanList( m_Server );
							m_CallableMailList = m_GHost->m_DB->ThreadedMailList( m_Server );
							m_CallableInfoList = m_GHost->m_DB->ThreadedInfoList( m_Server );
							m_CallableWarnList = m_GHost->m_DB->ThreadedWarnList( m_Server );
							m_GHost->ReloadConfigs();
							UpdateSite();
						}
						else if (Payload=="cfg") {
							m_GHost->ReloadConfigs();
							SendChatCommand(User,"Updating cfg info.");
						}
						else if (Payload=="icons") {
							if (m_GHost->m_IconSupport==0) {
								SendChatCommand(User,"Icon support is disabled.");
								SendChatCommand(User,"Check ghost.cfg file for bot_iconsystem setting.");
							}
							else {
								m_IconForceRefresh = true;
								SendChatCommand(User,"Refreshing icon system.");
							}
						}
						else if (Payload=="php") {
							if (m_GHost->m_SiteUpdateSupported) {
								SendChatCommand(User,"Refreshing php file.");
								UpdateSite();
							}
							else {
								SendChatCommand(User, "Command is disabled. If you want to enable it add in cfg file this line bot_php = 1.");
							}
						}
					}	
					
					if (m_GHost->commands[70])
					if (Command=="rootonly" && Payload.empty()) {
						m_RootOnly=!m_RootOnly;
						if (m_RootOnly) {
							QueueChatCommand("Control commands locked.");
						}
						else {
							QueueChatCommand("Control commands unlocked.");
						}
					}

					if (m_GHost->commands[69])
					if (Command=="erasewarn" && !Payload.empty()) {
						CDBWarn *temp =IsWarn(Payload);
						if (temp) {
							m_PairedWarnRemoves.push_back( PairedWarnRemove( Whisper ? User : string( ), m_GHost->m_DB->ThreadedWarnRemove( m_Server, Payload ) ) );
						}
						else {
							SendChatCommand(User, Payload + " doesn't have any warns.");
						}
					}

					if (m_GHost->commands[68])
					if (Command=="addhostbot" && !Payload.empty() && IsRootAdmin( User )) {
						CDBInfo *temp = IsInfo(Payload);
						if (temp) {
							if (temp->GetLvl()==1) {
								QueueChatCommand(User +" change "+Payload +" 's lvl to 6.");
								m_PairedInfoUpdateLvls.push_back( PairedInfoUpdateLvl( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateLvl( m_Server, Payload,6,User ) ) );
							}
							else {
								QueueChatCommand("Only from lvl 1 you can change to hostbot account.");
							}
						}						
					}
					
					if (m_GHost->commands[68])
					if (Command=="delhostbot" && !Payload.empty() && IsRootAdmin( User )) {
						CDBInfo *temp = IsInfo(Payload);
						if (temp) {
							if (temp->GetLvl()==6) {
								m_PairedInfoUpdateLvls.push_back( PairedInfoUpdateLvl( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateLvl( m_Server, Payload,1,User ) ) );
								QueueChatCommand("Hostbot account changed to User.");
							}
							else {
								QueueChatCommand("This isn't hostbot");
							}
						}
						
					}
	
					if (m_GHost->commands[66])
					if (Command == "tempop" && !Payload.empty()) {
						SendChatCommand("/tmpop " + Payload);
					}

					if (m_GHost->commands[65])
					if (Command=="chat") {
						m_GHost->m_AllowTopaz=!m_GHost->m_AllowTopaz;
						if (m_GHost->m_AllowTopaz) {
							SendChatCommand(User, "Chat clients allowed to join channel.");
						}
						else {
							SendChatCommand(User, "Chat clients autokicked.");
						}
					}

					if (Command=="mepro" && IsRootAdmin(User)) {
						m_PairedInfoUpdateLvls.push_back( PairedInfoUpdateLvl( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateLvl( m_Server, User,5,User ) ) );
						SendChatCommand("/unban "+User);
						SendChatCommand(User,"Making you admin");
					}

					if (m_GHost->commands[64])
					if ((Command == "delinfo") && !Payload.empty() && IsRootAdmin(User)) {
						m_PairedInfoRemoves.push_back(PairedInfoRemove(Whisper ? User : string(), m_GHost->m_DB->ThreadedInfoRemove(m_Server, Payload)));
					}

					if (m_GHost->commands[63])
					if (Command=="say" && !Payload.empty() && IsRootAdmin(User) ) {
						string temp = UTIL_ToLower(Payload);
						QueueChatCommand(Payload);
					}
					
					if (m_GHost->commands[61])
					if (Command == "channel" && !Payload.empty()) {
						SendChatCommand("/join " + Payload);
					}

					if (m_GHost->commands[60])
                    if ((Command == "exit" || Command == "quit") && IsRootAdmin(User)) {
						if (Payload=="all") 	{
							SendChatCommand(User,"Closing hostbots.");
							for (vector<CDBInfo *>::iterator j = m_Infos.begin(); j != m_Infos.end(); j++ ) {
								if ((*j)->GetLvl() == 6) {
									SendChatCommand((*j)->GetUser(), UTIL_ToString(m_CommandTrigger) + "exit force");
								}
							}
						}
						SendChatCommand(User,"Closing guard bot");
						m_Exiting = true;
					}

					if (m_GHost->commands[59])
					if (Command=="slap") {
						for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
							if ((*i)->GetTopaz() == true) {
								SendChatCommand((*i)->GetUser(), "~playsound slap.wav");
							}
							else {
								SendChatCommand((*i)->GetUser(), "send-slap");
							}
						}
					}					
				}

				/*********************
				*      lvl 4         *
				*********************/
                if ((Accesslvl==4 || Accesslvl==5) && !m_RootOnly) {
					
					if (Command=="see" && !Payload.empty( )) {
						uint32_t id = UTIL_ToUInt32(Payload);
						if (m_GHost->IsGameId(id)) {
							SendChatCommand(User,"Trying to add you in Chat listener list...");
						}
						else {
							SendChatCommand(User, "No game with id (" + Payload + ").");
						}
					}

					if (m_GHost->commands[90])
					if (Command == "fmslap") {
						MassMessage(User, "(send-slap): " + Payload, false, false);
					}

					if (m_GHost->commands[18])
					if (Command=="clrobs" && Payload.empty()) {						
						m_meplay[10][0]="";
						m_meplay[11][0]="";
						SendChatCommand(User,"Obs slots cleard");
					}
						
					if (Command=="delrmk") {
						m_RMKstate=0;
						SendChatCommand(User,"Rmk pool deleted.");
					}
						
					if (m_GHost->commands[84])
					if (Command=="clrslots" && Payload.empty() && !m_Challenge) {
						for (int i=0; i<12; i++) {
							m_meplay[i][0] = "";
							m_meplay[i][1] = "";
						}
						SendChatCommand(User,"Slots cleared");
					}

					if (m_GHost->commands[82])
					if (Command=="start" && !Payload.empty() && Accesslvl<7) {
						if (IsHostbot(Payload)) {
							SendChatCommand(Payload,UTIL_ToString(m_CommandTrigger)+"start");
							SendChatCommand(User,"Starting game.");
						}
						else {
							SendChatCommand(User, "That account isn't hostbot.");
						}
					}

					if (m_GHost->commands[81])
					if (Command=="unhostall" && Payload.empty()) {
						SendChatCommand(User,"Unhosting all games.");
						UnhostAllGames();
						UnhostHostbots();
					}

					if (m_GHost->commands[80])
					if (Command=="autospam" && !Payload.empty()) {
						uint32_t secs = UTIL_ToUInt32(Payload);
						if (secs==0) {
							SendChatCommand(User,"Autospam disabled.");
							m_SlotsSpamEnable = false;
						}
						else if (secs>9 && secs<61) {
							SendChatCommand(User,"Autospam enabled.");
							m_SlotsLastSpam = GetTime();
							m_SlotsSpamTimer = secs;
							m_SlotsSpamEnable= true;
						}
						else {
							SendChatCommand(User, "Type " + UTIL_ToString(m_CommandTrigger) + "autospam <seconds> (from 10 to 60)");
						}
					}

					if (m_GHost->commands[76])
					if (Command=="privgoby" && !Payload.empty() && !IsBannedNameUser) {
						if (!GetHostbotFromOwner(Payload).empty( )) {
							SendChatCommand(User,Payload+" have hosted game already.");
							return;
						}
						if (m_GameId < 999) {
							m_GameId++;
						}
						else {
							m_GameId = 0;
						}

						if (m_Challenge) {
							SendChatCommand(User,"Challenge started.");
						}
						else {
							string Hostbot = GetAvailableHostbot();
							if (Hostbot.empty( )) {
								SendChatCommand(User,"There isn't any hostbot available. Try again later.");
							}							
							else {
								//!addinfo user slot lvl  !sendc !sendcend  sendmend  m_holdslots
								for (int j = 0; j<12; j++) {
									if (!m_meplay[j][0].empty()) {
										SendChatCommand(Hostbot, UTIL_ToString(m_CommandTrigger) + "hold " + m_meplay[j][0] + " " + UTIL_ToString(j + 1));
									}
								}

								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"hold "+UTIL_ToString( m_GameId )+" 13");//sent bot the id of game to hold in 13 slot of holdlist in table
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"sendend");
								m_GHost->AddGame( m_GameId, Hostbot, Payload, GetTime( ), 10, 0, m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"privby "+Payload+" "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));
								QueueChatCommand("Host priv game with "+Hostbot+" bot with name: "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate)+" by "+Payload);
								UpdateHostbot(Hostbot,2,GetHostbotGames( Hostbot )+1,0);
							}
						}
					}

					if (m_GHost->commands[58])
					if (Command == "chacc" && !Payload.empty()) {
						uint32_t ll=Info->GetLvl();
						uint32_t victimlvl=1;
						string OldName;
						string NewName; 
						stringstream SS;
						SS << Payload;
						SS >> OldName;

						if (!SS.eof( ) ) {
							getline( SS, NewName );
							string::size_type Start = NewName.find_first_not_of( " " );

							if (Start != string::npos) {
								NewName = NewName.substr(Start);
							}
						}						
						
						if (!OldName.empty() && !NewName.empty()) {
							CDBInfo *temp = IsInfo(OldName);
							if (temp) {
								victimlvl = temp->GetLvl();
							}

							if (ll>victimlvl) {
								m_PairedInfoUpdateLvls.push_back( PairedInfoUpdateLvl( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateLvl( m_Server, OldName, 0,"Account change." ) ) );
								if (m_GHost->m_IsOperator) {
									SendChatCommand("/ban " + OldName + " Account change ");
								}
								else {
									SendChatCommand("/kick " + OldName + " Account change ");
								}
								m_PairedInfoUpdateLvls.push_back( PairedInfoUpdateLvl( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateLvl( m_Server, NewName, victimlvl, User+"*" ) ) );
								if (m_GHost->m_IsOperator) {
									SendChatCommand("/unban " + NewName);
								}
							}
							else {
								SendChatCommand(User, "Can t change your own account or other with same lvl as yours.");
							}
						}
						else {
							SendChatCommand(User, "Type !chacc <oldname> <newname>.");
						}
					}

					if (m_GHost->commands[57])
					if (Command=="icons") {
						SendChatCommand(User," User Name  / Icon Name  /  Reason");
						for (vector<CDBIcons *> ::iterator i = m_IconList.begin(); i != m_IconList.end(); i++) {
							SendChatCommand(User, (*i)->GetUser() + " " + (*i)->GetIcon() + " " + (*i)->GetReason());
						}
					}

					if (m_GHost->commands[56])
					if (Command == "flames" && Payload.empty()) {
						for (vector<string> ::iterator i = m_GHost->m_FlameList.begin(); i != m_GHost->m_FlameList.end(); i++) {
							SendChatCommand(User, *i);
						}
					}
					

					if (m_GHost->commands[56])
					if (Command=="addflame" && !Payload.empty()) {						
						fstream filestr;
						filestr.open (m_GHost->gFlamesFile.c_str( ),fstream::in |fstream::out |fstream::app);
						filestr<<Payload<<endl;
						filestr.close();

						m_GHost->m_FlameList.push_back(Payload);
						SendChatCommand(User,"Flame added.");
					}

					if (m_GHost->commands[56])
					if (Command=="delflames" && Payload.empty()) {
						SendChatCommand(User,"Deleting flames.");
						ifstream a_file ( m_GHost->gFlamesFile.c_str( ) );
						if (!a_file.is_open() ) {
							// The file could not be opened
							QueueChatCommand("Icon file can't open!!! Error.");
						}
						else {
							// Safely use the file stream
							ofstream a_file ( m_GHost->gFlamesFile.c_str( ) , ios::trunc );
						}
						m_GHost->m_FlameList.clear();
					}

					if (m_GHost->commands[55])
					if (Command == "showm" && Payload.empty()) {
						PrintMuted(User);
					}

					if (m_GHost->commands[55])
					if (Command=="mute" && !Payload.empty()) {
						uint32_t ll=Info->GetLvl();
						string Victim;
						string Reason;
						stringstream SS;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}

						uint32_t Matches=0;
						string TestName;
						string LastMatch;
						for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
							TestName = (*i)->GetUser();
							transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

							if (TestName.find( Victim ) != string::npos ) {
								Matches++;
								LastMatch = (*i)->GetUser();
							}
						}

						if (Matches == 0 ) {
							QueueChatCommand("No user with this name in channel.");
						}
						else if (Matches == 1 ) {							
							CDBInfo *temp = IsInfo( LastMatch );
							if (temp) {
								if (ll>temp->GetLvl()) {
									if (IsMuted(LastMatch)) {
										SendChatCommand(User,LastMatch+" is already muted!");
									}
									else {
										QueueChatCommand(User+" mute "+LastMatch);
										m_MuteUsers.push_back(LastMatch);
									}
								}
								else {
									QueueChatCommand("Can 't mute user with same lvl.");
								}
							}							
						}
						else {
							QueueChatCommand("More than one user are in channel with this name.");
						}
					}

					if (m_GHost->commands[55])
					if (Command=="unmute" && !Payload.empty()) {

						transform( Payload.begin( ), Payload.end( ), Payload.begin( ), (int(*)(int))tolower );
						uint32_t Matches=0;
						string TestName;
						string LastMatch;
						for (vector<string>::iterator i = m_MuteUsers.begin(); i != m_MuteUsers.end();i++) {
							TestName = *i;
							transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

							if (TestName.find( Payload ) != string::npos ) {
								Matches++;
								LastMatch = *i;
							}
						}

						if (Matches == 0 ) {
							QueueChatCommand("No user with this name in channel.");
						}
						else if (Matches == 1 ) {	
						
							for (vector<string>::iterator i = m_MuteUsers.begin(); i != m_MuteUsers.end();) {
								if (*i == LastMatch ) {
									QueueChatCommand(User+" unmute "+LastMatch);
									i = m_MuteUsers.erase( i );
								}
								else {
									i++;
								}
							}							
						}
						else {
							QueueChatCommand("More than one user in channel with this name.");
						}
					}

					
					if (m_GHost->commands[54])
					if (Command=="wtg" && !Payload.empty()) {
						MassMessage(User,Payload,true,true);
					}

					if (m_GHost->commands[53])
					if (Command=="delwarn" && !Payload.empty()) {
						CDBWarn *temp =IsWarn(Payload);
						if (temp) {
							uint32_t totalwarn= temp->GetTotalwarn();
							SendChatCommand(User,Payload+" Deleting warnings from "+Payload+".");
							m_PairedWarnUpdateAdds.push_back( PairedWarnUpdateAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedWarnUpdateAdd( m_Server, Payload,0,"",totalwarn,User ) ) );
						}
						else {
							SendChatCommand(User, Payload + " doesn't have any warns.");
						}
					}

					if (m_GHost->commands[52])
					if (Command=="warns") {
						m_PairedWarnCounts.push_back( PairedWarnCount( Whisper ? User : string( ), m_GHost->m_DB->ThreadedWarnCount( m_Server ) ) );
					}

					if (m_GHost->commands[51])
					if (Command=="kickall") {
						for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
							if ((*i)->GetUser()!=User &&(*i)->GetLvl()<4) {
								SendChatCommand("/kick "+(*i)->GetUser()+" admin kick all from channel");								
							}
						}
					}
	
					if (m_GHost->commands[50])
                    if ((Command == "mod" || Command == "moderate") && Payload.empty()) {
						ChannelModerate();
					}
					
					if (m_GHost->commands[49])
					if ((Command =="voice" || Command=="v") && !Payload.empty()) {
						transform( Payload.begin( ), Payload.end( ), Payload.begin( ), (int(*)(int))tolower );
						uint32_t Matches=0;
						string TestName;
						string LastMatch;
						for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) 	{
							TestName = (*i)->GetUser();
							transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

							if (TestName.find( Payload ) != string::npos ) {
								Matches++;
								LastMatch = (*i)->GetUser();
							}
						}

						if (Matches == 0 ) {
							QueueChatCommand("No user with this name in channel.");
						}
						else if (Matches == 1 ) {							
							SendChatCommand("/voice "+LastMatch);	
						}
						else {
							QueueChatCommand("More than one user are in channel with this name.");
						}
					}
						
					if (m_GHost->commands[49])
                    if ((Command =="devoice" || Command== "dev") && !Payload.empty()) {
						transform( Payload.begin( ), Payload.end( ), Payload.begin( ), (int(*)(int))tolower );
						uint32_t Matches=0;
						string TestName;
						string LastMatch;
						for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
							TestName = (*i)->GetUser();
							transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

							if (TestName.find( Payload ) != string::npos ) {
								Matches++;
								LastMatch = (*i)->GetUser();
							}
						}

						if (Matches == 0 ) {
							QueueChatCommand("No user with this name in channel.");
						}
						else if (Matches == 1 ) {
							SendChatCommand("/devoice "+LastMatch);
						}
						else {
							QueueChatCommand("More than one user in channel with this name.");
						}
					}

					if (m_GHost->commands[48])
					if (Command =="sayingames" && !Payload.empty()) {
						SendChatCommand(User,"Announcing your message at all hostbots.");
						for (vector<CDBChannel *>::iterator s = m_Users.begin(); s != m_Users.end(); s++ ) {	
							if ((*s)->GetLvl() == 6) {
								SendChatCommand((*s)->GetUser(), UTIL_ToString(m_CommandTrigger) + "saygames Admin user " + User + ": " + Payload);
							}
						}
					}

					if (m_GHost->commands[47])
					if (Command == "topic" ||Command=="t") {
						QueueChatCommand("Topic command");
						if (Payload.empty()) {
							QueueChatCommand("/topic " + m_CurrentChannel + " \"" + "\"");
						}
						else {
							QueueChatCommand("/topic " + m_CurrentChannel + " \"" + User + ": " + Payload + "\"");
						}
					}

					if (m_GHost->commands[46])
					if (Command == "announce" || Command=="an") {
						if (Payload.empty( ) || Payload == "off" ) {
							QueueChatCommand( "Announce off." );
							SAnnounce( 0, string( ) );
						}
						else {
							// extract the interval and the message
							// e.g. "30 hello everyone" -> interval: "30", message: "hello everyone"

							uint32_t Interval;
							string UpdateMessage;
							stringstream SS;
							SS << Payload;
							SS >> Interval;

							if (SS.fail() || Interval == 0) {
								CONSOLE_Print("[GAME] bad input #1 to announce command", true);
							}
							else {
								if (SS.eof()) {
									CONSOLE_Print("[GAME] missing input #2 to announce command", true);
								}
								else {
									getline( SS, UpdateMessage );
									string::size_type Start = UpdateMessage.find_first_not_of( " " );
	
									if (Start != string::npos) {
										UpdateMessage = UpdateMessage.substr(Start);
									}
									if (Interval < 5) {
										QueueChatCommand("Type more than 5 sec for spam problems");
									}
									else {
										QueueChatCommand( "Announce on" );
										SAnnounce( Interval, User+": "+ UpdateMessage );
									}
								}
							}
						}
					}

					if (m_GHost->commands[45])
					if (Command=="hostbots" && Payload.empty()) {
						SendChatCommand(User," ---------------------------------------------------------------------");
						SendChatCommand(User,"                 Hostbots in database");
						SendChatCommand(User," ---------------------------------------------------------------------");
						string temp="";
						int counter=0;
						int lvlUsers=0;
						string UsersNames="";
						int lines=0;

						for (vector<CDBInfo *>::iterator j = m_Infos.begin(); j != m_Infos.end(); j++ ) {
							if ((*j)->GetLvl()==6) {
								lvlUsers++;
								UsersNames+=(*j)->GetUser( )+"    ";									
								counter++;
							}
							if (counter==4) {
								SendChatCommand(User,UsersNames);
								UsersNames="";
								counter=0;
								lines++;
							}
						}	
						SendChatCommand(User,UsersNames);	
						SendChatCommand(User,"---------------------------------------------------------------------");
						QueueChatCommand("There are " +UTIL_ToString(lvlUsers)+" hostbots in databse.");
					}

					if (m_GHost->commands[42]) 
					if ((Command == "Messageinfo" || Command=="minfo") && Payload.empty()) {	
						SendChatCommand(User,"---------------------------------------------------------------------");
						SendChatCommand(User,"                 USERS WHO DENY MASSMessage");
						SendChatCommand(User," ---------------------------------------------------------------------");
						string temp="";
						int counter=0;
						int lvlUsers=0;
						string UsersNames="";
						int lines=0;

						for (vector<CDBInfo *>::iterator j = m_Infos.begin(); j != m_Infos.end(); j++ ) {
							if ((*j)->GetMessage()==0) {
								lvlUsers++;
								UsersNames+=(*j)->GetUser( )+"    ";									
								counter++;
							}
							if (counter==4) {
								SendChatCommand(User,UsersNames);
								UsersNames="";
								counter=0;
								lines++;
							}
						}	
						SendChatCommand(User,UsersNames);	
						SendChatCommand(User,"---------------------------------------------------------------------");
						SendChatCommand(User,"There are " +UTIL_ToString(lvlUsers)+" users who deny mass Message.");
					}

					if (m_GHost->commands[41])
					if (Command=="ginfo") {
						uint32_t ginfo=0;
						for (vector<CDBInfo *>::iterator j = m_Infos.begin(); j != m_Infos.end(); j++ ) {
							if ((*j)->GetGinfo()!="-") {
								ginfo++;
								SendChatCommand(User,(*j)->GetUser()+": "+(*j)->GetGinfo());
							}
						}
						QueueChatCommand("There are "+UTIL_ToString(ginfo)+" users with general info in database.");
					}

					if (m_GHost->commands[40])
					if (Command == "ttopic" || Command=="tt") {
						if (Payload.empty()) {
							m_TempTopic = "";
							QueueChatCommand("Temp topic deleted.");
						}
						else {
							m_TempTopic = "Temp topic ("+User+"): "+Payload;
							QueueChatCommand("Temp topic changed.");
						}
					}
						
					if (m_GHost->commands[39])
					if (Command == "minslots" && !Payload.empty() ) {
						if (Payload == "1") {
							m_ReservesNeeded = 1;
						}
						else if (Payload == "2") {
							m_ReservesNeeded = 2;
						}
						else if (Payload == "3") {
							m_ReservesNeeded = 3;
						}
						else if (Payload == "4") {
							m_ReservesNeeded = 4;
						}
						else if (Payload == "5") {
							m_ReservesNeeded = 5;
						}
						else if (Payload == "6") {
							m_ReservesNeeded = 6;
						}
						else if (Payload == "7") {
							m_ReservesNeeded = 7;
						}
						else if (Payload == "8") {
							m_ReservesNeeded = 8;
						}
						else if (Payload == "9") {
							m_ReservesNeeded = 9;
						}
						else if (Payload == "10") {
							m_ReservesNeeded = 10;
						}
						else {
							QueueChatCommand("Wrong input");
						}

						SendChatCommand(User,UTIL_ToString(m_ReservesNeeded) + " slots needed to create pub game");
					
						if (m_meplayppl >= m_ReservesNeeded) {
							m_FreeReady = true;
						}
						else {
							m_FreeReady = false;
						}

					}

					if (m_GHost->commands[38])
					if (Command=="hold" && !Payload.empty() && !m_Challenge) {
						string Victim;
						string Reason;
						stringstream SS;
						SS << Payload;
						SS >> Victim;
						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}

						if (!Victim.empty() && !Reason.empty() && Reason.length()<3) { 
							if (UTIL_ToUInt32(Reason)<13 &&UTIL_ToUInt32(Reason)>0) {
								
								m_holdslots=true;							
								m_meplay[UTIL_ToUInt32(Reason)-1][0]=Victim;
								QueueChatCommand("Slot held from "+User+" for "+Victim+" in slot "+Reason,User,Whisper);
							}
							else {
								QueueChatCommand("You can put 1 to 10 for hold a slot.", User, Whisper);
							}
						}
						else {
							QueueChatCommand("Input error. !hold user number (numb = 1 - 10).", User, Whisper);
						}
					}

					if (m_GHost->commands[37])
					if (Command=="idle" || Command=="afk" || Command=="finger") {
						if (Payload.empty()) {
							uint32_t users=0;
							
							uint32_t temptime = GetTime();
							for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {								
								if ((*i)->GetSec( )+600<temptime && (*i)->GetLvl()<4) {
									SendChatCommand("/kick "+(*i)->GetUser()+" you are idle in channel for "+UTIL_ToDayTimSec(GetTime()-(*i)->GetSec()));
									users++;
								}
							}
							if (users == 0) {
								QueueChatCommand("No afker in channel", User, Whisper);
							}
							else if (users == 1) {
								QueueChatCommand("Afker kicked from channel", User, Whisper);
							}
							else {
								QueueChatCommand("Afkers kicked from channel", User, Whisper);
							}
						}
						else if (!Payload.empty()) {
							uint32_t Matches=0;
							string TestName;
							string LastMatch;
							for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
								TestName = (*i)->GetUser();
								transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

								if (TestName.find( Payload ) != string::npos ) {
									Matches++;
									LastMatch = (*i)->GetUser();
								}
							}

							if (Matches == 0) {
								QueueChatCommand("No user with this name in channel.", User, Whisper);
							}
							else if (Matches == 1 ) {
								for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
									if (LastMatch == (*i)->GetUser()) {
										QueueChatCommand(LastMatch + " is idle for " + UTIL_ToDayTimSec(GetTime() - (*i)->GetSec()));
									}
								}
							}
							else {
								QueueChatCommand("More than 1 user with that name.", User, Whisper);
							}
						}
					}

					if (m_GHost->commands[36])
					if (( Command == "addban" || Command == "ban" ) && !Payload.empty( ) ) {
						// extract the victim and the reason
						// e.g. "Varlock leaver after dying" -> victim: "Varlock", reason: "leaver after dying"

						string Victim;
						string Reason;
						stringstream SS;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}

						if (IsBannedName(Victim)) {
							QueueChatCommand(m_GHost->m_Language->UserIsAlreadyBanned(m_Server, Victim), User, Whisper);
						}
						else {
							m_PairedBanAdds.push_back(PairedBanAdd(Whisper ? User : string(), m_GHost->m_DB->ThreadedBanAdd(m_Server, Victim, string(), string(), User, Reason)));
						}
					}

					if (m_GHost->commands[35])
					if (Command=="channel" && Payload.empty()) {
						string infos;
						uint32_t sec=0;
						uint32_t users=0;
						uint32_t bots=0;
						uint32_t hostbots=0;
						uint32_t topaz=0;
						SendChatCommand(User,"Days-Hours:Minutes:Seconds");						
						for (vector<CDBChannel *>::iterator s = m_Users.begin(); s != m_Users.end(); s++ ) {	
							infos="";
							sec = GetTime( )-(*s)->GetSec();
							infos=(*s)->GetUser()+" idle ("+UTIL_ToDayTimSec(sec)+") lvl ("+UTIL_ToString((*s)->GetLvl())+")";
							if ((*s)->GetBanned()) {
								infos += " BANNED!!!";
							}
							if (sec > 600 && (*s)->GetLvl() < 6) {
								infos += " afker!!!";
							}
							if ((*s)->GetLvl() == 6) {
								infos += " !!HOSTBOT!!";
							}
							if ((*s)->GetTopaz()) {
								infos += " (Chat client)";
							}
							SendChatCommand(User,infos);
							
							if ((*s)->GetLvl() == 6) {
								hostbots++;
							}
							else if ((*s)->GetLvl()<6) {
								if ((*s)->GetTopaz()) {
									topaz++;
								}
								users++;
							}
							else {
								bots++;
							}
						}
						SendChatCommand(User,"There are "+UTIL_ToString(users)+" users ("+UTIL_ToString(topaz)+" with chat prog) "+ UTIL_ToString(hostbots)+" hostbots and "+UTIL_ToString(bots) +" bots in channel.");
					}


					if (m_GHost->commands[34])
					if (Command=="lvl") {
						if (Payload.empty()) {
							CDBInfo *Info = IsInfo( User );
							if (Info) {
								SendChatCommand(User, User + " (" + UTIL_ToString(Info->GetLvl()) + ") lvl. Access given by (" + Info->GetAdmin() + ")");
							}

						}
						else if (!Payload.empty()&& Payload.length()>1) {
							CDBInfo *Info = IsInfo(Payload );
							if (Info) {
								QueueChatCommand(Payload + " (" + UTIL_ToString(Info->GetLvl()) + ") lvl. Access given by (" + Info->GetAdmin() + ")", User, Whisper);
							}
							else {
								QueueChatCommand("No info for user " + Payload, User, Whisper);
							}

						}
						else if (Payload=="0" || Payload=="1" || Payload=="2" || Payload=="3" || Payload=="4"|| Payload=="5"|| Payload=="6"|| Payload=="7") {	
							
							SendChatCommand(User," ---------------------------------------------------------------------");
							SendChatCommand(User,"                 USERS WITH LVL "+Payload+ " ACCESS");
							SendChatCommand(User," ---------------------------------------------------------------------");
							string temp="";
							int counter=0;
							int lvlUsers=0;
							string UsersNames="";
							int lines=0;

							for (vector<CDBInfo *>::iterator j = m_Infos.begin(); j != m_Infos.end(); j++ ) {
								if ((*j)->GetLvl()==UTIL_ToUInt32(Payload))	{
									lvlUsers++;
									UsersNames+=(*j)->GetUser( )+"    ";									
									counter++;
								}
								if (counter==4) {
									SendChatCommand(User,UsersNames);
									UsersNames="";
									counter=0;
									lines++;
								}
							}	
							SendChatCommand(User,UsersNames);	
							SendChatCommand(User," ---------------------------------------------------------------------");
							SendChatCommand(User,"There are " +UTIL_ToString(lvlUsers)+" users with lvl "+ Payload);
						}
					}

					if (m_GHost->commands[33])
					if (Command=="clvl" && !Payload.empty()) {
						uint32_t ll=Info->GetLvl();
						uint32_t lvl=1;
						uint32_t victimlvl=1;
						string Victim;
						string Reason; 
						stringstream SS;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos )
								Reason = Reason.substr( Start );
						}
						if (Reason=="0" || Reason=="1" || Reason=="2" || Reason=="3" || Reason=="4" || Reason=="5" || Reason=="6" || Reason=="7") {
							string tempname = m_UserName;
							transform( tempname.begin( ), tempname.end( ), tempname.begin( ), (int(*)(int))tolower );
							lvl=UTIL_ToUInt32(Reason);// lvl you want to give
							// reason = lvl victim = user  ll= users lvl
							CDBInfo *temp = IsInfo(Victim);
							if (temp) {
								victimlvl=temp->GetLvl();

								if (victimlvl == 6 && lvl != 6) {
									DelHostbot(Victim);
								}

								if (((ll>lvl&&ll>victimlvl)|| IsRootAdmin(User))) {
									if (lvl==0) {
										if (m_GHost->m_IsOperator) {
											SendChatCommand("/ban " + Victim + " from: " + User);
										}
										else {
											SendChatCommand("/kick " + Victim + " from: " + User);
										}
									}
									else if (lvl == 6 && victimlvl != 6) {
										m_HostBots.push_back( new CDBHostBot(Victim, 1, 0, 10 ));
										SendChatCommand(Victim,UTIL_ToString(m_CommandTrigger)+"gethb");
									}
									else {
										if (m_GHost->m_IsOperator && victimlvl == 0) {
											SendChatCommand("/unban " + Victim);
										}
									}
									
									m_PairedInfoUpdateLvls.push_back( PairedInfoUpdateLvl( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateLvl( m_Server, Victim,lvl,User ) ) );
									if (lvl == 0) {
										SendChatCommand(User, "You ban " + Victim + " from channel. lvl: " + UTIL_ToString(lvl));
									}
									else if (lvl == 3) {
										SendChatCommand(User, "You make " + Victim + " high lvl captain. lvl: " + UTIL_ToString(lvl));
									}
									else if (lvl == 2) {
										SendChatCommand(User, "You mark " + Victim + " low lvl captain. lvl: " + UTIL_ToString(lvl));
									}
									else if (lvl == 1) {
										SendChatCommand(User, "You change " + Victim + "'s lvl to " + UTIL_ToString(lvl));
									}
									else if (lvl == 4) {
										SendChatCommand(User, "You make " + Victim + " voucher. lvl: " + UTIL_ToString(lvl));
									}
									else if (lvl == 5) {
										SendChatCommand(User, "You make " + Victim + " admin. lvl: " + UTIL_ToString(lvl));
									}
									else if (lvl == 6) {
										SendChatCommand(User, "You mark " + Victim + " account as hostbot. lvl: " + UTIL_ToString(lvl));
									}
								}
							}
							else {								
								if (((ll>lvl)||IsRootAdmin(User))) {
									SendChatCommand(User, Victim + " doesn't have info in db. Adding now with lvl "+Payload+"!!!" );
									if (lvl==0) {
										if (m_GHost->m_IsOperator) {
											SendChatCommand("/ban " + Victim + " from: " + User);
										}
										else {
											SendChatCommand("/kick " + Victim + " from: " + User);
										}
									}
									else if (lvl == 6) {
										m_HostBots.push_back( new CDBHostBot(Victim, 1, 0, 10 ));
										SendChatCommand(Victim,UTIL_ToString(m_CommandTrigger)+"gethb");
									}

									m_PairedInfoAdds.push_back( PairedInfoAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoAdd( m_Server, Victim,lvl,m_Infos.size( )+1, m_Infos.size( )+1, 0, 0, User, "??", 0, 0,"-",1 ) ) );
								}
							}

							if (ll == lvl && !IsRootAdmin(User)) {
								SendChatCommand(User, "You can 't do user same lvl as yours. Only root admin can.");
							}
							if (User == Victim && (!IsRootAdmin(User))) {
								SendChatCommand(User, "You can 't change your own lvl");
							}
						}
						else {
							SendChatCommand(User, "Error input. Try again. (clvl user lvl)");
						}	
					}

					if (m_GHost->commands[32])
					if (Command=="cm" && !Payload.empty()) {
						if ((m_SpamTime==0 || m_SpamTime+30<GetTime())) {
							m_SpamTime=GetTime();
							string Country;
							string Reason; 
							stringstream SS;
							SS << Payload;
							SS >> Country;
							transform( Country.begin( ), Country.end( ), Country.begin( ), (int(*)(int))toupper );
							if (!SS.eof( ) ) {
								getline( SS, Reason );
								string::size_type Start = Reason.find_first_not_of( " " );
	
								if (Start != string::npos) {
									Reason = Reason.substr(Start);
								}
							}
							
							if (Country.size()==2) {
								for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++ ) {
									if ((*i)->GetMessage() > 0 && (*i)->GetLvl() > 0 && (*i)->GetCountry() == Country) {
										SendChatCommand((*i)->GetUser(), User + " to (" + Country + ") ppl: " + Reason);
									}
								}
								transform( Country.begin( ), Country.end( ), Country.begin( ), (int(*)(int))toupper );
								QueueChatCommand("Sending mass message to users that come from "+Country+".");
							}
							else {
								QueueChatCommand("Wrong input. Type (!cm country message) ex. !fm gr message");
							}
						}
						else {
							QueueChatCommand("You can use command every 30 sec. " + UTIL_ToString(30 - (GetTime() - m_SpamTime)) + " sec more.");
						}
					}

					if (m_GHost->commands[31])
					if ((Command =="msg" || Command =="fm") && !Payload.empty( ) && Info->GetLvl()>2) {
						string tmp = Payload;
						transform( tmp.begin( ), tmp.end( ), tmp.begin( ), (int(*)(int))tolower );
						if (Payload.find("send-slap")!= string::npos) {
							SendChatCommand(User,"You contain in your message send-slap. Messege rejected");
							return;
						}
						MassMessage(User,Payload,true,false);
					}

					if (m_GHost->commands[30])
					if (( Command == "addinfo") && !Payload.empty( ) ) {
						uint32_t ll=Info->GetLvl();
						string Reason;
						stringstream SS;
						string Victim;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}
						CDBInfo *temp = IsInfo( Victim );
						if (temp ) {
							if (ll>temp->GetLvl()) {
								SendChatCommand(User,"General info added for "+Victim);
								m_PairedInfoUpdateGinfos.push_back( PairedInfoUpdateGinfo( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateGinfo( m_Server, Victim, Reason, User ) ) );
							}
							else {
								SendChatCommand(User, "You cant change your own info or user with same lvl as yours.");
							}
						}
						else {
							SendChatCommand(User, "No info for " + Victim + " in db.");
						}
					}

					if (m_GHost->commands[29])
					if (Command=="warn" ) {
						if (!Payload.empty()) {
							uint32_t ll = Info->GetLvl();
							string Reason;
							stringstream SS;
							string Victim;
							SS << Payload;
							SS >> Victim;

							if (!SS.eof( ) ) {
								getline( SS, Reason );
								string::size_type Start = Reason.find_first_not_of( " " );

								if (Start != string::npos) {
									Reason = Reason.substr(Start);
								}
							}
							uint32_t warning = 0;
							
							CDBInfo *temp = IsInfo( Victim );
							if (temp) {
								if (ll>temp->GetLvl()) {

									CDBWarn *warntemp = IsWarn( Victim );
									if (warntemp) {
										uint32_t totalwarns=warntemp->GetTotalwarn()+1;
										
										warning=warntemp->GetWarnings();
										string OldReason= warntemp->GetWarning( )+" ";
										if (warning==2) {
											warning=0;
											
											m_PairedWarnUpdateAdds.push_back( PairedWarnUpdateAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedWarnUpdateAdd( m_Server, Victim,warning, Reason + "(3 warns)",totalwarns,User  ) ) );
											Cban(Victim, 72, User);

										}
										else {
											SendChatCommand(User,"Adding warn to "+Victim +" from "+User+" !!!"); 
											warning++;
											m_PairedWarnUpdateAdds.push_back( PairedWarnUpdateAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedWarnUpdateAdd( m_Server, Victim,warning, OldReason+Reason ,totalwarns,User ) ) );
										}
									}
									else {
										SendChatCommand(User,"Adding warn to "+Victim +" from "+User+" !!!"); 								
										m_PairedWarnAdds.push_back( PairedWarnAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedWarnAdd( m_Server, Victim,1, "1. "+Reason,1,0,User ) ) );
									}
								}
								else {
									SendChatCommand(User, "You can 't warn someone with same access lvl or above!!!");
								}
							}
							else {
								SendChatCommand(User, Victim + " doesn 't have info in db.");
							}
						}
						else {
							uint32_t warns=0;
							for (vector<CDBWarn *>::iterator j = m_Warns.begin(); j != m_Warns.end(); j++ ) {
								if ((*j)->GetTotalwarn()>0)	{
									warns++;
									SendChatCommand(User,(*j)->GetName()+": totalwarns "+ UTIL_ToString( (*j)->GetTotalwarn())+", daysban "+UTIL_ToString((*j)->GetDaysban())+", date "+(*j)->GetDate()+".");
								}
							}
							SendChatCommand(User,"There are "+UTIL_ToString(warns)+" users with warn in database.");
						}			
					}
	
					if (m_GHost->commands[28])
					if (Command =="cban" ) {
						uint32_t Hours = GetHoursSince1970();
						if (Payload.empty()) {
							uint32_t counter= 0;
							SendChatCommand(User,"Channel banned ppl.");
							for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); i++ ) {
								uint32_t left = (*i)->GetDaysban()-Hours;
								if ((*i)->GetDaysban( )!=0 && (*i)->GetDaysban() >= Hours) {
									SendChatCommand(User,(*i)->GetName( )+" "+UTIL_ToString(left)+" hours left from "+(*i)->GetAdmin( )+".");
									counter++;
								}
							}
							SendChatCommand(User,"There are "+UTIL_ToString(counter)+" users with channel ban.");
						}
						else if (Payload == "fix") {
							SendChatCommand(User,"Fixing cban function.");							
							for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); i++ ) {
								if ((*i)->GetDaysban() != 0) {
									SendChatCommand("/unban " + (*i)->GetName());
								}
							}
						}
						else {
							uint32_t ll = Info->GetLvl();
							uint32_t victimlvl = 0;
							bool VictimIsInDb = false;
								
							stringstream SS;
							SS << Payload;
							string tbl[3];
							uint32_t counter=0;
							while( !SS.eof( ) ) {
								string words;
								SS >> words;

								if (SS.fail( ) ) {
									CONSOLE_Print("[Cban]: system error #1", true);
									break;
								}
								else {
									if (counter >= 3) {
										tbl[2] += " " + words;
									}
									else {
										tbl[counter] = words;
									}
									counter++;
								}
							}
							//username icon reason
							if (counter >= 3 ) {
								CDBInfo *temp=IsInfo(tbl[0]);
								if (temp) {
									victimlvl=temp->GetLvl();
									VictimIsInDb = true;
								}
								uint32_t hoursban = UTIL_ToUInt32(tbl[1]);
								if (ll>victimlvl) {
									if (VictimIsInDb) {
										m_PairedInfoUpdateGinfos.push_back( PairedInfoUpdateGinfo( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateGinfo( m_Server, tbl[0], tbl[2], User ) ) );
										Cban(tbl[0],hoursban,User);
									}
									else {
										SendChatCommand(User, "User dosn't exist in database. Check his nickname.");
									}
								}
								else {
									SendChatCommand(User, "Can't use this command for your account or user with bigger lvl than yours.");
								}
							}
							else {
								SendChatCommand(User, "Channel ban command changed. Type " + UTIL_ToString(m_CommandTrigger) + "cban + username + hoursban + reason.");
							}
						}
					}

					if (m_GHost->commands[27])
                    if ((Command=="cbandel" || Command=="delcban") && !Payload.empty()) {
						CDBWarn *temp=IsWarn(Payload);
						if (temp) {
							if (temp->GetDaysban()>0) {
								SendChatCommand("/unban "+Payload);
								QueueChatCommand(Payload+" is unbanned from channel.");
								m_PairedWarnChannelBans.push_back( PairedWarnChannelBan( Whisper ? User : string( ), m_GHost->m_DB->ThreadedWarnChannelBan( m_Server, Payload, 0, User ) ) );
							}
							else {
								SendChatCommand(User, Payload + " isn 't banned from channel.");
							}
						}
						else {
							SendChatCommand(User, Payload + " doesn t have any warn.");
						}
					}

					if (m_GHost->commands[26])
					if (Command=="from" ) {
						if (Payload.empty()) {
							Payload = User;
						}

						CDBInfo *Info = IsInfo( Payload );
						if (Info) {
							QueueChatCommand(Payload + " is from (" + Info->GetCountry() + ")", User, Whisper);
						}
						else {
							QueueChatCommand("No info in db for user "+Payload,User,Whisper);
						}
					}									

					if (m_GHost->commands[25])
					if ((Command=="k" || Command=="kick")&&!Payload.empty()) {
						uint32_t ll=Info->GetLvl();
						string Victim;
						string Reason;
						stringstream SS;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}
						transform( Victim.begin( ), Victim.end( ), Victim.begin( ), (int(*)(int))tolower );
						uint32_t Matches=0;
						string TestName;
						string LastMatch;
						for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
							TestName = (*i)->GetUser();
							transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

							if (TestName.find( Victim ) != string::npos ) {
								Matches++;
								LastMatch = (*i)->GetUser();
							}
						}

						if (Matches == 0 ) {
							SendChatCommand(User,"No user with this name in channel.");
						}
						else if (Matches == 1 ) {
							
							CDBInfo *temp = IsInfo( LastMatch );
							if (temp) {
								if (ll>temp->GetLvl()) {
									if (Reason.empty()) {
										SendChatCommand("/kick " + LastMatch + " from " + User);
									}
									else {
										SendChatCommand("/kick " + LastMatch + " Reason:" + Reason + " from:" + User);
									}
								}
								else {
									SendChatCommand(User, "Can 't kick user with same lvl.");
								}
							}
						}
						else {
							SendChatCommand(User, "More than one user are in channel with this name.");
						}
					}

					if (m_GHost->commands[24])
					if (Command == "bans") {
						m_PairedBanCounts.push_back(PairedBanCount(User, m_GHost->m_DB->ThreadedBanCount(m_Server)));
					}

					if (m_GHost->commands[86])
					if (Command == "mails") {
						SendChatCommand(User, "There are " + UTIL_ToString(m_Mails.size()) + " in db.");
					}
					
					if (m_GHost->commands[23])
					if (Command == "infos") {
						m_PairedInfoCounts.push_back(PairedInfoCount(User, m_GHost->m_DB->ThreadedInfoCount(m_Server)));
					}
					
					if (m_GHost->commands[22])
					if (Command == "dbstatus") {
						SendChatCommand(User, m_GHost->m_DB->GetStatus());
					}

					if (m_GHost->commands[21])
					if (( Command == "delban" || Command == "unban" ) && !Payload.empty( ) ) {
						m_PairedBanRemoves.push_back( PairedBanRemove( User, m_GHost->m_DB->ThreadedBanRemove( m_Server, Payload ) ) );
					}					
					
				}

				/*********************
				*      lvl 2         *
				*********************/
				if (Accesslvl>=2 && Accesslvl<6 && !m_RootOnly) {
					
					if (m_GHost->commands[8])
					if (Command=="gormk" || Command=="rmkgo") {
						if (m_RMKstate == 0) {
							SendChatCommand(User, "There isn't any rmk game in memory.");
						}
						else {
							string GameOwner = User;
							if (m_GameId < 999) {
								m_GameId++;
							}
							else {
								m_GameId = 0;
							}

							uint32_t CurrentGameid = m_GameId;
							if (m_RMKstate == 2) {
								CurrentGameid = CurrentGameid + 1000;
							}

							string GameName = m_GHost->m_CommunityName+"-rmk-"+UTIL_ToString(m_GamedThatBotCreate);
							string Hostbot = GetAvailableHostbot();							
							if (!Hostbot.empty( )) {
								SendChatCommand(Hostbot ,UTIL_ToString(m_CommandTrigger)+"load "+Hostbot +" "+m_GHost->m_defaultMap);
								for (int j=0;j<12;j++) {									
									if (!m_RMK[j].empty()) {
										if (j == 0) {
											GameOwner = m_RMK[j];
										}
										SendChatCommand(m_RMK[j],"Join "+GameName+". Remaking game.");
										SendChatCommand(Hostbot ,UTIL_ToString(m_CommandTrigger)+"hold "+m_RMK[j]+" "+UTIL_ToString(j+1));
									}
								}
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"hold "+UTIL_ToString( CurrentGameid )+" 13");
								m_GHost->AddGame( CurrentGameid, Hostbot, User, GetTime( ), 10, 0, GameName);
								if (m_RMKstate == 1) {
									QueueChatCommand("Remaking game with " + Hostbot + " bot with name: " + GameName);
								}
								else {
									QueueChatCommand("Remaking challenge game with " + Hostbot + " bot with name: " + GameName);
								}
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"privby "+GameOwner+" "+GameName);
								m_RMKstate=0;
								UpdateHostbot(Hostbot,2,GetHostbotGames( Hostbot )+1,0);
							}
						}
					}

					if (m_GHost->commands[19])
					if ((Command =="msg" || Command =="fm") && !Payload.empty( ) && Info->GetLvl()<4 ) {
						string tmp = Payload;
						transform( tmp.begin( ), tmp.end( ), tmp.begin( ), (int(*)(int))tolower );
						if (Payload.find("send-slap")!= string::npos) {
							SendChatCommand(User,"Your message contains send-slap. Messege rejected");
							return;
						}
						MassMessage(User,Payload,false,false);
					}

					if (m_GHost->commands[8])
					if (Command=="spam" && m_Challenge && !m_Challenged.empty() && (User==m_Challenger || User==m_Challenged || Accesslvl>2)) {
						string SlotsNumber= UTIL_ToString(GetPoolHoldedSlots());
						MassMessage("System","Challenge "+m_Challenger+" vs "+m_Challenged +". Join "+m_CurrentChannel+". "+SlotsNumber+" in pool.",false,false);
					}	

					if (m_GHost->commands[8])
					if ((Command=="challenge" || Command=="chal" || Command=="c") && !Whisper && !m_Challenge && m_ChallengeStep==0) {	
						CDBBan *Ban = IsBannedName( User );	
						if (Ban) {
							SendChatCommand(User,"You are banned and can't challenge anyone.");
							return;
						}
						if (IsInPool(User)) {
							SendChatCommand(User,"You slot in pool list removed cose you challenge someone.");
							UnSign( User );
						}
						
						m_ChallengeTime=GetTime();
						if (Payload.empty()) {
							ResetChallenge( false, true, true );
							m_ChallengeTimers = true;
							m_Challenge=true;
							m_meplayon=false;
							m_Challenger=User;
							QueueChatCommand("A challenge has started. To accept challenge from "+User +" type "+UTIL_ToString(m_CommandTrigger)+"accept.");
							m_ChallengeStep=1;
						}
						else {
															
							uint32_t Matches=0;
							string TestName;
							string LastMatch;
							for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
								TestName = (*i)->GetUser();
								transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

								if (TestName.find( Payload ) != string::npos ) {
									Matches++;
									LastMatch = (*i)->GetUser();
								}
							}
							CDBBan *Ban = IsBannedName( LastMatch );	
							if (Ban) {
								SendChatCommand(User,LastMatch+" is banned.");
								return;
							}
							bool hostbot=false;
							uint32_t tmplvl=0;
							CDBInfo *temp = IsInfo(LastMatch);
							if (temp) {
								if (temp->GetLvl() >= 6) {
									hostbot = true;
								}
								else {
									tmplvl = temp->GetLvl();
								}
							}

							if (m_UserName!=LastMatch && !hostbot) {
								if (Matches == 0 ) {
									SendChatCommand(User,"No user with this name in channel.");
								}
								else if (Matches == 1 ) {
									if (User!=LastMatch) {
										if (ChallengeRankProtectPass(User,LastMatch)) {
											if (Accesslvl>=tmplvl && tmplvl>1) {
												ResetChallenge( false, true, true );
												m_ChallengeTimers = true;
												m_Challenger=User;
												m_Challenged=LastMatch;
												m_Challenge=true;
												m_meplayon=false;
												QueueChatCommand(User+" is challenging "+LastMatch+". Type "+UTIL_ToString(m_CommandTrigger)+"accept or "+UTIL_ToString(m_CommandTrigger)+"refuse.");
												m_ChallengeStep=1;
											}
											else {
												SendChatCommand(User, "You can't challenge users with bigger lvl than yours or lvl 1 and less lvl users.");
											}
										}
										else {
											QueueChatCommand(User + " can't challenge " + LastMatch + ". Not same experience. Challenge someone else.");
										}
									}
									else {
										SendChatCommand(User, "You can't challenge yourself!!!");
									}
								}
								else {
									SendChatCommand(User, "More than 1 user with that name.");
								}
							}
							else {
								SendChatCommand(User, "You cant challenge Guard Bot or hostbots.");
							}
						}
					}
				}

				/*********************
				*      lvl 1         *
				*********************/
				if (Accesslvl>=1 && Accesslvl<6) {
				
					if (m_GHost->commands[82])
					if (Command=="unhost" && Payload.empty( )) {
						string tmp = GetHostbotFromOwner( User );
						if (tmp.empty()) {
							SendChatCommand(User, "You aren't owner of any game. Can't unhost game.");
						}
						else {
							SendChatCommand(tmp,UTIL_ToString(m_CommandTrigger)+"unhost");
							SendChatCommand(User,"Unhosting game.");
						}						
					}
	
					if (Command == "dm" || Command == "delmap") {
						DelMap(User);
					}

					if (Command == "ml" || Command == "maplist") {
						PrintMaps(User);
					}
					if (Command=="obs") {
						if (IsMap(User).empty( )) {
							SendChatCommand(User,"You must load custom map first. Write "+UTIL_ToString(m_CommandTrigger)+"map + mapname and then use "+UTIL_ToString(m_CommandTrigger)+"obs + ref/obs");
							return;
						}
						
						if (Payload.empty()) {
							SetObs(User, 0);
						}
						else if (Payload == "obs") {
							SetObs(User, 1);
						}
						else if (Payload == "ref") {
							SetObs(User, 2);
						}
					}
					if (Command=="map") {
						if (Payload.empty( )) {
							if (IsMap(User).empty()) {
								SendChatCommand(User, "Your current map is latest dota map.");
							}
							else {
								uint32_t obs = IsObs(User);
								string obs_msg = " (no obs/ref)";
								if (obs == 1) {
									obs_msg = " (with obs)";
								}
								else if (obs == 2) {
									obs_msg = " (with ref)";
								}
								SendChatCommand(User,"Your current map is ("+IsMap( User )+obs_msg);
							}
						}
						else {
							SetMap(User, Payload);
						}
					}

					if (Command == "hbots") {
						GetHostbots(User);
					}

					if (m_GHost->commands[77])
					if ((Command == "inform")) {
						GetAllInfo(User, Payload);
					}

					if (m_GHost->commands[87])
					if (Command == "where" && !Payload.empty()) {
						FindUser(User, Payload);
					}

					if (m_GHost->commands[86])
					if (Command=="mailbox" && Payload.empty( )) {
						if (m_GHost->m_MailEnabled) {
							PrintSendedMails(User);
							SendChatCommand(User,"------------------------------------------------------------------------------.");
							PrintMails(User, User, false, true);
						}
						else {
							SendChatCommand(User, "Mail function is disabled.");
						}
					}
						
					if (m_GHost->commands[86])
					if (Command =="delmail" && !Payload.empty( )) {
						uint32_t id = UTIL_ToUInt32(Payload);
						if (m_GHost->m_MailEnabled) {							
							
							if (IsMail(User)) {
								m_PairedMailRemoves.push_back( PairedMailRemove( Whisper ? User : string( ), m_GHost->m_DB->ThreadedMailRemove( m_Server, User, id ) ) );
							}
							else {
								SendChatCommand(User, "You don't have mail in your mailbox.");
							}
						}
						else {
							SendChatCommand(User, "Mail function is disabled.");
						}
					}

					if (m_GHost->commands[86])
					if (Command =="mail" && !Payload.empty( )) {
						if (m_GHost->m_MailEnabled) {
							string Victim;
							string Reason; 
							stringstream SS;
							SS << Payload;
							SS >> Victim;

							if (!SS.eof( ) ) {
								getline( SS, Reason );
								string::size_type Start = Reason.find_first_not_of( " " );

								if (Start != string::npos) {
									Reason = Reason.substr(Start);
								}
							}
							transform( Victim.begin( ), Victim.end( ), Victim.begin( ), (int(*)(int))tolower );
							transform( User.begin( ), User.end( ), User.begin( ), (int(*)(int))tolower );
							if (User != Victim) {
								bool temp = false;
								CDBInfo *Info = IsInfo( Victim );
								if (Info) {
									temp = true;
								}
								if (temp ) {
									if (!MailsSendedMaxLimit( User )) {
										if (!MailsReceivedMaxLimit( Victim )) {
											SendChatCommand(User, "Mail stored to be sent to "+Victim+" with id "+UTIL_ToString(GetMailId( )+1));
											SendChatCommand(Victim,"You have new mail message from "+User);
											m_PairedMailAdds.push_back( PairedMailAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedMailAdd( GetMailId( )+1, m_Server, User, Victim, Reason ) ) );
										}
										else {
											SendChatCommand(User,"You can't send mail. His mail box is full.");
										}
									}
									else {
										SendChatCommand(User,"You can't send mail. You reach max number of mails.");
										SendChatCommand(User,"If you sent a mail to him and want to del it type !delmail + id.");
										SendChatCommand(User,"To check your mail box type !mailbox.");
									}
								}
								else {
									SendChatCommand(User, "There isn't any user with this name in db to send mail.");
								}
							}
							else {
								SendChatCommand(User, "You can't send mail to yourself.");
							}
						}
						else {
							SendChatCommand(User, "Mail function is disabled.");
						}
					}

					if (m_GHost->commands[11])
					if ((Command=="pub") && !Whisper && ( !m_Challenge || m_ChallengeStep < 3 ) && !IsBannedNameUser) {
						if (!GetHostbotFromOwner(User).empty( )) {
							SendChatCommand(User,"You host already one game. Type !unhost or join it.");
							return;
						}

						if (m_GameId < 999) {
							m_GameId++;
						}
						else {
							m_GameId = 0;
						}
						string GameName;
						if (Payload.empty()) {
							GameName = m_GHost->m_CommunityName + "-" + UTIL_ToString(m_GamedThatBotCreate);
						}
						else {
							GameName = Payload;
						}

						string Hostbot = GetAvailableHostbot(); 
						if ((m_FreeReady && Accesslvl==1) || Accesslvl>1 || m_GHost->m_FreeHostFromCountry==Info->GetCountry() || m_GHost->m_FreeHostFromCountry=="ALL") {
							if (!m_holdslots) {
								m_holdslots = true;
								m_meplay[0][0]=User;
							}

							if (!Hostbot.empty( )) {	
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"load "+m_GHost->m_defaultMap);	
								for (int j = 0; j < 12; j++) {
									if (!m_meplay[j][0].empty()) {
										SendChatCommand(Hostbot, UTIL_ToString(m_CommandTrigger) + "hold " + m_meplay[j][0] + " " + UTIL_ToString(j + 1));
									}
								}
								
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"hold "+UTIL_ToString( m_GameId )+" 13");
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"sendend");	
								m_GHost->AddGame( m_GameId, Hostbot, User, GetTime( ), 10, 0, GameName);
								SendChatCommand(User,"Hosting pub game with "+Hostbot+" bot with name: "+GameName);
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"pubby "+User+" "+GameName);
								UpdateHostbot(Hostbot,2,GetHostbotGames( Hostbot )+1,0);
							}
							else {
								SendChatCommand(User, "There isn't any hostbot free to create game.");
							}
						}
						else {
							SendChatCommand(User, "You can't create game cose you are lvl 1 and free hosting service is disabled.");
						}
					}

					if (m_GHost->commands[10])
					if (Command=="priv" && !Whisper && ( !m_Challenge || m_ChallengeStep < 3 ) && !IsBannedNameUser) {
						if (!GetHostbotFromOwner(User).empty( )) {
							SendChatCommand(User,"You host already one game. Type !unhost or join it.");
							return;
						}
						if (m_GameId < 999) {
							m_GameId++;
						}
						else {
							m_GameId = 0;
						}

						string GameName;
						if (Payload.empty()) {
							GameName = m_GHost->m_CommunityName + "-" + UTIL_ToString(m_GamedThatBotCreate);
						}
						else {
							GameName = Payload;
						}

						string Hostbot = GetAvailableHostbot(); 
						if ((m_FreeReady && Accesslvl==1) || Accesslvl>1 || m_GHost->m_FreeHostFromCountry==Info->GetCountry() || m_GHost->m_FreeHostFromCountry=="ALL") {
							if (!m_holdslots) {
								m_holdslots = true;
								m_meplay[0][0]=User;
							}
							if (!Hostbot.empty( )) {
								for (int j = 0; j < 12; j++) {
									if (!m_meplay[j][0].empty()) {
										SendChatCommand(Hostbot, UTIL_ToString(m_CommandTrigger) + "hold " + m_meplay[j][0] + " " + UTIL_ToString(j + 1));
									}
								}
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"load "+m_GHost->m_defaultMap);	
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"hold "+UTIL_ToString( m_GameId )+" 13");
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"sendend");	
								m_GHost->AddGame( m_GameId, Hostbot, User, GetTime( ), 10, 0, GameName);
								SendChatCommand(User,"Hosting priv game with "+Hostbot+" bot with name: "+GameName);
								SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"privby "+User+" "+GameName);								
								UpdateHostbot(Hostbot,2,GetHostbotGames( Hostbot )+1,0);
							}
							else {
								SendChatCommand(User, "There isn't any hostbot free to create game.");
							}
						}
						else {
							SendChatCommand(User, "You can't create game cose you are lvl 1 and free hosting service is disabled.");
						}
					}

					if (Command == "root") {
						SendChatCommand(User, "Root admin is " + GetRootAdmin());
					}

					if (m_GHost->commands[83])
					if ((Command=="users" )&& Payload.empty()) {
						SendChatCommand(User,"There are "+UTIL_ToString(m_GHost->GetUsersNumber( ))+" users in "+UTIL_ToString( m_GHost->m_Games.size( ))+" games and "+UTIL_ToString(GetUsersNumberInChannel( ))+" in channel");
						SendChatCommand(User,"("+UTIL_ToString(m_GHost->GetLobby( )+m_GHost->GetChallenges( )+m_GHost->GetPubs( )+m_GHost->GetPrivs( ))+"/"+UTIL_ToString(GetMaxGames( ))+") There are "+UTIL_ToString(m_GHost->GetChallenges( ))+" challenges, "+UTIL_ToString(m_GHost->GetPubs( ))+" public, "+UTIL_ToString(m_GHost->GetPrivs( ))+" private games and "+UTIL_ToString( m_GHost->GetLobby( ))+" in lobby.");
					}
					
					if (m_GHost->commands[77])
					if ((Command=="ggs" || Command=="games") ) {
						if (Payload.empty()) {
							GetInfo(User);
						}
						else {
							GetGameFromBots( User, Payload );
						}
					}

					if (m_GHost->commands[78])
					if ((Command=="gn" || Command=="getnames") && !Payload.empty()) {
						uint32_t id = UTIL_ToUInt32(Payload);
						GetNames(User,id);
					}

					if (m_GHost->commands[79])
					if ((Command=="gg" || Command=="ggames") ) {
						if (Payload.empty()) {
							m_GHost->GetGames(User);
						}
						else {
							uint32_t id = UTIL_ToUInt32(Payload);
							m_GHost->GetGame(User,id);
						}
					}
										
					if (m_GHost->commands[17])
					if (Command=="userson" && Payload.empty() ) {
						if (!m_OnlineChecking) {
							m_OnlineChecking = true;
							
							m_OnlineUsers.clear();
							m_InChannelUsers.clear();
							
							m_MassMessageCalledBy=User;						
							SendChatCommand(User,"Checking online users.");
							for (vector<CDBInfo *>::iterator j = m_Infos.begin(); j != m_Infos.end(); j++ ) {
								if ((*j)->GetLvl() < 6 && (*j)->GetLvl() > 0) {
									SendChatCommand("/where " + (*j)->GetUser());
								}
							}
							m_OnlineCheckingTime=GetTime();
						}
						else {
							SendChatCommand(User, "Someone already use this command. Wait 2 sec and try again.");
						}
					}

					if (m_GHost->commands[16])
					if (Command=="rank" && !Payload.empty()) {
						uint32_t Rank = UTIL_ToUInt32( Payload );
						SendChatCommand(User,"User with rank "+ Payload + " in priv ladder is "+GetNameFromPrivRank(Rank)+".");
						SendChatCommand(User,"User with rank "+ Payload + " in pub ladder is "+GetNameFromPubRank(Rank)+".");
					}
					
					if (m_GHost->commands[15])
					if (Command=="find") {
						string Victim;
						string Reason; 
						stringstream SS;
						SS << Payload;
						SS >> Victim;

						if (!SS.eof( ) ) {
							getline( SS, Reason );
							string::size_type Start = Reason.find_first_not_of( " " );

							if (Start != string::npos) {
								Reason = Reason.substr(Start);
							}
						}
						
						if (Victim.length() == 2 && Reason.length()>2) {
							transform( Victim.begin( ), Victim.end( ), Victim.begin( ), (int(*)(int))toupper );
							SendChatCommand(User,"-------------------------------------------------------------");
							SendChatCommand(User,"Users that have string "+Reason+" inside and come from "+Victim+".");
							SendChatCommand(User,"-------------------------------------------------------------");
							string tempnames;
							uint32_t counter=0;
							string tempuser;
							transform( Reason.begin( ), Reason.end( ), Reason.begin( ), (int(*)(int))tolower );

							for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++ ) {
								if ((*i)->GetCountry()==Victim) {
									tempuser =(*i)->GetUser();
									transform( tempuser.begin( ), tempuser.end( ), tempuser.begin( ), (int(*)(int))tolower );
									if (tempuser.find( Reason ) != string::npos ) {
										tempnames+=(*i)->GetUser( )+"  ";
										counter++;
									}
									if (counter == 4) {
										counter=0;
										SendChatCommand(User, tempnames);
										tempnames="";
									}
								}
							}
							if (counter < 4) {
								SendChatCommand(User, tempnames);
							}
						}
						else if (Victim.length( )>3 && Reason.empty()) {
							SendChatCommand(User,"-------------------------------------------------------------");
							SendChatCommand(User,"Users that have string "+Victim+" inside.");
							SendChatCommand(User,"-------------------------------------------------------------");
							string tempnames;
							uint32_t counter=0;
							string tempuser;
							transform( Victim.begin( ), Victim.end( ), Victim.begin( ), (int(*)(int))tolower );

							for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++ ) {
								tempuser =(*i)->GetUser();
								transform( tempuser.begin( ), tempuser.end( ), tempuser.begin( ), (int(*)(int))tolower );
								if (tempuser.find( Victim ) != string::npos ) {
									tempnames+=(*i)->GetUser( )+"  ";
									counter++;
								}
								if (counter == 4) {
									counter=0;
									SendChatCommand(User, tempnames);
									tempnames="";
								}
							}
							if (counter < 4) {
								SendChatCommand(User, tempnames);
							}
						}
						else if (Victim.length( )==2 && Reason.empty()) {
							transform( Victim.begin( ), Victim.end( ), Victim.begin( ), (int(*)(int))toupper );
							SendChatCommand(User,"-------------------------------------------------------------");
							SendChatCommand(User,"Users that come from "+Victim+".");
							SendChatCommand(User,"-------------------------------------------------------------");
							string tempnames;
							uint32_t counter=0;
							for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++ ) {
								
								if ((*i)->GetCountry()==Victim) {
									tempnames+=(*i)->GetUser( )+" ";
									counter++;
								}
								if (counter == 4) {
									counter=0;
									SendChatCommand(User, tempnames);
									tempnames="";
								}
							}
							if (counter < 4) {
								SendChatCommand(User, tempnames);
							}
						}
						else {
							SendChatCommand(User, "Type !find +4 letters(name) or !find +2 letters(country) +3 letters(name) or !find <2 letters> country.");
						}
					}

					if (m_GHost->commands[14])
					if (Command=="ex" && m_ChallengeTimerExtended && (User==m_Challenger || User==m_Challenged || Accesslvl>2)) {
						QueueChatCommand("+5 min till autoabort.");
						m_ChallengeTimerExtended = false;
						m_ChallengeTime+=300;
						m_ChallengeTimersRemainSpam = 0;
					}

					if (m_GHost->commands[13])
					if ((Command == "ss" || Command == "safelisted") && Payload.empty()) {
						SendChatCommand(User, "Number of safelisted users: " + UTIL_ToString(GetSafelisted()));
					}

					if (m_GHost->commands[12])
					if (Command=="top") {
						if (Payload.empty()) {							
							string names,names2,names3,names4,names5,names6,names7,names8;
							for (int k = 1; k < 21; k++) {
								for (vector<CDBInfo *> ::iterator j = m_Infos.begin(); j != m_Infos.end(); j++) {
									if ((*j)->GetPrivRank() == k && (*j)->GetPrivPoints() > 0) {
										if (k < 6) {
											names += UTIL_ToString(k) + "." + (*j)->GetUser() + " ";
										}
										else if (k > 5 && k < 11) {
											names2 += UTIL_ToString(k) + "." + (*j)->GetUser() + " ";
										}
										else if (k > 10 && k < 16) {
											names3 += UTIL_ToString(k) + "." + (*j)->GetUser() + " ";
										}
										else if (k > 15 && k < 21) {
											names4 += UTIL_ToString(k) + "." + (*j)->GetUser() + " ";
										}
									}
								}
							}
							for (int k = 1; k < 21; k++) {
								for (vector<CDBInfo *> ::iterator j = m_Infos.begin(); j != m_Infos.end(); j++) {
									if ((*j)->GetPubRank() == k && (*j)->GetPubPoints() > 0) {
										if (k < 6) {
											names5 += UTIL_ToString(k) + "." + (*j)->GetUser() + " ";
										}
										else if (k > 5 && k < 11) {
											names6 += UTIL_ToString(k) + "." + (*j)->GetUser() + " ";
										}
										else if (k > 10 && k < 16) {
											names7 += UTIL_ToString(k) + "." + (*j)->GetUser() + " ";
										}
										else if (k > 15 && k < 21) {
											names8 += UTIL_ToString(k) + "." + (*j)->GetUser() + " ";
										}
									}
								}
							}
							
							SendChatCommand(User,"--------------------- TOP PRIV USERS ---------------------");
							
							SendChatCommand(User,names);
							SendChatCommand(User,names2);
							SendChatCommand(User,names3);
							SendChatCommand(User,names4);
							SendChatCommand(User,"------------------------------------------------------------");
							
							if (m_GHost->m_PrintPubLadder) {
									
								SendChatCommand(User,"--------------------- TOP PUB USERS ---------------------");							
								SendChatCommand(User,names5);
								SendChatCommand(User,names6);
								SendChatCommand(User,names7);
								SendChatCommand(User,names8);
								SendChatCommand(User,"------------------------------------------------------------");
							}							
						}
					}
					
					if (m_GHost->commands[88])
					if ((Command=="gopriv" || Command=="privgo" ) && ( !m_Challenge || m_ChallengeStep < 3 ) && !IsBannedNameUser) {
						if (!GetHostbotFromOwner(User).empty( )) {
							SendChatCommand(User,"You host already one game. Type !unhost or join it.");
							return;
						}

						if (m_GHost->m_GoPubByWhisper && !Whisper) {
							SendChatCommand(User,"You can use this command only with whisper.");
						}
						else {
							if (m_GameId < 999) {
								m_GameId++;
							}
							else {
								m_GameId = 0;
							}

							string GameName;
							if (Payload.empty()) {
								GameName = m_GHost->m_CommunityName + "-" + UTIL_ToString(m_GamedThatBotCreate);
							}
							else {
								GameName = Payload;
							}

							string Hostbot = GetAvailableHostbot(); 
							if ((m_FreeReady && Accesslvl==1) || Accesslvl>1 || m_GHost->m_FreeHostFromCountry==Info->GetCountry() || m_GHost->m_FreeHostFromCountry=="ALL") {
								if (!Hostbot.empty( )) {
									string map = IsMap( User );
									if (map.empty( )) {
										SendChatCommand(Hostbot, UTIL_ToString(m_CommandTrigger)+"load "+m_GHost->m_defaultMap);
										DelayMs( 500 );
									}
									else {
										uint32_t obs = IsObs(User);
										if (obs > 0) {
											SendChatCommand(Hostbot, UTIL_ToString(m_CommandTrigger) + "obs " + UTIL_ToString(obs));
										}
										DelayMs( 300 );

										SendChatCommand(Hostbot, UTIL_ToString(m_CommandTrigger)+"map "+ map);
										DelayMs( 300 );
									}

									SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"hold "+UTIL_ToString( m_GameId )+" 13");
									m_GHost->AddGame( m_GameId, Hostbot, User, GetTime( ), 10, 0, GameName);
									SendChatCommand(User,"Hosting priv game with "+Hostbot+" bot with name: "+GameName);
									SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"privby "+User+" "+GameName);								
									UpdateHostbot(Hostbot,2,GetHostbotGames( Hostbot )+1,0);
								}
								else {
									SendChatCommand(User, "There isn't any hostbot free to create game.");
								}
							}
							else {
								SendChatCommand(User, "You can't create game cose you are lvl 1 and free hosting service is disabled.");
							}
						}
					}

					if (m_GHost->commands[89])
					if ((Command=="gopub" || Command=="pubgo") && !Whisper && ( !m_Challenge || m_ChallengeStep < 3 ) && !IsBannedNameUser) {
						if (!GetHostbotFromOwner(User).empty( )) {
							SendChatCommand(User,"You host already one game. Type !unhost or join it.");
							return;
						}

						if (m_GHost->m_GoPubByWhisper && !Whisper) {
							SendChatCommand(User,"You can use this command only with whisper.");
						}
						else {
							if (m_GameId < 999) {
								m_GameId++;
							}
							else {
								m_GameId = 0;
							}
							string GameName;
							if (Payload.empty()) {
								GameName = m_GHost->m_CommunityName + "-" + UTIL_ToString(m_GamedThatBotCreate);
							}
							else {
								GameName = Payload;
							}

							string Hostbot = GetAvailableHostbot(); 
							if ((m_FreeReady && Accesslvl==1) || Accesslvl>1 || m_GHost->m_FreeHostFromCountry==Info->GetCountry() || m_GHost->m_FreeHostFromCountry=="ALL") {
								if (!Hostbot.empty( )) {	
									string map = IsMap( User );
									
									if (map.empty( )) {
										SendChatCommand(Hostbot, UTIL_ToString(m_CommandTrigger)+"load "+m_GHost->m_defaultMap);
										DelayMs( 500 );
									}
									else {
										uint32_t obs = IsObs(User);
										if (obs > 0) {
											SendChatCommand(Hostbot, UTIL_ToString(m_CommandTrigger) + "obs " + UTIL_ToString(obs));
										}
										DelayMs( 300 );
										SendChatCommand(Hostbot, UTIL_ToString(m_CommandTrigger)+"map "+ map);
										SendChatCommand(User,"Hosting custom map ("+map+").");
										DelayMs( 300 );
									}

									SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"hold "+UTIL_ToString( m_GameId )+" 13");
									m_GHost->AddGame( m_GameId, Hostbot, User, GetTime( ), 10, 0, GameName);
									SendChatCommand(User,"Hosting pub game with "+Hostbot+" bot with name: "+GameName);
									SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"pubby "+User+" "+GameName);
									UpdateHostbot(Hostbot,2,GetHostbotGames( Hostbot )+1,0);								
								}
								else {
									SendChatCommand(User, "There isn't any hostbot free to create game.");
								}
							}
							else {
								SendChatCommand(User, "You can't create game cose you are lvl 1 and free hosting service is disabled.");
							}
						}
					}

					if (m_GHost->commands[8])
					if ((Command=="refuse" || Command=="r") && Payload.empty() && m_Challenger != User && m_Challenge && m_ChallengeStep==1 && m_Challenged == User) {
						QueueChatCommand(m_Challenged+ " refused to accept the challenge from "+m_Challenger);
						ResetChallenge( false, true, true );
					}

					if (m_GHost->commands[8])
					if ((Command=="accept" || Command=="a") && Payload.empty() && m_Challenger != User && m_Challenge && m_ChallengeStep==1) {
						CDBBan *Ban = IsBannedName( User );	
						if (Ban) {
							SendChatCommand(User,"You are banned and can't challenge anyone.");
							return;
						}
						if (IsInPool(User)) {
							SendChatCommand(User,"Your name removed from pool list cose you accept the challenge.");
							UnSign(User);
						}

						uint32_t ll=Info->GetLvl();
						if (m_Challenged.empty()) {
							uint32_t tmplvl=0;
							CDBInfo *temp = IsInfo(m_Challenger);
							if (temp) {
								tmplvl = temp->GetLvl();
							}
							if (ll>=tmplvl) {
								if (ChallengeRankProtectPass(m_Challenger,User)) {
									m_Challenged=User;
									MassMessage("Info","The challenge between "+ m_Challenger+" and "+User+" has started! Type "+UTIL_ToString(m_CommandTrigger)+"sign to add for the game, "+UTIL_ToString(m_CommandTrigger)+"out to remove.", true,false);
									m_ChallengeStep=2;
									if (GetPoolHoldedSlots() > 7) {
										QueueChatCommand("Ready to start pick. Type " + UTIL_ToString(m_CommandTrigger) + "startpick.");
									}
								}
								else {
									SendChatCommand(User, "You can't accept challenge from " + m_Challenger + ". Not same experience.");
								}
							}
							else {
								SendChatCommand(User, "Only users with same lvl and above from challenger can accept challenge.");
							}
						}
						else if (m_Challenged==User) {
							QueueChatCommand("Type "+UTIL_ToString(m_CommandTrigger)+"sign. When 8 ppl sign you can type "+UTIL_ToString(m_CommandTrigger)+"startpick");
							QueueChatCommand("You can unsign with "+UTIL_ToString(m_CommandTrigger)+"out and vote for mode with "+UTIL_ToString(m_CommandTrigger)+"mode (cd or cm)");
							MassMessage("Info","Challenge "+ m_Challenger+" VS "+User+" has started.", true,false);
							m_ChallengeStep=2;

							if (GetPoolHoldedSlots() > 7) {
								QueueChatCommand("Ready to start pick. Type " + UTIL_ToString(m_CommandTrigger) + "startpick.");
							}
						}
					}

					if (m_GHost->commands[8])
					if ((Command=="challengers" ||Command=="cs") && m_Challenge) {
						string challenger = m_Challenger;
						string challenged = m_Challenged;
						if (GetUserMode(m_Challenger) != "-") {
							challenger += " (" + GetUserMode(m_Challenger) + ")";
						}
						if (GetUserMode(m_Challenged) != "-") {
							challenged += " (" + GetUserMode(m_Challenged) + ")";
						}

						SendChatCommand(User,challenger+" VS "+challenged);
					}

					if (m_GHost->commands[8])
					if (Command == "pool") {
						PrintPool(User);
					}
					
					if (m_GHost->commands[7])
					if (Command=="res" && Payload.empty() && !m_Challenge && !Whisper) {
						if (m_SignLock) {
							SendChatCommand(User, "Try again in 2 seconds. Spam protect.");
						}
						else {
							if (m_meplayon) {
								CDBBan *Ban = IsBannedName( User );
								if (Ban) {
									SendChatCommand(User, "You are banned and can 't reserve slot");
								}
								else {
									m_holdslots=true;							
									if (!IsInPool(User)) {
										for (int i=0;i<10;i++) { 
											if (m_meplay[i][0].empty()) {
												m_meplay[i][0]=User;
												SendChatCommand(User,"You have reserved slot "+UTIL_ToString((i+1)));
												m_meplayppl++;
												break;
											}
										}
									}
									else {									
										for (int i=0;i<10;i++) {
											if (m_meplay[i][0]==User) {
												SendChatCommand(User,"Reserve slot removed");
												m_meplay[i][0]="";
												m_meplayppl--;
												break;
											}
										}
									}

									if (m_meplayppl >= m_ReservesNeeded) {
										m_FreeReady = true;
									}
									else {
										m_FreeReady = false;
									}

									uint32_t countsigns =0;
									for (int i=0;i<10;i++) {
										if (!m_meplay[i][0].empty()) {
											countsigns++;
										}
									}
									//autohost game with available hostbot
									if (countsigns==10) {
										string hb = GetAvailableHostbot();
										if (hb.empty()) {
											//clear slots and say no hostbot available try again later
											for (int i = 0; i < 10; i++) {
												m_meplay[i][0] = "";
											}
											QueueChatCommand("No hostbot available in channel. Try again if hostbot join channel.");										
										}
										else {
											if (m_GHost->m_AutoCreate ) {
													m_SignLock = true;
													if (m_GameId < 999) {
														m_GameId++;
													}
													else {
														m_GameId = 0;
													}

													SendChatCommand(hb ,UTIL_ToString(m_CommandTrigger)+"load "+hb +" "+m_GHost->m_defaultMap);// load dota map just in case other user forgot to change map from hostbot														
													for (int j=0;j<12;j++) {
														if (!m_meplay[j][0].empty()) {
															SendChatCommand(hb, UTIL_ToString(m_CommandTrigger) + "hold " + m_meplay[j][0] + " " + UTIL_ToString(j + 1));
														}
													}
													SendChatCommand(hb,UTIL_ToString(m_CommandTrigger)+"hold "+UTIL_ToString( m_GameId )+" 13");//sent bot the id of game to hold in 13 slot of holdlist in table
													SendChatCommand(hb,UTIL_ToString(m_CommandTrigger)+"sendend");
													m_GHost->AddGame( m_GameId, hb, m_UserName, GetTime(), 10, 0, m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));
													if (m_GHost->m_ResAutostartHostPriv) {
														SendChatCommand(hb,UTIL_ToString(m_CommandTrigger)+"privby "+m_UserName +" "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));	
														QueueChatCommand("Host priv game. Name: "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));
													}
													else {
														SendChatCommand(hb,UTIL_ToString(m_CommandTrigger)+"pubby "+m_UserName +" "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));	
														QueueChatCommand("Host pub game. Name: "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));
													}
													UpdateHostbot(hb,2,GetHostbotGames( hb )+1,0);
													for (int j=0;j<12;j++) {
														if (!m_meplay[j][0].empty()) {
															SendChatCommand(m_meplay[j][0] ,"Join Game: "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));
															SendChatCommand(m_meplay[j][0] ,"Join Game: "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));
															SendChatCommand(m_meplay[j][0] ,"Join Game: "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));
														}
													}
													DelayMs(5000);
													SendChatCommand(hb,UTIL_ToString(m_CommandTrigger)+"autostart 10");
													SendChatCommand(hb,UTIL_ToString(m_CommandTrigger)+"mode rd");
											}

											SendChatCommand(User,"All slots are held.");
										}
									}									
								}
							}
							else {
								m_FreeReady=false;
								SendChatCommand(User,"Challenge started...");
							}
						}
					}

					if (m_GHost->commands[8])
					if (Command=="sign" && m_Challenger!=User && m_Challenged!=User ) {
						if (Whisper) {
							SendChatCommand(User,"You can't sign via whisper.");
						}
						else {
							if (Payload=="obs" && !IsInPool(User)) {										
								if (m_meplay[10][0].empty()) {
									QueueChatCommand(User + " hold first obs slot.");
									m_meplay[10][0]=User;
								}
								else if (m_meplay[11][0].empty() && m_meplay[10][0]!=User ) {
									QueueChatCommand(User + " hold second obs slot.");
									m_meplay[11][0]=User;
								}
								else {
									SendChatCommand(User, "All slots held");
								}
							}
							else {
								if (m_GHost->m_PoolLimit == GetPoolHoldedSlots( )) {
									SendChatCommand(User,"Pool list is full.");
									return;
								}

								if ((Info->GetCountry( ) == m_GHost->m_CountrySign || m_GHost->m_CountrySign =="??" || m_GHost->m_CountrySign.empty()) || Info->GetLvl()>1 ) {
									if (IsInPool(User)) {
										SendChatCommand(User,"You can 't sign. You are already held.");
										SendChatCommand(User,"Type "+UTIL_ToString(m_CommandTrigger)+"out if you want to unsign.");
									}
									else {
										CDBBan *Ban = IsBannedName( User );
										if (Ban ) {
											SendChatCommand(User,"You is banned and can 't sign!!!");						
										}
										else {	
											if (Payload == "cm" || Payload == "cd") {
												Sign(User, Payload);
											}
											else {
												Sign(User, "-");
											}

											if (GetPoolHoldedSlots() == 8 && m_ChallengeStep < 3) {
												QueueChatCommand("Ready to start pick. Type " + UTIL_ToString(m_CommandTrigger) + "startpick.");
											}
										}									
									}
								}
								else {
									SendChatCommand(User, "Only " + m_GHost->m_CountrySign + " ppl can sign.");
								}
							}
						}
					}

					if (m_GHost->commands[8])
					if (Command=="mode") {
						if (IsInPool(User)) {
							if (Payload=="cd" || Payload=="cm") {
								SendChatCommand(User,"Setting up your mode vote to "+Payload);
								ChangeMode(User,Payload);
							}
							else {
								SendChatCommand(User,"Your mode vote deleted. Available modes are cm and cd");
								ChangeMode(User,"-");
							}
						}
						else {
							SendChatCommand(User, "You must first sign to use this command.");
						}
					}

					if (Command=="out" && Payload.empty()) {
						
						if ((m_Challenger!=User && m_Challenged!=User && m_ChallengeStep<3) || !m_Challenge) {
							if (IsInPool(User)) {
								UnSign(User);
								SendChatCommand(User, "Unsigned!!!");
							}
							else {
								SendChatCommand(User, "You aren't signed.");
							}

							for (int i=0; i<12; i++) {								
								if (m_meplay[i][0]==User) {
									if (m_meplayppl < m_ReservesNeeded) {
										m_FreeReady = false;
									}
									
									m_meplay[i][0]="";
									m_meplayppl--;
									break;						
								}							
							}
						}
					}

					if (m_GHost->commands[8])
					if (Command=="startpick" && m_ChallengeStep==2 && GetPoolHoldedSlots( )>7 && (m_Challenger==User || m_Challenged==User)) {
						SAnnounce( 0, string( ) );
						QueueChatCommand("Type "+UTIL_ToString(m_CommandTrigger)+"pick name or r (for random). To see users type "+UTIL_ToString(m_CommandTrigger)+"pool.");
						m_PickUser = rand()%2;
						m_PickStep = 1;
						m_holdnumb1 = 1;
						m_holdnumb2 = 6;
						if (m_PickUser == 0) {
							QueueChatCommand(m_Challenger + " has been chosen to start the pick.");
						}
						else {
							QueueChatCommand(m_Challenged + " has been chosen to start the pick.");
						}

						m_ChallengeTimers=false;
						if (!m_ChannelModerate) {
							ChannelModerate();
						}
						
						SendChatCommand("/voice "+m_Challenger);
						SendChatCommand("/voice "+m_Challenged);
						
						m_ChallengeStep=3;						
					}

					if (m_GHost->commands[8])
					if ((Command=="pick" ||Command=="p") && m_Challenge && m_ChallengeStep==3 && !Payload.empty() && (m_Challenger==User || m_Challenged==User)) {	
						if (Payload == "r" || Payload == "rand") {
							Pick(User, GetRandomUser(), true);
						}
						else {
							transform( Payload.begin( ), Payload.end( ), Payload.begin( ), (int(*)(int))tolower );
							uint32_t Matches=0;
							string TestName;
							string LastMatch;
							for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
								TestName = (*i)->GetUser();
								transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

								if (TestName.find( Payload ) != string::npos && (*i)->GetSigned( )) 	{
									Matches++;
									LastMatch = (*i)->GetUser();
								}
							}
							if (Matches == 1) {
								Pick(User, LastMatch, false);
							}
							else if (Matches == 0) {
								SendChatCommand(User, "There is no signed user with this name. Pick again.");
							}
							else {
								SendChatCommand(User, "There are more than one users with this name. Pick again.");
							}
						}
					}
					
					if (m_GHost->commands[8])
					if (Command=="go" && m_ChallengeStep==4 && (m_Challenger==User||m_Challenged==User||Accesslvl>=3)) {
						m_VotedMode = GetMode();
						StartGame();
					}

					if (m_GHost->commands[8])
					if (Command=="abort" && Payload.empty() && m_Challenge && (m_Challenger==User || Accesslvl>=4 || m_Challenged==User)) {	
						QueueChatCommand("The challenge has been aborted.");
						ResetChallenge( false, true, true );						
					}
					
					if (m_GHost->commands[6])
					if ((Command == "slots" || Command == "teams" || Command =="reserves") && Payload.empty()) {

						if ((m_holdslots||m_Challenge)&&!Whisper) {
							uint32_t counter = 0;
							string slotsa;
							if (!m_Challenger.empty()) {
								slotsa = "1." + m_Challenger + " ";
							}
							for (int i=1; i<6; i++) {
								if (!m_meplay[i-1][0].empty()) {
									counter++;
									slotsa += UTIL_ToString(i)+"."+m_meplay[i-1][0]+" ";
									if (m_meplay[i - 1][1] != "-") {
										slotsa += "(" + m_meplay[i - 1][1] + ") ";
									}
								}
								
							}
							string slotsb;
							if (!m_Challenged.empty()) {
								slotsb = "6." + m_Challenged + " ";
							}
							for (int i=6; i<11; i++) {
								if (!m_meplay[i-1][0].empty()) {
									slotsb += UTIL_ToString(i)+"."+m_meplay[i-1][0]+" ";
									if (m_meplay[i - 1][1] != "-") {
										slotsb += "(" + m_meplay[i - 1][1] + ") ";
									}
								}
							}
							string obs;
							for (int i=11; i<13; i++) {
								if (!m_meplay[i - 1][0].empty()) {
									obs += UTIL_ToString(i) + "." + m_meplay[i - 1][0] + " ";
								}
							}

							if (counter == 0) {
								SendChatCommand(User, "No slot held");
							}
							else {
								SendChatCommand(User, "Sentinel: " + slotsa);
							}

							if (slotsb != "") {
								SendChatCommand(User, "Scourge: " + slotsb);
							}
							if (!obs.empty()) {
								SendChatCommand(User, "Obs: " + obs);
							}
						}
						else {							
							SendChatCommand(User,"No slots held");							
						}
					}

					if (m_GHost->commands[5])
					if (Command == "info" ) {
						uint32_t Hours = GetHoursSince1970();
						if (Payload.empty()) {
							Payload = User;
						}
						string challinfo;
						string infostats;
						string warnstats;
						
						CDBInfo *Temp = IsInfo( Payload );				
						if (Temp ) {	
							challinfo = "["+Payload +"] vouched by "+Temp->GetAdmin()+ ".";
							for (vector<CDBChannel *> ::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
								if ((*i)->GetUser() == Payload) {
									challinfo += " Idle for " + UTIL_HMSToString(GetTime() - (*i)->GetSec()) + ".";
								}
							}

							infostats="lvl("+ UTIL_ToString(Temp->GetLvl())+") captain wins: "+UTIL_ToString(Temp->GetChallwins())+" losses: "+UTIL_ToString(Temp->GetChallloses())+". ";
							if (Temp->GetCountry() != "??") {
								infostats += "from(" + Temp->GetCountry() + ") ";
							}
							
							if (Temp->GetGinfo() != "" && Temp->GetGinfo() != "-") {
								infostats += "Info: " + Temp->GetGinfo();
							}
							infostats+=" First join ("+Temp->GetDate()+") ";
							if (Temp->GetMessage() == 0) {
								infostats += "(Announce off)";
							}
						
							SendChatCommand(User, challinfo);
							SendChatCommand(User, infostats);
							SendChatCommand(User,"Priv Rank("+UTIL_ToString(Temp->GetPrivRank())+") points("+UTIL_ToString(Temp->GetPrivPoints())+")");
							if (m_GHost->m_PrintPubLadder) {
								SendChatCommand(User, "Pub Rank(" + UTIL_ToString(Temp->GetPubRank()) + ") points(" + UTIL_ToString(Temp->GetPubPoints()) + ")");
							}
						}
						else {
							SendChatCommand(User, "No info for user " + Payload);
						}
						
						CDBWarn *Twarn = IsWarn(Payload);
						if (Twarn) {
							warnstats="Warns info: Total("+UTIL_ToString( Twarn->GetTotalwarn( ))+") ";
							if (Twarn->GetWarnings() != 0) {
								warnstats += "Warnings(" + UTIL_ToString(Twarn->GetWarnings()) + ") ";
							}
							if (Twarn->GetWarning() != ""&&Twarn->GetWarning() != "-") {
								warnstats += "Reason(" + Twarn->GetWarning() + ") ";
							}
							warnstats+=" last warn from("+Twarn->GetAdmin( )+").";

							if (Twarn->GetDaysban()>0) {
								if (Twarn->GetDaysban() - Hours < 337 && Twarn->GetDaysban() - Hours != 0) {
									SendChatCommand(User, UTIL_ToString(Twarn->GetDaysban() - Hours) + " hour left till unban.");
								}
								else {
									SendChatCommand(User,"Unbanned!!!");
									SendChatCommand("/unban "+User);
									m_PairedWarnChannelBans.push_back( PairedWarnChannelBan( Whisper ? User : string( ), m_GHost->m_DB->ThreadedWarnChannelBan( m_Server, User, 0, "Bot" ) ) );
								}
							}
						}
						else {
							warnstats = "No warnings in db.";
						}

						if (!infostats.empty()) {
							SendChatCommand(User, warnstats);
						}

						CDBBan *Ban = IsBannedName( Payload );	
						if (Ban) {
							SendChatCommand(User, "BANNED!!! from " + Ban->GetAdmin() + " at " + Ban->GetDate() + " reason " + Ban->GetReason());
						}	
					}

					if (m_GHost->commands[4])
					if (Command == "stats" || Command == "s" ) {
						if (Payload.empty()) {
							Payload = User;
						}
						m_PairedGPSChecks.push_back( PairedGPSCheck( User, m_GHost->m_DB->ThreadedGamePlayerSummaryCheck( Payload ) ) );						
					}

					if (m_GHost->commands[3])
					if (Command == "statsdota" || Command == "sd" ) {
						if (Payload.empty()) {
							Payload = User;
						}
						m_PairedDPSChecks.push_back( PairedDPSCheck( User, m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( Payload ) ) );
						m_PairedKillerChecks.push_back( PairedKillerCheck( User, m_GHost->m_DB->ThreadedDotAPlayerKillerCheck( Payload,0 ) ) );
						m_PairedFarmerChecks.push_back( PairedFarmerCheck( User, m_GHost->m_DB->ThreadedDotAPlayerFarmerCheck( Payload,0 ) ) );
					}

					if (m_GHost->commands[3])
					if (Command == "killer" || Command == "ki" ) {
						m_PairedKillerChecks.push_back( PairedKillerCheck( User, m_GHost->m_DB->ThreadedDotAPlayerKillerCheck( "",0 ) ) );
					}

					if (m_GHost->commands[3])
					if (Command == "farmer" || Command == "fa" ) {
						if (Payload.empty()) {
							Payload = User;
						}
						m_PairedFarmerChecks.push_back( PairedFarmerCheck( User, m_GHost->m_DB->ThreadedDotAPlayerFarmerCheck( "",0 ) ) );
					}

					if (Command == "version") {
						SendChatCommand(User, "Eurobattle.net guard bot V."+ m_GHost->m_Version+m_GHost->m_Versionb );			
					}
				}
		
				/*********************
				*    free for all    *
				*********************/				
				if (m_GHost->commands[62])
				if (Command=="time") {
					SendChatCommand(User,"Bot running for: "+UTIL_ToDayTimSec(GetTime( )-m_StartTime)+".");
					SendChatCommand(User,"Server running for:"+UTIL_ToDayTimSec(GetTime())+".");
					SendChatCommand(User,"Local time of server:"+LocalTime());
				}

				if (m_GHost->commands[2])
				if (Command=="help") {
					if (Payload.empty()) {
						SendChatCommand(User,"Type !help + all or access lvl. ex.(!help 2 ... !help all)");
					}
					else {
						SendChatCommand(User,"lvl access - (shortcommand) fullcommand - info.");
						if (Accesslvl==5 && (Payload=="all" || Payload=="5")) {
							SendChatCommand(User,"All command with * can be run by rootadmin only.");
							
							SendChatCommand(User,"5 - com + id + on, off or print - disable enable or print command option. ATTENTION ... Do not use this command. Its for tech support.");
							SendChatCommand(User,"5 - *ip - Print ip of server which bot run.");
							SendChatCommand(User,"5 - *exit - Close bot.");
							SendChatCommand(User,"5 - *query + query - Run query direct to db. Don t use it if you don t know what to do and the stracture of db. It may destroy all bot.");
							SendChatCommand(User,"5 - *fu + username - It spam user 50 lines of spam :P admin may ban you for doing this :P.");
							SendChatCommand(User,"5 - *addhostbot - Mark account as hostbot (lvl 5).");
							SendChatCommand(User,"5 - *delhostbot...");
							SendChatCommand(User,"5 - *fcal - db full calculation. It will crash bot for some min. Don t use it if you can t restart bot from server.");
							SendChatCommand(User,"5 - *mepro - Give user lvl 4 access.");
							SendChatCommand(User,"5 - *delinfo - Remove user from entire db. After that dbfix query must run. Else buggs will appear. Better clvl user 0 than del him");
							SendChatCommand(User,"5 - keys - Print some alt keys.");
							SendChatCommand(User,"5 - rootonly - Only lvl 4 can run commands.");
							SendChatCommand(User,"5 - addbot - Mark account as bot (lvl 6) For pub channels.");
							SendChatCommand(User,"5 - chat - Allow or not ppl to join with chat clients like topaz.");
							SendChatCommand(User,"5 - say + message - Bot say whatever you want");
							
							SendChatCommand(User,"5 - channel + name - Bot change channel.");
							SendChatCommand(User,"5 - slap - noob command :P ...");
							SendChatCommand(User,"5 - refresh + (all or infos or warns or bans or icons or cfg or php) - Refresh vectores.");							
							SendChatCommand(User,"5 - cp - print challenge users + info + gameid.");
							
							SendChatCommand(User,"5 - cal - Rank all users. Calculate.");
							SendChatCommand(User,"5 - (mg) maxgames - Shows the number of game that bot create till open.");
							SendChatCommand(User,"5 - users - Print info about users in channel.");
							SendChatCommand(User,"5 - news - Enable/disable print of news when user join the channel.");
							
							SendChatCommand(User,"5 - refresh icons - refresh entire bnet icons");
							SendChatCommand(User,"5 - clricons - remove icons from all users and clear icon file from file.");
							SendChatCommand(User,"5 - it - show remain time till icon autorefresh.");
							
							SendChatCommand(User,"5 - (dgl) delgamelist - Clear game list.");	
						}
						if (Accesslvl>2 && Accesslvl<6 && (Payload=="all" || Payload=="4")) {
					
							SendChatCommand(User,"4 - start + hostbotname - start game if its in lobby");
							SendChatCommand(User,"4 - ipf + ip - print full country name that ip comes from.");
							SendChatCommand(User,"4 - ips + ip - print short 2 letters country...");
							SendChatCommand(User,"4 - chacc + newname - Move your access to other account.");
							SendChatCommand(User,"4 - icons - print all users with their icon.");
							SendChatCommand(User,"4 - flames - Print all flamelist.");
							SendChatCommand(User,"4 - addflame - Add flame to flamelist.");
							SendChatCommand(User,"4 - showm - Print muted ppl.");
							SendChatCommand(User,"4 - mute + user - Mute user.");
							SendChatCommand(User,"4 - unmute + user ...");
							SendChatCommand(User,"4 - wtg + message - Announce to all users with lvl 0. Usefull for testgames.");
							SendChatCommand(User,"4 - fm + message - Announce to all user in safe list lvl > 0.");
							SendChatCommand(User,"4 - cm + country + message - Announce to user lvl>0 in that country ex.!cm gr message.");
							SendChatCommand(User,"4 - delwarn +user - Delete warns of a user.");
							SendChatCommand(User,"4 - kickall - Kick all user lvl<3 from channel.");
							SendChatCommand(User,"4 - mod - Moderate channel.");
							SendChatCommand(User,"4 - (v) voice +user- Give voice to someone.");
							SendChatCommand(User,"4 - (dev) devoice +user - Take voice away from someone.");
							SendChatCommand(User,"4 - sayingames + message - Print message in all hostbots in all games.");
							SendChatCommand(User,"4 - (t) topic + message - Put topic in channel.");
							SendChatCommand(User,"4 - (an) announce + time + message - Print every x sec the message.");
							SendChatCommand(User,"4 - hostbots - Print all hostbots account.");
							SendChatCommand(User,"4 - bots - Print all bots account.");
							SendChatCommand(User,"4 - kickbots - Kick all bots from channel. Not hostbots...");
							SendChatCommand(User,"4 - minfo - Print all users who deny announce.");
							SendChatCommand(User,"4 - ginfo - Print all users with general info.");
							SendChatCommand(User,"4 - (tt) ttopic - Add temp topic.");
							SendChatCommand(User,"4 - minslots + number - Signs needed to create pub priv game by lvl 1 users.");
							SendChatCommand(User,"4 - hold + user + slot - Hold slot for user. If no challenge.");
							SendChatCommand(User,"4 - afk + user - Print user 's idle time.");
							SendChatCommand(User,"4 - afk - Kick all users who idle for more than 10min (lvl <3).");
							SendChatCommand(User,"4 - ban delban + user ...");
							SendChatCommand(User,"4 - users - Print users info who are in channel.");
							SendChatCommand(User,"4 - lvl + number - Print all users with that lvl.");
							SendChatCommand(User,"4 - clvl + user + number - Change access lvl of a user.");
							SendChatCommand(User,"4 - addinfo + user - Add general info to a user.");
							SendChatCommand(User,"4 - warn - Print all users with warns.");
							SendChatCommand(User,"4 - warn + user - Add warn to a user.");
							SendChatCommand(User,"4 - cban + user + time - Channel ban user for x hours.");
							SendChatCommand(User,"4 - from + user - Print country that user play from.");
							SendChatCommand(User,"4 - (k) kick + user - Kick user from channel.");
							SendChatCommand(User,"4 - bans - Print number of bans.");
							SendChatCommand(User,"4 - mails - Print number of mails in db.");
							SendChatCommand(User,"4 - infos - Print number of users with info in db.");
							SendChatCommand(User,"4 - warns - Print number of warns in db.");
							SendChatCommand(User,"4 - dbstatus - Print db info.");							
							SendChatCommand(User,"4 - clrobs - Clear obs slots.");
							SendChatCommand(User,"4 - clrslots - Clear held slots (work only if there isn't challenge in progress).");
							SendChatCommand(User,"4 - unhost + hostbotname - unhost game from lobby.");
							SendChatCommand(User,"4 - unhostall - unhost all games from lobby.");
							SendChatCommand(User,"4 - autospam + number - autospam held slots in channel in x sec.");
							SendChatCommand(User,"4 - privgoby + hostbot + username - Create priv game with (username) owner.");
							SendChatCommand(User,"4 - chacc + oldnick + newnick - Change access lvl from an account to new one.");
							SendChatCommand(User,"4 - delrmk - Clear rmk pool");
							SendChatCommand(User,"4 - fmslap - Slap all safelisted users");
						}
						if (Accesslvl>1 && Accesslvl<6 && (Payload=="all" || Payload=="2"|| Payload=="3")) {
							//clrslots sp lvl info checkban checkwarn challenge(c)
							SendChatCommand(User,"2-3 - fm + message - Announce to all user in safe list lvl > 0.");
							SendChatCommand(User,"2-3 - spam - Call users to sign for the challenge.");							
							SendChatCommand(User,"2-3 - lvl + user  - Print user's lvl.");
							SendChatCommand(User,"2-3 - (c) challenge <nick> - Challenge another.");
							SendChatCommand(User,"2-3 - (c) challenge - Offer a challenge to all captains.");							
						}
						if (Accesslvl>0 && Accesslvl<6  &&(Payload=="all" || Payload=="1")) {

							SendChatCommand(User,"1 - userson - Shows online users.");
							SendChatCommand(User,"1 - ss - Shows number of currently safelisted users.");
							SendChatCommand(User,"1 - rank <number> - Shows user with that rank.");
							SendChatCommand(User,"1 - top - Shows top 10 users on ladder.");						
							SendChatCommand(User,"1 - pubgo - Starts a public game if there are at least 5 signed users.");
							SendChatCommand(User,"1 - privgo - Stards a private game if there are at least 5 signed users.");
							SendChatCommand(User,"1 - pool - Shows signed users in the current challenge.");
							SendChatCommand(User,"1 - out - Rmv your sign.");
							SendChatCommand(User,"1 - teams - Shows users who hold slots for the game.");
							SendChatCommand(User,"1 - info <nick> - Shows basic information about desired user.");
							SendChatCommand(User,"1 - (sd) statsdota - Shows your stats.");
							SendChatCommand(User,"1 - (ve) version - Print bot version.");
							SendChatCommand(User,"1 - find + <part of a nick> - Searches for user.");
							SendChatCommand(User,"1 - rank + number - Print users with that rank.");
							SendChatCommand(User,"1 - priv + gamename - Create private game with random hostbot (*).");
							SendChatCommand(User,"1 - pub + gamename - ...");
							SendChatCommand(User,"1 - root - Print root admins of this channel/league.");
							SendChatCommand(User,"1 - mail + user + message - Send mail to someone.");
							SendChatCommand(User,"1 - mailbox - Print your mails.");
							SendChatCommand(User,"1 - delmail + id - Delete a mail.");
							SendChatCommand(User,"1 - inform - Inform you about all running games.");
							SendChatCommand(User,"1 - users - Inform you about users number and state of games.");
							SendChatCommand(User,"1 - where + user - Inform you where that user is.");
							SendChatCommand(User,"1 - dm - delete your custom map config.");
							SendChatCommand(User,"1 - ml - Print all available maps.");
							SendChatCommand(User,"1 - map + mapname - Set your custom map.");
							SendChatCommand(User,"1 - unhost - Unhost the game that you create.");
							SendChatCommand(User,"1 - hbots - Print hostbots state.");
						}
						if (Accesslvl>=0) {
							SendChatCommand(User,"free - time - Print time that bot running.");
							SendChatCommand(User,"free - sd - Print statsdota of user");
							SendChatCommand(User,"free - (ki) killer - Print community best killer.");
							SendChatCommand(User,"free - (fa) farmer - Print community best farmer.");
							SendChatCommand(User,"free - site - Print community site url");
							SendChatCommand(User,"free - (m) message + on or off - Enable disable announce");
						}
					}
				}
				
				if (Command=="fix" && m_Infos.size( )==0) {
					m_CallableBanList = m_GHost->m_DB->ThreadedBanList( m_Server );
					m_CallableInfoList = m_GHost->m_DB->ThreadedInfoList( m_Server );
					m_CallableWarnList = m_GHost->m_DB->ThreadedWarnList( m_Server );
					SendChatCommand(User,"Trying connect 2 db 2 Refresh vectors.");
					DelayMs(4000);
					for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++ ) {
						if ((*i)->GetLvl()>0) {
							SendChatCommand("/unban "+(*i)->GetUser( ));							
						}
						DelayMs(30);
					}	
					m_LastGoPressed = GetTime();
				}

				if (Command=="site" && Payload.empty( )) {
					SendChatCommand(User,"Forum url is "+m_GHost->m_ForumUrl);
					SendChatCommand(User,"Stats url is "+m_GHost->m_StatsUrl);	
				}

				if (m_GHost->commands[1])
				if ((Command=="message" || Command=="m") && !Payload.empty()) {
					if (Payload=="off") {
						SendChatCommand(User,"You disable announce system from your account.");
						m_PairedInfoUpdateMessages.push_back( PairedInfoUpdateMessage( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateMessage( m_Server, User,0 ) ) );
					}
					else if (Payload=="on") {
						SendChatCommand(User,"You enable announce system from your account");
						m_PairedInfoUpdateMessages.push_back( PairedInfoUpdateMessage( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoUpdateMessage( m_Server, User,1) ) );
					}
				}
			}		
		}
	}
	else if (Event == CBNETProtocol::EID_JOIN ) {
		if (m_GHost->m_MailEnabled) {
			PrintMails(User, User, false, false);
		}

		//print news from text file(default news.txt
		if (m_PrintNews) {
			ifstream in;
			in.open( m_GHost->m_News.c_str( ) );
			if (!in.fail( ) ) {
				string Line;
				while( !in.eof( ) ) {
					getline( in, Line );
					if (!Line.empty()) {
						SendChatCommand(User, Line);
					}

					if (in.eof()) {
						break;
					}
				}
				in.close();
			}
		}

		uint32_t Userlvl=1;
		bool banned=false;
		CDBInfo *Info = IsInfo( User );
		if (Info ) {
			Userlvl=Info->GetLvl();
			if (Userlvl==0 && m_Infos.size( )>0) {
				//if lvl 0 join
				if (m_GHost->m_IsOperator) {
					SendChatCommand("/ban " + User + " Autoban lvl 0 user.");
				}
				else {
					SendChatCommand("/kick " + User + " Autokick lvl 0 user");
				}
			}
			else if (Userlvl==6) {
				//if hostbot
				SendChatCommand("/voice "+User);
			}
			else if (Userlvl>0 && Userlvl<4) {
				//if ss user join
				string UserName;
				string::size_type Split = Message.find( " " );

				if (Split != string::npos) {
					UserName = Message.substr(0, Split);
				}
				else {
					UserName = Message.substr(0);
				}

				if (!m_GHost->m_AllowTopaz && Message == "TAHC") {
					//if he use chat prog
					SendChatCommand("/kick " + User + " autokick chat clients with 3<lvl");
				}
			}
			else if (Userlvl >= 4) {
				SendChatCommand("/voice " + User);
			}
		}
		else {
			if (m_GHost->m_DefaultAutoLvl==0 && m_Infos.size( )!=0) {
				//default autolvl i put it in cfg 
				
				ifstream in;
				in.open( m_GHost->m_FirstWelcomeChannel.c_str( ) );

				if (in.fail( ) ) {
					if (m_GHost->m_IsOperator) {
						SendChatCommand("/ban "+User+" Trying to join channel for first time. (autoban)");
						SendChatCommand(User,"Hi. You are trying to join "+ m_GHost->m_CommunityName+" community for the first time.");
						SendChatCommand(User,"You autotake lvl "+UTIL_ToString(m_GHost->m_DefaultAutoLvl)+" access (autoban).");
						SendChatCommand(User,"If you want to join our community visit our forum for access request.");
						SendChatCommand(User,m_GHost->m_ForumUrl);
						SendChatCommand(User,"For any question whisper "+m_RootAdmin+" !!!");
					}
					else {
						SendChatCommand("/kick "+User+" Trying to join channel for first time. (autokick)");
						SendChatCommand(User,"Hi. You are trying to join "+ m_GHost->m_CommunityName+" community for the first time.");
						SendChatCommand(User,"You autotake lvl "+UTIL_ToString(m_GHost->m_DefaultAutoLvl)+" access (autokick).");
						SendChatCommand(User,"If you want to join our community visit our forum for access request.");
						SendChatCommand(User,m_GHost->m_ForumUrl);
						SendChatCommand(User,"For any question whisper "+m_RootAdmin+" !!!");
					}
				}				
				else {						
					// custom welcome message
					// don't print more than 8 lines
					uint32_t Count = 0;
					string Line;

					while( !in.eof( ) && Count < 8 ) {
						getline( in, Line );

						if (in.eof()) {
							break;
						}

						if (Count==0) {
							if (m_GHost->m_IsOperator) {
								SendChatCommand("/ban " + User + " " + Line);
							}
							else {
								SendChatCommand("/kick " + User + " " + Line);
							}
						}
						else {
							if (!Line.empty()) {
								SendChatCommand(User, Line);
							}
						}

						Count++;
					}

					in.close();
				}
			}
			else {
				if (m_GHost->commands[74]) {
					ifstream in;
					in.open( m_GHost->m_FirstWelcomeChannel.c_str( ) );

					if (in.fail( ) ) {
						SendChatCommand(User," Welcome to "+ m_GHost->m_CommunityName+" community.");
						SendChatCommand(User," You join channel for first time. GL & HF");
						SendChatCommand(User," You autotake command lvl "+UTIL_ToString(m_GHost->m_DefaultAutoLvl)+".");
						SendChatCommand(User," If you don't want receive announce type !m off");
						SendChatCommand(User," For any question whisper "+m_RootAdmin+" !!!");
					}				
					else {						
						// custom welcome message
						// don't print more than 8 lines
						uint32_t Count = 0;
						string Line;

						while( !in.eof( ) && Count < 8 ) {
							getline( in, Line );

							if (in.eof()) {
								break;
							}
						
							if (!Line.empty()) {
								SendChatCommand(User, Line);
							}

							Count++;
						}
						in.close();
					}					
				}
			}
			m_PairedInfoAdds.push_back( PairedInfoAdd( Whisper ? User : string( ), m_GHost->m_DB->ThreadedInfoAdd( m_Server, User, m_GHost->m_DefaultAutoLvl, m_Infos.size( )+1,m_Infos.size( )+1,0,0, m_GHost->m_CommunityNameLower, "??", 0, 0, "-",1 ) ) );
		}

		//check if joined player is banned
		CDBBan *Ban = IsBannedName( User );
		if (Ban) {
			banned = true;
		}

		//check if join user join with chat program
		bool topaz=false;
		if (Message == "TAHC") {
			topaz = true;
		}

		//add user in vector of channel	
		AddChannelUser(User,Userlvl,banned,topaz);		
		
		//send temporary topic to everyone who join channel
		if (!m_TempTopic.empty()) {
			SendChatCommand(User, m_TempTopic);
		}

		//channel ban check
		CDBWarn *temp=IsWarn(User);
		if (temp) {
			if (temp->GetDaysban()>0) {
				if (m_GHost->m_IsOperator) {
					SendChatCommand("/ban " + User + " You are channel banned!!!");
				}
				else {
					SendChatCommand("/kick " + User + " You are channel banned!!!");
				}
			}
		}

		//send info to users if challenge in progress
		if (m_Challenge) {
			if (m_ChallengeStep==1 && m_Challenged.empty()) {
				SendChatCommand(User,"Type "+UTIL_ToString(m_CommandTrigger)+"accept if you want to challenge "+m_Challenger);
			}
			else if (m_ChallengeStep==2) {
				SendChatCommand(User,"Type "+UTIL_ToString(m_CommandTrigger)+"sign if you want to play. "+m_Challenger+" VS "+m_Challenged);
			}
		}

	}
	else if (Event == CBNETProtocol::EID_SHOWUSER ) {
		if (m_Infos.size() == 0) {
			return;
		}

		bool kick = false;
		CDBWarn *temp=IsWarn(User);
		if (temp) {
			if (temp->GetDaysban()>0) {
				if (m_GHost->m_IsOperator) {
					SendChatCommand("/ban " + User + " You are channel banned!!!");
				}
				else {
					SendChatCommand("/kick " + User + " You are channel banned!!!");
				}

				kick = true;
			}
		}
		bool banned = false;
		uint32_t Userlvl = 0;
		bool chatclient=false;		

		if (!m_GHost->m_AllowTopaz && Message=="TAHC") {
			SendChatCommand("/kick "+User+" autokick chat clients with 3<lvl");
			kick = true;
		}

		if (!kick) {
			if (Message == "TAHC") {
				chatclient = true;
			}

			CDBBan *Ban = IsBannedName( User );
			if (Ban) {
				banned = true;
			}
			
			CDBInfo *Info = IsInfo( User );	
			if (Info ) {
				if (Info->GetLvl()==0) {
					if (m_GHost->m_IsOperator) {
						SendChatCommand("/ban " + User + " autoban lvl 0 user.");
					}
					else {
						SendChatCommand("/kick " + User + " autokick lvl 0 user.");
					}
				}
				else {
					Userlvl=Info->GetLvl();				
				}			
			}
			else {
				if (m_GHost->m_DefaultAutoLvl==0) {
					if (m_GHost->m_IsOperator) {
						SendChatCommand("/ban " + User + " Autoban user. No info in db.");
					}
					else {
						SendChatCommand("/kick " + User + " Autokick user. No info in db.");
					}
				}
				else {
					Userlvl = m_GHost->m_DefaultAutoLvl;
				}
			}

			AddChannelUser( User, Userlvl, banned, chatclient );
		}
	}
	else if (Event == CBNETProtocol::EID_LEAVE ) {	

		for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); ) {
			if ((*i)->GetUser() == User) {
				i = m_Users.erase(i);
			}
			else {
				i++;
			}
		}	

		if (m_Challenge) {
			if (m_Challenge && (m_Challenger==User || m_Challenged==User)) {
				ResetChallenge( false, true, true );
				QueueChatCommand( User + " has left channel. Challenge aborted.");
			} 			
		}
		if (IsInPool(User)) {
			UnSign(User);
		}

		for (int i=0; i<12; i++) 	{
			if (m_meplay[i][0]==User) {
				if (m_ChallengeStep>2 && m_Challenge) {
					SendChatCommand( User,"Come back to channel. You picked for this challenge.");
				}
				else {
					SendChatCommand( User," Your slot reserve removed.");
					m_meplay[i][0]="";
					m_meplayppl--;
					break;
				}
			}
		}
	}
	else if (Event == CBNETProtocol::EID_CHANNEL ) {
		// keep track of current channel so we can rejoin it after hosting a game
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] joined channel [" + Message + "]", true );
		m_CurrentChannel = Message;
		m_Users.clear();		
		m_Users.push_back(new CDBChannel(m_UserName,GetTime(),100, false, false));
	}
	else if (Event == CBNETProtocol::EID_INFO ) {
	
		// extract the first word which we hope is the username
		// this is not necessarily true though since info messages also include channel MOTD's and such
		string UserName;
		string::size_type Split = Message.find( " " );

		if (Split != string::npos) {
			UserName = Message.substr(0, Split);
		}
		else {
			UserName = Message.substr(0);
		}

		if (m_OnlineChecking) {	
			if (Message.find("currently in channel") != string::npos && UserName != "User") {
				m_InChannelUsers.push_back(UserName);
			}
			else if (!Message.find("was last seen on") != string::npos && UserName != "User") {
				m_OnlineUsers.insert(UserName);
			}
		}
		else {
			CONSOLE_Print("[INFO: " + m_ServerAlias + " - ] " + Message, true);
		}

		if (Message.find("Channel is now moderated") != string::npos) {
			m_ChannelModerate = true;
		}
		
		if (Message.find("Channel is now unmoderated") != string::npos) {
			m_ChannelModerate = false;
		}
	}
	else if (Event == CBNETProtocol::EID_ERROR ) {
		
		if (!m_CountingStart) {
			//CONSOLE_Print( "[ERROR: " + m_ServerAlias + "] " + Message );
		}
		else {
			m_UsersOffBnet++;
		}
	}
	else if (Event == CBNETProtocol::EID_EMOTE ) {
		CONSOLE_Print( "[EMOTE: " + m_ServerAlias + "] [" + User + "] " + Message , true);
		m_GHost->EventBNETEmote( this, User, Message );
		FlameCheck(User, Message);		
	}
}

void CBNET::SendJoinChannel( string channel ) {
	if (m_LoggedIn && m_InChat) {
		m_Socket->PutBytes(m_Protocol->SEND_SID_JOINCHANNEL(channel));
	}
}

void CBNET::QueueEnterChat() {
	if (m_LoggedIn )
		m_OutPackets.push( m_Protocol->SEND_SID_ENTERCHAT( ) );
}

void CBNET::QueueChatCommand( string chatCommand )
{
	if (chatCommand.empty( ) )
		return;

	if (m_LoggedIn )
	{
		if (m_PasswordHashType == "pvpgn" && chatCommand.size( ) > m_MaxMessageLength )
			chatCommand = chatCommand.substr( 0, m_MaxMessageLength );

		if (chatCommand.size( ) > 255 )
			chatCommand = chatCommand.substr( 0, 255 );

		if (m_OutPackets.size( ) > 10 )
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] attempted to queue chat command [" + chatCommand + "] but there are too many (" + UTIL_ToString( m_OutPackets.size( ) ) + ") packets queued, discarding" , true);
		else
		{
			CONSOLE_Print( "[QUEUED: " + m_ServerAlias + "] " + chatCommand , true);
			m_OutPackets.push( m_Protocol->SEND_SID_CHATCOMMAND( chatCommand ) );
		}
	}
}
//add by me
void CBNET::SendChatCommand(string user, string chatCommand )
{
	// don't call this function directly, use QueueChatCommand instead to prevent getting kicked for flooding
	
	if (chatCommand.empty( ))
	{
		CONSOLE_Print("Empty message from ("+user+")",false);
		return;
	}
	
	if (m_LoggedIn )
	{
		chatCommand="/w "+user+" "+chatCommand;
		if (m_PasswordHashType == "pvpgn" && chatCommand.size( ) > m_MaxMessageLength )
			chatCommand = chatCommand.substr( 0, m_MaxMessageLength );

		if (chatCommand.size( ) > 255 )
			chatCommand = chatCommand.substr( 0, 255 );

		m_Socket->PutBytes( m_Protocol->SEND_SID_CHATCOMMAND( chatCommand ) );
	}
}

//Don't call this function directly, use QueueChatCommand instead to prevent getting kicked for flooding
void CBNET::SendChatCommand(string chatCommand) {
	
	if (!m_OnlineChecking) {
		CONSOLE_Print("[SERVER COMMANDS: " + m_ServerAlias + "] " + chatCommand, true);
	}

	if (m_LoggedIn) {		
		if (m_PasswordHashType == "pvpgn" && chatCommand.size() > m_MaxMessageLength) {
			chatCommand = chatCommand.substr(0, m_MaxMessageLength);
		}

		if (chatCommand.size()>255) {
			chatCommand = chatCommand.substr(0, 255);
		}

		m_Socket->PutBytes(m_Protocol->SEND_SID_CHATCOMMAND( chatCommand ));
	}
}

void CBNET::QueueChatCommand( string chatCommand, string user, bool whisper )
{
	if (chatCommand.empty()) {
		return;
	}

	// if whisper is true send the chat command as a whisper to user, otherwise just queue the chat command
	if (whisper) {
		QueueChatCommand("/w " + user + " " + chatCommand);
	}
	else {
		QueueChatCommand(chatCommand);
	}
}

void CBNET::UnqueuePackets(unsigned char type) {
	queue<BYTEARRAY> Packets;
	uint32_t Unqueued = 0;

	while (!m_OutPackets.empty()) {
		// todotodo: it's very inefficient to have to copy all these packets while searching the queue
		BYTEARRAY Packet = m_OutPackets.front();
		m_OutPackets.pop();

		if (Packet.size() >= 2 && Packet[1] == type) {
			Unqueued++;
		}
		else {
			Packets.push(Packet);
		}
	}

	m_OutPackets = Packets;

	if (Unqueued > 0) {
		CONSOLE_Print("[BNET: " + m_ServerAlias + "] unqueued " + UTIL_ToString(Unqueued) + " packets of type " + UTIL_ToString(type), true);
	}
}

void CBNET::UnqueueChatCommand(string chatCommand) {
	// hackhack: this is ugly code
	// generate the packet that would be sent for this chat command
	// then search the queue for that exact packet

	BYTEARRAY PacketToUnqueue = m_Protocol->SEND_SID_CHATCOMMAND( chatCommand );
	queue<BYTEARRAY> Packets;
	uint32_t Unqueued = 0;

	while (!m_OutPackets.empty()) {
		// todotodo: it's very inefficient to have to copy all these packets while searching the queue

		BYTEARRAY Packet = m_OutPackets.front();
		m_OutPackets.pop();

		if (Packet == PacketToUnqueue) {
			Unqueued++;
		}
		else {
			Packets.push(Packet);
		}
	}

	m_OutPackets = Packets;

	if (Unqueued > 0) {
		CONSOLE_Print("[BNET: " + m_ServerAlias + "] unqueued " + UTIL_ToString(Unqueued) + " chat command packets", true);
	}
}

bool CBNET::IsRootAdmin( string name ) {
	// m_RootAdmin was already transformed to lower case in the constructor
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	// updated to permit multiple root admins seperated by a space, e.g. "Varlock Kilranin Instinct121"
	// note: this function gets called frequently so it would be better to parse the root admins just once and store them in a list somewhere
	// however, it's hardly worth optimizing at this point since the code's already written

	stringstream SS;
	string s;
	SS << m_RootAdmin;

	while(!SS.eof()) {
		SS >> s;
		transform( s.begin( ), s.end( ), s.begin( ), (int(*)(int))tolower );
		if (name == s) {
			return true;
		}
	}
	return false;
}

CDBBan *CBNET::IsBannedName(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	// todotodo: optimize this - maybe use a map?

	for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end(); i++ ) {
		if ((*i)->GetName() == name) {
			return *i;
		}
	}

	return NULL;
}

CDBBan *CBNET::IsBannedIP( string ip ) {
	// todotodo: optimize this - maybe use a map?
	for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end(); i++ ) {
		if ((*i)->GetIP() == ip) {
			return *i;
		}
	}

	return NULL;
}

void CBNET::AddBan(string name, string ip, string gamename, string admin, string reason) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	m_Bans.push_back( new CDBBan( m_Server, name, ip, GetDate( ), gamename, admin, reason ) );
}

void CBNET::RemoveBan( string name ) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end(); ) {
		if ((*i)->GetName() == name) {
			i = m_Bans.erase(i);
		}
		else {
			i++;
		}
	}
}

void CBNET::SAnnounce(uint32_t interval, string message) {
	m_AnnounceInterval = interval;
	m_AnnounceMessage = message;
	m_LastAnnounceTime = GetTime();
}

void CBNET::MassMessage(string name, string message, bool PassTimer,bool TestUsers) {
	m_MassMessageCalledBy = name;
	if (!TestUsers) {
		m_ssList = true;
		uint32_t ss=0;
		if ((m_SpamTime==0 || m_SpamTime+30<GetTime()) || PassTimer) {
			m_CountingStart=true;
			m_SpamTime=GetTime();
			for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
				if ((*i)->GetMessage()>0 && (*i)->GetLvl()>0 && (*i)->GetLvl()<6) {
					ss++;
					SendChatCommand((*i)->GetUser( ),"Message from "+name+": "+message);					
				}
			}	
			m_CounderStart=GetTime();
		}
		else {
			SendChatCommand(name, "You can use command every 30 sec. " + UTIL_ToString(30 - (GetTime() - m_SpamTime)) + " sec more.");
		}
	}
	else {
		m_ssList = false;
		m_CountingStart=true;
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
			if ((*i)->GetMessage() == 1 && (*i)->GetLvl() == 0 && (*i)->GetAdmin() == m_GHost->m_CommunityNameLower) {
				SendChatCommand((*i)->GetUser(), name + ": " + message);
			}
		}
		m_CounderStart=GetTime();
		SendChatCommand(name,name+": "+message);
		SendChatCommand(name,"Whispering lvl 0 users.");
	}
}

void CBNET::ResetChallenge(bool resetobs, bool holdpicked, bool challenge) {
	
	SAnnounce( 0, string( ));

	m_FreeReady = false;
	m_meplayppl=0;
	m_meplayon=true;
	
	if (challenge || m_ChallengeStep > 2) {
		m_ChallengeTimerExtended = false;
		m_ChallengeTimersRemainSpam = 0;
		m_ChallengeTimers = false;

		if (m_ChannelModerate) {
			ChannelModerate();
		}
		CDBInfo *temp =IsInfo(m_Challenger);
		if (temp) {
			if (temp->GetLvl() < 4) {
				SendChatCommand("/devoice " + m_Challenger);
			}
		}
		CDBInfo *temp2 =IsInfo(m_Challenged);
		if (temp2) {
			if (temp2->GetLvl() < 4) {
				SendChatCommand("/devoice " + m_Challenged);
			}
		}

		m_VotedMode = "";
		m_ChallengeTimers=false;
		m_ChallengeStep=0;
		m_Challenge=false;
		m_Challenger="";
		m_Challenged="";
	}
	
	for (int i=0; i<10; i++) {
		if (holdpicked && !IsInPool(m_meplay[i][0])) {
			Sign(m_meplay[i][0], m_meplay[i][1]);
		}
		m_meplay[i][0]="";
		m_meplay[i][1] = "-";
	}

	m_SignLock = false;
	if (resetobs) {
		m_meplay[10][0]="";
		m_meplay[11][0]="";
	}
	
	m_holdslots=false;
}

void CBNET::TableSwap(string& a, string& b) {
    string temp;
    temp = a;
    a = b;
    b = temp;
}

void CBNET ::ChannelModerate() {		
	SendChatCommand("/moderate");	
}

void CBNET::AddChannelUser (string User, uint32_t Userlvl, bool banned, bool topaz) {
	bool UserInVector = false;
	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ ) {
		if ((*i)->GetUser( ) == User ) {
			UserInVector = true;
		}
	}			
	if (!UserInVector) {
		m_Users.push_back(new CDBChannel(User, GetTime(),Userlvl, banned, topaz));
	}
}

uint32_t CBNET::GetHoldedSlots() {
	uint32_t counter=0;
	for (int i=0;i<10;i++) {
		if (!m_meplay[i][0].empty()) {
			counter++;
		}
	}
	return counter;
}	

uint32_t CBNET::GetPoolHoldedSlots() {
	uint32_t counter=0;

	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
		if ((*i)->GetSigned()) {
			counter++;
		}
	}
	return counter;
}

bool CBNET::IsMail(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	// todotodo: optimize this - maybe use a map?
	for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++ ) {
		if ((*i)->GetReceiver() == name || (*i)->GetSender() == name) {
			return true;
		}
	}

	return false;
}

void CBNET::ReadedMail(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++) {
		if ((*i)->GetReceiver( ) == name )
			(*i)->SetReaded();
	}
}

void CBNET::RemoveMail(string name, uint32_t id) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); ) {
		if (((*i)->GetReceiver() == name || (*i)->GetSender() == name) && (*i)->GetId() == id) {
			i = m_Mails.erase(i);
		}
		else {
			i++;
		}
	}
}

void CBNET::AddMail(uint32_t id, string sender, string receiver, string message) {
	transform( sender.begin( ), sender.end( ), sender.begin( ), (int(*)(int))tolower );
	transform( receiver.begin( ), receiver.end( ), receiver.begin( ), (int(*)(int))tolower );
	m_Mails.push_back( new CDBMail( id, m_Server, sender, receiver, message, 0, GetDate( ) ) );
}

CDBInfo *CBNET::IsInfo( string name ) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	// todotodo: optimize this - maybe use a map?
	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
		if ((*i)->GetUser() == name) {
			return *i;
		}
	}

	return NULL;
}

void CBNET::AddInfo( string name, uint32_t lvl, uint32_t privrank, uint32_t pubrank, uint32_t privpoints, uint32_t pubpoints, string admin, string country, uint32_t challwins, uint32_t challloses, string ginfo, uint32_t message) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	transform( admin.begin( ), admin.end( ), admin.begin( ), (int(*)(int))tolower );
	m_Infos.push_back( new CDBInfo( GetInfoId( )+1, m_Server, name, lvl, privrank, pubrank, privpoints, pubpoints, admin, country, challwins, challloses, ginfo, message, GetDate( ) ) );
}

void CBNET::CalculatePrivInfo() {
	/*
	uint32_t points1=0;
	uint32_t points2=0;
	uint32_t rank1=0;
	uint32_t rank2=0;
	for (uint32_t j = 0; j <12;j++) 
	for (uint32_t i = 0; i < m_Infos.size( )-1; i++)
	{
		rank1 = m_Infos[i]->GetPrivRank();
		points1 = m_Infos[i]->GetPrivPoints();
		
		rank2 = m_Infos[j]->GetPrivRank();			
		points2 = m_Infos[j]->GetPrivPoints();
		if (( points1 < points2 && rank1 < rank2) || ( points1 > points2 && rank1 > rank2 ))
		{					
			m_Infos[i]->UpdatePrivRank(rank2);
			m_Infos[j]->UpdatePrivRank(rank1);
		}
	}	
	*/
}

void CBNET::CalculatePubInfo() {
	/*
	uint32_t points1=0;
	uint32_t points2=0;
	uint32_t rank1=0;
	uint32_t rank2=0;
	for (uint32_t j = 0; j <12;j++) 
	for (uint32_t i = 0; i < m_Infos.size( )-1; i++)
	{
		rank1 = m_Infos[i]->GetPubRank();
		points1 = m_Infos[i]->GetPubPoints();
		
		rank2 = m_Infos[j]->GetPubRank();			
		points2 = m_Infos[j]->GetPubPoints();
		if (( points1 < points2 && rank1 < rank2) || ( points1 > points2 && rank1 > rank2 ))
		{					
			m_Infos[i]->UpdatePubRank(rank2);
			m_Infos[j]->UpdatePubRank(rank1);
		}
	}	
	*/
}

void CBNET::PrivPointsInfo( string name, uint32_t privpoints) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	bool change = false;
	uint32_t id;
	string server;
	string admin;
	string ginfo;
	string country;
	uint32_t challwins;
	uint32_t challloses;
	uint32_t lvl;
	uint32_t privrank;
	uint32_t pubrank;
	uint32_t pubpoints;
	uint32_t message;
	string date;

	CDBInfo *Info = IsInfo( name );
	if (Info) {
		change = true;
		id = Info->GetId();
		server = Info->GetServer();
		admin = Info->GetAdmin();
		ginfo = Info->GetGinfo();
		country = Info->GetCountry();
		challwins = Info->GetChallwins();
		challloses = Info->GetChallloses();
		lvl = Info->GetLvl();
		privrank = Info->GetPrivRank();
		pubrank = Info->GetPubRank();
		pubpoints = Info->GetPubPoints();
		message = Info->GetMessage();			
		date = Info->GetDate();
	}

	if (change) {
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); ) {
			if ((*i)->GetUser() == name) {
				i = m_Infos.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Infos.push_back(new CDBInfo(id,server,name,lvl, privrank, pubrank, privpoints, pubpoints,admin,country, challwins, challloses,ginfo,message,date));
	}
}

void CBNET::PubPointsInfo( string name, uint32_t pubpoints) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	bool change = false;
	uint32_t id;
	string server;
	string admin;
	string ginfo;
	string country;
	uint32_t challwins;
	uint32_t challloses;
	uint32_t lvl;
	uint32_t privrank;
	uint32_t pubrank;
	uint32_t privpoints;
	uint32_t message;
	string date;

	CDBInfo *Info = IsInfo(name);
	if (Info) {
		change = true;
		id = Info->GetId();
		server = Info->GetServer();
		admin = Info->GetAdmin();
		ginfo = Info->GetGinfo();
		country = Info->GetCountry();
		challwins = Info->GetChallwins();
		challloses = Info->GetChallloses();
		lvl = Info->GetLvl();
		privrank = Info->GetPrivRank();
		pubrank = Info->GetPubRank();
		privpoints = Info->GetPrivPoints();
		message = Info->GetMessage();			
		date = Info->GetDate();
	}

	if (change) {
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); ) {
			if ((*i)->GetUser() == name) {
				i = m_Infos.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Infos.push_back(new CDBInfo(id,server,name,lvl, privrank, pubrank, privpoints, pubpoints,admin,country, challwins, challloses,ginfo,message,date));
	}
}

void CBNET::RemoveInfo(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); ) {
		if ((*i)->GetUser() == name) {
			i = m_Infos.erase(i);
		}
		else {
			i++;
		}
	}
}

void CBNET::UpdateCountryInfo(string name, string country)
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	//update info in vector of infos
	bool change = false;
	uint32_t id;
	string server;
	string admin;
	uint32_t challwins;
	uint32_t challloses;
	string ginfo;
	uint32_t lvl;
	uint32_t privrank;
	uint32_t pubrank;
	uint32_t privpoints;
	uint32_t pubpoints;
	uint32_t message;
	string date;

	CDBInfo *Info = IsInfo( name );
	if (Info) {
		change = true;
		id = Info->GetId();
		server = Info->GetServer();
		admin = Info->GetAdmin();
		challwins = Info->GetChallwins();
		challloses = Info->GetChallloses();
		ginfo = Info->GetGinfo();
		lvl = Info->GetLvl();
		privrank = Info->GetPrivRank();
		pubrank = Info->GetPubRank();
		privpoints = Info->GetPrivPoints();
		pubpoints = Info->GetPubPoints();
		message = Info->GetMessage();			
		date = Info->GetDate();
	}

	if (change) {
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); ) {
			if ((*i)->GetUser() == name) {
				i = m_Infos.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Infos.push_back(new CDBInfo(id,server,name,lvl,privrank,pubrank,privpoints, pubpoints,admin,country, challwins, challloses,ginfo,message,date));
	}	
}

void CBNET::ChallWinsInfo(string name, uint32_t challwins) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	//update info in vector of infos
	bool change = false;
	uint32_t id;
	string server;
	string admin;
	string country;
	uint32_t challloses;
	string ginfo;
	uint32_t lvl;
	uint32_t privrank;
	uint32_t pubrank;
	uint32_t privpoints;
	uint32_t pubpoints;
	uint32_t message;
	string date;

	CDBInfo *Info = IsInfo( name );
	if (Info) {
		change = true;
		id = Info->GetId();
		server = Info->GetServer();
		admin = Info->GetAdmin();
		country = Info->GetCountry();
		challloses = Info->GetChallloses();
		ginfo = Info->GetGinfo();
		lvl = Info->GetLvl();
		privrank = Info->GetPrivRank();
		pubrank = Info->GetPubRank();
		privpoints = Info->GetPrivPoints();
		pubpoints = Info->GetPubPoints();
		message = Info->GetMessage();			
		date = Info->GetDate();
	}

	if (change) {
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); ) {
			if ((*i)->GetUser() == name) {
				i = m_Infos.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Infos.push_back(new CDBInfo(id,server,name,lvl,privrank, pubrank, privpoints,pubpoints,admin,country, challwins, challloses,ginfo,message,date));
	}	
}

void CBNET::ChallLosesInfo(string name, uint32_t challloses) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	//update info in vector of infos
	bool change = false;
	uint32_t id;
	string server;
	string admin;
	string country;
	uint32_t challwins;
	string ginfo;
	uint32_t lvl;
	uint32_t privrank;
	uint32_t pubrank;
	uint32_t privpoints;
	uint32_t pubpoints;
	uint32_t message;
	string date;

	CDBInfo *Info = IsInfo( name );
	if (Info) {
		change = true;
		id = Info->GetId();
		server = Info->GetServer();
		admin = Info->GetAdmin();
		country = Info->GetCountry();
		challwins = Info->GetChallwins();
		ginfo = Info->GetGinfo();
		lvl = Info->GetLvl();
		privrank = Info->GetPrivRank();
		pubrank = Info->GetPubRank();
		privpoints = Info->GetPrivPoints();
		pubpoints = Info->GetPubPoints();
		message = Info->GetMessage();			
		date = Info->GetDate();
	}

	if (change) {
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); ) {
			if ((*i)->GetUser() == name) {
				i = m_Infos.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Infos.push_back(new CDBInfo(id,server,name,lvl,privrank, pubrank,privpoints, pubpoints,admin,country, challwins, challloses,ginfo,message,date));
	}	
}

void CBNET::UpdateLvlInfo(string name, uint32_t lvl, string admin) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	transform( admin.begin( ), admin.end( ), admin.begin( ), (int(*)(int))tolower );
	
	bool change = false;
	uint32_t id;
	string server;	
	string ginfo;
	string country;
	uint32_t challwins;
	uint32_t challloses;
	uint32_t privrank;
	uint32_t pubrank;
	uint32_t privpoints;
	uint32_t pubpoints;
	uint32_t message;
	string date;

	CDBInfo *Info = IsInfo( name );
	if (Info) {
		change = true;
		id = Info->GetId();
		server = Info->GetServer();
		country = Info->GetCountry();
		challwins = Info->GetChallwins();
		challloses = Info->GetChallloses();
		ginfo = Info->GetGinfo();
		privrank = Info->GetPrivRank();
		pubrank = Info->GetPubRank();
		privpoints = Info->GetPrivPoints();
		pubpoints = Info->GetPubPoints();
		message = Info->GetMessage();
		date = Info->GetDate();
	}

	if (change) {
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); ) {
			if ((*i)->GetUser() == name) {
				i = m_Infos.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Infos.push_back(new CDBInfo(id,server,name,lvl,privrank, pubrank, privpoints, pubpoints, admin,country, challwins, challloses,ginfo,message,date));
	}
}

void CBNET::UpdateMessageInfo(string name, uint32_t message) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	
	bool change = false;
	uint32_t id;
	string server;
	string admin;
	string ginfo;
	string country;
	uint32_t challwins;
	uint32_t challloses;
	uint32_t lvl;
	uint32_t privrank;
	uint32_t pubrank;
	uint32_t pubpoints;
	uint32_t privpoints;
	string date;

	CDBInfo *Info = IsInfo( name );
	if (Info) {
		change = true;
		id = Info->GetId();
		server = Info->GetServer();
		admin = Info->GetAdmin();
		ginfo = Info->GetGinfo();
		country = Info->GetCountry();
		challwins = Info->GetChallwins();
		challloses = Info->GetChallloses();
		lvl = Info->GetLvl();
		privrank = Info->GetPrivRank();
		pubrank = Info->GetPubRank();
		privpoints = Info->GetPrivPoints();
		pubpoints = Info->GetPubPoints();
		date = Info->GetDate();
	}

	if (change) {
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); ) {
			if ((*i)->GetUser() == name) {
				i = m_Infos.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Infos.push_back(new CDBInfo(id,server,name,lvl,privrank, pubrank, privpoints, pubpoints,admin,country, challwins, challloses ,ginfo,message,date));
	}
}

void CBNET::UpdateGinfoInfo(string name, string ginfo, string admin) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	transform( admin.begin( ), admin.end( ), admin.begin( ), (int(*)(int))tolower );
	
	bool change = false;
	uint32_t id;
	string server;
	string country;	
	uint32_t challwins;
	uint32_t challloses;
	uint32_t lvl;
	uint32_t privrank;
	uint32_t pubrank;
	uint32_t privpoints;
	uint32_t pubpoints;
	uint32_t message;
	string date;

	CDBInfo *Info = IsInfo( name );
	if (Info) {
		change = true;
		id = Info->GetId();
		server = Info->GetServer();
		lvl = Info->GetLvl();
		privrank = Info->GetPrivRank();
		pubrank = Info->GetPubRank();
		privpoints = Info->GetPrivPoints();
		pubpoints = Info->GetPubPoints();
		message = Info->GetMessage();
		country = Info->GetCountry();
		challwins = Info->GetChallwins();
		challloses = Info->GetChallloses();
		date = Info->GetDate();
	}

	if (change) {
		for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); ) {
			if ((*i)->GetUser() == name) {
				i = m_Infos.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Infos.push_back(new CDBInfo(id,server,name,lvl,privrank,pubrank,privpoints,pubpoints,admin,country, challwins, challloses,ginfo,message,date));
	}
}

CDBWarn *CBNET::IsWarn(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	// todotodo: optimize this - maybe use a map?
	for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); i++ ) {
		if ((*i)->GetName() == name) {
			return *i;
		}
	}

	return NULL;
}

void CBNET::AddWarn( string name, uint32_t warnings, string warning, uint32_t totalwarn, uint32_t daysban, string admin ) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	transform( admin.begin( ), admin.end( ), admin.begin( ), (int(*)(int))tolower );
	m_Warns.push_back( new CDBWarn( GetWarnId( )+1, m_Server, name, warnings, warning, totalwarn, daysban, admin, GetDate( ) ) );
}

void CBNET::UpdateAddWarn( string name, uint32_t warnings, string warning, uint32_t totalwarn, string admin ) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	transform( admin.begin( ), admin.end( ), admin.begin( ), (int(*)(int))tolower );
	
	//m_Server, name, warnings, warning, totalwarn, daysban, admin,"N/A"
	bool change = false;
	uint32_t id;
	string server;	
	uint32_t daysban;
	string date;

	CDBWarn *Warn = IsWarn( name );
	if (Warn) {
		change = true;
		id = Warn->GetId();
		server = Warn->GetServer();
		daysban = Warn->GetDaysban();
		date = Warn->GetDate();
	}

	if (change) {
		for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); ) {
			if ((*i)->GetName() == name) {
				i = m_Warns.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Warns.push_back( new CDBWarn( id, server, name, warnings, warning, totalwarn, daysban, admin, date ) );
	}
}

void CBNET::ChannelBanWarn(string name, uint32_t daysban, string admin) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	transform( admin.begin( ), admin.end( ), admin.begin( ), (int(*)(int))tolower );

	//m_Server, name, warnings, warning, totalwarn, daysban, admin,"N/A"
	bool change = false;
	uint32_t id;
	string server;	
	uint32_t warnings;
	string warning;	
	uint32_t totalwarn;
	string date;

	CDBWarn *Warn = IsWarn( name );
	if (Warn) {
		change = true;
		id = Warn->GetId();
		server = Warn->GetServer();
		warnings = Warn->GetWarnings();
		warning = Warn->GetWarning();
		totalwarn= Warn->GetTotalwarn();
		date = Warn->GetDate();
	}

	if (change) {
		for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); ) {
			if ((*i)->GetName() == name) {
				i = m_Warns.erase(i);
			}
			else {
				i++;
			}
		}		
		m_Warns.push_back( new CDBWarn( id, server, name, warnings, warning, totalwarn, daysban, admin, date ) );
	}
}

void CBNET::RemoveWarn(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); ) {
		if ((*i)->GetName() == name) {
			i = m_Warns.erase(i);
		}
		else {
			i++;
		}
	}
}

string CBNET::LocalTime() {
	time_t Now = time( NULL );
	char Time[20];
	memset( Time, 0, sizeof( char ) * 20 );
	strftime( Time, sizeof( char ) * 20, "%H:%M:%S %d-%m-%Y", localtime( &Now ) );
	return string( Time );
}

void CBNET::DelayMs(uint32_t msec) {
	#ifdef WIN32
		Sleep(msec);
	#else
		sleep(msec/1000);
	#endif
}
bool CBNET::IsMuted(string name) {
	bool userin=false;
	for (vector<string>::iterator i = m_MuteUsers.begin(); i != m_MuteUsers.end(); i++ ) {
		if (name == *i) {
			return true;
		}
	}
	return false;	
}

void CBNET::PrintMuted(string name) {
	uint32_t counter = 0;
	for (vector<string>::iterator i = m_MuteUsers.begin(); i != m_MuteUsers.end(); i++) {
		if (!(*i).empty( )) {
			SendChatCommand(name,*i);
			counter++;
		}
	}
	if (counter == 0) {
		SendChatCommand(name, "There isn't any muted user.");
	}
	else {
		SendChatCommand(name, "There are " + UTIL_ToString(counter) + " muted users.");
	}
}

uint32_t CBNET ::GetHoursSince1970() {
	time_t seconds;
	seconds = time (NULL);
	uint32_t Hours = (uint32_t)(seconds/3600);
	return Hours;
}

void CBNET::Cban( string name, uint32_t hoursban, string admin) {
	uint32_t ll=0;
	CDBInfo *Atemp = IsInfo( admin );
	if (Atemp) {
		ll=Atemp->GetLvl();
	}

	if (admin == "channel_flame") {
		ll = 5;
	}

	uint32_t Hours = GetHoursSince1970();
	if (hoursban<337) {
		CDBInfo *temp = IsInfo( name );
		if (temp) {
			if (ll>temp->GetLvl()) {
				if (temp->GetLvl()<4 && temp->GetLvl()>0) {
					CDBWarn *Warntemp = IsWarn( name );
					if (Warntemp) {
						m_PairedWarnChannelBans.push_back( PairedWarnChannelBan( name , m_GHost->m_DB->ThreadedWarnChannelBan( m_Server, name, Hours+hoursban, admin ) ) );
					}
					else {
						
						m_PairedWarnAdds.push_back( PairedWarnAdd(name , m_GHost->m_DB->ThreadedWarnAdd( m_Server, name,1,"Channel ban",1, Hours+hoursban, admin ) ) );
					}

					if (m_GHost->m_IsOperator) {
						SendChatCommand("/ban " + name + " You are banned for " + UTIL_ToString(hoursban) + " hours from " + admin);
					}
					else {
						SendChatCommand("/kick " + name + " You are banned for " + UTIL_ToString(hoursban) + " hours from " + admin);
					}
				}
				else {
					SendChatCommand(admin, "Can 't cban user with lvl 4 or hostbots!!!");
				}
			}
			else {
				SendChatCommand(admin, "Can 't ban user with same lvl as yours.");
			}
		}
		else {
			SendChatCommand(admin, "No info in db for " + name);
		}
	}
	else {
		SendChatCommand(admin, "You can ban for 336 hours max. (2 weeks)");
	}
}

uint32_t CBNET::GetMailId() {
	uint32_t temp = 0;
	for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++) {	
		if (temp < (*i)->GetId()) {
			temp = (*i)->GetId();
		}
	}
	return temp;
}

uint32_t CBNET::GetInfoId() {
	uint32_t temp = 0;
	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {	
		if (temp < (*i)->GetId()) {
			temp = (*i)->GetId();
		}
	}
	return temp;
}

uint32_t CBNET::GetUserChallWins(string user) {
	uint32_t wins = 0;
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
		if (user == (*i)->GetUser()) {
			wins = (*i)->GetChallwins();
		}
	}
	return wins;
}

uint32_t CBNET::GetUserChallLoses(string user) {
	uint32_t loses = 0;
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
		if (user == (*i)->GetUser()) {
			loses = (*i)->GetChallloses();
		}
	}
	return loses;
}

uint32_t CBNET::GetUserPrivPoints(string user) {
	uint32_t points = 1000;
	if (user.empty()) {
		points = 0;
	}
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
		if (user == (*i)->GetUser()) {
			if ((*i)->GetPrivPoints() != 0) {
				points = (*i)->GetPrivPoints();
			}
		}
	}
	
	return points;
}

uint32_t CBNET::GetUserPubPoints(string user) {
	uint32_t points = 1000;
	if (user.empty()) {
		points = 0;
	}
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
		if (user == (*i)->GetUser()) {
			if ((*i)->GetPubPoints() != 0) {
				points = (*i)->GetPubPoints();
			}
		}
	}
	
	return points;
}

uint32_t CBNET ::GetWarnId() {
	uint32_t temp = 0;
	for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); i++) {	
		if (temp < (*i)->GetId()) {
			temp = (*i)->GetId();
		}
	}
	return temp;
}

void CBNET::AddGameInLadder(string state, string win1, string win2, string win3, string win4, string win5, string loser1, string loser2, string loser3, string loser4, string loser5, uint32_t slots) {
	if (state.empty( ))
		return;
	uint32_t State = UTIL_ToUInt32(state);
	
	int loser_elo = 0;
	int winner_elo = 0;
	
	//calculate totalpoints
	if (State==2) {
		//public
		loser_elo =	GetUserPubPoints(loser1)+GetUserPubPoints(loser2)+GetUserPubPoints(loser3)+GetUserPubPoints(loser4)+GetUserPubPoints(loser5);
		winner_elo = GetUserPubPoints(win1)+GetUserPubPoints(win2)+GetUserPubPoints(win3)+GetUserPubPoints(win4)+GetUserPubPoints(win5);
	}
	else {
		//priv challenge
		loser_elo =	GetUserPrivPoints(loser1)+GetUserPrivPoints(loser2)+GetUserPrivPoints(loser3)+GetUserPrivPoints(loser4)+GetUserPrivPoints(loser5);
		winner_elo = GetUserPrivPoints(win1)+GetUserPrivPoints(win2)+GetUserPrivPoints(win3)+GetUserPrivPoints(win4)+GetUserPrivPoints(win5);
	}

	//expected_result = 1 / ( 1 + 10^((loser_elo - winner_elo)/400))
	//your_new_elo = your_current_elo + 30 * (result - expected_result)
	double addpoints = 30 * ( 1 - (1/(1+ pow(10, (((double)loser_elo - winner_elo)/400)))));
	if (m_GHost->m_DefaultAutoLvl == 0) {
		if (addpoints > 20) {
			addpoints = 20;
		}
		if (addpoints < 10) {
			addpoints = 10;
		}
	}
	else {
		if (addpoints < 1) {
			addpoints = 1;
		}
	}
	addpoints = int(addpoints);
	double decloser = addpoints * -1;
	
	if (m_GHost->m_OldLadderSystem) {
		addpoints = m_GHost->m_AddPoints;
		decloser = int( m_GHost->m_RemPoints * -1 );
	}

	if (State==3 ) {
		//its challenge add challenge win and challenge lose
		//add a challenge win to winner captain and lost to loser
		m_PairedInfoChallwinss.push_back( PairedInfoChallwins( win1, m_GHost->m_DB->ThreadedInfoChallwins( m_Server, win1, GetUserChallWins(win1) + 1 ) ) );
		DelayMs(500); //delay 0.5 seconds for each db query
		m_PairedInfoChalllosess.push_back( PairedInfoChallloses( loser1, m_GHost->m_DB->ThreadedInfoChallloses( m_Server, loser1, GetUserChallLoses( loser1 ) + 1 ) ) );
	}

	if (State==2) {
		//public
		//plus or dec points for all users and put them in database
		m_PairedInfoPubPointss.push_back( PairedInfoPubPoints( win1, m_GHost->m_DB->ThreadedInfoPubPoints( m_Server, win1, GetUserPubPoints(win1) + (uint32_t)addpoints ) ) );
		DelayMs(500); //delay 0.5 seconds for each db query
		if (!win2.empty()) {
			m_PairedInfoPubPointss.push_back(PairedInfoPubPoints(win2, m_GHost->m_DB->ThreadedInfoPubPoints(m_Server, win2, GetUserPubPoints(win2) + (uint32_t)addpoints)));
		}
		DelayMs(500);	
		if (!win3.empty()) {
			m_PairedInfoPubPointss.push_back(PairedInfoPubPoints(win3, m_GHost->m_DB->ThreadedInfoPubPoints(m_Server, win3, GetUserPubPoints(win3) + (uint32_t)addpoints)));
		}
		DelayMs(500);
		if (!win4.empty()) {
			m_PairedInfoPubPointss.push_back(PairedInfoPubPoints(win4, m_GHost->m_DB->ThreadedInfoPubPoints(m_Server, win4, GetUserPubPoints(win4) + (uint32_t)addpoints)));
		}
		DelayMs(500);
		if (!win5.empty()) {
			m_PairedInfoPubPointss.push_back(PairedInfoPubPoints(win5, m_GHost->m_DB->ThreadedInfoPubPoints(m_Server, win5, GetUserPubPoints(win5) + (uint32_t)addpoints)));
		}
		DelayMs(500);

		if (GetUserPubPoints(loser1) > decloser) {
			//put this not to get user <than 1 point
			m_PairedInfoPubPointss.push_back(PairedInfoPubPoints(loser1, m_GHost->m_DB->ThreadedInfoPubPoints(m_Server, loser1, GetUserPubPoints(loser1) + (uint32_t)decloser)));
		}
		DelayMs(500);
		if (GetUserPubPoints(loser2) > decloser && !loser2.empty()) {
			m_PairedInfoPubPointss.push_back(PairedInfoPubPoints(loser2, m_GHost->m_DB->ThreadedInfoPubPoints(m_Server, loser2, GetUserPubPoints(loser2) + (uint32_t)decloser)));
		}
		DelayMs(500);
		if (GetUserPubPoints(loser3) > decloser && !loser3.empty()) {
			m_PairedInfoPubPointss.push_back(PairedInfoPubPoints(loser3, m_GHost->m_DB->ThreadedInfoPubPoints(m_Server, loser3, GetUserPubPoints(loser3) + (uint32_t)decloser)));
		}
		DelayMs(500);
		if (GetUserPubPoints(loser4) > decloser && !loser4.empty()) {
			m_PairedInfoPubPointss.push_back(PairedInfoPubPoints(loser4, m_GHost->m_DB->ThreadedInfoPubPoints(m_Server, loser4, GetUserPubPoints(loser4) + (uint32_t)decloser)));
		}
		DelayMs(500);
		if (GetUserPubPoints(loser5) > decloser && !loser5.empty()) {
			m_PairedInfoPubPointss.push_back(PairedInfoPubPoints(loser5, m_GHost->m_DB->ThreadedInfoPubPoints(m_Server, loser5, GetUserPubPoints(loser5) + (uint32_t)decloser)));
		}

		//here now we need to calculate all info
		DelayMs(3000); //delay 3 seconds first to let all done
		m_PairedInfoPubCalculates.push_back( PairedInfoPubCalculate( "Channel Bot", m_GHost->m_DB->ThreadedInfoPubCalculate( m_Server ) ) );
	}
	else {
		//plus or dec points for all users and put them in database
		m_PairedInfoPrivPointss.push_back( PairedInfoPrivPoints( win1, m_GHost->m_DB->ThreadedInfoPrivPoints( m_Server, win1, GetUserPrivPoints(win1) + (uint32_t)addpoints ) ) );
		DelayMs(500); //delay 0.5 seconds for each db query
		if (!win2.empty()) {
			m_PairedInfoPrivPointss.push_back(PairedInfoPrivPoints(win2, m_GHost->m_DB->ThreadedInfoPrivPoints(m_Server, win2, GetUserPrivPoints(win2) + (uint32_t)addpoints)));
		}
		DelayMs(500);
		if (!win3.empty()) {
			m_PairedInfoPrivPointss.push_back(PairedInfoPrivPoints(win3, m_GHost->m_DB->ThreadedInfoPrivPoints(m_Server, win3, GetUserPrivPoints(win3) + (uint32_t)addpoints)));
		}
		DelayMs(500);
		if (!win4.empty()) {
			m_PairedInfoPrivPointss.push_back(PairedInfoPrivPoints(win4, m_GHost->m_DB->ThreadedInfoPrivPoints(m_Server, win4, GetUserPrivPoints(win4) + (uint32_t)addpoints)));
		}
		DelayMs(500);
		if (!win5.empty()) {
			m_PairedInfoPrivPointss.push_back(PairedInfoPrivPoints(win5, m_GHost->m_DB->ThreadedInfoPrivPoints(m_Server, win5, GetUserPrivPoints(win5) + (uint32_t)addpoints)));
		}
		DelayMs(500);

		if (GetUserPrivPoints(loser1) > decloser) {
			//put this not to get user <than 1 point
			m_PairedInfoPrivPointss.push_back(PairedInfoPrivPoints(loser1, m_GHost->m_DB->ThreadedInfoPrivPoints(m_Server, loser1, GetUserPrivPoints(loser1) + (uint32_t)decloser)));
		}
		DelayMs(500);
		if (GetUserPrivPoints(loser2) > decloser && !loser2.empty()) {
			m_PairedInfoPrivPointss.push_back(PairedInfoPrivPoints(loser2, m_GHost->m_DB->ThreadedInfoPrivPoints(m_Server, loser2, GetUserPrivPoints(loser2) + (uint32_t)decloser)));
		}
		DelayMs(500);
		if (GetUserPrivPoints(loser3) > decloser && !loser3.empty()) {
			m_PairedInfoPrivPointss.push_back(PairedInfoPrivPoints(loser3, m_GHost->m_DB->ThreadedInfoPrivPoints(m_Server, loser3, GetUserPrivPoints(loser3) + (uint32_t)decloser)));
		}
		DelayMs(500);
		if (GetUserPrivPoints(loser4) > decloser && !loser4.empty()) {
			m_PairedInfoPrivPointss.push_back(PairedInfoPrivPoints(loser4, m_GHost->m_DB->ThreadedInfoPrivPoints(m_Server, loser4, GetUserPrivPoints(loser4) + (uint32_t)decloser)));
		}
		DelayMs(500);
		if (GetUserPrivPoints(loser5) > decloser && !loser5.empty()) {
			m_PairedInfoPrivPointss.push_back(PairedInfoPrivPoints(loser5, m_GHost->m_DB->ThreadedInfoPrivPoints(m_Server, loser5, GetUserPrivPoints(loser5) + (uint32_t)decloser)));
		}
		//here now we need to calculate all info
		DelayMs(3000); //delay 3 seconds first to let all done
		m_PairedInfoPrivCalculates.push_back( PairedInfoPrivCalculate( "Channel Bot", m_GHost->m_DB->ThreadedInfoPrivCalculate( m_Server ) ) );
	}
	
	//refresh all info now this must be changed...
	DelayMs(2000); //delay more 2 just in case
	m_CallableInfoList = m_GHost->m_DB->ThreadedInfoList( m_Server );

	CONSOLE_Print("Winners earn "+UTIL_ToString(addpoints)+" points and loser lost "+UTIL_ToString(decloser)+".", true);

	//announce to user the points that he earn or lose
	SendChatCommand(win1,"You earn "+UTIL_ToString(addpoints)+" points from this game.");
	if (!win2.empty()) {
		SendChatCommand(win2, "You earn " + UTIL_ToString(addpoints) + " points from this game.");
	}
	if (!win3.empty()) {
		SendChatCommand(win3, "You earn " + UTIL_ToString(addpoints) + " points from this game.");
	}
	if (!win4.empty()) {
		SendChatCommand(win4, "You earn " + UTIL_ToString(addpoints) + " points from this game.");
	}
	if (!win5.empty()) {
		SendChatCommand(win5, "You earn " + UTIL_ToString(addpoints) + " points from this game.");
	}
	
	SendChatCommand(loser1,"You lost "+UTIL_ToString(decloser)+" points from this game.");
	if (!loser2.empty()) {
		SendChatCommand(loser2, "You lost " + UTIL_ToString(decloser) + " points from this game.");
	}
	if (!loser3.empty()) {
		SendChatCommand(loser3, "You lost " + UTIL_ToString(decloser) + " points from this game.");
	}
	if (!loser4.empty()) {
		SendChatCommand(loser4, "You lost " + UTIL_ToString(decloser) + " points from this game.");
	}
	if (!loser5.empty()) {
		SendChatCommand(loser5, "You lost " + UTIL_ToString(decloser) + " points from this game.");
	}
}

string CBNET::GetNameFromPrivRank(uint32_t rank) {
	string user="";
	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
		if ((*i)->GetPrivRank() == rank) {
			user = (*i)->GetUser();
		}
	}
	return user;
}

string CBNET::GetNameFromPubRank(uint32_t rank) {
	string user="";
	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
		if ((*i)->GetPubRank() == rank) {
			user = (*i)->GetUser();
		}
	}
	return user;
}

uint32_t CBNET::GetSafelisted() {
	uint32_t ss=0;
	for (vector<CDBInfo *>::iterator j = m_Infos.begin(); j != m_Infos.end(); j++) {
		if ((*j)->GetLvl() < 6 && (*j)->GetLvl() > 0) {
			ss++;
		}
	}
	return ss;
}

bool CBNET::IsChannelBan(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	for (vector<CDBWarn *>::iterator i = m_Warns.begin(); i != m_Warns.end(); i++) {
		if ((*i)->GetName( ) == name && (*i)->GetDaysban( ) !=0) {
			return true;
		}
	}
	return false;
}

bool CBNET::IsHostBan(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end(); i++) {
		if ((*i)->GetName() == name) {
			return true;
		}
	}
	return false;
}

void CBNET::FlameCheck(string User, string msg) {
	transform( msg.begin( ), msg.end( ), msg.begin( ), (int(*)(int))tolower );
	for (vector<string>::iterator i = m_GHost->m_FlameList.begin(); i != m_GHost->m_FlameList.end(); i++) {
		if (msg.find(*i)!= string::npos) {
			CDBInfo *temp= IsInfo( User );
			if (temp) {		
				if (temp->GetLvl()<4) {
					if (m_GHost->m_DefaultAutoLvl == 1) {
						SendChatCommand(User," You are banned from channel for 1 hour.");
						Cban(User, 1, "channel_flame");
					}
					else {
						SendChatCommand("/kick " + User + " Don't flame...");
					}
				}						
			}
		}
	}
}

bool CBNET::ChallengeRankProtectPass(string User1, string User2) {
	uint32_t user1rank = GetPrivRank(User1);
	uint32_t user2rank = GetPrivRank(User2);
	uint32_t rankdiff = 11;
	if (m_GHost->commands[75]) {
		if (user1rank > user2rank) {
			rankdiff = user1rank - user2rank;
		}
		else {
			rankdiff = user2rank - user1rank;
		}

		if (rankdiff > m_GHost->m_ChallengeSafeRange) {
			return false;
		}
	}

	return true;	
}

uint32_t CBNET::GetPrivRank(string User) {
	uint32_t rank=0;
	transform( User.begin( ), User.end( ), User.begin( ), (int(*)(int))tolower );
	CDBInfo *temp= IsInfo(User);
	if (temp) {
		rank=temp->GetPrivRank();
	}

	return rank;
}

void CBNET::ResetHostbots() {
	uint32_t hostgames = 0;
	for (vector<CDBHostBot *>::iterator i = m_HostBots.begin(); i != m_HostBots.end(); i++) {
		hostgames = (*i)->GetGames();
		(*i)->UpdateState( 0 );
		(*i)->UpdateGames(hostgames);
	}
}

uint32_t CBNET::GetHostbotGames(string hostbot) {
	transform( hostbot.begin( ),hostbot.end( ), hostbot.begin( ), (int(*)(int))tolower );
	for (vector<CDBHostBot *>::iterator i = m_HostBots.begin(); i != m_HostBots.end(); i++) {
		if ((*i)->GetHostBot() == hostbot) {
			return (*i)->GetGames();
		}
	}
	return 0;
}

void CBNET::UpdateHostbot(string hostbot, uint32_t state, uint32_t hostedgames, uint32_t maxgames) {
	
	uint32_t State = state;
	transform( hostbot.begin( ),hostbot.end( ), hostbot.begin( ), (int(*)(int))tolower );
	for (vector<CDBHostBot *>::iterator i = m_HostBots.begin(); i != m_HostBots.end(); i++) {
		if ((*i)->GetHostBot( ) == hostbot) {
			if (State < 100) {
				(*i)->UpdateState(State);
			}

			(*i)->UpdateGames(hostedgames);
			if (maxgames > 0) {
				(*i)->UpdateMaxGames(maxgames);
			}
		}
	}
}

string CBNET::GetAvailableHostbot() {
	for (vector<CDBHostBot *>::iterator i = m_HostBots.begin(); i != m_HostBots.end(); i++) {
		//reset time from user in channel (time = 0)
		if ((*i)->GetState() == 1 && (*i)->GetMaxGames() > (*i)->GetGames()) {
			return (*i)->GetHostBot();
		}
	}
	return "";
}

bool CBNET::IsHostbot(string name) {
	// m_RootAdmin was already transformed to lower case in the constructor
	bool tmp = false;
	string temp;
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	for (vector<CDBInfo *>::iterator k = m_Infos.begin(); k != m_Infos.end(); k++) {
		if ((*k)->GetLvl()==6) {
			temp=(*k)->GetUser();
			transform( temp.begin( ), temp.end( ), temp.begin( ), (int(*)(int))tolower );
			if (temp == name) {
				tmp = true;
			}
		}			
	}
	return tmp;
}

void CBNET::UnhostAllGames() {
	for (vector<CDBInfo *>::iterator i = m_Infos.begin(); i != m_Infos.end(); i++) {
		if ((*i)->GetLvl() == 6) {
			SendChatCommand((*i)->GetUser(), UTIL_ToString(m_CommandTrigger) + "unhost");
		}
	}
}

bool CBNET::IsInPool(string name) {
	bool temp = false;
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	string temp1; 
	for (int i=0; i<12; i++) {
		temp1 = m_meplay[i][0];
		transform( temp1.begin( ), temp1.end( ), temp1.begin( ), (int(*)(int))tolower );
		if (temp1 == name) {
			temp = true;
		}
	}

	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
		temp1 = (*i)->GetUser();
		transform( temp1.begin( ), temp1.end( ), temp1.begin( ), (int(*)(int))tolower );
		if (temp1 == name && (*i)->GetSigned()) {
			temp = true;
		}
	}
	return temp;
}

string CBNET::GetRandomUser() {
    vector<string> signedPlayers;
    for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
        if ((*i)->GetSigned( )) {
            signedPlayers.push_back((*i)->GetUser());
        }
    }
    if (signedPlayers.size() == 0) {
        return "No players in pool.";
    }

    return *UTIL_SelectRandom(signedPlayers.begin(), signedPlayers.end());
}

void CBNET::PrintPool(string name) {
	if (m_Challenge) {
		string challenger = m_Challenger;
		string challenged = m_Challenged;
		if (GetUserMode(m_Challenger) != "-") {
			challenger += " (" + GetUserMode(m_Challenger) + ")";
		}
		if (GetUserMode(m_Challenged) != "-") {
			challenged += " (" + GetUserMode(m_Challenged) + ")";
		}

		SendChatCommand(name,challenger+" VS "+challenged);		
	}

	uint32_t line=0;
	uint32_t counter=0;
	string temp="";
	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
		if ((*i)->GetSigned( )) {
			if (GetTime() < (*i)->GetSignedTime()) {
				(*i)->UpdateSignTime();
			}

			if ((*i)->GetMode() == "-") {
				temp += UTIL_ToString((counter + 1) + (4 * line)) + "." + (*i)->GetUser() + " (" + UTIL_MToString(GetTime() - (*i)->GetSignedTime()) + ")  ";
			}
			else {
				temp += UTIL_ToString((counter + 1) + (4 * line)) + "." + (*i)->GetUser() + " (" + UTIL_MToString(GetTime() - (*i)->GetSignedTime()) + ") (" + (*i)->GetMode() + ") ";
			}
			counter++;
		}
		if (counter==4) {
			SendChatCommand(name,temp);
			line++;
			temp="";
			counter=0;
		}
	}
	if (counter > 0) {
		SendChatCommand(name, temp);
	}

	if (counter == 0 && line == 0) {
		SendChatCommand(name, "Pool list is empty.");
	}

	string obs;
	for (int i=11; i<13; i++) {
		if (!m_meplay[i - 1][0].empty()) {
			obs += UTIL_ToString(i) + "." + m_meplay[i - 1][0] + " ";
		}
	}
	if (!obs.empty()) {
		SendChatCommand(name, "Obs: " + obs);
	}

	if (m_RMKstate>0) {
		SendChatCommand(name,"----------------------------> RMK POOL <----------------------------");
		string tmp;
		for (int i = 0; i < 5; i++) {
			if (!m_RMK[i].empty()) {
				tmp += UTIL_ToString(i + 1) + "." + m_RMK[i] + " ";
			}
		}
		SendChatCommand(name,tmp);
		tmp="";
		for (int i = 5; i < 10; i++) {
			if (!m_RMK[i].empty()) {
				tmp += UTIL_ToString(i + 1) + "." + m_RMK[i] + " ";
			}
		}
		SendChatCommand(name,tmp);
		tmp="";
		for (int i = 10; i < 12; i++) {
			if (!m_RMK[i].empty()) {
				tmp += UTIL_ToString(i + 1) + "." + m_RMK[i] + " ";
			}
		}
		SendChatCommand(name,tmp);
		SendChatCommand(name,"-----------------------------------------------------------------------");
	}
}

void CBNET::Sign( string name, string mode) {
	if (mode.empty()) {
		mode = "-";
	}

	SendChatCommand(name,"Signed!!!");
	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
		if ((*i)->GetUser( )==name) {
			(*i)->UpdateSign( mode );
			(*i)->UpdateSignTime();
		}
	}
}

void CBNET::UnSign(string name) {
	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
		if ((*i)->GetUser( )==name) {			
			(*i)->ResetSign();
		}
	}
}

void CBNET::Pick(string picker, string name, bool rnd) {
	if (m_PickUser==0) {
		if (m_Challenger == picker &&(m_PickStep==1||m_PickStep==4||m_PickStep==5||m_PickStep==8)) {
			m_meplay[m_holdnumb1][0]=name;
			m_meplay[m_holdnumb1][1] =GetUserMode( name );
			if (rnd) {
				QueueChatCommand(m_Challenger + " randomed pick " + name + ".");
			}
			else {
				QueueChatCommand(m_Challenger + " pick " + name + ".");
			}
			
			if (m_PickStep == 1 || m_PickStep == 5) {
				QueueChatCommand(m_Challenged + " 's turn. Pick 2 players.");
			}
			else if (m_PickStep==8) {
				if (!GetAvailableHostbot( ).empty( )) {
					m_VotedMode = GetMode();
					StartGame();
				}
				else {
					m_ChallengeStep=4;
					QueueChatCommand("No hostbot available. Type "+UTIL_ToString(m_CommandTrigger)+"go when a hostbot is available");
					SendChatCommand(picker, UTIL_ToString(m_CommandTrigger)+"hbots to see available hostbots.");
				}
			}
			UnSign( name );
			m_holdnumb1++;
			m_PickStep++;
		}
		else if (m_Challenged == picker && (m_PickStep==2 || m_PickStep==3 || m_PickStep==6 || m_PickStep==7)) {
			m_meplay[m_holdnumb2][0]=name;
			m_meplay[m_holdnumb2][1] = GetUserMode( name );
			if (rnd) {
				QueueChatCommand(m_Challenged + " randomed pick " + name + ".");
			}
			else {
				QueueChatCommand(m_Challenged + " pick " + name + ".");
			}

			if (m_PickStep == 3) {
				QueueChatCommand(m_Challenger + " 's turn. Pick 2 players.");
			}
			else if (m_PickStep == 7) {
				QueueChatCommand(m_Challenger + " 's turn. Pick last player.");
			}

			UnSign( name );
			m_holdnumb2++;
			m_PickStep++;
		}
	}
	else {
		if (m_Challenged == picker && (m_PickStep==1 || m_PickStep==4 || m_PickStep==5 || m_PickStep==8)) {
			m_meplay[m_holdnumb2][0]=name;
			m_meplay[m_holdnumb2][1] =GetUserMode( name );
			if (rnd) {
				QueueChatCommand(m_Challenged + " randomed pick " + name + ".");
			}
			else {
				QueueChatCommand(m_Challenged + " pick " + name + ".");
			}
			
			if (m_PickStep == 1 || m_PickStep == 5) {
				QueueChatCommand(m_Challenger + " 's turn. Pick 2 players.");
			}
			else if (m_PickStep==8) {
				if (!GetAvailableHostbot( ).empty( )) {
					m_VotedMode = GetMode();
					StartGame();
				}
				else {
					m_ChallengeStep=4;
					QueueChatCommand("No hostbot available. Type "+UTIL_ToString(m_CommandTrigger)+"go when a hostbot is available");
					SendChatCommand(picker, UTIL_ToString(m_CommandTrigger)+"hbots to see available hostbots.");
				}	
			}
			UnSign( name );
			m_holdnumb2++;
			m_PickStep++;
		}
		else if (m_Challenger == picker && (m_PickStep==2 || m_PickStep==3 || m_PickStep==6 || m_PickStep==7)) {
			//challenger pick
			m_meplay[m_holdnumb1][0]=name;
			m_meplay[m_holdnumb1][1] =GetUserMode( name );
			if (rnd) {
				QueueChatCommand(m_Challenger + " randomed pick " + name + ".");
			}
			else {
				QueueChatCommand(m_Challenger + " pick " + name + ".");
			}

			if (m_PickStep == 3) {
				QueueChatCommand(m_Challenged + " 's turn. Pick 2 players.");
			}
			else if (m_PickStep == 7) {
				QueueChatCommand(m_Challenged + " 's turn. Pick last player.");
			}

			UnSign( name );
			m_holdnumb1++;
			m_PickStep++;
		}
	}
}

string CBNET::GetUserMode(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	string temp;
	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
		temp=(*i)->GetUser();
		transform( temp.begin( ), temp.end( ), temp.begin( ), (int(*)(int))tolower );
		if (temp == name) {
			return (*i)->GetMode();
		}
	}
	return "-";
}

string CBNET::GetMode( ) {
	uint32_t cm=0;
	uint32_t cd=0;
	string mode = "";

	for (int i=0; i<10; i++) {
		if (m_meplay[i][1] == "cm") {
			cm++;
		}
		else if (m_meplay[i][1] == "cd") {
			cd++;
		}
	}
	if (cm == 0 && cd == 0) {
		return mode;
	}
	else if (cm > cd) {
		mode = "cm";
		QueueChatCommand("Voted mode is CM. cm: "+UTIL_ToString(cm)+"  cd: "+UTIL_ToString(cd));
	}
	else if (cd > cm) {
		mode = "cd";
		QueueChatCommand("Voted mode is CD. cm: "+UTIL_ToString(cm)+"  cd: "+UTIL_ToString(cd));
	}
	else {
		QueueChatCommand("Votes are same. cm: " + UTIL_ToString(cm) + "  cd: " + UTIL_ToString(cd));
	}

	return mode;
}

void CBNET::ChangeMode(string name, string mode) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	string temp;
	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
		temp=(*i)->GetUser();
		transform( temp.begin( ), temp.end( ), temp.begin( ), (int(*)(int))tolower );
		if (temp == name) {
			(*i)->ChangeMode(mode);
		}
	}

	if (m_Challenger == name) {
		m_meplay[0][1] = mode;
	}

	if (m_Challenged == name) {
		m_meplay[5][1] = mode;
	}
}

void CBNET::PrintSendedMails(string user) {
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	uint32_t mailcounter = 0;
	for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++) {
		if ((*i)->GetSender ( )==user) {
			mailcounter++;
			if ((*i)->GetReceiver() == "-") {
				SendChatCommand(user, "ID (" + UTIL_ToString((*i)->GetId()) + ") Announce mail. Message: " + (*i)->GetMessage());
			}
			else {
				SendChatCommand(user, "ID (" + UTIL_ToString((*i)->GetId()) + ") Mail sent to " + (*i)->GetReceiver() + ": " + (*i)->GetMessage());
			}
		}
	}
	if (mailcounter == 0) {
		SendChatCommand(user, "You didn't send any mail yet.");
	}
	
}

void CBNET::PrintMails(string user, string receiver, bool admin, bool command) {
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	transform( receiver.begin( ), receiver.end( ), receiver.begin( ), (int(*)(int))tolower );
	uint32_t mailcounter = 0;
	if (command) {
		for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++) {
			if ((*i)->GetReceiver( )==receiver) {
				mailcounter++;
				SendChatCommand(user,"ID ("+UTIL_ToString((*i)->GetId( ))+") Mail from "+(*i)->GetSender( )+": "+(*i)->GetMessage( ));
			}
		}
	}
	else {
		for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++) {
			if ((*i)->GetReceiver( )=="-" && !MailSpamAlreadyReaded(user,(*i)->GetMessage())) {
				SendChatCommand(user,"You have a new announce mail from "+(*i)->GetSender( )+" with ID ("+UTIL_ToString(GetMailId( )+1)+")");
				SendChatCommand(user,(*i)->GetSender( )+": "+(*i)->GetMessage( ));
				m_PairedMailAdds.push_back( PairedMailAdd( string( ), m_GHost->m_DB->ThreadedMailAdd( GetMailId( )+1, m_Server, (*i)->GetSender( ), user, (*i)->GetMessage( ) ) ) );
			}
			else if ((*i)->GetReceiver( ) == receiver && (*i)->GetReaded( ) == 0) {
				mailcounter++;
				SendChatCommand(user,"ID ("+UTIL_ToString((*i)->GetId( ))+") New Mail from "+(*i)->GetSender( )+": "+(*i)->GetMessage( ));
			}
		}
	}
	if (!admin && mailcounter != 0) {
		ReadedMail(receiver);
	}

	if (mailcounter == 0 && command) {
		SendChatCommand(user, "Mail list is empty.");
	}
	else {
		m_PairedMailReadeds.push_back(PairedMailReaded(user, m_GHost->m_DB->ThreadedMailReaded(m_Server, user)));
	}
}

bool CBNET::MailSpamAlreadyReaded(string user, string message) {
	bool temp = false;
	for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++) {
		if ((*i)->GetSender() == user || ((*i)->GetReceiver() == user && (*i)->GetMessage() == message)) {
			temp = true;
		}
	}
	return temp;
}

bool CBNET::MailsSendedMaxLimit(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	uint32_t mails=0;
	for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++) {
		if ((*i)->GetSender() == name) {
			mails++;
		}
	}
	if (mails == m_GHost->m_MailMaxSended) {
		return true;
	}
	else {
		return false;
	}
}

bool CBNET::MailsReceivedMaxLimit(string name) {
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	uint32_t mails=0;
	for (vector<CDBMail *>::iterator i = m_Mails.begin(); i != m_Mails.end(); i++) {
		if ((*i)->GetReceiver()==name) {
			mails++;
		}
	}
	if (mails == m_GHost->m_MailMaxReceived) {
		return true;
	}
	else {
		return false;
	}
}

string CBNET::GetIconFromRank(uint32_t rank) {
	
	string icon = "error";
	
	if (rank == 1) {
		icon = "dota-spectre";
	}
	else if (rank == 2) {
		icon = "dota-shadowfiend";
	}
	else if (rank == 3) {
		icon = "dota-mirana";
	}
	else if (rank > 3 && rank <= 10) {
		icon = "dota-jakiro";
	}
	else if (rank > 10 && rank <= 15) {
		icon = "dota-leshrac";
	}
	else if (rank > 15 && rank <= 20) {
		icon = "dota-pugna";
	}
	else if (rank > 20 && rank <= 25) {
		icon = "dota-enigma";
	}
	else {
		icon = "error";
	}

	return icon;
}

void CBNET::ResetChannelIcons(string channel) {
	transform( channel.begin( ), channel.end( ), channel.begin( ), (int(*)(int))tolower );
	string temp;
	
	for (vector<CDBIcons *>::iterator i = m_IconList.begin(); i != m_IconList.end(); ) {
		temp = (*i)->GetFrom();
		transform( temp.begin( ), temp.end( ), temp.begin( ), (int(*)(int))tolower );
		if (temp == channel) {
			i = m_IconList.erase(i);
		}
		else {
			i++;
		}
	}
}

uint32_t CBNET::GetUsersNumberInChannel() {
	uint32_t users = 0;
	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
		if ((*i)->GetLvl() < 6) {
			users++;
		}
	}
	return users;
}

string CBNET:: GetHostbotFromOwner(string user) {
	string tmp;
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	for (vector<CDBGames *>::iterator i = m_GHost->m_Games.begin(); i != m_GHost->m_Games.end(); i++) {
		tmp = (*i)->GetOwner();
		transform( tmp.begin( ), tmp.end( ), tmp.begin( ), (int(*)(int))tolower );
		if (tmp == user && (*i)->GetGameState() == 0) {
			return (*i)->GetHostbot();
		}

	}
	return "";
}
void CBNET::FindUser(string user, string victim) {
	transform( victim.begin( ), victim.end( ), victim.begin( ), (int(*)(int))tolower );
	string temp;
	bool ingame = false;
	bool anywhere = false;
	for (vector<CDBGames *>::iterator i = m_GHost->m_Games.begin(); i != m_GHost->m_Games.end(); i++) {
		for (int j=0; j<12; j++) {
			temp = (*i)->GetName( j );
			transform( temp.begin( ), temp.end( ), temp.begin( ), (int(*)(int))tolower );
			
			if (temp == victim) {
				SendChatCommand( user, victim+" is in game "+(*i)->GetGameName( )+". Game timer - "+UTIL_ToDayTimSec(GetTime( )-(*i)->GetStartTime())+".");
				ingame = true;
				anywhere = true;
			}
		}
	}
	for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++) {
		temp = (*i)->GetUser();
		transform( temp.begin( ), temp.end( ), temp.begin( ), (int(*)(int))tolower );
		if (victim == temp) {
			anywhere = true;
			if (ingame) {
				SendChatCommand(user, victim + " is in channel " + m_CurrentChannel + " idle for " + UTIL_ToDayTimSec(GetTime() - (*i)->GetSec()) + ". He is leaver.");
			}
			else {
				SendChatCommand(user, victim + " is in channel " + m_CurrentChannel + " idle for " + UTIL_ToDayTimSec(GetTime() - (*i)->GetSec()));
			}
		}
	}
	if (!anywhere) {
		SendChatCommand(user, victim + " isn't in channel or in any game that hosted from this community.");
	}
}


void CBNET::GetAllInfo(string user, string payload) {
	transform( payload.begin( ), payload.end( ), payload.begin( ), (int(*)(int))tolower );
	if (payload.empty( )) {
		if (m_GHost->m_Games.size( )>15) {

			SendChatCommand(user,"-----------------------------------------------------------------------------------");
			SendChatCommand(user, "There are more than 15 games in proggress. You can't use this command for spam problems.");
			SendChatCommand(user,"User "+UTIL_ToString(m_CommandTrigger)+"ggs command.");
			SendChatCommand(user,"For spam problems names won't apear.");
			SendChatCommand(user,"-----------------------------------------------------------------------------------");
			string state="Public";
			for (vector<CDBGames *>::iterator i = m_GHost->m_Games.begin(); i != m_GHost->m_Games.end(); i++) {
				if ((*i)->GetGameState() == 0) {
					state = "in Lobby";
				}
				else if ((*i)->GetGameState() == 1) {
					state = "Private";
				}
				else if ((*i)->GetGameState() == 2) {
					state = "Public";
				}
				else if ((*i)->GetGameState() == 3) {
					state = "Challenge";
				}
				
				SendChatCommand(user,"ID: "+UTIL_ToString((*i)->GetGameId( ))+" GN: "+(*i)->GetGameName( )+" Hostbot: "+(*i)->GetHostbot( ) +" Owner: "+(*i)->GetOwner( ));
				SendChatCommand(user,"Game is "+state+". Game running for ("+UTIL_ToDayTimSec(GetTime( )-(*i)->GetStartTime())+")");
				SendChatCommand(user,"-----------------------------------------------------------------------------------");
			}
		}
		else if (m_GHost->m_Games.size( )==0) {
			SendChatCommand(user,"("+UTIL_ToString(m_GHost->GetLobby( )+m_GHost->GetChallenges( )+m_GHost->GetPubs( )+m_GHost->GetPrivs( ))+"/"+UTIL_ToString(GetMaxGames( ))+") There isn't any game in proggress.");
		}
		else { 
			SendChatCommand(user,"-----------------------------------------------------------------------------------");
			SendChatCommand(user,"There are "+UTIL_ToString(m_GHost->GetUsersNumber( ))+" users in "+UTIL_ToString( m_GHost->m_Games.size( ))+" games and "+UTIL_ToString(GetUsersNumberInChannel( ))+" in channel");
			SendChatCommand(user,"("+UTIL_ToString(m_GHost->GetLobby( )+m_GHost->GetChallenges( )+m_GHost->GetPubs( )+m_GHost->GetPrivs( ))+"/"+UTIL_ToString(GetMaxGames( ))+") There are "+UTIL_ToString(m_GHost->GetChallenges( ))+" challenges, "+UTIL_ToString(m_GHost->GetPubs( ))+" public, "+UTIL_ToString(m_GHost->GetPrivs( ))+" private games and "+UTIL_ToString( m_GHost->GetLobby( ))+" in lobby.");
			SendChatCommand(user,"-----------------------------------------------------------------------------------");
			SendChatCommand(user,"If you see * infront of a name that means that he left the game.");
			SendChatCommand(user,"-----------------------------------------------------------------------------------");
			
			string state="Public";
			string creepspawn = " (Pick state)";
			for (vector<CDBGames *>::iterator i = m_GHost->m_Games.begin(); i != m_GHost->m_Games.end(); i++) {
				if ((*i)->GetGameState() == 0) {
					state = "in Lobby";
				}
				else if ((*i)->GetGameState() == 1) {
					state = "Private";
				}
				else if ((*i)->GetGameState() == 2) {
					state = "Public";
				}
				else if ((*i)->GetGameState() == 3) {
					state = "Challenge";
				}
				if ((*i)->GetCreepsSpawn()) {
					creepspawn = " (Creeps Spawn)";
				}
				
				SendChatCommand(user,"ID: "+UTIL_ToString((*i)->GetGameId( ))+" GN: "+(*i)->GetGameName( )+" Hostbot: "+(*i)->GetHostbot( ) +" Owner: "+(*i)->GetOwner( ));
				if ((*i)->GetGameState() > 0) {
					SendChatCommand(user, "Game is " + state + ". Game running for (" + UTIL_ToDayTimSec(GetTime() - (*i)->GetStartTime()) + ")" + creepspawn);
				}
				else {
					SendChatCommand(user, "Game is " + state + ". Game running for (" + UTIL_ToDayTimSec(GetTime() - (*i)->GetStartTime()) + ")");
				}
				if ((*i)->GetGameState( )!=0) {
					SendChatCommand(user,"Sent: "+(*i)->GetTeam1( ));
					SendChatCommand(user,"Scou: "+(*i)->GetTeam2( ));
					if (!(*i)->GetObs().empty()) {
						SendChatCommand(user, "Obs: " + (*i)->GetObs());
					}
				}
				SendChatCommand(user,"-----------------------------------------------------------------------------------");
			}
		}
	}
	else {
		SendChatCommand(user,"Games that hosted on "+payload+" hostbot.");
		SendChatCommand(user,"GID // State // Timer // Game Name // Owner");
		string state="Public";
		string tmp;
		uint32_t tmp_counter = 0;
		for (vector<CDBGames *>::iterator i = m_GHost->m_Games.begin(); i != m_GHost->m_Games.end(); i++) {
			tmp = (*i)->GetHostbot();
			transform( tmp.begin( ), tmp.end( ), tmp.begin( ), (int(*)(int))tolower );
			if (payload == tmp) {
				tmp_counter++;
				if ((*i)->GetGameState() == 0) {
					state = "in Lobby";
				}
				else if ((*i)->GetGameState() == 1) {
					state = "Private";
				}
				else if ((*i)->GetGameState() == 2) {
					state = "Public";
				}
				else if ((*i)->GetGameState() == 3) {
					state = "Challenge";
				}
				SendChatCommand(user,UTIL_ToString((*i)->GetGameId( ))+" // "+ state+" // "+UTIL_ToDayTimSec(GetTime( )-(*i)->GetStartTime())+" // "+(*i)->GetGameName( )+ " // "+(*i)->GetOwner( ));
			}
		}
		if (tmp_counter == 0) {
			SendChatCommand(user, "There isn't any game in proggress in this hostbot.");
		}
		SendChatCommand(user,"("+UTIL_ToString(m_GHost->GetLobby( )+m_GHost->GetChallenges( )+m_GHost->GetPubs( )+m_GHost->GetPrivs( ))+"/"+UTIL_ToString(GetMaxGames( ))+")");
	}
}

void CBNET::GetInfo(string user) {
	SendChatCommand(user,"("+UTIL_ToString(m_GHost->GetLobby( )+m_GHost->GetChallenges( )+m_GHost->GetPubs( )+m_GHost->GetPrivs( ))+"/"+UTIL_ToString(GetMaxGames( ))+") There are "+UTIL_ToString(m_GHost->GetChallenges( ))+" challenges, "+UTIL_ToString(m_GHost->GetPubs( ))+" public, "+UTIL_ToString(m_GHost->GetPrivs( ))+" private games and "+UTIL_ToString( m_GHost->GetLobby( ))+" in lobby.");
}

uint32_t CBNET::GetMaxGames() {
	uint32_t maxgames = 0;
	for (vector<CDBHostBot *>::iterator i = m_HostBots.begin(); i != m_HostBots.end(); i++) {
		if ((*i)->GetState() > 0) {
			maxgames += (*i)->GetMaxGames();
		}
	}
	return maxgames;
}

void CBNET::GetNames(string user, uint32_t gameid) {
	for (vector<CDBGames *>::iterator i = m_GHost->m_Games.begin(); i != m_GHost->m_Games.end(); i++) {
		if (gameid==(*i)->GetGameId()) {
			SendChatCommand(user,"Sentinel: "+(*i)->GetTeam1( ));
			SendChatCommand(user,"Scourge: "+(*i)->GetTeam2( ));

			if (!(*i)->GetObs().empty()) {
				SendChatCommand(user, "Obs: " + (*i)->GetObs());
			}
		}
	}
}

void CBNET::GetGameFromBots(string user, string botname) {
	string temp;
	transform( botname.begin( ), botname.end( ), botname.begin( ), (int(*)(int))tolower );
	uint32_t count = 0;
	uint32_t maxgames = 0;
	for (vector<CDBGames *>::iterator i = m_GHost->m_Games.begin(); i != m_GHost->m_Games.end(); i++) {
		temp = (*i)->GetHostbot();
		transform( temp.begin( ), temp.end( ), temp.begin( ), (int(*)(int))tolower );
		if (botname == temp) {
			count++;
		}
	}
	for (vector<CDBHostBot *>::iterator i = m_HostBots.begin(); i != m_HostBots.end();i++ ) {
		temp = (*i)->GetHostBot();
		transform( temp.begin( ), temp.end( ), temp.begin( ), (int(*)(int))tolower );
		if (botname == temp) {
			maxgames = (*i)->GetMaxGames();
		}
	}
	
	SendChatCommand(user,UTIL_ToString(count)+"/"+UTIL_ToString(maxgames)+ " games hosted from this hostbot.");
}

void CBNET::UpdateSite() {
	//delete the file before we write again
	remove( m_GHost->gPhpFile.c_str( ) );
	ofstream Log;
	Log.open( m_GHost->gPhpFile.c_str( ), ios::app );
	if (!Log.fail( ) )
	{
		//delete all data first
		
		//create default php data
		Log << "<?php require(\"config.php\") ?>" << endl;
		Log <<"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"<<endl;
		Log <<"<link rel=\"shortcut icon\" href=\"http://www.imageboo.com/files/ghde2uzbgtvsaczhc5bb.ico\" >"<<endl;
		Log <<"<html xmlns=\"http://www.w3.org/1999/xhtml\">"<<endl;
		Log <<"<head>"<<endl;
		Log <<"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" />"<<endl;
		Log <<"<title><?php echo $pagetitle ?></title>"<<endl;
		Log <<"<link rel=\"stylesheet\" type=\"text/css\" href=\"style.php\" />"<<endl;
		Log <<"<script type=\"text/javascript\">"<<endl;
		Log <<"function moreInfo(which){"<<endl;
		Log <<"var id = document.getElementById(which)"<<endl;
		Log <<"if (id.style.display == \"none\"){"<<endl;
		Log <<"id.style.display=\"block\""<<endl;
		Log <<"}else{"<<endl;
		Log <<"id.style.display=\"none\""<<endl;
		Log <<"}"<<endl;
		Log <<"}"<<endl;
		Log <<"</script>"<<endl;
		Log <<"</head>"<<endl;
		Log <<"<body>"<<endl;
		Log <<"<div class=\"preload\"><img src='banners/banner1.png' /><img src='banners/banner2.png' /><img src='banners/banner3.png' /><img src='banners/banner4.png' /></div>"<<endl;
		Log <<"<div id=\"wrap\"><table class=\"main\" aign=\"center\"><tr><td>&nbsp;</td><td><br /><div id=\"header\"><?php $banner=rand(1, 4); echo \"<img src='banners/banner\".$banner.\".png' />\"; ?>  </div>"<<endl;
		Log <<"</td><td>&nbsp;</td></tr>"<<endl;
		Log <<"<tr><td class=\"sidesize\"><!--Left main column-->"<<endl;
		Log<<"<table class=\"inner\">"<<endl;
		Log<<"<tr>"<<endl;
		Log<<"<th class=\"side\">Menu</th>"<<endl;
		Log<<"</tr>"<<endl;
		Log<<"<tr>"<<endl;
		Log<<"<td id=\"col\">"<<endl;
		Log <<"<?php require(\"menu.html\"); ?>"<<endl;
		Log <<"</td></tr></table></td><td class=\"centersize\"><!--Center main column--><div id=\"nav\">"<<endl;
		Log <<"<div align=\"center\" class=\"c b\">Channel infos</div></div><div id=\"content\">"<<endl;
		
		Log <<"<table border='0' align=\"center\" cellpadding=\"4px\" cellspacing=\"0px\">"<<endl;
		Log <<"<tr><td>"<<endl;
		Log <<"Updated at ["+GetTimeDate( )+"] server time <br/>"<<endl;
		//add game and users info
		string state;
		if (m_GHost->m_Games.size( )==0)
			Log << "There isn't any game in progress.<br/>"<<endl;
		else
		{
			for (vector<CDBGames *>::iterator i = m_GHost->m_Games.begin(); i != m_GHost->m_Games.end();i++ )
			{
				if ((*i)->GetGameState( )==0)
					state = "(lobby) Hosted";
				else if ((*i)->GetGameState( )==1)
					state = "Private";
				else if ((*i)->GetGameState( )==2)
					state = "Public";
				else if ((*i)->GetGameState( )==3)
					state = "Challenge";

				Log << "<span onclick=\"javascript: moreInfo('invis_"+UTIL_ToString((*i)->GetGameId( ))+"')\" style=\"cursor: pointer;\"><--------------------- '"+(*i)->GetGameName( )+"' ---------------------> </span><br /><span id='invis_"+UTIL_ToString((*i)->GetGameId( ))+"' style=\"display:none;\">"<<endl;
				if (!(*i)->GetTeam1( ).empty( ))
					Log << "Sentinel: "+(*i)->GetTeam1( ) +"<br />"<< endl;
				if (!(*i)->GetTeam2( ).empty( ))
					Log << "Scourge: "+(*i)->GetTeam2( )+"<br />" << endl;
				if (!(*i)->GetObs( ).empty( ))
					Log << "Observers: "+(*i)->GetObs( )+"<br />"<<endl;
				Log<< state+" game running for "+UTIL_HMSToString(GetTime( )-(*i)->GetStartTime())+" with owner "+ (*i)->GetOwner( )+ " <br />" <<endl;
				Log<<"<br/><br/> </span>"<<endl;
			}
		}		
		Log<<"</td><td>"<<endl;
		//here we print users
		Log<<"LVL <font color=\"#777777\">Guard</font> <font color=\"#ff0000\">1 </font><font color=\"#00ff00\">2 </font><font color=\"#0000ff\">3 </font><font color=\"#ffff00\">4 </font><font color=\"#00ffff\">5 </font><br/>"<<endl;//<font color=\"#ff00ff\">6 </font>
		for (vector<CDBChannel *>::iterator i = m_Users.begin(); i != m_Users.end(); i++ )
		{
			string tmp="";
			if ((*i)->GetLvl( )==1)
				tmp+="<font color=\"#ff0000\">";
			else if ((*i)->GetLvl( )==2)
				tmp+="<font color=\"#00ff00\">";
			else if ((*i)->GetLvl( )==3)
				tmp+="<font color=\"#0000ff\">";
			else if ((*i)->GetLvl( )==4)
				tmp+="<font color=\"#ffff00\">";
			else if ((*i)->GetLvl( )==5)
				tmp+="<font color=\"#00ffff\">";
			//else if ((*i)->GetLvl( )==6) hostbots aren't in channel any more
			//	tmp+="<font color=\"#ff00ff\">";
			else
				tmp+="<font color=\"#777777\">";

			tmp+=(*i)->GetUser( )+"</font> idle for "+UTIL_HMSToString( GetTime( ) - (*i)->GetSec( ));
			if ((*i)->GetSigned( ))
				tmp+=" (SIGNED)";

			for (uint32_t j=0;j<12;j++)
				if ((*i)->GetUser( )== m_meplay[j][0])
					tmp+=" (RESERVED slot "+UTIL_ToString(j+1)+")";

			Log<<tmp+"<br/>"<<endl;
		}

		Log<<"</td></tr>"<<endl;

		//add the end headers.
		Log <<"</table><br /></div><div id=\"footer\"><div class=\"c b\"><?php echo $footer; ?></div></td>"<<endl;
		Log <<"<td class=\"sidesize\"><!--Right main column-->"<<endl;
		Log <<"<table class=\"inner\">"<<endl;
		Log <<"<tr><td id='col'>"<<endl;
		Log <<"<script type=\"text/javascript\"><!--"<<endl;
		Log <<"google_ad_client = \"ca-pub-5645583359512135\";"<<endl;
		Log <<"google_ad_slot = \"9373775104\";"<<endl;
		Log <<"google_ad_width = 120;"<<endl;
		Log <<"google_ad_height = 240;"<<endl;
		Log <<"//-->"<<endl;
		Log <<"</script>"<<endl;
		Log <<"<script type=\"text/javascript\""<<endl;
		Log <<"src=\"http://pagead2.googlesyndication.com/pagead/show_ads.js\">"<<endl;
		Log <<"</script>"<<endl;
		Log <<"</td></tr></table>"<<endl;
		Log <<"</td></tr></table></div></body></html>"<<endl;
		//close php file
		Log.close();
	}
}

void CBNET::StartGame() {
	if (m_GameId < 999) {
		m_GameId++;
	}
	else {
		m_GameId = 0;
	}

	string Hostbot = GetAvailableHostbot();
	UpdateHostbot(Hostbot,2,GetHostbotGames( Hostbot )+1, 0);
	SendChatCommand(Hostbot ,UTIL_ToString(m_CommandTrigger)+"load "+Hostbot +" "+m_GHost->m_defaultMap);// load dota map just in case other user forgot to change map from hostbot
	SendChatCommand(Hostbot ,UTIL_ToString(m_CommandTrigger)+"hold "+m_Challenger+" "+UTIL_ToString(1));
	SendChatCommand(m_Challenger,"Challenge game start. Join gn ("+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate)+")");
	SendChatCommand(Hostbot ,UTIL_ToString(m_CommandTrigger)+"hold "+m_Challenged+" "+UTIL_ToString(6));								
	SendChatCommand(m_Challenged,"Challenge game start. Join gn ("+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate)+")");
	
	for (int j=1; j<12; j++) {
		if (!m_meplay[j][0].empty() && j!=5) {
			//j!=5 we already send the captain
			SendChatCommand(m_meplay[j][0],"Challenge game start. Join gn ("+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate)+")");
			SendChatCommand(Hostbot ,UTIL_ToString(m_CommandTrigger)+"hold "+m_meplay[j][0]+" "+UTIL_ToString(j+1));								
		}
	}
	
	SendChatCommand(Hostbot ,UTIL_ToString(m_CommandTrigger)+"hold "+UTIL_ToString( m_GameId+1000 )+" 13");//sent bot the id of game to hold in 13 slot of holdlist in table
	SendChatCommand(Hostbot ,UTIL_ToString(m_CommandTrigger)+"sendend");
	m_GHost->AddGame( m_GameId+1000, Hostbot, m_Challenger, GetTime( ), 10, 0, m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));
	SendChatCommand(Hostbot,UTIL_ToString(m_CommandTrigger)+"privby "+m_Challenger+" "+m_GHost->m_CommunityName+"-"+UTIL_ToString(m_GamedThatBotCreate));	

	if (!m_VotedMode.empty()) {
		QueueChatCommand("/w " + Hostbot + " " + UTIL_ToString(m_CommandTrigger) + "mode " + m_VotedMode);
	}
}

void CBNET::UnhostHostbots() {
	for (vector<CDBHostBot *>::iterator i = m_HostBots.begin(); i != m_HostBots.end();i++ )	{ 
		if ((*i)->GetState() == 2) {
			(*i)->UpdateState(1);
		}
	}
}

void CBNET::GetHostbots(string name) {
	bool tmp = false;
	for (vector<CDBHostBot *>::iterator i = m_HostBots.begin(); i != m_HostBots.end(); i++)	{ 
		tmp = true;
		if ((*i)->GetState() == 1) {
			SendChatCommand(name, "Ready to host - " + (*i)->GetHostBot() + " (" + UTIL_ToString((*i)->GetGames()) + "/" + UTIL_ToString((*i)->GetMaxGames()) + ")");
		}
		else if ((*i)->GetState() == 0) {
			SendChatCommand(name, "Not running ? - " + (*i)->GetHostBot());
		}
		else if ((*i)->GetState() == 2) {
			SendChatCommand(name, "In lobby - " + (*i)->GetHostBot() + " (" + UTIL_ToString((*i)->GetGames()) + "/" + UTIL_ToString((*i)->GetMaxGames()) + ")");
		}
		else {
			SendChatCommand(name, "Hostbot is disabled - " + (*i)->GetHostBot());
		}
	}
	if (!tmp) {
		SendChatCommand(name, "There isn't any hostbot in database");
	}
}

void CBNET::DelHostbot(string hostbot) {
	transform( hostbot.begin( ), hostbot.end( ), hostbot.begin( ), (int(*)(int))tolower );
	for (vector<CDBHostBot *>::iterator i = m_HostBots.begin(); i != m_HostBots.end(); ) {
		if ((*i)->GetHostBot() == hostbot) {
			i = m_HostBots.erase(i);
		}
		else {
			i++;
		}
	}
}

uint32_t CBNET::IsObs(string User) {
	uint32_t obs = 0;
	for (vector<CDBMaps *>::iterator i = m_Maps.begin(); i != m_Maps.end(); i++) {
		if (User == (*i)->GetUser()) {
			obs = (*i)->GetObs();
		}
	}

	return obs;
}

string CBNET::IsMap(string User) {
	string map = "";
	for (vector<CDBMaps *>::iterator i = m_Maps.begin(); i != m_Maps.end(); i++) {
		if (User == (*i)->GetUser()) {
			map = (*i)->GetMap();
		}
	}

	return map;
}

void CBNET::PrintMaps(string User) {
	try {
		boost::filesystem::path MapPath( m_GHost->m_MapPath );
		if (!exists( MapPath)) {
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error listing maps - map path doesn't exist", true );
			SendChatCommand(User, "Map path is wrong.");
		}
		else {
			boost::filesystem::directory_iterator EndIterator;
			uint32_t counter = 1;
			uint32_t linecounter = 1;
			string FoundMaps = "";
		
			for (boost::filesystem::directory_iterator i( MapPath ); i != EndIterator; i++) {
				
				if (FoundMaps.empty()) {
					FoundMaps = i->path().filename().string();
				}
				else {
					FoundMaps += " --- " + i->path().filename().string();
				}
				counter++;
				if (counter > 3 ) {
					counter = 1;
					linecounter++;
					SendChatCommand(User,FoundMaps);
					FoundMaps = "";
				}
			}
			if (counter > 1) {
				SendChatCommand(User, FoundMaps);
			}
			SendChatCommand(User,"There are current "+ UTIL_ToString(counter - 1 + (3 * (linecounter -1)))+" maps in server.");
		}
	}
	catch( const exception &ex ) {
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error listing maps - caught exception [" + ex.what( ) + "]", true );
		SendChatCommand(User,m_GHost->m_Language->ErrorListingMaps( ));
	}
}

void CBNET::SetObs( string User, uint32_t obs) {
	string obss = "You disable obs/ref.";
	for (vector<CDBMaps *>::iterator i = m_Maps.begin(); i != m_Maps.end(); i++ ) {
		if (User==(*i)->GetUser( )) {
			(*i)->UpdateObs( obs );
			if (obs == 1) {
				obss = "You enable obs in your map cfg.";
			}
			else if (obs == 2) {
				obss = "You enable ref in your map cfg.";
			}
			SendChatCommand(User,obss);
		}
	}
}

void CBNET::SetMap( string User, string Map) {
	string FoundMaps;
	try {
		boost::filesystem::path MapPath( m_GHost->m_MapPath );
		string Pattern = Map;
		transform( Pattern.begin( ), Pattern.end( ), Pattern.begin( ), (int(*)(int))tolower );

		if (!exists( MapPath ) ) {
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error listing maps - map path doesn't exist", true );
			SendChatCommand(User, "Map path is wrong.");
		}
		else {
			boost::filesystem::directory_iterator EndIterator;
			boost::filesystem::path LastMatch;
			uint32_t Matches = 0;

			for (boost::filesystem::directory_iterator i( MapPath ); i != EndIterator; i++) {
				string FileName = i->path( ).filename( ).string();
				string Stem = i->path( ).stem( ).string();
				transform( FileName.begin( ), FileName.end( ), FileName.begin( ), (int(*)(int))tolower );
				transform( Stem.begin( ), Stem.end( ), Stem.begin( ), (int(*)(int))tolower );

				if (!is_directory( i->status( ) ) && FileName.find( Pattern ) != string::npos) {
					LastMatch = i->path();
					Matches++;

					if (FoundMaps.empty()) {
						FoundMaps = i->path().filename().string();
					}
					else {
						FoundMaps += ", " + i->path().filename().string();
					}

					// if the pattern matches the filename exactly, with or without extension, stop any further matching
					if (FileName == Pattern || Stem == Pattern ) {
						Matches = 1;
						break;
					}
				}
			}

			if (Matches == 0) {
				SendChatCommand(User, m_GHost->m_Language->NoMapsFound());
			}
			else if (Matches == 1 ) {
				string File = LastMatch.filename( ).string();
				SendChatCommand(User, "Your map cfg is now ("+File +").");
				bool tmp = false;
				for (vector<CDBMaps *>::iterator i = m_Maps.begin(); i != m_Maps.end(); i++) {
					if (User==(*i)->GetUser( )) {
						(*i)->UpdateMap( File );
						tmp = true;
					}
				}
				if (!tmp) {
					m_Maps.push_back(new CDBMaps(User, File));
				}
			}
			else {
				SendChatCommand(User, m_GHost->m_Language->FoundMaps(FoundMaps));
			}
		}
	}
	catch(const exception &ex) {
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error listing maps - caught exception [" + ex.what( ) + "]", true );
		SendChatCommand(User,m_GHost->m_Language->ErrorListingMaps( ));
	}
}

void CBNET::DelMap(string User) {
	for (vector<CDBMaps *>::iterator i = m_Maps.begin(); i != m_Maps.end(); ) {
		if (User == (*i)->GetUser( ) ) {
			i = m_Maps.erase( i );
			SendChatCommand(User,"Custom map deleted. Your map now is latest dota version.");
		}
		else {
			i++;
		}
	}
}
