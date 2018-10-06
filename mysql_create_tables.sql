CREATE TABLE admins (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	name VARCHAR(15) NOT NULL,
	server VARCHAR(100) NOT NULL
);

CREATE TABLE bans (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	server VARCHAR(100) NOT NULL,
	name VARCHAR(15) NOT NULL,
	ip VARCHAR(15) NOT NULL,
	date DATE NOT NULL,
	gamename VARCHAR(31) NOT NULL,
	admin VARCHAR(15) NOT NULL,
	reason VARCHAR(255) NOT NULL
);

CREATE TABLE mails (
	id INT NOT NULL,
	server VARCHAR(100) NOT NULL,
	sender VARCHAR(15) NOT NULL,
	receiver VARCHAR(15) NOT NULL,
	message VARCHAR(150) NOT NULL,
	readed INT NOT NULL,
	date DATETIME NOT NULL
);

CREATE TABLE infos (
	id INT NOT NULL AUTO_INCREMENT,
	botid INT NOT NULL,
	server VARCHAR(100) NOT NULL,
	name VARCHAR(15) NOT NULL,
	lvl INT NOT NULL,
	privrank INT NOT NULL,
	pubrank INT NOT NULL,
	privpoints INT NOT NULL,
	pubpoints INT NOT NULL,
	admin VARCHAR(15) NOT NULL,
	country VARCHAR(3) NOT NULL,
	ginfo VARCHAR(100) NOT NULL,
	challwins INT NOT NULL,
	challloses INT NOT NULL,
	message INT NOT NULL,
	date DATE NOT NULL,
	PRIMARY KEY( `id`, `name`)
);

CREATE TABLE warns (
	id INT NOT NULL AUTO_INCREMENT,
	botid INT NOT NULL,
	server VARCHAR(100) NOT NULL,
	name VARCHAR(15) NOT NULL,
	warnings INT NOT NULL,
	warning VARCHAR(100) NOT NULL,
	totalwarn INT NOT NULL,
	daysban INT NOT NULL,
	admin VARCHAR(15) NOT NULL,
	date DATE NOT NULL,
	PRIMARY KEY( `id`, `name`)
);
CREATE TABLE games (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	server VARCHAR(100) NOT NULL,
	map VARCHAR(100) NOT NULL,
	datetime VARCHAR(20) NOT NULL,
	gamename VARCHAR(31) NOT NULL,
	ownername VARCHAR(15) NOT NULL,
	duration INT NOT NULL,
	gamestate INT NOT NULL,
	creatorname VARCHAR(15) NOT NULL,
	creatorserver VARCHAR(100) NOT NULL
);
CREATE TABLE gameplayers (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	gameid INT NOT NULL,
	name VARCHAR(15) NOT NULL,
	ip VARCHAR(15) NOT NULL,
	spoofed INT NOT NULL,
	reserved INT NOT NULL,
	loadingtime INT NOT NULL,
	`left` INT NOT NULL,
	leftreason VARCHAR(100) NOT NULL,
	team INT NOT NULL,
	colour INT NOT NULL,
	spoofedrealm VARCHAR(100) NOT NULL,
	INDEX( gameid )
);
CREATE TABLE dotagames (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	gameid INT NOT NULL,
	winner INT NOT NULL,
	min INT NOT NULL,
	sec INT NOT NULL,
	addpoints INT NOT NULL,
	rempoints INT NOT NULL
);
CREATE TABLE dotaplayers (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	gameid INT NOT NULL,
	colour INT NOT NULL,
	kills INT NOT NULL,
	deaths INT NOT NULL,
	creepkills INT NOT NULL,
	creepdenies INT NOT NULL,
	assists INT NOT NULL,
	gold INT NOT NULL,
	neutralkills INT NOT NULL,
	item1 CHAR(4) NOT NULL,
	item2 CHAR(4) NOT NULL,
	item3 CHAR(4) NOT NULL,
	item4 CHAR(4) NOT NULL,
	item5 CHAR(4) NOT NULL,
	item6 CHAR(4) NOT NULL,
	hero CHAR(4) NOT NULL,
	newcolour INT NOT NULL,
	towerkills INT NOT NULL,
	raxkills INT NOT NULL,
	courierkills INT NOT NULL,
	INDEX( gameid, colour )
);
CREATE TABLE downloads (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	map VARCHAR(100) NOT NULL,
	mapsize INT NOT NULL,
	datetime DATETIME NOT NULL,
	name VARCHAR(15) NOT NULL,
	ip VARCHAR(15) NOT NULL,
	spoofed INT NOT NULL,
	spoofedrealm VARCHAR(100) NOT NULL,
	downloadtime INT NOT NULL
);


CREATE TABLE `players` (
	`playerID` int(11) NOT NULL auto_increment,
	`name` varchar(30) default NULL,
	`slot` int(2) default NULL,
	`round` int(2) default NULL,
	`team` varchar(20) default NULL,
	`tourID` int(4) default NULL,
	PRIMARY KEY  (`playerID`)
);

CREATE TABLE  `teams` (
	`teamID` int(11) NOT NULL auto_increment,
	`name` varchar(30) default NULL,
	`slot` int(2) default NULL,
	`round` int(2) default NULL,
	`tourID` int(4) default NULL,
	PRIMARY KEY  (`teamID`)
);

CREATE TABLE `tour` (
	`tourID` int(11) NOT NULL auto_increment,
	`name` varchar(30) default NULL,
	`slots` int(2) default NULL,
	`ppt` int(2) default NULL,
	`mode` varchar(20) default NULL,
	`system` varchar(20) default NULL,
	`started` varchar(20) default NULL,
	`status` varchar(10) default NULL,
	PRIMARY KEY  (`tourID`)
);

CREATE TABLE  `news` (
	`id` int(11) NOT NULL auto_increment,
	`poster` varchar(20) default NULL,
	`datetime` varchar(50) default NULL,
	`content` text,
	`title` varchar(50)
	default NULL,
	PRIMARY KEY  (`id`)

);

CREATE INDEX infos_privpoints_index ON infos(privpoints);
CREATE INDEX infos_pubpoints_index ON infos(pubpoints);
CREATE INDEX infos_privrank_index ON infos(privrank);
CREATE INDEX infos_pubrank_index ON infos(pubrank);
CREATE INDEX infos_lvl_index ON infos(lvl);
CREATE INDEX dotaplayers_gameid_index ON dotaplayers(gameid);
CREATE INDEX dotaplayers_colour_index ON dotaplayers(colour);
CREATE INDEX gameplayers_gameid_index ON gameplayers(gameid);
CREATE INDEX gameplayers_colour_index ON gameplayers(colour);
CREATE INDEX gameplayers_name_index ON gameplayers(name);
CREATE INDEX gameplayers_team_index ON gameplayers(team);
CREATE INDEX dotagames_gameid_index ON dotagames(gameid);
CREATE INDEX dotagames_winner_index ON dotagames(winner);
CREATE INDEX games_id_index ON games(id);


