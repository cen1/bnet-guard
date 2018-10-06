# bnet-guard

*Major refactor was done prior to the public release but there are still some areas with "bad code" which accumulated from the early days. We will take care of these in due time.*

Guard is a bnet chat bot based on ghost++ for managing DotA and CG leagues. It takes care of channel management, player warns and bans, player level access, delegating game hosting to ghost++ hostbots and collecting game stats. It also includes a challenge system for professional DotA gaming. You can use guard for channel management only or connect 1 or more slaves which act as game hosts, see [bnet-host](https://github.com/cen1/bnet-host) project. The number of slaves determines how many games can be in lobby simultaneously.

# Supported operating systems

-Linux  
-FreeBSD  
-Windows

# Building

*nix

```
cmake -G "Unix Makefiles" -H./ -B./build
cd build
make
```

Windows

```
cmake -G "Visual Studio 14 2015" -H./ -B./build
# now open .sln file in build directory, build from Visual Studio
```

Enable verbose builds with `-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON`

## Dependencies

`bnet-guard` depends on:  
-bncsutil  
-boost  
-mysql  
-udpserver.dll (win only)  

### Dependency installation on Linux

```
apt-get install build-essential libboost-filesystem-dev libboost-system-dev libboost-thread-dev libboost-chrono-dev libboost-date-time-dev libmysqlclient-dev
```
For bncsutil see the [bncsutil GitHub repo](https://github.com/BNETDocs/bncsutil).

If your dependencies are in some other directory, expand your $PATH variable with all the locations, for example:
```
export PATH=/home/me/3rdparty/bncsutil/include:/home/me/3rdparty/bncsutil/lib:/home/me/3rdparty/mysql/include:/home/me/3rdparty/mysql/lib:/home/me/3rdparty/boost/include:/home/me/3rdparty/boost/lib:$PATH
```

### Dependency installation on Windows

Some dependency binaries are already included in the repo for convenience but you are encouraged to build/install your own.

Either get binary releases which include headers from the respective projects or build them from source (recommended). In addition to standard install paths, CMake is configured to traverse the following directories:

```
depends/[mysql,bncsutil,boost,udpserver]/lib
depends/[mysql,bncsutil,boost]/include
```

[MySQL c connector](https://dev.mysql.com/downloads/connector/c/)  
[Boost](https://www.boost.org/) - You only need to build the following components: filesystem, system, dev, thread, chrono, date-time  
[Bncsutil](https://github.com/BNETDocs/bncsutil)

# Installation

## Database
Create tables in MySQL database using the `mysql_create_tables.sql` script.

## Prepare to run
Put the guard binary and guard.cfg in the same folder. Change guard.cfg as needed, then run with `./guard` or in screen `screen -dmS guard ./guard`.

You need a valid bnet account which the bot will use to connect to the specified realm. You can then command the bot from channel or by whispering. Use <command trigger>help for full list of commands (e.g. /w myguard !help).

Bot account should have channel OP for some functionality to work (safelist, kick, ban etc).

# Usage

Bot can be put in public mode, public for specific country or private (whitelist), depending on what kind of DotA league you want to operate. In public mode, all players can use the bot to host the games. In private mode, each player needs to be approved by league managers.

Private mode is achieved by `bot_defaultautolvl = 0`.  
Public mode is achieved by `bot_defaultautolvl = 1`.

Further, you can restrict communities by country. See `guard.cfg` for more.
Public games and challenge (private) game ladders are completely separate.

## Player levels

0 - player is banned  
1 - regular player  
2 - low lvl captain  
3 - high lvl captain  
4 - voucher   
5 - admin  
6 - hostbot  

Each level has progressively more access to guard commands.

## Guard commands
-----
Format \<comman trigger>\<command name> <params...>

### Level 5 - admin
-----
**refreshicons** - forces icon system refresh. Gives server icons according to stats.

**delgamelist** - forcefully clears the game list (locally)

**fix** - refreshes ban info from db and unbans players

**news** - enables or disables printing of news

**icontimer | it** - prints time until icon refresh

**delgame \<gameId>** - deletes game from the list

**annmail \<text>** - sends mass mail to all players

**clricons** - removes all given icons

**com \<number> \<on | off | print>** - enables, disables or prints the status of command number, *!com 70 print*

**maxgames | mg** - prints number of created games

**cal** - calculates ranks on private and public ladder

**refresh \<infos | mails | warns | bans | cfg | icons | all>** - refresh internal memory from db, refreshes config, icon system or php site

**rootonly** - locks commands so only root can run them

**erasewarn \<player>** - removes warn form player

**addhostbot \<account>** - makes <account> into a hostbot (changes level to 6)

**delhostbot \<account>** - removes hostbot status from \<account>

**tempop \<player>** - makes player a temp OP in channel

**chat** - enables or disables allowance of chat bots in channel

**mepro** - if you are root admin adds you as lvl 5 admin. Use after first run and empty database.

**delinfo \<player>** - removes player from db

**say \<text>** - say something

**channel \<channel>** - move guard to a channel

**exit | quit [all]** - shutdown guard. If *all* is specified, tells hostbots to also shut down.

**slap** - sends a *send-slap* whisper to all users

### Level 4 - voucher
-----
**see <gameId>**

**fmslap \<text>** - sends a *(send-slap): <text>* whisper to all users

**clrobs** - clears observer slots

**delrmk** - delete rmk pool

**clrslots** - clears player slots

**start \<hbname>** - starts game hosted by <hbname>

**unhostall** - unhosts all games

**autospam \<seconds>** - set timer for slot availability spam. Set to 0 to disable.

**privgoby \<hbname>** - host a private game by <hbname>

**chacc \<oldaccount> \<newaccount>** - changes player account (to transfer stats)

**icons** - prints current icon list

**flames** - prints flame list

**addflame \<flame>** - adds flame to the flame list

**delflames** - deletes all flames

**showm** - prints muted users

**mute \<player> [reason]** - mutes a player, optional reason

**unmute \<player>** - unmutes a player

**wtg \<text>** - mass message

**delwarn \<player>** - removes warnings from player

**warns** - prints number of warns

**kickall** - kicks all from channel

**mod | moderate** - moderates the channel

**v | voice \<player>** - voices player in channel

**dev | devoice \<player>** - devoices player in channel

**sayingames \<text>** - say something via hostbots in-game

**t | topic \<text>** - set channel topic

**an | announce [seconds] [text]** - with no parameters disables or enables announcement, with parameters sets inerval and text to announce

**hostbots** - prints hostbot information

**minfo | MessageInfo** - prints users who are ignoring mass messages

**ginfo** - prints information about all users in db

**tt | ttopic [text]** - sets new temp topic o removes it if parameter not set

**minslots <1-10>** - set minimum amount of slots before game can be made public

**hold \<player> [reason]** - hold a slot for a player, optional reason

**idle | afk | finger [player]** - kicks afk-ers from channel. If parameter is given, prints the time the player has been afk.

**addban | ban \<player> [reason]** - bans a player

**channel** - prints info abut users in channel

**lvl [player | 1-6]** -prints your or player's level or players at specified level

**clvl \<player> <newlvl>** - changes player's level

**cm \<country> <text>** - sends mass message to players from country

**msg | fm \<text>** - send a mass message

**addinfo \<player> \<reason>** - adds player+s general info to database

**warn \<player> [reason]** - warns a player

**cban [fix]** - prints channel ban info, unbans expired if param present

**cban \<player> \<hours> \<reason>** - bans player from channel

**cbandel | delcban** - unbans player from channel

**from [player]** - prints from information for you or player

**kick \<player> [reason]** - kicks player from channel

**bans**

**mails**

**infos**

**dbstatus** - prints db status

**delban | unban \<player>**

### Level 3 -high lvl captain
-----
No specific commands.

### Level 2 - low lvl captain
-----
**gormk | rmkgo** - remakes the game

**msg | fm \<text>** - mass message

**spam** -  spams challenge status to online users

**challenge | chal | c [player]** - starts a challenge. This is the main command for starting competitive games. Challenge must then be accepted by someone or by th player you challenged. The sign phase then begins where players sign into the game, followed by a pick phase.

### Level 1 - regular player
-----
**unhost** - unhost your game

**dm | delmap** - delete current map pick

**ml | maplist** - prints map list

**obs [obs | ref]** - enables or disables obs on a map, enables or disables ref

**map \<map>** - loads map

**hbots** - prints hostbot info

**inform** - prints general league info

**where \<player>** - prints player's location

**mailbox** - prints your mailbox

**delmail \<id>** - deletes mail from mailbox

**mail \<player> \<text>** - send "mail" to player. These are like in-game persistent messages you can read from guard.

**pub \<game name>** - host a public game

**priv \<game name>** - host a private game

**root** - print root admin

**users** - prints online users

**ggs | games** - prints active games

**gn | getnames \<game id>** - print player names in game

**gg | ggames [game id]** - prints information about active game(s)

**userson** - gather information about current online suer activity

**rank \<1-N>** - prints name of the player at specified rank, !rank 1

**find \<country> [substring]** - find players that come form country and optionally match

**find \<substring>** - find players that match

**ex** - extends challenge timer by 5 minutes

**ss | safelisted** - prints number of safelisted users

**top** - prints top ladder players

**gopriv | privgo [game name]** - host a private non-ladder (not ranked) game

**gopub | pubgo [game name]** - host a public non-ladder (not ranked) game

**info [player]** - prints your or player info

**s | stats [player]** - prints your or player stats

**sd | statsdota** - prints your DotA stats

**ki | killer** - prints DotA kill stats information

**fa | farmer** - prints DotA farmer information

**version** - prints guard version

#### Challenge commands

**refuse** - refuse a challenge

**a | accept** - accept a challenge. This starts a challenge and layers need to !sign into the game.

**cs | challengers** - prints the names of the challenger and the challenged

**pool** - prints player pool of players who signed

**res** - reserve a slot. Once 10 players reserve, game will automatically be started. This is alternative way to host a professional game as opposed to direct challenge being issued.

**sign [obs]** - sign up or the challenge

**out** - sign out of the challenge

**mode [cm | cd]** - vote for DotA play mode. Leave empty to remove your vote.

**startpick** - starts the pick phase once the pool is full

**pick \<r | rand | player>** - as captain, pick a player for your team

**go** - start the game after pick phase

**abort** - abort challenge

**slots | teams | reserves** - prints slot information of current challenge

### Level 6 - hostbot
-----
**addinfo \<data>** - updates plaeyr stats info

**addnames \<data>** - add player names with slots

**ag \<data>** - add game

**ban \<data>** - ban player

**cban \<victim>** - channel ban

**checkjoin**

**checklvl**

**checkobs**

**country \<data>** - update player country information

**delgame \<gameId>** - delete game

**fm**

**handshake** - sends <command trigger>handshake back to the hostbot

**hbstate \<data>** - reports current hostbot state to the guard

**mail \<text>** - mails text to guard admins

**ok**

**pg \<data>** - add game to game list

**pu**

**rmk \<data>**

**say \<text>** - say something in chat

**ugn \<data>** - update game name

**ugo \<data>** - update game owner

**ugs** -update game state

**ugsl \<data>** - update game slots

**ugt \<gameId>** - update game time


## Icon system

If guard account has sufficient realm access, it can set battle.net icons to players depending on their rank. This way the rank of the player is visually represented inside the channel. Icon system needs to be enabled in the configuration file and works automatically.

## RPM and DEB repos
RPM and DEB packages will be available once the code is completely cleaned up.

