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

#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include "ghost.h"
#include "util.h"
#include "crc32.h"
#include "sha1.h"
#include "csvparser.h"
#include "config.h"
#include "language.h"
#include "socket.h"
#include "ghostdb.h"
#include "ghostdbsqlite.h"
#include "ghostdbmysql.h"
#include "bnet.h"
#include "md5.h"
#include "bnetprotocol.h"


#ifdef WIN32
  #include <ws2tcpip.h>		// for WSAIoctl
  #include <windows.h>
  #include <winsock.h>

  LARGE_INTEGER gHighPerfStart;
  LARGE_INTEGER gHighPerfFrequency;
#endif

#ifndef WIN32
  #include <sys/time.h>
#endif

#ifdef __APPLE__
  #include <mach/mach_time.h>
#endif

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)  

// UDP server
#ifdef WIN32
  typedef int _stdcall _UDPInit(void);
  typedef _UDPInit* UDPInit;
  typedef int _stdcall _UDPClose(void);
  typedef _UDPClose* UDPClose;
  typedef int _stdcall _UDPSend(PCHAR s);
  typedef _UDPSend* UDPSend;
  typedef PCHAR _stdcall _UDPWhoIs(PCHAR c, PCHAR s);
  typedef _UDPWhoIs* UDPWhoIs;

  UDPInit myudpinit;
  UDPClose myudpclose;
  UDPSend myudpsend;
  UDPWhoIs myudpwhois;

  HMODULE m_UDP;					// the chat server
#endif

// Globals
string gCFGFile;
string gLogFile;

uint32_t gLogMethod;
ofstream *gLog = NULL;
CGHost *gGHost = NULL;

uint32_t GetTime() {
	return GetTicks() / 1000;
}

string GetDate() {
	time_t Now = time(NULL);
	char Time[20];
	memset(Time, 0, sizeof(char) * 20);
	strftime(Time, sizeof(char) * 20, "%Y-%m-%d", localtime(&Now));
	return string(Time); //%H:%M:%S
}

string GetTimeDate() {
	time_t Now = time(NULL);
	string Time = asctime(localtime(&Now));
	Time.erase(Time.size() - 1);
	return Time ;
}

uint32_t GetTicks() {
#ifdef WIN32
	// don't use GetTickCount anymore because it's not accurate enough (~16ms resolution)
	// use a high performance timer instead
	// and make sure to always query the same processor
	// note: this code is a LOT slower than GetTickCount, it might be better to only call it once per loop and store the result

	LARGE_INTEGER HighPerfStop;
	DWORD_PTR OldMask = SetThreadAffinityMask(GetCurrentThread(), 0);
	QueryPerformanceCounter(&HighPerfStop);
	SetThreadAffinityMask(GetCurrentThread(), OldMask);
	return (uint32_t)((HighPerfStop.QuadPart - gHighPerfStart.QuadPart) * 1000 / gHighPerfFrequency.QuadPart);
#elif __APPLE__
	uint64_t current = mach_absolute_time();
	static mach_timebase_info_data_t info = { 0, 0 };
	// get timebase info
	if (info.denom == 0)
		mach_timebase_info(&info);
	uint64_t elapsednano = current * (info.numer / info.denom);
	// convert ns to ms
	return elapsednano / 1e6;
#else
	uint32_t ticks;
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	ticks = t.tv_sec * 1000;
	ticks += t.tv_nsec / 1000000;
	return ticks;
#endif
}

void SignalCatcher2(int s) {
	CONSOLE_Print("[!!!] caught signal " + UTIL_ToString(s) + ", exiting NOW", true);

	if (gGHost) {
		if (gGHost->m_Exiting) {
			exit(1);
		}
		else {
			gGHost->m_Exiting = true;
		}
	}
	else {
		exit(1);
	}
}

void SignalCatcher(int s) {
	// signal(SIGABRT, SignalCatcher2);
	signal(SIGINT, SignalCatcher2);

	CONSOLE_Print("[!!!] caught signal " + UTIL_ToString(s) + ", exiting nicely", true);

	if (gGHost) {
		gGHost->m_ExitingNice = true;
	}
	else {
		exit(1);
	}
}

void CONSOLE_Print(string message, bool log) {
	cout << message << endl;

	// logging	
	if (!gLogFile.empty() && log) {
		if (gLogMethod == 1) {
			ofstream Log;
			Log.open(gLogFile.c_str(), ios :: app);

			if (!Log.fail()) {
				time_t Now = time(NULL);
				string Time = asctime(localtime(&Now));

				// erase the newline

				Time.erase(Time.size() - 1);
				Log << "[" << Time << "] " << message << endl;
				Log.close();
			}
		}
		else if (gLogMethod == 2) {
			if (gLog && !gLog->fail()) {
				time_t Now = time(NULL);
				string Time = asctime(localtime(&Now));

				// erase the newline

				Time.erase(Time.size() - 1);
				*gLog << "[" << Time << "] " << message << endl;
				gLog->flush();
			}
		}
	}
}

void DEBUG_Print(string message) {
	cout << message << endl;
}

void DEBUG_Print(BYTEARRAY b) {
	cout << "{ ";

	for (unsigned int i = 0; i < b.size(); i++) {
		cout << hex << (int)b[i] << " ";
	}

	cout << "}" << endl;
}

//
// Main
//
int main(int argc, char **argv) {

	gCFGFile = "guard.cfg";
	if (argc > 1 && argv[1]) {
		gCFGFile = argv[1];
	}

	// Read config file
	CConfig CFG;
	CFG.Read(gCFGFile);
	gLogFile = CFG.GetString("bot_log", string());
	gLogMethod = CFG.GetInt("bot_logmethod", 1);

	if (!gLogFile.empty()) {
		if (gLogMethod == 1) {
			// log method 1: open, append, and close the log for every message
			// this works well on Linux but poorly on Windows, particularly as the log file grows in size
			// the log file can be edited/moved/deleted while GHost++ is running
		}
		else if (gLogMethod == 2) {
			// log method 2: open the log on startup, flush the log for every message, close the log on shutdown
			// the log file CANNOT be edited/moved/deleted while GHost++ is running
			gLog = new ofstream();
			gLog->open(gLogFile.c_str(), ios :: app);
		}
	}

	CONSOLE_Print("[GHOST] starting up", true);

	if (!gLogFile.empty()) {
		if (gLogMethod == 1) {
			CONSOLE_Print("[GHOST] using log method 1, logging is enabled and [" + gLogFile + "] will not be locked", true);
		}
		else if (gLogMethod == 2) {
			if (gLog->fail()) {
				CONSOLE_Print("[GHOST] using log method 2 but unable to open [" + gLogFile + "] for appending, logging is disabled", true);
			}
			else {
				CONSOLE_Print("[GHOST] using log method 2, logging is enabled and [" + gLogFile + "] is now locked", true);
			}
		}
	}
	else {
		CONSOLE_Print("[GHOST] no log file specified, logging is disabled", true);
	}

	// catch SIGINT
	signal(SIGINT, SignalCatcher);

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL
	signal(SIGPIPE, SIG_IGN);
#endif

#ifdef WIN32
	// initialize high performance timer

	DWORD_PTR OldMask = SetThreadAffinityMask(GetCurrentThread(), 0);
	
	if (!QueryPerformanceFrequency(&gHighPerfFrequency)) {
		// without a performance frequency we can't convert the performance counter to a meaningful value
		// some possible solutions: revert to using GetTickCount or try another type of timer
		// for now we just exit

		CONSOLE_Print("[GHOST] error getting Windows high performance timer resolution (error " + UTIL_ToString(GetLastError()) + ")" , true);
		return 1;
	}

	QueryPerformanceCounter(&gHighPerfStart);
	SetThreadAffinityMask(GetCurrentThread(), OldMask);

	// print the timer resolution

	CONSOLE_Print("[GHOST] using Windows high performance timer with resolution " + UTIL_ToString((double)(1000000.0 / gHighPerfFrequency.QuadPart), 2) + " microseconds", true);
#elif __APPLE__
	// not sure how to get the resolution
#else
	// print the timer resolution

	struct timespec Resolution;

	if (clock_getres(CLOCK_MONOTONIC, &Resolution) == -1)
		CONSOLE_Print("[GHOST] error getting monotonic timer resolution", true);
	else
		CONSOLE_Print("[GHOST] using monotonic timer with resolution " + UTIL_ToString((double)(Resolution.tv_nsec / 1000), 2) + " microseconds", true);
#endif

#ifdef WIN32
	// initialize winsock
	CONSOLE_Print("[GHOST] starting winsock", true);
	WSADATA wsadata;

	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		CONSOLE_Print("[GHOST] error starting winsock", true);
		return 1;
	}

	// increase process priority

	CONSOLE_Print("[GHOST] setting process priority to \"above normal\"", true);
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	/* Priorities
		REALTIME_PRIORITY_CLASS		
		HIGH_PRIORITY_CLASS
		ABOVE_NORMAL_PRIORITY_CLASS
		NORMAL_PRIORITY_CLASS 
		BELOW_NORMAL_PRIORITY_CLASS 
		IDLE_PRIORITY_CLASS 
	*/
#endif

	// initialize ghost
	gGHost = new CGHost(&CFG);

	while (1) {
		// block for 50ms on all sockets - if you intend to perform any timed actions more frequently you should change this
		// that said it's likely we'll loop more often than this due to there being data waiting on one of the sockets but there aren't any guarantees
		
		if (gGHost->Update(50000)) {
			break;
		}
	}

	// shutdown ghost
	CONSOLE_Print("[GHOST] shutting down", true);
	delete gGHost;
	gGHost = NULL;

#ifdef WIN32
	// shutdown winsock
	CONSOLE_Print("[GHOST] shutting down winsock" , true);
	WSACleanup();
#endif

	if (gLog) {
		if (!gLog->fail()) {
			gLog->close();
		}

		delete gLog;
	}

	return 0;
}

//
// CGHost
//
CGHost :: CGHost(CConfig *CFG) 
{
#ifdef WIN32
	LPCWSTR udpl=L"udpserver.dll";
	m_UDP = LoadLibraryW(udpl);

	myudpinit=(UDPInit)::GetProcAddress(m_UDP,"UDPInit");
	myudpclose=(UDPClose)::GetProcAddress(m_UDP,"UDPClose");
	//myudpsend=(UDPSend)::GetProcAddress(m_UDP,"UDPSend");
	myudpwhois=(UDPWhoIs)::GetProcAddress(m_UDP,"UDPWhoIs");

	myudpinit();
#endif
	m_UDPSocket = new CUDPSocket();
	m_UDPSocket->SetBroadcastTarget(CFG->GetString("udp_broadcasttarget", string()));
	m_UDPSocket->SetDontRoute(CFG->GetInt("udp_dontroute", 0) == 0 ? false : true);
	m_CRC = new CCRC32();
	m_CRC->Initialize();
	m_SHA = new CSHA1();

	CONSOLE_Print("[GHOST] opening primary MySQL database", true);
	m_DB = new CGHostDBMySQL(CFG);
	
	CONSOLE_Print("[GHOST] opening secondary (local) database" , true);

	// get a list of local IP addresses
	// this list is used elsewhere to determine if a player connecting to the bot is local or not

	CONSOLE_Print("[GHOST] attempting to find local IP addresses" , true);

#ifdef WIN32
	// use a more reliable Windows specific method since the portable method doesn't always work properly on Windows
	// code stolen from: http://tangentsoft.net/wskfaq/examples/getifaces.html

	SOCKET sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);

	if (sd == SOCKET_ERROR) {
		CONSOLE_Print("[GHOST] error finding local IP addresses - failed to create socket (error code " + UTIL_ToString(WSAGetLastError()) + ")", true);
	}
	else {
		INTERFACE_INFO InterfaceList[20];
		unsigned long nBytesReturned;

		if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList, sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR) {
			CONSOLE_Print("[GHOST] error finding local IP addresses - WSAIoctl failed (error code " + UTIL_ToString(WSAGetLastError()) + ")", true);
		}
		else {
			int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

			for (int i = 0; i < nNumInterfaces; i++) {
				sockaddr_in *pAddress;
				pAddress = (sockaddr_in *)&(InterfaceList[i].iiAddress);
				CONSOLE_Print("[GHOST] local IP address #" + UTIL_ToString(i + 1) + " is [" + string(inet_ntoa(pAddress->sin_addr)) + "]", true);
				m_LocalAddresses.push_back(UTIL_CreateByteArray((uint32_t)pAddress->sin_addr.s_addr, false));
			}
		}

		closesocket(sd);
	}
#else
	// use a portable method

	char HostName[255];

	if (gethostname(HostName, 255) == SOCKET_ERROR) {
		CONSOLE_Print("[GHOST] error finding local IP addresses - failed to get local hostname", true);
	}
	else {
		CONSOLE_Print("[GHOST] local hostname is [" + string(HostName) + "]", true);
		struct hostent *HostEnt = gethostbyname(HostName);

		if (!HostEnt) {
			CONSOLE_Print("[GHOST] error finding local IP addresses - gethostbyname failed", true);
		}
		else {
			for (int i = 0; HostEnt->h_addr_list[i] != NULL; i++) {
				struct in_addr Address;
				memcpy(&Address, HostEnt->h_addr_list[i], sizeof(struct in_addr));
				CONSOLE_Print("[GHOST] local IP address #" + UTIL_ToString(i + 1) + " is [" + string(inet_ntoa(Address)) + "]" , true);
				m_LocalAddresses.push_back(UTIL_CreateByteArray((uint32_t)Address.s_addr, false));
			}
		}
	}
#endif

	gPhpFile = CFG->GetString("bot_phpdir", "realm");
	gPhpFile+=".php";
	
	m_SiteLastUpdateTimer = GetTime();
	m_SiteUpdateSupported = CFG->GetInt("bot_php", 0) == 0 ? false : true;
	m_SiteUpdatedTime = CFG->GetInt("bot_phptimer", 60);	
	m_PoolLimit = CFG->GetInt("bot_poollimit", 256);
	m_UDPCommandSocket = new CUDPServer();
	m_UDPGuardPort = CFG->GetInt("udp_cmdport", 5000);
	m_UDPCommandSocket->Bind(CFG->GetString("udp_cmdbindip", "127.0.0.1"), m_UDPGuardPort);

	m_Language = NULL;
	m_Exiting = false;
	m_ExitingNice = false;
	m_Version = "13.0.0";
	m_Versionb = "f";
	m_HostCounter = 1;
	m_PrintPubLadder  = CFG->GetInt("bot_printpub", 1) == 0 ? false : true;

	m_ForumUrl = CFG->GetString("site_forum", "http://eurobattle.net");
	m_StatsUrl = CFG->GetString("site_stats", "http://eurobattle.net");
	m_TFT = CFG->GetInt("bot_tft", 1) == 0 ? false : true;
	
	if (m_TFT) {
		CONSOLE_Print("[GHOST] acting as Warcraft III: The Frozen Throne", true);
	}
	else {
		CONSOLE_Print("[GHOST] acting as Warcraft III: Reign of Chaos", true);
	}	

	m_MailEnabled = CFG->GetInt("bot_mail", 1) == 0 ? false : true;;
	m_MailMaxSended = CFG->GetInt("bot_maxmailssend", 10);
	m_MailMaxReceived = CFG->GetInt("bot_maxmailsreceive", 5);;
	
	m_OldLadderSystem = CFG->GetInt("bot_oldladdersystem", 0) == 0 ? false : true;
	m_AddPoints = CFG->GetInt("bot_addpoints", 5);
	m_RemPoints = CFG->GetInt("bot_rempoints", 3);
	
	m_IsOperator = CFG->GetInt("bot_operator", 1) == 0 ? false : true;
	m_AutoCreate = CFG->GetInt("bot_autocreate", 1) == 0 ? false : true;
	m_ChallengeSafeRange = CFG->GetInt("bot_challengesaferange", 10);
	
	gFlamesFile = CFG->GetString("bot_flames", "flames.txt");
	m_FlameWordsNumber = 0;
	
	gIconsFile = CFG->GetString("bot_icons", "icons.txt");
	
	m_FreeHostFromCountry = CFG->GetString("bot_freehostbycountry", "...");	
	transform(m_FreeHostFromCountry.begin(), m_FreeHostFromCountry.end(), m_FreeHostFromCountry.begin(), (int(*)(int))toupper);
	
	m_DefaultAutoLvl= CFG->GetInt("bot_defaultautolvl", 0);
	m_CommunityName = CFG->GetString("bot_community", "...");
	m_CommunityNameLower = m_CommunityName;
	transform(m_CommunityNameLower.begin(), m_CommunityNameLower.end(), m_CommunityNameLower.begin(), (int(*)(int))tolower);

	m_AllowTopaz = CFG->GetInt("bot_chatclient", 1) == 0 ? false : true;
	m_FirstWelcomeChannel = CFG->GetString("bot_fwchannel", "fwchannel.txt");
	m_News = CFG->GetString("bot_news","news.txt");
	m_IconSupport = CFG->GetInt("bot_iconsystem", 0) == 0 ? false : true;
	m_IconTimer = CFG->GetInt("bot_icontimer", 86400);
	m_CountrySign = CFG->GetString("bot_countrysign", "");
	transform(m_CountrySign.begin(), m_CountrySign.end(), m_CountrySign.begin(), (int(*)(int))toupper);

	//Here we load in table all command option (enable or not) from cfg file
	if (CFG->GetInt("bot_comallenable", 1) == 0 ? false : true) {
		for (int i = 1; i < 100; i++) {
			commands[i] = true;
		}
	}
	else {
		CONSOLE_Print("[Console: Read custom commands.", true);
		for (int i = 1; i < 100; i++) {
			commands[i] = CFG->GetInt("bot_com" + UTIL_ToString(i), 1) == 0 ? false : true;
		}
	}
	
	m_LANWar3Version = CFG->GetInt("lan_war3version", 24);
	SetConfigs(CFG);

	// load the battle.net connections
	// we're just loading the config data and creating the CBNET classes here, the connections are established later (in the Update function)
	for (uint32_t i = 1; i < 10; i++) {
		string Prefix;

		if (i == 1) {
			Prefix = "bnet_";
		}
		else {
			Prefix = "bnet" + UTIL_ToString(i) + "_";
		}

		string Server = CFG->GetString(Prefix + "server", string());
		string ServerAlias = CFG->GetString(Prefix + "serveralias", string());
		string CDKeyROC = CFG->GetString(Prefix + "cdkeyroc", string());
		string CDKeyTFT = CFG->GetString(Prefix + "cdkeytft", string());
		string CountryAbbrev = CFG->GetString(Prefix + "countryabbrev", "USA");
		string Country = CFG->GetString(Prefix + "country", "United States");
		string UserName = CFG->GetString(Prefix + "username", string());
		string UserPassword = CFG->GetString(Prefix + "password", string());
		string FirstChannel = CFG->GetString(Prefix + "firstchannel", "The Void");
		string RootAdmin = CFG->GetString(Prefix + "rootadmin", string());
		string BNETCommandTrigger = CFG->GetString(Prefix + "commandtrigger", "!");

		if (BNETCommandTrigger.empty()) {
			BNETCommandTrigger = "!";
		}

		bool HoldFriends = CFG->GetInt(Prefix + "holdfriends", 1) == 0 ? false : true;
		bool HoldClan = CFG->GetInt(Prefix + "holdclan", 1) == 0 ? false : true;
		bool PublicCommands = CFG->GetInt(Prefix + "publiccommands", 1) == 0 ? false : true;
		string BNLSServer = CFG->GetString(Prefix + "bnlsserver", string());
		int BNLSPort = CFG->GetInt(Prefix + "bnlsport", 9367);
		int BNLSWardenCookie = CFG->GetInt(Prefix + "bnlswardencookie", 0);
		unsigned char War3Version = CFG->GetInt(Prefix + "custom_war3version", 24);
		BYTEARRAY EXEVersion = UTIL_ExtractNumbers(CFG->GetString(Prefix + "custom_exeversion", string()), 4);
		BYTEARRAY EXEVersionHash = UTIL_ExtractNumbers(CFG->GetString(Prefix + "custom_exeversionhash", string()), 4);
		string PasswordHashType = CFG->GetString(Prefix + "custom_passwordhashtype", string());
		string PVPGNRealmName = CFG->GetString(Prefix + "custom_pvpgnrealmname", "PvPGN Realm");
		uint32_t MaxMessageLength = CFG->GetInt(Prefix + "custom_maxmessagelength", 200);

		if (Server.empty()) {
			break;
		}

		if (CDKeyROC.empty()) {
			CONSOLE_Print("[GHOST] missing " + Prefix + "cdkeyroc, skipping this battle.net connection", true);
			continue;
		}

		if (m_TFT && CDKeyTFT.empty()) {
			CONSOLE_Print("[GHOST] missing " + Prefix + "cdkeytft, skipping this battle.net connection", true);
			continue;
		}

		if (UserName.empty()) {
			CONSOLE_Print("[GHOST] missing " + Prefix + "username, skipping this battle.net connection", true);
			continue;
		}

		if (UserPassword.empty()) {
			CONSOLE_Print("[GHOST] missing " + Prefix + "password, skipping this battle.net connection", true);
			continue;
		}

		CONSOLE_Print("[GHOST] found battle.net connection #" + UTIL_ToString(i) + " for server [" + Server + "]", true);
		m_BNETs.push_back(new CBNET(this, Server, ServerAlias, BNLSServer, (uint16_t)BNLSPort, (uint32_t)BNLSWardenCookie, CDKeyROC, CDKeyTFT, CountryAbbrev, Country, UserName, UserPassword, FirstChannel, RootAdmin, BNETCommandTrigger[0], HoldFriends, HoldClan, PublicCommands, War3Version, EXEVersion, EXEVersionHash, PasswordHashType, PVPGNRealmName, MaxMessageLength, i));
	}

	if (m_BNETs.empty()) {
		CONSOLE_Print("[GHOST] warning - no battle.net connections found in config file", true);
	}

	LoadIPToCountryData();
	
	CONSOLE_Print("[GHOST] Guard bot Version " + m_Version+m_Versionb + " (with MySQL support)", true);
}

CGHost :: ~CGHost() {
	
	delete m_UDPSocket;
	delete m_CRC;
	delete m_SHA;

	for (vector<CBNET *> ::iterator i = m_BNETs.begin(); i != m_BNETs.end(); i++) {
		delete *i;
	}

	
	delete m_DB;

	// warning: we don't delete any entries of m_Callables here because we can't be guaranteed that the associated threads have terminated
	// this is fine if the program is currently exiting because the OS will clean up after us
	// but if you try to recreate the CGHost object within a single session you will probably leak resources!

	if (!m_Callables.empty()) {
		CONSOLE_Print("[GHOST] warning - " + UTIL_ToString(m_Callables.size()) + " orphaned callables were leaked (this is not an error)", true);
	}

	delete m_Language;
}

bool CGHost :: Update(long usecBlock) {
	uint32_t debugStart=GetTicks();

	if (m_DB->HasError()) {
		CONSOLE_Print("[GHOST] database error - " + m_DB->GetError() , true);
		return true;
	}

	//Try to exit nicely if requested to do so
	if (m_ExitingNice) {
		if (!m_BNETs.empty()) {
			CONSOLE_Print("[GHOST] deleting all battle.net connections in preparation for exiting nicely" , true);

			for (vector<CBNET *> ::iterator i = m_BNETs.begin(); i != m_BNETs.end(); i++) {
				delete *i;
			}
			m_BNETs.clear();
		}

		if (m_Callables.empty()) {
			CONSOLE_Print("[GHOST] all threads finished, exiting nicely" , true);
			m_Exiting = true;
		}		
	}

	// update callables
	for (vector<CBaseCallable *> :: iterator i = m_Callables.begin(); i != m_Callables.end();) {
		if ((*i)->GetReady()) {
			m_DB->RecoverCallable(*i);
			delete *i;
			i = m_Callables.erase(i);
		}
		else {
			i++;
		}
	}

	unsigned int NumFDs = 0;

	// take every socket we own and throw it in one giant select statement so we can block on all sockets
	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO(&fd);
	FD_ZERO(&send_fd);

	// 1. all battle.net sockets
	for (vector<CBNET *> ::iterator i = m_BNETs.begin(); i != m_BNETs.end(); i++) {
		NumFDs += (*i)->SetFD(&fd, &send_fd, &nfds);
	}

	// 2. udp command receiving socket
	m_UDPCommandSocket->SetFD(&fd,  &send_fd, &nfds);
	NumFDs++;

	if (usecBlock < 1000) {
		usecBlock = 1000;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usecBlock;

	struct timeval send_tv;
	send_tv.tv_sec = 0;
	send_tv.tv_usec = 0;

#ifdef WIN32
	select(1, &fd, NULL, NULL, &tv);
	select(1, NULL, &send_fd, NULL, &send_tv);
#else
	select(nfds + 1, &fd, NULL, NULL, &tv);
	select(nfds + 1, NULL, &send_fd, NULL, &send_tv);
#endif

	if (NumFDs == 0) {
		// we don't have any sockets (i.e. we aren't connected to battle.net maybe due to a lost connection and there aren't any games running)
		// select will return immediately and we'll chew up the CPU if we let it loop so just sleep for 50ms to kill some time
		MILLISLEEP(50);
	}

	bool BNETExit = false;

	// update battle.net connections
	for (vector<CBNET *> :: iterator i = m_BNETs.begin(); i != m_BNETs.end(); i++) {
		if ((*i)->Update(&fd, &send_fd)) {
			BNETExit = true;
		}
	}

	// UDP COMMANDSOCKET CODE
	sockaddr_in recvAddr;
	string udpcommand;
	m_UDPCommandSocket->RecvFrom(&fd, &recvAddr, &udpcommand);

	if (udpcommand.size()) {
		// default server to relay the message to
		string udptarget = m_UDPCommandSpoofTarget;
		bool relayed = false;

		string Hostbot;
		string Command; 
		stringstream SS;
		SS << udpcommand;
		SS >> Hostbot;

		if (!SS.eof()) {
			getline(SS, Command);
			string :: size_type Start = Command.find_first_not_of(" ");

			if (Start != string::npos) {
				Command = Command.substr(Start);
			}
		}

		// special case, a udp command starting with || . ex: ||readwelcome
		if (Command.substr(0,1)=="-") { 
			UDPCommands(string(inet_ntoa(recvAddr.sin_addr))+" "+Command.substr(0,Command.length()));
		} 
		else {			
			string tmp = Command;
			string Victim;
			string Reason; 
			stringstream SS;
			SS << tmp;
			SS >> Victim;

			if (!SS.eof()) {
				getline(SS, Reason);
				string :: size_type Start = Reason.find_first_not_of(" ");

				if (Start != string::npos) {
					Reason = Reason.substr(Start);
				}
			}
			
			for (vector<CBNET *> :: iterator i = m_BNETs.begin(); i != m_BNETs.end(); i++) {				
				// is this the right one or should we just send it to the first in list?
				if (udptarget == (*i)->GetServer() || udptarget.empty()) {
						
					CONSOLE_Print("[UDPCMDSOCK] HB ("+Hostbot+") Relaying cmd [" + Command + "] to server [" + (*i)->GetServer() + "]", true);

					// spoof a whisper from the rootadmin belonging to this connection
					CIncomingChatEvent *chatCommand = new CIncomingChatEvent(CBNETProtocol::EID_WHISPER, 0, Hostbot, Command);
					(*i)->ProcessChatEvent(chatCommand);
					relayed = true;
					break;
				}
			}
			if (!relayed) {
				CONSOLE_Print("[UDPCMDSOCK] Could not relay cmd [" + udpcommand + "] to server [" + udptarget + "]: server unknown", true);
			}
		}
	}

	uint32_t debugEnd=GetTicks();
	if ((debugEnd - debugStart) > 2000) {
		CONSOLE_Print("GUARD UPDATE TOOK MORE THAN 2 SECONDS!!!");
	}

	return m_Exiting || BNETExit;
}

void CGHost :: EventBNETConnecting(CBNET *bnet) {
	
}

void CGHost :: EventBNETConnected(CBNET *bnet) {

}

void CGHost :: EventBNETDisconnected(CBNET *bnet) {
	
}

void CGHost :: EventBNETLoggedIn(CBNET *bnet) {
	
}

void CGHost :: EventBNETGameRefreshed(CBNET *bnet) {
	
}

void CGHost :: EventBNETGameRefreshFailed(CBNET *bnet) {
	
}

void CGHost :: EventBNETConnectTimedOut(CBNET *bnet) {
	
}

void CGHost :: EventBNETWhisper(CBNET *bnet, string user, string message) {
	
}

void CGHost :: EventBNETChat(CBNET *bnet, string user, string message) {
	
}

void CGHost :: EventBNETEmote(CBNET *bnet, string user, string message) {
	
}

void CGHost :: UDPCommands(string Message) {
	string User;
	string Command;
	string Payload;
	string :: size_type CommandStart = Message.find(" ");

	if (CommandStart != string :: npos) {
		User = Message.substr(0, CommandStart);
		Message = Message.substr(CommandStart + 2);
	}
	else {
		Message = Message.substr(1);
	}

	m_LastIp = User;

	string :: size_type PayloadStart = Message.find(" ");

	if (PayloadStart != string :: npos) {
		Command = Message.substr(0, PayloadStart);
		Payload = Message.substr(PayloadStart + 1);
	}
	else {
		Command = Message.substr(0);
	}

	transform(Command.begin(), Command.end(), Command.begin(), (int(*)(int))tolower);

	CONSOLE_Print("[GHOST] received UDP command [" + Command + "] with payload [" + Payload + "]"+" from User ["+User+"]" , true);
	
	/*********************
	*     UDP command    *
	*********************/
	if (Command == "say" && !Payload.empty()) {
		for (vector<CBNET *> ::iterator i = m_BNETs.begin(); i != m_BNETs.end(); i++) {
			(*i)->QueueChatCommand(Payload);
		}
	}

	if (Command=="pg" && !Payload.empty()) {						
		stringstream SS;
		SS << Payload;
		string tbl[11];
		uint32_t counter=0;
		while(!SS.eof()) {
			string words;
			SS >> words;

			if (SS.fail()) {
				break;
			}
			else {//userthatbanned fromuser reason gamename tbl[0] is game id to del it from games
				if (counter > 3) {
					tbl[3] += " " + words;
				}
				else {
					tbl[counter] = words;
				}

				counter++;
			}
		}
		if (counter >= 4) {
			uint32_t gameid = UTIL_ToUInt32(tbl[0]);
			uint32_t starttime = GetTime();
			uint32_t gamestate = UTIL_ToUInt32(tbl[2]);
			AddGame(gameid,User,tbl[1],starttime,10,gamestate,tbl[3]);
		}
	}
	
	if (Command=="pu" && !Payload.empty()) {
		//gameid username slotnumber
		stringstream SS;
		SS << Payload;
		string tbl[11];
		uint32_t counter=0;
		while(!SS.eof()) {
			string words;
			SS >> words;

			if (SS.fail()) {
				break;
			}
			else { //userthatbanned fromuser reason gamename tbl[0] is game id to del it from games
				tbl[counter]=words;
				counter++;
			}
		}	
	}
	
	if (Command=="ugn" &&!Payload.empty()) {
		string Id;
		string variable; 
		stringstream SS;
		SS << Payload;
		SS >> Id;

		if (!SS.eof()) {
			getline(SS, variable);
			string :: size_type Start = variable.find_first_not_of(" ");

			if (Start != string::npos) {
				variable = variable.substr(Start);
			}
		}
		uint32_t id = UTIL_ToUInt32(Id);
		UpdateGameName(id,variable);
		UpdateGameTime(id,GetTime(),false);
	}

	if (Command=="ugt" &&!Payload.empty()) {
		uint32_t id = UTIL_ToUInt32(Payload);
		UpdateGameTime(id,GetTime(),true);
	}

	if (Command=="ugo" &&!Payload.empty()) {
		string Id;
		string variable; 
		stringstream SS;
		SS << Payload;
		SS >> Id;

		if (!SS.eof()) {
			getline(SS, variable);
			string :: size_type Start = variable.find_first_not_of(" ");

			if (Start != string::npos) {
				variable = variable.substr(Start);
			}
		}
		uint32_t id = UTIL_ToUInt32(Id);
		UpdateGameOwner(id,variable);
	}
	
	if (Command=="ugsl" &&!Payload.empty()) {
		stringstream SS;
		SS << Payload;
		string tbl[3];
		uint32_t counter=0;
		
		while (!SS.eof()) {
			string words;
			SS >> words;

			if (SS.fail()) {
				break;
			}
			else {
				tbl[counter]=words;
				counter++;
			}
		}
		if (counter == 2) {
			uint32_t id = UTIL_ToUInt32(tbl[0]);
			uint32_t slots_num = UTIL_ToUInt32(tbl[1]);
			UpdateGameSlots(id, slots_num,"");	
		}
		else if (counter==3) {
			uint32_t id = UTIL_ToUInt32(tbl[0]);
			uint32_t slots_num = UTIL_ToUInt32(tbl[1]);
			UpdateGameSlots(id, slots_num, tbl[2]);
		}
	}

	if (Command=="ugs" &&!Payload.empty()) {
		string Id;
		string variable; 
		stringstream SS;
		SS << Payload;
		SS >> Id;

		if (!SS.eof()) {
			getline(SS, variable);
			string :: size_type Start = variable.find_first_not_of(" ");

			if (Start != string::npos) {
				variable = variable.substr(Start);
			}
		}
		uint32_t state = UTIL_ToUInt32(variable);
		uint32_t id = UTIL_ToUInt32(Id);
		UpdateGameState(id,state);
	}

	if (Command=="delgame" && !Payload.empty()) {
		uint32_t id = UTIL_ToUInt32(Payload);
		DelGame(id);
	}

	if (Command=="addnames" && !Payload.empty()) {
		stringstream SS;
		SS << Payload;
		string tbl[3];
		uint32_t counter=0;
		
		while (!SS.eof()) {
			string words;
			SS >> words;

			if (SS.fail()) {
				break;
			}
			else {
				if (counter < 3) {
					tbl[counter] = words;
				}
				counter++;
			}
		}
		if (counter == 3) {
			uint32_t id = UTIL_ToUInt32(tbl[0]);
			uint32_t slot = UTIL_ToUInt32(tbl[2]);

			AddNames(id, tbl[1], slot);
		}		
	}
}

void CGHost :: ReloadConfigs() {
	CConfig CFG;
	CFG.Read(gCFGFile);
	SetConfigs(&CFG);
}

void CGHost :: SetConfigs(CConfig *CFG) {
	// this doesn't set EVERY config value since that would potentially require reconfiguring the battle.net connections
	// it just set the easily reloadable values

	m_LanguageFile = CFG->GetString("bot_language", "language.cfg");
	delete m_Language;
	m_Language = new CLanguage(m_LanguageFile);
	m_Warcraft3Path = UTIL_AddPathSeperator(CFG->GetString("bot_war3path", "./w3"));
	m_BindAddress = CFG->GetString("bot_bindaddress", string());
	string BotCommandTrigger = CFG->GetString("bot_commandtrigger", "!");
	m_MapPath = UTIL_AddPathSeperator(CFG->GetString("bot_mappath", string()));
	m_defaultMap = CFG->GetString("bot_defaultmap", "map");

	if (BotCommandTrigger.empty()) {
		BotCommandTrigger = "!";
	}

	m_CommandTrigger = BotCommandTrigger[0];

	gLogFile = CFG->GetString("bot_log", string());
	gLogMethod = CFG->GetInt("bot_logmethod", 1);

	m_ForumUrl = CFG->GetString("site_forum", "http://eurobattle.net");
	m_StatsUrl = CFG->GetString("site_stats", "http://eurobattle.net");

	m_IconSupport = CFG->GetInt("bot_iconsystem", 0) == 0 ? false : true;
	gIconsFile = CFG->GetString("bot_icons", "icons.txt");

	gFlamesFile = CFG->GetString("bot_flames", "flames.txt");
	m_FlameWordsNumber = 0;

	m_MailEnabled = CFG->GetInt("bot_mail", 1) == 0 ? false : true;;
	m_MailMaxSended = CFG->GetInt("bot_maxmailssend", 10);
	m_MailMaxReceived = CFG->GetInt("bot_maxmailsreceive", 5);

	m_AutoCreate = CFG->GetInt("bot_autocreate", 1) == 0 ? false : true;
	m_ResAutostartHostPriv = CFG->GetInt("bot_resautohostpriv", 1) == 0 ? false : true;

	m_CommunityName = CFG->GetString("bot_community", "...");
	m_CommunityNameLower = UTIL_ToLower(m_CommunityName);

	m_GoPubByWhisper = CFG->GetInt("bot_onlywhisper", 0) == 0 ? false : true;
	m_CountOnly5v5 = CFG->GetInt("bot_countonly5v5", 0) == 0 ? false : true;
	m_IsOperator = CFG->GetInt("bot_operator", 1) == 0 ? false : true;
	m_ChallengeSafeRange = CFG->GetInt("bot_challengesaferange", 10);
	m_FreeHostFromCountry = UTIL_ToLower(CFG->GetString("bot_freehostbycountry", "..."));	
	m_DefaultAutoLvl= CFG->GetInt("bot_defaultautolvl", 0);
	m_AllowTopaz = CFG->GetInt("bot_chatclient", 1) == 0 ? false : true;
	m_FirstWelcomeChannel = CFG->GetString("bot_fwchannel", "fwchannel.txt");
	m_CountrySign = UTIL_ToLower(CFG->GetString("bot_countrysign", ""));

	//Here we load in table all command option (enable or not) from cfg file
	if (CFG->GetInt("bot_comallenable", 1) == 0 ? false : true) {
		for (int i = 1; i < 100; i++) {
			commands[i] = true;
		}
	}
	else {
		CONSOLE_Print("[Console: Read custom commands.", true);
		for (int i = 1; i < 100; i++) {
			commands[i] = CFG->GetInt("bot_com" + UTIL_ToString(i), 1) == 0 ? false : true;
		}
	}
}

void CGHost :: LoadIPToCountryData() {
	ifstream in;
	in.open("ip-to-country.csv");

	if (in.fail()) {
		CONSOLE_Print("[GHOST] warning - unable to read file [ip-to-country.csv], iptocountry data not loaded", true);
	}
	else {
		CONSOLE_Print("[GHOST] started loading [ip-to-country.csv]", true);

		// the begin and commit statements are optimizations
		// we're about to insert ~4 MB of data into the database so if we allow the database to treat each insert as a transaction it will take a LONG time
		// todotodo: handle begin/commit failures a bit more gracefully
		m_DB->Begin();

		unsigned char Percent = 0;
		string Line;
		string IP1;
		string IP2;
		string Country;
		string Countrymid;
		string FullCountry;
		CSVParser parser;

		// get length of file for the progress meter

		in.seekg(0, ios::end);
		uint32_t FileLength = (uint32_t)in.tellg();
		in.seekg(0, ios::beg);
		string tmp;
		while (!in.eof()) {
			getline(in, Line);

			if (Line.empty()) {
				continue;
			}

			parser << Line;
			parser >> IP1;
			parser >> IP2;
			//parser >> tmp;
			//parser >> tmp;
			parser >> Country;
			parser >> Countrymid;
			parser >> FullCountry;

			m_DB->FromAdd(UTIL_ToUInt32(IP1), UTIL_ToUInt32(IP2), Country, FullCountry);

			// it's probably going to take awhile to load the iptocountry data (~10 seconds on my 3.2 GHz P4 when using SQLite3)
			// so let's print a progress meter just to keep the user from getting worried

			unsigned char NewPercent = (unsigned char)((float)in.tellg() / FileLength * 100);

			if (NewPercent != Percent && NewPercent % 10 == 0) {
				Percent = NewPercent;
				CONSOLE_Print("[GHOST] iptocountry data: " + UTIL_ToString(Percent) + "% loaded", true);
			}
		}

		if (!m_DB->Commit()) {
			CONSOLE_Print("[GHOST] warning - failed to commit local database transaction, iptocountry data not loaded", true);
		}
		else {
			CONSOLE_Print("[GHOST] finished loading [ip-to-country.csv]", true);
		}

		in.close();
	}
}

void CGHost :: AddGame(uint32_t gameid, string hostbot, string owner, uint32_t starttime, uint32_t slots, uint32_t gamestate, string gamename) {
	m_Games.push_back(new CDBGames(gameid, hostbot, owner, starttime, slots, gamestate, gamename));
}

void CGHost :: DelGame(uint32_t gameid) {
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end();) {
		if ((*i)->GetGameId() == gameid) {
			i = m_Games.erase(i);
		}
		else {
			i++;
		}
	}
}

void CGHost :: AddNames(uint32_t gameid, string names, uint32_t slot) {
	if (slot > 12) {
		return;
	}
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameId() == gameid) {
			(*i)->PutNames(names, slot);
		}
	}
}

bool CGHost::IsGameId(uint32_t gameid) {
	for (vector<CDBGames *> ::iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameId() == gameid) {
			return true;
		}
	}

	return false;
}

string CGHost::GetHostbotName(uint32_t gameid) {
	for (vector<CDBGames *> ::iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameId() == gameid) {
			return (*i)->GetHostbot();
		}
	}
	return "";
}

uint32_t CGHost::GetIdFromName(string name) {
	uint32_t id = 12;
	string temp;
	transform(name.begin(), name.end(), name.begin(), (int(*)(int))tolower);
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		for (int j=0; j<12; j++) {
			temp=(*i)->GetName(j);
			transform(temp.begin(), temp.end(), temp.begin(), (int(*)(int))tolower);
			if (name == temp) {
				id = j;
			}
		}
	}
	return id;
}

void CGHost :: UpdateGameSlots(uint32_t gameid, uint32_t slots, string name) {
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameId() == gameid) {
			(*i)->UpdateGameSlots(slots);
			if (!name.empty() && (*i)->GetGameState() != 0) {
				(*i)->DelName(GetIdFromName(name));
			}
		}
	}
}

void CGHost :: UpdateGameState(uint32_t gameid, uint32_t state) {
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameId() == gameid) {
			(*i)->UpdateGameState(state);
		}
	}
}

void CGHost :: UpdateGameName(uint32_t gameid, string gamename) {
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameId() == gameid) {
			(*i)->UpdateGameName(gamename);
		}
	}
}

void CGHost :: UpdateGameTime(uint32_t gameid, uint32_t gamestarttime, bool creepspawn) {
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameId() == gameid) {
			(*i)->UpdateGameTime(gamestarttime);

			if (creepspawn) {
				(*i)->UpdateCreepsSpawn();
			}
		}
	}
}

void CGHost :: UpdateGameOwner(uint32_t gameid, string owner) {
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameId() == gameid) {
			(*i)->UpdateGameOwner(owner);
		}
	}
}

void CGHost :: GetGames(string user) {
	bool tmp = false;
	string state ="Public";
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		tmp = true;
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
		for (vector<CBNET *> ::iterator j = m_BNETs.begin(); j != m_BNETs.end(); j++) {
			(*j)->SendChatCommand(user, "ID(" + UTIL_ToString((*i)->GetGameId()) + ") GN(" + (*i)->GetGameName() + ") game is " + state + " HB name(" + (*i)->GetHostbot() + ")");
		}
	}
	if (!tmp) {
		for (vector<CBNET *> ::iterator j = m_BNETs.begin(); j != m_BNETs.end(); j++) {
			(*j)->SendChatCommand(user, "There isn't any game in progress.");
		}
	}
}

void CGHost :: GetGame(string user, uint32_t gameid) {
	bool tmp = false;
	string state ="Public";
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		tmp = true;
		if ((*i)->GetGameId()==gameid) {
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

			for (vector<CBNET *> :: iterator j = m_BNETs.begin(); j != m_BNETs.end(); j++) {
				(*j)->SendChatCommand(user,"ID("+UTIL_ToString((*i)->GetGameId())+") GN("+(*i)->GetGameName()+") Owner ("+(*i)->GetOwner()+")");
				(*j)->SendChatCommand(user,"Game is "+state+" HB name("+(*i)->GetHostbot()+") Time ("+UTIL_ToDayTimSec(GetTime()-(*i)->GetStartTime())+")");
			}
		}
	}
	if (!tmp) {
		for (vector<CBNET *> ::iterator j = m_BNETs.begin(); j != m_BNETs.end(); j++) {
			(*j)->SendChatCommand(user, "There isn't any game in proggress.");
		}
	}

}

uint32_t CGHost :: GetUsersNumber() {
	uint32_t users = 0;
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		users +=(*i)->GetUsersSlots();
	}
	return users;
}


uint32_t CGHost :: GetChallenges() {
	uint32_t challenges = 0;
	for (vector<CDBGames *> ::iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameState() == 3) {
			challenges++;
		}
	}
	return challenges;
}

uint32_t CGHost :: GetPubs() {
	uint32_t pubs = 0;
	for (vector<CDBGames *> ::iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameState() == 2) {
			pubs++;
		}
	}
	return pubs;
}

uint32_t CGHost :: GetPrivs() {
	uint32_t privs = 0;
	for (vector<CDBGames *> ::iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		if ((*i)->GetGameState() == 1) {
			privs++;
		}
	}
	return privs;
}

uint32_t CGHost :: GetLobby() {
	uint32_t lobby = 0;
	for (vector<CDBGames *> :: iterator i = m_Games.begin(); i != m_Games.end(); i++) {
		CONSOLE_Print("[DEBUG] getlobby game: "+(*i)->GetGameName());
		if ((*i)->GetGameState() == 0) {
			lobby++;
		}
	}
	return lobby;
}

void CGHost :: UDPSend(string ip, uint16_t port, string message) {
	BYTEARRAY b;
	char *c = new char[message.length()+2];
	strncpy(c,message.c_str(), message.length());
	c[message.length()]=0;
	b=UTIL_CreateByteArray(c,message.length());
	m_UDPSocket->SendTo(ip, port, b);
}

string CGHost :: GetDay() {
	time_t Now = time(NULL);
	char Time[20];
	memset(Time, 14, sizeof(char) * 14);
	strftime(Time, sizeof(char) * 14, "%d-%m-%Y", localtime(&Now));
	return string(Time);
}
