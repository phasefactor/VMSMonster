{

	This is Monster, a multiuser adventure game system
	where the players create the universe.

	Written by Rich Skrenta at Northwestern University, 1988.

		skrenta@nuacc.acns.nwu.edu
		skrenta@nuacc.bitnet

}

program monster(input,output);

const

%include 'privusers.pas'

	veryshortlen = 12;	{ very short string length for userid's etc }
	shortlen = 20;		{ ordinary short string }

	maxobjs = 15;		{ max objects allow on floor in a room }
	maxpeople = 10;		{ max people allowed in a room }
	maxplayers = 300;	{ max log entries to make for players }
	maxcmds = 75;		{ top value for cmd keyword slots }
	maxshow = 50;		{ top value for set/show keywords }
	maxexit = 6;		{ 6 exits from each loc: NSEWUD }
	maxroom = 1000;		{ Total maximum ever possible	}
	maxdetail = 5;		{ max num of detail keys/descriptions per room }
	maxevent = 15;		{ event slots per event block }
	maxindex = 10000;	{ top value for bitmap allocation }
	maxhold = 6;		{ max # of things a player can be holding }
	maxerr = 15;		{ # of consecutive record collisions before the
				  the deadlock error message is printed }
	numevnts = 10;		{ # of different event records to be maintained }
	numpunches = 12;	{ # of different kinds of punches there are }
	maxparm = 20;		{ parms for object USEs }
	maxspells = 50;		{ total number of spells available }

	descmax = 10;		{ lines per description block }


	DEFAULT_LINE = 32000;	{ A virtual one liner record number that
				  really means "use the default one liner
				  description instead of reading one from
				  the file" }

{ Mnemonics for directions }

	north = 1;
	south = 2;
	east = 3;
	west = 4;
	up = 5;
	down = 6;


{ Index record mnemonics }

	I_BLOCK = 1;	{ True if description block is not used		}
	I_LINE = 2;	{ True if line slot is not used			}
	I_ROOM = 3;	{ True if room slot is not in use		}
	I_PLAYER = 4;	{ True if slot is not occupied by a player	}
	I_ASLEEP = 5;	{ True if player is not playing			}
	I_OBJECT = 6;	{ True if object record is not being used	}
	I_INT = 7;	{ True if int record is not being used		}

{ Integer record mnemonics }

	N_LOCATION = 1;		{ Player's location }
	N_NUMROOMS = 2;		{ How many rooms they've made }
	N_ALLOW = 3;		{ How many rooms they're allowed to make }
	N_ACCEPT = 4;		{ Number of open accept exits they have }
	N_EXPERIENCE = 5;	{ How "good" they are }
	N_SELF = 6;		{ player's self descriptions }

{ object kind mnemonics }

	O_BLAND = 0;		{ bland object, good for keys }
	O_WEAPON = 1;
	O_ARMOR = 2;
	O_THRUSTER = 3;		{ use puts player through an exit }
	O_CLOAK = 4;

	O_BAG = 100;
	O_CRYSTAL = 101;
	O_WAND = 102;
	O_HAND = 103;


{ Command Mnemonics }
	error = 0;
	setnam = 1;
	help = 2;
	quest = 3;
	quit = 4;
	look = 5;
	go = 6;
	form = 7;
	link = 8;
	unlink = 9;
	c_whisper = 10;
	poof = 11;
	desc = 12;
	dbg = 14;
	say = 15;

	c_rooms = 17;
	c_system = 18;
	c_disown = 19;
	c_claim = 20;
	c_create = 21;
	c_public = 22;
	c_accept = 23;
	c_refuse = 24;
	c_zap = 25;
	c_hide = 26;
	c_l = 27;
	c_north = 28;
	c_south = 29;
	c_east = 30;
	c_west = 31;
	c_up = 32;
	c_down = 33;
	c_n = 34;
	c_s = 35;
	c_e = 36;
	c_w = 37;
	c_u = 38;
	c_d = 39;
	c_custom = 40;
	c_who = 41;
	c_players = 42;
	c_search = 43;
	c_unhide = 44;
	c_punch = 45;
	c_ping = 46;
	c_health = 47;
	c_get = 48;
	c_drop = 49;
	c_inv = 50;
	c_i = 51;
	c_self = 52;
	c_whois = 53;
	c_duplicate = 54;

	c_version = 56;
	c_objects = 57;
	c_use = 58;
	c_wield = 59;
	c_brief = 60;
	c_wear = 61;
	c_relink = 62;
	c_unmake = 63;
	c_destroy = 64;
	c_show = 65;
	c_set = 66;

	e_detail = 100;		{ pseudo command for log_action of desc exit }
	e_custroom = 101;	{ customizing this room }
	e_program = 102;	{ customizing (programming) an object }
	e_usecrystal = 103;	{ using a crystal ball }


{ Show Mnemonics }

	s_exits = 1;
	s_object = 2;
	s_quest = 3;
	s_details = 4;


{ Set Mnemonics }

	y_quest = 1;
	y_altmsg = 2;
	y_group1 = 3;
	y_group2 = 4;


{ Event Mnemonics }

	E_EXIT = 1;		{ player left room			}
	E_ENTER = 2;		{ player entered room			}
	E_BEGIN = 3;		{ player joined game here		}
	E_QUIT = 4;		{ player here quit game			}
	
	E_SAY = 5;		{ someone said something		}
	E_SETNAM = 6;		{ player set his personal name		}
	E_POOFIN = 8;		{ someone poofed into this room		}
	E_POOFOUT = 9;		{ someone poofed out of this room	}
	E_DETACH = 10;		{ a link has been destroyed		}
	E_EDITDONE = 11;	{ someone is finished editing a desc	}
	E_NEWEXIT = 12;		{ someone made an exit here		}
	E_BOUNCEDIN = 13;	{ an object "bounced" into the room	}
	E_EXAMINE = 14;		{ someone is examining something	}
	E_CUSTDONE = 15;	{ someone is done customizing an exit	}
	E_FOUND = 16;		{ player found something		}
	E_SEARCH = 17;		{ player is searching room		}
	E_DONEDET = 18;		{ done adding details to a room		}
	E_HIDOBJ = 19;		{ someone hid an object here		}
	E_UNHIDE = 20;		{ voluntarily revealed themself		}
	E_FOUNDYOU = 21;	{ someone found someone else hiding	}
	E_PUNCH = 22;		{ someone has punched someone else	}
	E_MADEOBJ = 23;		{ someone made an object here		}
	E_GET = 24;		{ someone picked up an object		}
	E_DROP = 25;		{ someone dropped an object		}
	E_DROPALL = 26;		{ quit & dropped stuff on way out	}
	E_IHID = 27;		{ tell others that I have hidden (!)	}
	E_NOISES = 28;		{ strange noises from hidden people	}
	E_PING = 29;		{ send a ping to a potential zombie	}
	E_PONG = 30;		{ ping answered				}
	E_HIDEPUNCH = 31;	{ someone hidden is attacking		}
	E_SLIPPED = 32;		{ attack caused obj to drop unwillingly }
	E_ROOMDONE = 33;	{ done customizing this room		}
	E_OBJDONE = 34;		{ done programming an object		}
	E_HPOOFOUT = 35;	{ someone hiding poofed	out		}
	E_FAILGO = 36;		{ a player failed to go through an exit }
	E_HPOOFIN = 37;		{ someone poofed into a room hidden	}
	E_TRYPUNCH = 38;	{ someone failed to punch someone else	}
	E_PINGONE = 39;		{ someone was pinged away . . .		}
	E_CLAIM = 40;		{ someone claimed this room		}
	E_DISOWN = 41;		{ owner of this room has disowned it	}
	E_WEAKER = 42;		{ person is weaker from battle		}
	E_OBJCLAIM = 43;	{ someone claimed an object		}
	E_OBJDISOWN = 44;	{ someone disowned an object		}
	E_SELFDONE = 45;	{ done editing self description		}
	E_WHISPER = 46;		{ someone whispers to someone else	}
	E_WIELD = 47;		{ player wields a weapon		}
	E_UNWIELD = 48;		{ player puts a weapon away		}
	E_DONECRYSTALUSE = 49;	{ done using the crystal ball		}
	E_WEAR = 50;		{ someone has put on something		}
	E_UNWEAR = 51;		{ someone has taken off something	}
	E_DESTROY = 52;		{ someone has destroyed an object	}
	E_HIDESAY = 53;		{ anonymous say				}
	E_OBJPUBLIC = 54;	{ someone made an object public		}
	E_SYSDONE = 55;		{ done with system maint. mode		}
	E_UNMAKE = 56;		{ remove typedef for object		}
	E_LOOKDETAIL = 57;	{ looking at a detail of this room	}
	E_ACCEPT = 58;		{ made an "accept" exit here		}
	E_REFUSE = 59;		{ got rid of an "accept" exit here	}
	E_DIED = 60;		{ someone died and evaporated		}
	E_LOOKYOU = 61;		{ someone is looking at you		}
	E_FAILGET = 62;		{ someone can't get something		}
	E_FAILUSE = 63;		{ someone can't use something		}
	E_CHILL = 64;		{ someone scrys you			}
	E_NOISE2 = 65;		{ say while in crystal ball		}
	E_LOOKSELF = 66;	{ someone looks at themself		}
	E_INVENT = 67;		{ someone takes inventory		}
	E_POOFYOU = 68;		{ MM poofs someone away . . . .		}
	E_WHO = 69;		{ someone does a who			}
	E_PLAYERS = 70;		{ someone does a players		}
	E_VIEWSELF = 71;	{ someone views a self description	}
	E_REALNOISE = 72;	{ make the real noises message print	}
	E_ALTNOISE = 73;	{ alternate mystery message		}
	E_MIDNIGHT = 74;	{ it's midnight now, tell everyone	}

	E_ACTION = 100;		{ base command action event }


{ Misc. }

	GOODHEALTH = 7;


type
	string = varying[80] of char;
	veryshortstring = varying[veryshortlen] of char;
	shortstring = varying[shortlen] of char;

	{ This is a list of description block numbers;
	  If a number is zero, there is no text for that block }
	

	{ exit kinds:
		0: no way - blocked exit
		1: open passageway
		2: object required

		6: exit only exists if player is holding the key
	}

	exit = record
		toloc: integer;		{ location exit goes to }
		kind: integer;		{ type of the exit }
		slot: integer;		{ exit slot of toloc target }

		exitdesc,  { one liner description of exit  }
		closed,    { desc of a closed door }
		fail,	   { description if can't go thru   }
		success,   { desc while going thru exit     }
		goin,      { what others see when you go into the exit }
{		ofail,	}
		comeout:   { what others see when you come out of the exit }
			  integer; { all refer to the liner file }
				   { if zero defaults will be printed }

		hidden: integer;	{ **** about to change this **** }
		objreq: integer;	{ object required to pass this exit }

		alias: veryshortstring; { alias for the exit dir, a keyword }

		reqverb: boolean;	{ require alias as a verb to work }
		reqalias: boolean;	{ require alias only (no direction) to
					  pass through the exit }
		autolook: boolean;	{ do a look when user comes out of exit }
	end;


	{ index record # 1 is block index }
	{ index record # 2 is line index }
	{ index record # 3 is room index }
	{ index record # 4 is player alloc index }
	{ index record # 5 is player awake (in game) index }
	indexrec = record
		indexnum: integer;	{ validation number }
		free: packed array[1..maxindex] of boolean;
		top: integer;   { max records available }
		inuse: integer; { record #s in use }
	end;


	{ names are record #1   }
	{ owners are record # 2 }
	{ player pers_names are record # 3 }
	{ userids are record # 4 }
	{ object names are record # 5 }
	{ object creators are record # 6 }
	{ date of last play is # 7 }
	{ time of last play is # 8 }
	namrec = record
		validate: integer;
		loctop: integer;
		idents: array[1..maxroom] of shortstring;
	end;

	objectrec = record
		objnum: integer;	{ allocation number for the object }
		onum: integer;		{ number index to objnam/objown }
		oname: shortstring;	{ duplicate of name of object }
		kind: integer;		{ what kind of object this is }
		linedesc: integer;	{ liner desc of object Here }

		home: integer;		{ if object at home, then print the }
		homedesc: integer;	{ home description }

		actindx: integer;	{ action index -- programs for the future }
		examine: integer;	{ desc block for close inspection }
		worth: integer;		{ how much it cost to make (in gold) }
		numexist: integer;	{ number in existence }

		sticky: boolean;	{ can they ever get it? }
		getobjreq: integer;	{ object required to get this object }
		getfail: integer;	{ fail-to-get description }
		getsuccess: integer;	{ successful picked up description }

		useobjreq: integer;	{ object require to use this object }
		uselocreq: integer;	{ place have to be to use this object }
		usefail: integer;	{ fail-to-use description }
		usesuccess: integer;	{ successful use of object description }

		usealias: veryshortstring;
		reqalias: boolean;
		reqverb: boolean;

		particle: integer;	{ a,an,some, etc... "particle" is not
					  be right, but hey, it's in the code }

		parms: array[1..maxparm] of integer;

		d1: integer;		{ extra description # 1 }
		d2: integer;		{ extra description # 2 }
		exp3,exp4,exp5,exp6: integer;
	end;

	anevent = record
		sender,			{ slot of sender }
		action,			{ what event this is, E_something }
		target,			{ opt target of action }
		parm: integer;		{ expansion parm }
		msg: string;		{ string for SAY and other cmds }
		loc: integer;		{ room that event is targeted for }
	end;

	eventrec = record
		validat: integer;	{ validation number for record locking }
		evnt: array[1..maxevent] of anevent;
		point: integer;		{ circular buffer pointer }
	end;

	peoplerec = record
		kind: integer;		   { 0=none,1=player,2=npc }
		parm: integer;		   { index to npc controller (object?) }

		username: veryshortstring; { actual userid of person }
		name: shortstring;	   { chosen name of person }
		hiding: integer;	   { degree to which they're hiding }
		act,targ: integer;	   { last thing that this person did }

		holding: array[1..maxhold] of integer;	{ objects being held }
		experience: integer;

		wearing: integer;	{ object that they're wearing }
		wielding: integer;	{ weapon they're wielding }
		health: integer;	{ how healthy they are }

		self: integer;		{ self description }

		ex1,ex2,ex3,ex4,ex5: integer;
	end;

	spellrec = record
		recnum: integer;
		level: array[1..maxspells] of integer;
	end;

	descrec = record
		descrinum: integer;
		lines: array[1..descmax] of string;
		desclen: integer;  { number used in this block }
	end;

	linerec = record
		linenum: integer;
		theline: string;
	end;

	room = record
		valid: integer;		{ validation number for record locking }
		locnum: integer;
		owner: veryshortstring; { who owns the room: userid if private
							     '' if public
							     '*' if disowned }
		nicename: string;	{ pretty name for location }
		nameprint: integer;	{ code for printing name:
						0: don't print it
						1: You're in
						2: You're at
					}

		primary: integer;	{ room descriptions }
		secondary: integer;
		which: integer;		{ 0 = only print primary room desc.
					  1 = only print secondary room desc.
					  2 = print both
					  3 = print primary then secondary
						if has magic object }

		magicobj: integer;	{ special object for this room }
		effects: integer;
		parm: integer;

		exits: array[1..maxexit] of exit;

		pile: integer;		{ if more than maxobjs objects here }
		objs: array[1..maxobjs] of integer;	{ refs to object file }
		objhide: array[1..maxobjs] of integer;	{ how much each object
							  is hidden }
							{ see hidden on exitrec
							  above }

		objdrop: integer;	{ where objects go when they're dropped }
		objdesc: integer;	{ what it says when they're dropped }
		objdest: integer;	{ what it says in target room when
					  "bounced" object comes in }

		people: array[1..maxpeople] of peoplerec;

		grploc1,grploc2: integer;
		grpnam1,grpnam2: shortstring;

		detail: array[1..maxdetail] of veryshortstring;
		detaildesc: array[1..maxdetail] of integer;

		trapto: integer;	{ where the "trapdoor" goes }
		trapchance: integer;	{ how often the trapdoor works }

		rndmsg: integer;	{ message that randomly prints }

		xmsg2: integer;		{ another random block }
		exp2,exp3,exp4: integer;
		exitfail: integer;	{ default fail description for exits }
		ofail: integer;		{ what other's see when you fail, default }
	end;


	intrec = record
		intnum: integer;
		int: array[1..maxplayers] of integer;
	end;


var
	old_prompt: [external] string;
	line:	    [external] string;
	oldcmd:	string;		{ string for '.' command to do last command }

	inmem: boolean;	 { Is this rooms roomrec (here....) in memory?
			   We call gethere many times to make sure
			   here is current.  However, we only want to
			   actually do a getroom if the roomrec has been
			   modified	}
	brief: boolean := FALSE;	{ brief/verbose descriptions }

	rndcycle: integer;		{ integer for rnd_event }
	debug: boolean;
	ping_answered: boolean;		  { flag for ping answers }
	hiding : boolean := FALSE;	  { is player hiding? }
	midnight_notyet: boolean := TRUE; { hasn't been midnight yet }
	first_puttoken: boolean := TRUE;  { flag for first place into world }
	logged_act : boolean := FALSE;	  { flag to indicate that a log_action
					  has been called, and the next call
					  to clear_command needs to clear the
					  action parms in the here roomrec }

	roomfile : file of room;
	eventfile: file of eventrec;
	namfile: file of namrec;
	descfile: file of descrec;
	linefile: file of linerec;
	indexfile: file of indexrec;
	intfile: file of intrec;
	objfile: file of objectrec;
	spellfile: file of spellrec;

	cmds: array[1..maxcmds] of shortstring := (

		'name',		{ setnam = 1	}
		'help',		{ help = 2	}
		'?',		{ quest = 3	}
		'quit',		{ quit = 4	}
		'look',		{ look = 5	}
		'go',		{ go = 6	}
		'form',		{ form = 7	}
		'link',		{ link = 8	}
		'unlink',	{ unlink = 9	}
		'whisper',	{ c_whisper = 10}
		'poof',		{ poof = 11	}
		'describe',	{ desc = 12	}
		'',
		'debug',	{ dbg = 14	}
		'say',		{ say = 15	}
		'',		{		}
		'rooms',	{ c_rooms = 17	}
		'system',	{ c_system = 18	}
		'disown',	{ c_disown = 19	}
		'claim',	{ c_claim = 20	}
		'make',		{ c_create = 21	}
		'public',	{ c_public = 22	}
		'accept',	{ c_accept = 23	}
		'refuse',	{ c_refuse = 24	}
		'zap',		{ c_zap = 25	}
		'hide',		{ c_hide = 26	}
		'l',		{ c_l = 27	}
		'north',	{ c_north = 28	}
		'south',	{ c_south = 29	}
		'east',		{ c_east = 30	}
		'west',		{ c_west = 31	}
		'up',		{ c_up = 32	}
		'down',		{ c_down = 33	}
		'n',		{ c_n = 34	}
		's',		{ c_s = 35	}
		'e',		{ c_e = 36	}
		'w',		{ c_w = 37	}
		'u',		{ c_u = 38	}
		'd',		{ c_d = 39	}
		'customize',	{ c_custom = 40	}
		'who',		{ c_who = 41	}
		'players',	{ c_players = 42}
		'search',	{ c_search = 43	}
		'reveal',	{ c_unhide = 44	}
		'punch',	{ c_punch = 45	}
		'ping',		{ c_ping = 46	}
		'health',	{ c_health = 47	}
		'get',		{ c_get = 48	}
		'drop',		{ c_drop = 49	}
		'inventory',	{ c_inv = 50	}
		'i',		{ c_i = 51	}
		'self',		{ c_self = 52	}
		'whois',	{ c_whois = 53	}
		'duplicate',	{ c_duplicate = 54 }
		'',
		'version',	{ c_version = 56}
		'objects',	{ c_objects = 57}
		'use',		{ c_use = 58	}
		'wield',	{ c_wield = 59	}
		'brief',	{ c_brief = 60	}
		'wear',		{ c_wear = 61	}
		'relink',	{ c_relink = 62	}
		'unmake',	{ c_unmake = 63	}
		'destroy',	{ c_destroy = 64}
		'show',		{ c_show = 65	}
		'set',		{ c_set = 66	}
		'',
		'',
		'',
		'',
		'',
		'',
		'',
		'',
		''
	);


	numcmds: integer;	{ number of main level commands there are }
	show: array[1..maxshow] of shortstring;
	numshow: integer;
	setkey: array[1..maxshow] of shortstring;
	numset: integer;

	direct: array[1..maxexit] of shortstring :=
		('north','south','east','west','up','down');

	spells: array[1..maxspells] of string;	  { names of spells }
	numspells: integer;		{ number of spells there actually are }

	done: boolean;		{ flag for QUIT }
	userid: veryshortstring;	{ userid of this player }
	location: integer;	{ current place number }

	hold_kind: array[1..maxhold] of integer; { kinds of the objects i'm
						   holding }

	myslot: integer := 1;	{ here.people[myslot]... is this player }
	myname: shortstring;	{ personal name this player chose (setname) }
	myevent: integer;	{ which point in event buffer we are at }

	found_exit: array[1..maxexit] of boolean;
				{ has exit i been found by the player? }

	mylog: integer;		{ which log entry this player is }
	mywear: integer;	{ what I'm wearing }
	mywield: integer;	{ weapon I'm wielding }
	myhealth: integer;	{ how well I'm feeling }
	myexperience: integer;	{ how experienced I am }
	myself: integer;	{ self description block }

	healthcycle: integer;	{ used in rnd_event to control how quickly a
				  player heals }

	here: room;		{ current room record }
	event: eventrec;
	privd: boolean;

	objnam,			{ object names }
	objown,			{ object owners }
	nam,			{ record 1 is room names }
	own,			{ rec 2 is room owners }
	pers,			{ 3 is player personal names }
	user,			{ 4 is player userid	}
	adate,			{ 5 is date of last play }
	atime			{ 6 is time of last play }
 		: namrec;

	anint: intrec;		{ info about game players }
	obj: objectrec;
	spell: spellrec;

	block: descrec;		{ a text block of descmax lines }
	indx: indexrec;		{ an record allocation record }
	oneliner: linerec;	{ a line record }

	heredsc: descrec;


[external]
procedure wait(seconds: real);	{ system SLEEP call }
external;

[external]
function random:real;	{ system random number generator }
external;

[external]
function rnd100: integer;	{ returns a random # between 0-100 }
external;

[external]
procedure setup_guts;	{ disables ctrl-Y/ctrl-C }
			{ necessary to prevent ZOMBIES in the world }
extern;

[external]
procedure finish_guts;	{ re-enables ctrl-Y/ctrl-C }
extern;

[external] function get_userid:string;
external;

[external] function trim(s: string): string;
external;

[external]
procedure grab_line(prompt: string; var s:string; echo:boolean := true);
{ Input routine.   Gets a line of text from user which checking
  for async events }
external;

[external]
procedure putchars(s: string);
extern;

procedure xpoof(loc: integer);
forward;

procedure do_exit(exit_slot: integer);
forward;

function put_token(room: integer;var aslot:integer;hidelev:integer := 0):boolean;
forward;

procedure take_token(aslot, roomno: integer);
forward;

procedure maybe_drop;
forward;

procedure do_program(objnam: string);
forward;

function drop_everything(pslot: integer := 0): boolean;
forward;


procedure collision_wait;
var
	wait_time: real;

begin
	wait_time := random;
	if wait_time < 0.001 then
		wait_time := 0.001;
	wait(wait_time);
end;


{ increment err; if err is too high, suspect deadlock }
{ this is called by all getX procedures to ease deadlock checking }
procedure deadcheck(var err: integer; s:string);

begin
	err := err + 1;
	if err > maxerr then begin
		writeln('%warning- ',s,' seems to be deadlocked; notify the Monster Manager');
		finish_guts;
		halt;
		err := 0;
	end;
end;



{ first procedure of form getX
  attempts to get given room record
  resolves record access conflicts, checks for deadlocks
  Locks record; use freeroom immediately after getroom if data is
  for read-only
}
procedure getroom(n: integer:= 0);
var
	err: integer;

begin
	if n = 0 then
		n := location;
	roomfile^.valid := 0;
	err := 0;
	if debug then
		writeln('%getroom(',n:1,')');
	find(roomfile,n,error := continue);
	while roomfile^.valid <> n do begin
		deadcheck(err,'getroom');
		collision_wait;
		find(roomfile,n,error := continue);
	end;
	here := roomfile^;

	inmem := false;
		{ since this getroom could be doing anything, we will
		  assume that it is bozoing the correct here record for
		  this room.  If this getroom called by gethere, then
		  gethere will correct inmem immediately.  Otherwise
		  the next gethere will restore the correct here record. }
end;

procedure putroom;

begin
	locate(roomfile,here.valid);
	roomfile^ := here;
	put(roomfile);
end;

procedure freeroom;	{ unlock the record if you're not going to write it }

begin
	unlock(roomfile);
end;

procedure gethere(n: integer := 0);

begin
	if (n = 0) or (n = location) then begin
		if not(inmem) then begin
			getroom;	{ getroom(n) okay here also }
			freeroom;
			inmem := true;
		end else if debug then
			writeln('%gethere - here already in memory');
	end else begin
		getroom(n);
		freeroom;
	end;
end;


procedure getown;
var
	err: integer;

begin
	namfile^.validate := 0;
	err := 0;
	find(namfile,2,error := continue);
	while namfile^.validate <> 2 do begin
		deadcheck(err,'getown');
		collision_wait;
		find(namfile,2,error := continue);
	end;
	own := namfile^;
end;



procedure getnam;
var
	err: integer;

begin
	namfile^.validate := 0;
	err := 0;
	find(namfile,1,error := continue);
	while namfile^.validate <> 1 do begin
		deadcheck(err,'getnam');
		collision_wait;
		find(namfile,1,error := continue);
	end;
	nam := namfile^;
end;

procedure freenam;

begin
	unlock(namfile);
end;

procedure freeown;

begin
	unlock(namfile);
end;

procedure putnam;

begin
	locate(namfile,1);
	namfile^:= nam;
	put(namfile);
end;

procedure putown;

begin
	locate(namfile,2);
	namfile^:= own;
	put(namfile);
end;


procedure getobj(n: integer);
var
	err: integer;

begin
	if n = 0 then
		n := location;
	objfile^.objnum := 0;
	err := 0;
	find(objfile,n,error := continue);
	while objfile^.objnum <> n do begin
		deadcheck(err,'getobj');
		collision_wait;
		find(objfile,n,error := continue);
	end;
	obj := objfile^;
end;

procedure putobj;

begin
	locate(objfile,obj.objnum);
	objfile^ := obj;
	put(objfile);
end;

procedure freeobj;	{ unlock the record if you're not going to write it }

begin
	unlock(objfile);
end;



procedure getint(n: integer);
var
	err: integer;

begin
	intfile^.intnum := 0;
	err := 0;
	find(intfile,n,error := continue);
	while intfile^.intnum <> n do begin
		deadcheck(err,'getint');
		collision_wait;
		find(intfile,n,error := continue);
	end;
	anint := intfile^;
end;


procedure freeint;

begin
	unlock(intfile);
end;

procedure putint;
var
	n: integer;

begin
	n := anint.intnum;
	locate(intfile,n);
	intfile^:= anint;
	put(intfile);
end;



procedure getspell(n: integer := 0);
var
	err: integer;

begin
	if n = 0 then
		n := mylog;

	spellfile^.recnum := 0;
	err := 0;
	find(spellfile,n,error := continue);
	while spellfile^.recnum <> n do begin
		deadcheck(err,'getspell');
		collision_wait;
		find(spellfile,n,error := continue);
	end;
	spell := spellfile^;
end;


procedure freespell;

begin
	unlock(spellfile);
end;

procedure putspell;
var
	n: integer;

begin
	n := spell.recnum;
	locate(spellfile,n);
	spellfile^:= spell;
	put(spellfile);
end;



procedure getuser;	{ get log rec with everyone's userids in it }
var
	err: integer;

begin
	namfile^.validate := 0;
	err := 0;
	find(namfile,4,error := continue);
	while namfile^.validate <> 4 do begin
		deadcheck(err,'getuser');
		collision_wait;
		find(namfile,4,error := continue);
	end;
	user := namfile^;
end;

procedure freeuser;

begin
	unlock(namfile);
end;

procedure putuser;

begin
	locate(namfile,4);
	namfile^:= user;
	put(namfile);
end;



procedure getdate;	{ get log rec with date of last play in it }
var
	err: integer;

begin
	namfile^.validate := 0;
	err := 0;
	find(namfile,7,error := continue);
	while namfile^.validate <> 7 do begin
		deadcheck(err,'getdate');
		collision_wait;
		find(namfile,7,error := continue);
	end;
	adate := namfile^;
end;

procedure freedate;

begin
	unlock(namfile);
end;

procedure putdate;

begin
	locate(namfile,7);
	namfile^:= adate;
	put(namfile);
end;


procedure gettime;	{ get log rec with time of last play in it }
var
	err: integer;

begin
	namfile^.validate := 0;
	err := 0;
	find(namfile,8,error := continue);
	while namfile^.validate <> 8 do begin
		deadcheck(err,'gettime');
		collision_wait;
		find(namfile,8,error := continue);
	end;
	atime := namfile^;
end;

procedure freetime;

begin
	unlock(namfile);
end;

procedure puttime;

begin
	locate(namfile,8);
	namfile^:= atime;
	put(namfile);
end;



procedure getobjnam;
var
	err: integer;

begin
	namfile^.validate := 0;
	err := 0;
	find(namfile,5,error := continue);
	while namfile^.validate <> 5 do begin
		deadcheck(err,'getobjnam');
		collision_wait;
		find(namfile,5,error := continue);
	end;
	objnam := namfile^;
end;

procedure freeobjnam;

begin
	unlock(namfile);
end;

procedure putobjnam;

begin
	locate(namfile,5);
	namfile^:= objnam;
	put(namfile);
end;



procedure getobjown;
var
	err: integer;

begin
	namfile^.validate := 0;
	err := 0;
	find(namfile,6,error := continue);
	while namfile^.validate <> 6 do begin
		deadcheck(err,'getobjown');
		collision_wait;
		find(namfile,6,error := continue);
	end;
	objown := namfile^;
end;

procedure freeobjown;

begin
	unlock(namfile);
end;

procedure putobjown;

begin
	locate(namfile,6);
	namfile^:= objown;
	put(namfile);
end;



procedure getpers;	{ get log rec with everyone's pers names in it }
var
	err: integer;

begin
	namfile^.validate := 0;
	err := 0;
	find(namfile,3,error := continue);
	while namfile^.validate <> 3 do begin
		deadcheck(err,'getpers');
		collision_wait;
		find(namfile,3,error := continue);
	end;
	pers := namfile^;
end;

procedure freepers;

begin
	unlock(namfile);
end;

procedure putpers;

begin
	locate(namfile,3);
	namfile^:= pers;
	put(namfile);
end;




procedure getevent(n: integer := 0);
var
	err: integer;

begin
	if n = 0 then
		n := location;

	n := (n mod numevnts) + 1;

	eventfile^.validat := 0;
	err := 0;
	find(eventfile,n,error := continue);
	while eventfile^.validat <> n do begin
		deadcheck(err,'getevent');
		collision_wait;
		find(eventfile,n,error := continue);
	end;
	event := eventfile^;
end;

procedure freeevent;

begin
	unlock(eventfile);
end;

procedure putevent;

begin
	locate(eventfile,event.validat);
	eventfile^:= event;
	put(eventfile);
end;


procedure getblock(n: integer);
var
	err: integer;

begin
	if debug then
		writeln('%getblock: ',n:1);
	descfile^.descrinum := 0;
	err := 0;
	find(descfile,n,error := continue);
	while descfile^.descrinum <> n do begin
		deadcheck(err,'getblock');
		collision_wait;
		find(descfile,n,error := continue);
	end;
	block := descfile^;
end;

procedure putblock;
var
	n: integer;

begin
	n := block.descrinum;
	if debug then
		writeln('%putblock: ',n:1);
	if n <> 0 then begin
		locate(descfile,n);
		descfile^ := block;
		put(descfile);
	end;
end;

procedure freeblock;	{ unlock the record if you're not going to write it }

begin
	unlock(descfile);
end;





{ *** new code begins here *** }


procedure getline(n: integer);
var
	err: integer;

begin
	if n = -1 then begin
		oneliner.theline := '';
	end else begin
		err := 0;
		linefile^.linenum := 0;
		find(linefile,n,error := continue);
		while linefile^.linenum <> n do begin
			deadcheck(err,'getline');
			collision_wait;
			find(linefile,n,error := continue);
		end;
		oneliner := linefile^;
	end;
end;

procedure putline;

begin
	if oneliner.linenum > 0 then begin
		locate(linefile,oneliner.linenum);
		linefile^ := oneliner;
		put(linefile);
	end;
end;

procedure freeline;	{ unlock the record if you're not going to write it }

begin
	unlock(linefile);
end;




{
Index record 1 -- Description blocks that are free
Index record 2 -- One liners that are free
}


procedure getindex(n: integer);
var
	err: integer;

begin
	indexfile^.indexnum := 0;
	err := 0;
	find(indexfile,n,error := continue);
	while indexfile^.indexnum <> n do begin
		deadcheck(err,'getindex');
		collision_wait;
		find(indexfile,n,error := continue);
	end;
	indx := indexfile^;
end;

procedure putindex;

begin
	locate(indexfile,indx.indexnum);
	indexfile^ := indx;
	put(indexfile);
end;

procedure freeindex;	{ unlock the record if you're not going to write it }

begin
	unlock(indexfile);
end;



{
First procedure of form alloc_X
Allocates the oneliner resource using the indexrec bitmaps

Return the number of a one liner if one is available
and remove it from the free list
}
function alloc_line(var n: integer):boolean;
var
	found: boolean;

begin
	getindex(I_LINE);
	if indx.inuse = indx.top then begin
		freeindex;
		n := 0;
		alloc_line := false;
		writeln('There are no available one line descriptions.');
	end else begin
		n := 1;
		found := false;
		while (not found) and (n <= indx.top) do begin
			if indx.free[n] then
				found := true
			else
				n := n + 1;
		end;
		if found then begin
			indx.free[n] := false;
			alloc_line := true;
			indx.inuse := indx.inuse + 1;
			putindex;
		end else begin
			freeindex;
			writeln('%serious error in alloc_line; notify Monster Manager');
			
			alloc_line := false;
		end;
	end;
end;

{
put the line specified by n back on the free list
zeroes n also, for convenience
}
procedure delete_line(var n: integer);

begin
	if n = DEFAULT_LINE then
		n := 0
	else if n > 0 then begin
		getindex(I_LINE);
		indx.inuse := indx.inuse - 1;
		indx.free[n] := true;
		putindex;
	end;
	n := 0;
end;



function alloc_int(var n: integer):boolean;
var
	found: boolean;

begin
	getindex(I_INT);
	if indx.inuse = indx.top then begin
		freeindex;
		n := 0;
		alloc_int := false;
		writeln('There are no available integer records.');
	end else begin
		n := 1;
		found := false;
		while (not found) and (n <= indx.top) do begin
			if indx.free[n] then
				found := true
			else
				n := n + 1;
		end;
		if found then begin
			indx.free[n] := false;
			alloc_int := true;
			indx.inuse := indx.inuse + 1;
			putindex;
		end else begin
			freeindex;
			writeln('%serious error in alloc_int; notify Monster Manager');
			
			alloc_int := false;
		end;
	end;
end;


procedure delete_int(var n: integer);

begin
	if n > 0 then begin
		getindex(I_INT);
		indx.inuse := indx.inuse - 1;
		indx.free[n] := true;
		putindex;
	end;
	n := 0;
end;



{
Return the number of a description block if available and
remove it from the free list
}
function alloc_block(var n: integer):boolean;
var
	found: boolean;

begin
	if debug then
		writeln('%alloc_block entry');
	getindex(I_BLOCK);
	if indx.inuse = indx.top then begin
		freeindex;
		n := 0;
		alloc_block := false;
		writeln('There are no available description blocks.');
	end else begin
		n := 1;
		found := false;
		while (not found) and (n <= indx.top) do begin
			if indx.free[n] then
				found := true
			else
				n := n + 1;
		end;
		if found then begin
			indx.free[n] := false;
			alloc_block := true;
			indx.inuse := indx.inuse + 1;
			putindex;
			if debug then
				writeln('%alloc_block successful');
		end else begin
			freeindex;
			writeln('%serious error in alloc_block; notify Monster Manager');
			alloc_block := false;
		end;
	end;
end;




{
puts a description block back on the free list
zeroes n for convenience
}
procedure delete_block(var n: integer);

begin
	if n = DEFAULT_LINE then
		n := 0		{ no line really exists in the file }
	else if n > 0 then begin
		getindex(I_BLOCK);
		indx.inuse := indx.inuse - 1;
		indx.free[n] := true;
		putindex;
		n := 0;
	end else if n < 0 then begin
		n := (- n);
		delete_line(n);
	end;
end;



{
Return the number of a room if one is available
and remove it from the free list
}
function alloc_room(var n: integer):boolean;
var
	found: boolean;

begin
	getindex(I_ROOM);
	if indx.inuse = indx.top then begin
		freeindex;
		n := 0;
		alloc_room := false;
		writeln('There are no available free rooms.');
	end else begin
		n := 1;
		found := false;
		while (not found) and (n <= indx.top) do begin
			if indx.free[n] then
				found := true
			else
				n := n + 1;
		end;
		if found then begin
			indx.free[n] := false;
			alloc_room := true;
			indx.inuse := indx.inuse + 1;
			putindex;
		end else begin
			freeindex;
			writeln('%serious error in alloc_room; notify Monster Manager');
			alloc_room := false;
		end;
	end;
end;

{
Called by DEL_ROOM()
put the room specified by n back on the free list
zeroes n also, for convenience
}
procedure delete_room(var n: integer);

begin
	if n <> 0 then begin
		getindex(I_ROOM);
		indx.inuse := indx.inuse - 1;
		indx.free[n] := true;
		putindex;
		n := 0;
	end;
end;



function alloc_log(var n: integer):boolean;
var
	found: boolean;

begin
	getindex(I_PLAYER);
	if indx.inuse = indx.top then begin
		freeindex;
		n := 0;
		alloc_log := false;
		writeln('There are too many monster players, you can''t find a space.');
	end else begin
		n := 1;
		found := false;
		while (not found) and (n <= indx.top) do begin
			if indx.free[n] then
				found := true
			else
				n := n + 1;
		end;
		if found then begin
			indx.free[n] := false;
			alloc_log := true;
			indx.inuse := indx.inuse + 1;
			putindex;
		end else begin
			freeindex;
			writeln('%serious error in alloc_log; notify Monster Manager');
			alloc_log := false;
		end;
	end;
end;

procedure delete_log(var n: integer);

begin
	if n <> 0 then begin
		getindex(I_PLAYER);
		indx.inuse := indx.inuse - 1;
		indx.free[n] := true;
		putindex;
		n := 0;
	end;
end;


function lowcase(s: string):string;
var
	sprime: string;
	i: integer;

begin
	if length(s) = 0 then
		lowcase := ''
	else begin
		sprime := s;
		for i := 1 to length(s) do
			if sprime[i] in ['A'..'Z'] then
			   sprime[i] := chr(ord('a')+(ord(sprime[i])-ord('A')));
		lowcase := sprime;
	end;
end;


{ lookup a spell with disambiguation in the spell list }

function lookup_spell(var n: integer;s:string): boolean;
var
	i,poss,maybe,num: integer;

begin
	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to numspells do begin
		if s = spells[i] then
			num := i
		else if index(spells[i],s) = 1 then begin
			maybe := maybe + 1;
			poss := i;
		end;
	end;
	if num <> 0 then begin
		n := num;
		lookup_spell := true;
	end else if maybe = 1 then begin
		n := poss;
		lookup_spell := true;
	end else if maybe > 1 then begin
		lookup_spell := false;
	end else begin
		lookup_spell := false;
	end;
end;


function lookup_user(var pnum: integer;s: string): boolean;
var
	i,poss,maybe,num: integer;

begin
	getuser;
	freeuser;
	getindex(I_PLAYER);
	freeindex;

	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to indx.top do begin
		if not(indx.free[i]) then begin
			if s = user.idents[i] then
				num := i
			else if index(user.idents[i],s) = 1 then begin
				maybe := maybe + 1;
				poss := i;
			end;
		end;
	end;
	if num <> 0 then begin
		pnum := num;
		lookup_user := true;
	end else if maybe = 1 then begin
		pnum := poss;
		lookup_user := true;
	end else if maybe > 1 then begin
{		writeln('-- Ambiguous direction');	}
		lookup_user := false;
	end else begin
		lookup_user := false;
{		writeln('-- Unknown direction');	}
	end;
end;


function alloc_obj(var n: integer):boolean;
var
	found: boolean;

begin
	getindex(I_OBJECT);
	if indx.inuse = indx.top then begin
		freeindex;
		n := 0;
		alloc_obj := false;
		writeln('All of the possible objects have been made.');
	end else begin
		n := 1;
		found := false;
		while (not found) and (n <= indx.top) do begin
			if indx.free[n] then
				found := true
			else
				n := n + 1;
		end;
		if found then begin
			indx.free[n] := false;
			alloc_obj := true;
			indx.inuse := indx.inuse + 1;
			putindex;
		end else begin
			freeindex;
			writeln('%serious error in alloc_obj; notify Monster Manager');
			alloc_obj := false;
		end;
	end;
end;


procedure delete_obj(var n: integer);

begin
	if n <> 0 then begin
		getindex(I_OBJECT);
		indx.inuse := indx.inuse - 1;
		indx.free[n] := true;
		putindex;
		n := 0;
	end;
end;




function lookup_obj(var pnum: integer;s: string): boolean;
var
	i,poss,maybe,num: integer;
	tmp: string;

begin
	getobjnam;
	freeobjnam;
	getindex(I_OBJECT);
	freeindex;

	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to indx.top do begin
		if not(indx.free[i]) then begin
			if s = objnam.idents[i] then
				num := i
			else if index(objnam.idents[i],s) = 1 then begin
				maybe := maybe + 1;
				poss := i;
			end;
		end;
	end;
	if num <> 0 then begin
		pnum := num;
		lookup_obj := true;
	end else if maybe = 1 then begin
		pnum := poss;
		lookup_obj := true;
	end else if maybe > 1 then begin
{		writeln('-- Ambiguous direction');	}
		lookup_obj := false;
	end else begin
		lookup_obj := false;
{		writeln('-- Unknown direction');	}
	end;
end;



{ returns true if object N is in this room }

function obj_here(n: integer): boolean;
var
	i: integer;
	found: boolean;

begin
	i := 1;
	found := false;
	while (i <= maxobjs) and (not found) do begin
		if here.objs[i] = n then
			found := true
		else
			i := i + 1;
	end;
	obj_here := found;
end;




{ returns true if object N is being held by the player }

function obj_hold(n: integer): boolean;
var
	i: integer;
	found: boolean;

begin
	if n = 0 then
		obj_hold := false
	else begin
		i := 1;
		found := false;
		while (i <= maxhold) and (not found) do begin
			if here.people[myslot].holding[i] = n then
				found := true
			else
				i := i + 1;
		end;
		obj_hold := found;
	end;
end;



{ return the slot of an object that is HERE }
function find_obj(objnum: integer): integer;
var
	i: integer;

begin
	i := 1;
	find_obj := 0;
	while i <= maxobjs do begin
		if here.objs[i] = objnum then
			find_obj := i;
		i := i + 1;
	end;
end;



{ similar to lookup_obj, but only returns true if the object is in
  this room or is being held by the player }

function parse_obj(var n: integer; s: string;override: boolean := false): boolean;
var
	slot: integer;

begin
	if lookup_obj(n,s) then begin
		if obj_here(n) or obj_hold(n) then

			{ took out a great block of code that wouldn't let
			  parse_obj work if player couldn't see object }

			parse_obj := true;
	end else
		parse_obj := false;
end;




function lookup_pers(var pnum: integer;s: string): boolean;
var
	i,poss,maybe,num: integer;
	pname: string;

begin
	getpers;
	freepers;
	getindex(I_PLAYER);
	freeindex;

	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to indx.top do begin
		if not(indx.free[i]) then begin
			pname := lowcase(pers.idents[i]);

			if s = pname then
				num := i
			else if index(pname,s) = 1 then begin
				maybe := maybe + 1;
				poss := i;
			end;
		end;
	end;
	if num <> 0 then begin
		pnum := num;
		lookup_pers := true;
	end else if maybe = 1 then begin
		pnum := poss;
		lookup_pers := true;
	end else if maybe > 1 then begin
{		writeln('-- Ambiguous direction');	}
		lookup_pers := false;
	end else begin
		lookup_pers := false;
{		writeln('-- Unknown direction');	}
	end;
end;



function parse_pers(var pnum: integer;s: string): boolean;
var
	persnum: integer;
	i,poss,maybe,num: integer;
	pname: string;

begin
	gethere;
	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to maxpeople do begin
{		if here.people[i].username <> '' then begin	}

		if here.people[i].kind > 0 then begin
			pname := lowcase(here.people[i].name);

			if s = pname then
				num := i
			else if index(pname,s) = 1 then begin
				maybe := maybe + 1;
				poss := i;
			end;
		end;
	end;
	if num <> 0 then begin
		persnum := num;
		parse_pers := true;
	end else if maybe = 1 then begin
		persnum := poss;
		parse_pers := true;
	end else if maybe > 1 then begin
		persnum := 0;
		parse_pers := false;
	end else begin
		persnum := 0;
		parse_pers := false;
	end;
	if persnum > 0 then begin
		if here.people[persnum].hiding > 0 then
			parse_pers := false
		else begin
			parse_pers := true;
			pnum := persnum;
		end;
	end;
end;





{
Returns TRUE if player is owner of room n
If no n is given default will be this room (location)
}
function is_owner(n: integer := 0;surpress:boolean := false): boolean;

begin
	gethere(n);
	if (here.owner = userid) or (privd) then
		is_owner := true
	else begin
		is_owner := false;
		if not(surpress) then
			writeln('You are not the owner of this room.');
	end;
end;


function room_owner(n: integer): string;

begin
	if n <> 0 then begin
		gethere(n);
		room_owner := here.owner;
		gethere;	{ restore old state! }
	end else
		room_owner := 'no room';
end;

{
Returns TRUE if player is allowed to alter the exit
TRUE if either this room or if target room is owned by player
}

function can_alter(dir: integer;room: integer := 0): boolean;

begin
	gethere;
	if (here.owner=userid) or (privd) then begin
		can_alter := true
	end else begin
		if here.exits[dir].toloc > 0 then begin
			if room_owner(here.exits[dir].toloc) = userid then
				can_alter := true
			else
				can_alter := false;
		end else
			can_alter := false;
	end;
end;

function can_make(dir: integer;room: integer := 0): boolean;

begin
	gethere(room);	{ 5 is accept door }
	if (here.exits[dir].toloc <> 0) then begin
		writeln('There is already an exit there.  Use UNLINK or RELINK.');
		can_make := false;
	end else begin
		if (here.owner = userid) or		{ I'm the owner }
		   (here.exits[dir].kind = 5) or	{ there's an accept }
		   (privd) or		{ Monster Manager }
		   (here.owner = '*')			{ disowned room }
							 then
			can_make := true
		else begin
			can_make := false;
			writeln('You are not allowed to create an exit there.');
		end;
	end;
end;


{
print a one liner
}
procedure print_line(n: integer);

begin
	if n = DEFAULT_LINE then
		writeln('<default line>')
	else if n > 0 then begin
		getline(n);
		freeline;
		writeln(oneliner.theline);
	end;
end;



procedure print_desc(dsc: integer;default:string := '<no default supplied>');
var
	i: integer;

begin
	if dsc = DEFAULT_LINE then begin
		writeln(default);
	end else if dsc > 0 then begin
		getblock(dsc);
		freeblock;
		i := 1;
		while i <= block.desclen do begin
			writeln(block.lines[i]);
			i := i + 1;
		end;
	end else if dsc < 0 then begin
		print_line(abs(dsc));
	end;
end;




procedure make_line(var n: integer;prompt : string := '';limit:integer := 79);
var
	s: string;
	ok: boolean;

begin
	writeln('Type ** to leave line unchanged, * to make [no line]');
	grab_line(prompt,s);
	if s = '**' then begin
		writeln('No changes.');
	end else if s = '***' then begin
		n := DEFAULT_LINE;
	end else if s = '*' then begin
		if debug then
			writeln('%deleting line ',n:1);
		delete_line(n);
	end else if s = '' then begin
		if debug then
			writeln('%deleting line ',n:1);
		delete_line(n);
	end else if length(s) > limit then begin
		writeln('Please limit your string to ',limit:1,' characters.');
	end else begin
		if (n = 0) or (n = DEFAULT_LINE) then begin
			if debug then
				writeln('%makeline: allocating line');
			ok := alloc_line(n);
		end else
			ok := true;

		if ok then begin
			if debug then
				writeln('%ok in makeline');
			getline(n);
			oneliner.theline := s;
			putline;

			if debug then
				writeln('%completed putline in makeline');
		end;
	end;
end;


{ translate a direction s [north, south, etc...] into the integer code }

function lookup_dir(var dir: integer;s:string): boolean;
var
	i,poss,maybe,num: integer;

begin
	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to maxexit do begin
		if s = direct[i] then
			num := i
		else if index(direct[i],s) = 1 then begin
			maybe := maybe + 1;
			poss := i;
		end;
	end;
	if num <> 0 then begin
		dir := num;
		lookup_dir := true;
	end else if maybe = 1 then begin
		dir := poss;
		lookup_dir := true;
	end else if maybe > 1 then begin
		lookup_dir := false;
{		writeln('-- Ambiguous direction');	}
	end else begin
		lookup_dir := false;
{		writeln('-- Unknown direction');	}
	end;
end;


function lookup_show(var n: integer;s:string): boolean;
var
	i,poss,maybe,num: integer;

begin
	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to numshow do begin
		if s = show[i] then
			num := i
		else if index(show[i],s) = 1 then begin
			maybe := maybe + 1;
			poss := i;
		end;
	end;
	if num <> 0 then begin
		n := num;
		lookup_show := true;
	end else if maybe = 1 then begin
		n := poss;
		lookup_show := true;
	end else if maybe > 1 then begin
		lookup_show := false;
{		writeln('-- Ambiguous direction');	}
	end else begin
		lookup_show := false;
{		writeln('-- Unknown direction');	}
	end;
end;

function lookup_set(var n: integer;s:string): boolean;
var
	i,poss,maybe,num: integer;

begin
	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to numset do begin
		if s = setkey[i] then
			num := i
		else if index(setkey[i],s) = 1 then begin
			maybe := maybe + 1;
			poss := i;
		end;
	end;
	if num <> 0 then begin
		n := num;
		lookup_set := true;
	end else if maybe = 1 then begin
		n := poss;
		lookup_set := true;
	end else if maybe > 1 then begin
		lookup_set := false;
	end else begin
		lookup_set := false;
	end;
end;


function lookup_room(var n: integer; s: string): boolean;
var
	found: boolean;
	top: integer;

	i,
	poss,
	maybe,
	num:	integer;

begin
	if s <> '' then begin
		s := lowcase(s);		{ case insensitivity }
		getnam;
		freenam;
		getindex(I_ROOM);
		freeindex;
		top := indx.top;


		i := 1;
		maybe := 0;
		num := 0;
		for i := 1 to top do begin
			if s = nam.idents[i] then
				num := i
			else if index(nam.idents[i],s) = 1 then begin
				maybe := maybe + 1;
				poss := i;
			end;
		end;
		if num <> 0 then begin
			lookup_room := true;
			n := num;
		end else if maybe = 1 then begin
			lookup_room := true;
			n := poss;
		end else if maybe > 1 then begin
			lookup_room := false;
		end else begin
			lookup_room := false;
		end;

	end else
		lookup_room := false;
end;


function exact_room(var n: integer;s: string): boolean;
var
	match: boolean;

begin
	if debug then
		writeln('%exact room: s = ',s);
	if lookup_room(n,s) then begin
		if nam.idents[n] = lowcase(s) then
			exact_room := true
		else
			exact_room := false;
	end else
		exact_room := false;
end;


function exact_pers(var n: integer;s: string): boolean;
var
	match: boolean;

begin
	if lookup_pers(n,s) then begin
		if lowcase(pers.idents[n]) = lowcase(s) then
			exact_pers := true
		else
			exact_pers := false;
	end else
		exact_pers := false;
end;


function exact_user(var n: integer;s: string): boolean;
var
	match: boolean;

begin
	if lookup_user(n,s) then begin
		if lowcase(user.idents[n]) = lowcase(s) then
			exact_user := true
		else
			exact_user := false;
	end else
		exact_user := false;
end;


function exact_obj(var n: integer;s: string): boolean;
var
	match: boolean;

begin
	if lookup_obj(n,s) then begin
		if objnam.idents[n] = lowcase(s) then
			exact_obj := true
		else
			exact_obj := false;
	end else
		exact_obj := false;
end;



{
Return n as the direction number if s is a valid alias for an exit
}
function lookup_alias(var n: integer; s: string): boolean;
var
	i,poss,maybe,num: integer;

begin
	gethere;
	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to maxexit do begin
		if s = here.exits[i].alias then
			num := i
		else if index(here.exits[i].alias,s) = 1 then begin
			maybe := maybe + 1;
			poss := i;
		end;
	end;
	if num <> 0 then begin
		n := num;
		lookup_alias := true;
	end else if maybe = 1 then begin
		n := poss;
		lookup_alias := true;
	end else if maybe > 1 then begin
		lookup_alias := false;
	end else begin
		lookup_alias := false;
	end;
end;


procedure exit_default(dir, kind: integer);

begin
	case kind of

	1: writeln('There is a passage leading ',direct[dir],'.');
	2: writeln('There is a locked door leading ',direct[dir],'.');
	5:	case dir of
			north,south,east,west:
				writeln('A note on the ',direct[dir],' wall says "Your exit here."');
			up: writeln('A note on the ceiling says "Your exit here."');
			down: writeln('A note on the floor says "Your exit here."');
		end;
	otherwise writeln('There is an exit: ',direct[dir]);
	end;
end;


{
Prints out the exits here for DO_LOOK()
}
procedure show_exits;
var
	i: integer;
	one: boolean;
	cansee: boolean;

begin
	one := false;
	for i := 1 to maxexit do begin
		if (here.exits[i].toloc <> 0) or { there is an exit }
		   (here.exits[i].kind = 5) then begin { there could be an exit }

			if (here.exits[i].hidden = 0) or
			   (found_exit[i]) then
				cansee := true
			else
				cansee := false;

			if here.exits[i].kind = 6 then begin
				{ door kind only visible with object }
				if obj_hold( here.exits[i].objreq ) then
					cansee := true
				else
					cansee := false;
			end;

			if cansee then begin
				if here.exits[i].exitdesc = DEFAULT_LINE then begin
					exit_default(i,here.exits[i].kind);
					{ give it direction and type }
					one := true;
				end else if here.exits[i].exitdesc > 0 then begin
					print_line(here.exits[i].exitdesc);
					one := true;
				end;
			end;
		end;
	end;
	if one then
		writeln;
end;


procedure setevent;

begin
	getevent;
	freeevent;
	myevent := event.point;
end;



function isnum(s: string): boolean;
var
	i: integer;

begin
	isnum := true;
	if length(s) < 1 then
		isnum := false
	else begin
		i := 1;
		while i <= length(s) do begin
			if not (s[i] in ['0'..'9']) then
				isnum := false;
			i := i + 1;
		end;
	end;
end;

function number(s: string): integer;
var
	i: integer;

begin
	if (length(s) < 1) or not(s[1] in ['0'..'9']) then
		number := 0
	else begin
		readv(s,i);
		number := i;
	end;
end;



procedure log_event(	send: integer := 0;	{ slot of sender }
			act:integer;		{ what event occurred }
			targ: integer := 0;	{ target of event }
			p: integer := 0;	{ expansion parameter }
			s: string := '';	{ string for messages }
			room: integer := 0	{ room to log event in }
		   );

begin
	if room = 0 then
		room := location;
	getevent(room);
	event.point := event.point + 1;
	if debug then
		writeln('%logging event ',act:1,' to point ',event.point:1);
	if event.point > maxevent then
		event.point := 1;
	with event.evnt[event.point] do begin
		sender := send;
		action := act;
		target := targ;
		parm := p;
		msg := s;
		loc := room;
	end;
	putevent;
end;

procedure log_action(theaction,thetarget: integer);

begin
	if debug then
		writeln('%log_action(',theaction:1,',',thetarget:1,')');
	getroom;
	here.people[myslot].act := theaction;
	here.people[myslot].targ := thetarget;
	putroom;

	logged_act := true;
	log_event(myslot,E_ACTION,thetarget,theaction,myname);
end;


function desc_action(theaction,thetarget: integer): string;
var
	s: string;

begin
	case theaction of	{ use command mnemonics }
		look:      s:= ' looking around the room.';
		form:      s:= ' creating a new room.';
		desc:      s:= ' editing the description to this room.';
		e_detail:  s := ' adding details to the room.';
		c_custom:  s := ' customizing an exit here.';
		e_custroom:s := ' customizing this room.';
		e_program: s := ' customizing an object.';
		c_self:	   s := ' editing a self-description.';
		e_usecrystal: s := ' hunched over a crystal orb, immersed in its glow.';
		link:	   s := ' creating an exit here.';
		c_system:  s := ' in system maintenance mode.';

		otherwise s := ' here.'
	end;
	desc_action := s;
end;


function protected(n: integer := 0): boolean;

begin
	if n = 0 then
		n := myslot;
	if here.people[n].act in [e_detail,c_custom,
				  e_custroom,e_program,
				  c_self,c_system] then
		protected := true
	else
		protected := false;
end;



{
user procedure to designate an exit for acceptance of links
}
procedure do_accept(s: string);
var
	dir: integer;

begin
	if lookup_dir(dir,s) then begin
		if can_make(dir) then begin
			getroom;
			here.exits[dir].kind := 5;
			putroom;

			log_event(myslot,E_ACCEPT,0,0);
			writeln('Someone will be able to make an exit ',direct[dir],'.');
		end;
	end else
		writeln('To allow others to make an exit, type ACCEPT <direction of exit>.');
end;


{
User procedure to refuse an exit for links
Note: may be unlink
}
procedure do_refuse(s: string);
var
	dir: integer;
	ok: boolean;

begin
	if not(is_owner) then
		{ is_owner prints error message itself }
	else if lookup_dir(dir,s) then begin
		getroom;
		with here.exits[dir] do begin
			if (toloc = 0) and (kind = 5) then begin
				kind := 0;
				ok := true;
			end else
				ok := false;
		end;
		putroom;
		if ok then begin
			log_event(myslot,E_REFUSE,0,0);
			writeln('Exits ',direct[dir],' will be refused.');
		end else
			writeln('Exits were not being accepted there.');
	end else
		writeln('To undo an Accept, type REFUSE <direction>.');
end;



function systime:string;
var
	hourstring: string;
	hours: integer;
	thetime: packed array[1..11] of char;
	dayornite: string;

begin
	time(thetime);
	if thetime[1] = ' ' then
		hours := ord(thetime[2]) - ord('0')
	else
		hours := (ord(thetime[1]) - ord('0'))*10 +
			  (ord(thetime[2]) - ord('0'));

	if hours < 12 then
		dayornite := 'am'
	else
		dayornite := 'pm';
	if hours >= 13 then
		hours := hours - 12;
	if hours = 0 then
		hours := 12;

	writev(hourstring,hours:2);

	systime := hourstring + ':' + thetime[4] + thetime[5] + dayornite;
end;



{ substitute a parameter string for the # sign in the source string }
function subs_parm(s,parm: string): string;
var
	right,left: string;
	i: integer;		{ i is point to break at }

begin
	i := index(s,'#');
	if (i > 0) and ((length(s) + length(parm)) <= 80) then begin
		if i >= length(s) then begin
			right := '';
			left := s;
		end else if i < 1 then begin
			right := s;
			left := '';
		end else begin
			right := substr(s,i+1,length(s)-i);
			left := substr(s,1,i);
		end;
		if length(left) <= 1 then
			left := ''
		else
			left := substr(left,1,length(left)-1);

		subs_parm := left + parm + right;
	end else begin
		subs_parm := s;
	end;
end;


procedure time_health;

begin
	if healthcycle > 0 then begin		{ how quickly they heal }
		if myhealth < 7 then begin	{ heal a little bit }
			myhealth := myhealth + 1;

			getroom;
			here.people[myslot].health := myhealth;
			putroom;

			{show new health rating }
		case myhealth of
			9: writeln('You are now in exceptional health.');
			8: writeln('You feel much stronger.  You are in better than average condition.');
			7: writeln('You are now in perfect health.');
			6: writeln('You only feel a little bit dazed now.');
			5: begin
				writeln('You only have some minor cuts and abrasions now.  Most of your serious wounds');
				writeln('have healed.');
			   end;
			4: writeln('You are only suffering from some minor wounds now.');
			3: writeln('Your most serious wounds have healed, but you are still in bad shape.');
			2: writeln('You have healed somewhat, but are still very badly wounded.');
			1: writeln('You are in critical condition, but there may be hope.');
			0: writeln('are still dead.');
			otherwise writeln('You don''t seem to be in any condition at all.');
		end;

		putchars(chr(10)+old_prompt+line);

		end;
		healthcycle := 0;
	end else
		healthcycle := healthcycle + 1;
end;


procedure time_noises;
var
	n: integer;

begin
	if rnd100 <= 2 then begin
		n := rnd100;
		if n in [0..40] then
			log_event(0,E_NOISES,rnd100,0)
		else if n in [41..60] then
			log_event(0,E_ALTNOISE,rnd100,0);
	end;
end;


procedure time_trapdoor(silent: boolean);
var
	fall: boolean;

begin
	if rnd100 < here.trapchance then begin
			{ trapdoor fires! }

		if here.trapto > 0 then begin
				{ logged action should cover {protected) }
			if {(protected) or} (logged_act) then
				fall := false
			else if here.magicobj = 0 then
				fall := true
			else if obj_hold(here.magicobj) then
				fall := false
			else
				fall := true;
		end else
			fall := false;

		if fall then begin
			do_exit(here.trapto);
			if not(silent) then
				putchars(chr(10)+old_prompt+line);
		end;
	end;
end;


procedure time_midnight;

begin
	if systime = '12:00am' then
		log_event(0,E_MIDNIGHT,rnd100,0);
end;


{ cause random events to occurr (ha ha ha) }

procedure rnd_event(silent: boolean := false);
var
	n: integer;

begin
	if rndcycle = 200 then begin	{ inside here 3 times/min }

		time_noises;
		time_health;
		time_trapdoor(silent);
		time_midnight;

		rndcycle := 0;
	end else
		rndcycle := rndcycle + 1;
end;


procedure do_die;
var
	some: boolean;

begin
	writeln;
	writeln('        *** You have died ***');
	writeln;
	some := drop_everything;
	myhealth := 7;
	take_token(myslot,location);
	log_event(0,E_DIED,0,0,myname);
	if put_token(2,myslot) then begin
		location := 2;
		inmem := false;
		setevent;
{ log entry to death loc }
{ perhaps turn off refs to other people }
	end else begin
		writeln('The Monster universe regrets to inform you that you cannot be ressurected at');
		writeln('the moment.');
		halt;
	end;
end;


procedure poor_health(p: integer);
var
	some: boolean;

begin
	if myhealth > p then begin
		myhealth := myhealth - 1;
		getroom;
		here.people[myslot].health := myhealth;
		putroom;
		log_event(myslot,E_WEAKER,myhealth,0);

		{ show new health rating }
		write('You ');
		case here.people[myslot].health of
			9: writeln('are still in exceptional health.');
			8: writeln('feel weaker, but are in better than average condition.');
			7: writeln('are somewhat weaker, but are in perfect health.');
			6: writeln('feel a little bit dazed.');
			5: writeln('have some minor cuts and abrasions.');
			4: writeln('have some wounds, but are still fairly strong.');
			3: writeln('are suffering from some serious wounds.'); 
			2: writeln('are very badly wounded.');
			1: writeln('have many serious wounds, and are near death.');
			0: writeln('are dead.');
			otherwise writeln('don''t seem to be in any condition at all.');
		end;
	end else begin { they died }
		do_die;
	end;
end;



{ count objects here }

function find_numobjs: integer;
var
	sum,i: integer;

begin
	sum := 0;
	for i := 1 to maxobjs do
		if here.objs[i] <> 0 then
			sum := sum + 1;
	find_numobjs := sum;
end;



{ optional parameter is slot of player's objects to count }

function find_numhold(player: integer := 0): integer;
var
	sum,i: integer;

begin
	if player = 0 then
		player := myslot;

	sum := 0;
	for i := 1 to maxhold do
		if here.people[player].holding[i] <> 0 then
			sum := sum + 1;
	find_numhold := sum;
end;




procedure take_hit(p: integer);
var
	i: integer;

begin
	if p > 0 then begin
		if rnd100 < (55 + (p-1) * 30) then { chance that they're hit }
			poor_health(p);

		if find_numobjs < maxobjs + 1 then begin
			{ maybe they drop something if they're hit }
			for i := 1 to p do
				maybe_drop;
		end;
	end;
end;


function punch_force(sock: integer): integer;
var
	p: integer;

begin
	if sock in [2,3,6,7,8,11,12] then	{ no punch or a graze }
		p := 0
	else if sock in [4,9,10] then	{ hard punches }
		p := 2
	else	{ 1,5,13,14,15 }
		p := 1;		{ all others are medium punches }
	punch_force := p;
end;

procedure put_punch(sock: integer;s: string);

begin
	case sock of
		1: writeln('You deliver a quick jab to ',s,'''s jaw.');
		2: writeln('You swing at ',s,' and miss.');
		3: writeln('A quick punch, but it only grazes ',s,'.');
		4: writeln(s,' doubles over after your jab to the stomach.');
		5: writeln('Your punch lands square on ',s,'''s face!');
		6: writeln('You swing wild and miss.');
		7: writeln('A good swing, but it misses ',s,' by a mile!');
		8: writeln('Your punch is blocked by ',s,'.');
		9: writeln('Your roundhouse blow sends ',s,' reeling.');
		10:writeln('You land a solid uppercut on ',s,'''s chin.');
		11:writeln(s,' fends off your blow.');
		12:writeln(s,' ducks and avoids your punch.');
		13:writeln('You thump ',s,' in the ribs.');
		14:writeln('You catch ',s,'''s face on your elbow.');
		15:writeln('You knock the wind out of ',s,' with a punch to the chest.');
	end;
end;


procedure get_punch(sock: integer;s: string);

begin
	case sock of
		1: writeln(s,' delivers a quick jab to your jaw!');
		2: writeln(s,' swings at you but misses.');
		3: writeln(s,'''s fist grazes you.');
		4: writeln('You double over after ',s,' lands a mean jab to your stomach!');
		5: writeln('You see stars as ',s,' bashes you in the face.');
		6: writeln('You only feel the breeze as ',s,' swings wildly.');
		7: writeln(s,'''s swing misses you by a yard.');
		8: writeln('With lightning reflexes you block ',s,'''s punch.');
		9: writeln(s,'''s blow sends you reeling.');
		10:writeln('Your head snaps back from ',s,'''s uppercut!');
		11:writeln('You parry ',s,'''s attack.');
		12:writeln('You duck in time to avoid ',s,'''s punch.');
		13:writeln(s,' thumps you hard in the ribs.');
		14:writeln('Your vision blurs as ',s,' elbows you in the head.');
		15:writeln(s,' knocks the wind out of you with a punch to your chest.');
	end;
end;

procedure view_punch(a,b: string;p: integer);

begin
	case p of
		1: writeln(a,' jabs ',b,' in the jaw.');
		2: writeln(a,' throws a wild punch at the air.');
		3: writeln(a,'''s fist barely grazes ',b,'.');
		4: writeln(b,' doubles over in pain with ',a,'''s punch');
		5: writeln(a,' bashes ',b,' in the face.');
		6: writeln(a,' takes a wild swing at ',b,' and misses.');
		7: writeln(a,' swings at ',b,' and misses by a yard.');
		8: writeln(b,'''s punch is blocked by ',a,'''s quick reflexes.');
		9: writeln(b,' is sent reeling from a punch by ',a,'.');
		10:writeln(a,' lands an uppercut on ',b,'''s head.');
		11:writeln(b,' parrys ',a,'''s attack.');
		12:writeln(b,' ducks to avoid ',a,'''s punch.');
		13:writeln(a,' thumps ',b,' hard in the ribs.');
		14:writeln(a,'''s elbow connects with ',b,'''s head.');
		15:writeln(a,' knocks the wind out of ',b,'.');
	end;
end;




procedure desc_health(n: integer;header:shortstring := '');

begin
	if header = '' then
		write(here.people[n].name,' ')
	else
		write(header);

	case here.people[n].health of
		9: writeln('is in exceptional health, and looks very strong.');
		8: writeln('is in better than average condition.');
		7: writeln('is in perfect health.');
		6: writeln('looks a little dazed.');
		5: writeln('has some minor cuts and abrasions.');
		4: writeln('has some minor wounds.');
		3: writeln('is suffering from some serious wounds.'); 
		2: writeln('is very badly wounded.');
		1: writeln('has many serious wounds, and is near death.');
		0: writeln('is dead.');
		otherwise writeln('doesn''t seem to be in any condition at all.');
	end;
end;


function obj_part(objnum: integer;doread: boolean := TRUE): string;
var
	s: string;

begin
	if doread then begin
		getobj(objnum);
		freeobj;
	end;
	s := obj.oname;
	case obj.particle of
		0:;
		1: s := 'a ' + s;
		2: s := 'an ' + s;
		3: s := 'some ' + s;
		4: s := 'the ' + s;
	end;
	obj_part := s;
end;


procedure print_subs(n: integer;s: string);

begin
	if (n > 0) and (n <> DEFAULT_LINE) then begin
		getline(n);
		freeline;
		writeln(subs_parm(oneliner.theline,s));
	end else if n = DEFAULT_LINE then
		writeln('%<default line> in print_subs');
end;



{ print out a (up to) 10 line description block, substituting string s for
  up to one occurance of # per line }

procedure block_subs(n: integer;s: string);
var
	p,i: integer;

begin
	if n < 0 then
		print_subs(abs(n),s)
	else if (n > 0) and (n <> DEFAULT_LINE) then begin
		getblock(n);
		freeblock;
		i := 1;
		while i <= block.desclen do begin
			p := index(block.lines[i],'#');
			if (p > 0) then
				writeln(subs_parm(block.lines[i],s))
			else
				writeln(block.lines[i]);
			i := i + 1;
		end;
	end;
end;


procedure show_noises(n: integer);

begin
	if n < 33 then
		writeln('There are strange noises coming from behind you.')
	else if n < 66 then
		writeln('You hear strange rustling noises behind you.')
	else
		writeln('There are faint noises coming from behind you.');
end;


procedure show_altnoise(n: integer);

begin
	if n < 33 then
		writeln('A chill wind blows, ruffling your clothes and chilling your bones.')
	else if n < 66 then
		writeln('Muffled scuffling sounds can be heard behind you.')
	else
		writeln('A loud crash can be heard in the distance.');
end;


procedure show_midnight(n: integer;var printed: boolean);

begin
	if midnight_notyet then begin
		if n < 50 then begin
			writeln('A voice booms out of the air from all around you!');
			writeln('The voice says,  " It is now midnight. "');
		end else begin
			writeln('You hear a clock chiming in the distance.');
			writeln('It rings twelve times for midnight.');
		end;
		midnight_notyet := false;
	end else
		printed := false;
end;




procedure handle_event(var printed: boolean);
var
	n,send,act,targ,p: integer;
	s: string;
	sendname: string;

begin
	printed := true;
	if debug then
		writeln('%handling event ',myevent);
	with event.evnt[myevent] do begin
		send := sender;
		act := action;
		targ := target;
		p := parm;
		s := msg;
	end;
	if send <> 0 then
		sendname := here.people[send].name
	else
		sendname := '<Unknown>';

	case act of
		E_EXIT: begin
				if here.exits[targ].goin = DEFAULT_LINE then
					writeln(s,' has gone ',direct[targ],'.')
				else if (here.exits[targ].goin <> 0) and
				(here.exits[targ].goin <> DEFAULT_LINE) then begin
					block_subs(here.exits[targ].goin,s);
				end else
					printed := false;
			end;
		E_ENTER: begin
				if here.exits[targ].comeout = DEFAULT_LINE then
					writeln(s,' has come into the room from: ',direct[targ])
				else if (here.exits[targ].comeout <> 0) and
				(here.exits[targ].comeout <> DEFAULT_LINE) then begin
					block_subs(here.exits[targ].comeout,s);
				end else
					printed := false;
			end;
		E_BEGIN:writeln(s,' appears in a brilliant burst of multicolored light.');
		E_QUIT:writeln(s,' vanishes in a brilliant burst of multicolored light.');
		E_SAY: begin
			if length(s) + length(sendname) > 73 then begin
				writeln(sendname,' says,');
				writeln('"',s,'"');
			end else begin
				if (rnd100 < 50) or (length(s) > 50) then
					writeln(sendname,': "',s,'"')
				else
					writeln(sendname,' says, "',s,'"');
			end;
		       end;
		E_HIDESAY: begin
				writeln('An unidentified voice speaks to you:');
				writeln('"',s,'"');
			   end;
		E_SETNAM: writeln(s);
		E_POOFIN: writeln('In an explosion of orange smoke ',s,' poofs into the room.');
		E_POOFOUT: writeln(s,' vanishes from the room in a cloud of orange smoke.');
		E_DETACH: begin
				writeln(s,' has destroyed the exit ',direct[targ],'.');
			  end;
		E_EDITDONE:begin
				writeln(sendname,' is done editing the room description.');
			   end;
		E_NEWEXIT: begin
				writeln(s,' has created an exit here.');
			   end;
		E_CUSTDONE:begin
				writeln(sendname,' is done customizing an exit here.');
			   end;
		E_SEARCH: writeln(sendname,' seems to be looking for something.');
		E_FOUND: writeln(sendname,' appears to have found something.');
		E_DONEDET:begin
				writeln(sendname,' is done adding details to the room.');
			  end;
		E_ROOMDONE: begin
				writeln(sendname,' is finished customizing this room.');
			    end;
		E_OBJDONE: begin
				writeln(sendname,' is finished customizing an object.');
			   end;
		E_UNHIDE:writeln(sendname,' has stepped out of the shadows.');
		E_FOUNDYOU: begin
				if targ = myslot then begin { found me! }
					writeln('You''ve been discovered by ',sendname,'!');
					hiding := false;
					getroom;
{ they're not hidden anymore }		here.people[myslot].hiding := 0;
					putroom;
				end else
					writeln(sendname,' has found ',here.people[targ].name,' hiding in the shadows!');
			    end;
		E_PUNCH: begin
				if targ = myslot then begin { punched me! }
					get_punch(p,sendname);
					take_hit( punch_force(p) );
{ relic, but not harmful }		ping_answered := true;
					healthcycle := 0;
				end else
					view_punch(sendname,here.people[targ].name,p);
			 end;
		E_MADEOBJ: writeln(s);
		E_GET: writeln(s);
		E_DROP: begin
				writeln(s);
				if here.objdesc <> 0 then
					print_subs(here.objdesc,obj_part(p));
			end;
		E_BOUNCEDIN: begin
				if (targ = 0) or (targ = DEFAULT_LINE) then
					writeln(obj_part(p),' has bounced into the room.')
				else begin
					print_subs(targ,obj_part(p));
				end;
			     end;
		E_DROPALL: writeln('Some objects drop to the ground.');
		E_EXAMINE: writeln(s);
		E_IHID: writeln(sendname,' has hidden in the shadows.');
		E_NOISES: begin
				if (here.rndmsg = 0) or
				   (here.rndmsg = DEFAULT_LINE) then begin
					show_noises(targ);
				end else
					print_line(here.rndmsg);
			  end;
		E_ALTNOISE: begin
				if (here.xmsg2 = 0) or
				   (here.xmsg2 = DEFAULT_LINE) then
					show_altnoise(targ)
				else
					block_subs(here.xmsg2,myname);
			    end;
		E_REALNOISE: show_noises(targ);
		E_HIDOBJ: writeln(sendname,' has hidden the ',s,'.');
		E_PING: begin
				if targ = myslot then begin
					writeln(sendname,' is trying to ping you.');
					log_event(myslot,E_PONG,send,0);
				end else
					writeln(sendname,' is pinging ',here.people[targ].name,'.');
			end;
		E_PONG: begin
				ping_answered := true;
			end;
		E_HIDEPUNCH: begin
				if targ = myslot then begin
					writeln(sendname,' pounces on you from the shadows!');
					take_hit(2);
				end else begin
					writeln(sendname,' jumps out of the shadows and attacks ',here.people[targ].name,'.');
				end;
			     end;
		E_SLIPPED: begin
				writeln('The ',s,' has slipped from ',
					sendname,'''s hands.');
			   end;
		E_HPOOFOUT:begin
				if rnd100 > 50 then
					writeln('Great wisps of orange smoke drift out of the shadows.')
				else
					printed := false;
			   end;
		E_HPOOFIN:begin
				if rnd100 > 50 then
					writeln('Some wisps of orange smoke drift about in the shadows.')
				else
					printed := false;
			  end;
		E_FAILGO: begin
				if targ > 0 then begin
					write(sendname,' has failed to go ');
					writeln(direct[targ],'.');
				end;
			  end;
		E_TRYPUNCH: begin
				if targ = myslot then
					writeln(sendname,' fails to punch you.')
				else
					writeln(sendname,' fails to punch ',here.people[targ].name,'.');
			    end;
		E_PINGONE:begin
				if targ = myslot then begin { ohoh---pinged away }
					writeln('The Monster program regrets to inform you that a destructive ping has');
					writeln('destroyed your existence.  Please accept our apologies.');
					halt;  { ugggg }
				end else
					writeln(s,' shimmers and vanishes from sight.');
			  end;
		E_CLAIM: writeln(sendname,' has claimed this room.');
		E_DISOWN: writeln(sendname,' has disowned this room.');
		E_WEAKER: begin
{				inmem := false;
				gethere;		}

				here.people[send].health := targ;

{ This is a hack for efficiency so we don't read the room record twice;
  we need the current data now for desc_health, but checkevents, our caller,
  is about to re-read it anyway; we make an incremental fix here so desc_health
  is happy, then checkevents will do the real read later }

				desc_health(send);
			  end;
		E_OBJCLAIM: writeln(sendname,' is now the owner of the ',s,'.');
		E_OBJDISOWN: writeln(sendname,' has disowned the object ',s,'.');
		E_SELFDONE: writeln(sendname,'''s self-description is finished.');
		E_WHISPER: begin
				if targ = myslot then begin
					if length(s) < 39 then
						writeln(sendname,' whispers to you, "',s,'"')
					else begin
						writeln(sendname,' whispers something to you:');
						write(sendname,' whispers, ');
						if length(s) > 50 then
							writeln;
						writeln('"',s,'"');
					end;
				end else if (privd) or (rnd100 > 85) then begin
					writeln('You overhear ',sendname,' whispering to ',here.people[targ].name,'!');
					write(sendname,' whispers, ');
					if length(s) > 50 then
						writeln;
					writeln('"',s,'"');
				end else
					writeln(sendname,' is whispering to ',here.people[targ].name,'.');
			   end;
		E_WIELD: writeln(sendname,' is now wielding the ',s,'.');
		E_UNWIELD: writeln(sendname,' is no longer wielding the ',s,'.');
		E_WEAR: writeln(sendname,' is now wearing the ',s,'.');
		E_UNWEAR: writeln(sendname,' has taken off the ',s,'.');
		E_DONECRYSTALUSE: begin
					writeln(sendname,' emerges from the glow of the crystal.');
					writeln('The orb becomes dark.');
				  end;
		E_DESTROY: writeln(s);
		E_OBJPUBLIC: writeln('The object ',s,' is now public.');
		E_SYSDONE: writeln(sendname,' is no longer in system maintenance mode.');
		E_UNMAKE: writeln(sendname,' has unmade ',s,'.');
		E_LOOKDETAIL: writeln(sendname,' is looking at the ',s,'.');
		E_ACCEPT: writeln(sendname,' has accepted an exit here.');
		E_REFUSE: writeln(sendname,' has refused an Accept here.');
		E_DIED: writeln(s,' expires and vanishes in a cloud of greasy black smoke.');
		E_LOOKYOU: begin
				if targ = myslot then begin
					writeln(sendname,' is looking at you.')
				end else
					writeln(sendname,' looks at ',here.people[targ].name,'.');
			   end;
		E_LOOKSELF: writeln(sendname,' is making a self-appraisal.');
		E_FAILGET: writeln(sendname,' fails to get ',obj_part(targ),'.');
		E_FAILUSE: writeln(sendname,' fails to use ',obj_part(targ),'.');
		E_CHILL: if (targ = 0) or (targ = DEFAULT_LINE) then
				writeln('A chill wind blows over you.')
			 else
				print_desc(targ);
		E_NOISE2:begin
				case targ of
					1: writeln('Strange, gutteral noises sound from everywhere.');
					2: writeln('A chill wind blows past you, almost whispering as it ruffles your clothes.');
					3: writeln('Muffled voices speak to you from the air!');
					otherwise writeln('The air vibrates with a chill shudder.');
				end;
			 end;
		E_INVENT: writeln(sendname,' is taking inventory.');
		E_POOFYOU: begin
				if targ = myslot then begin
					writeln;
					writeln(sendname,' directs a firey burst of bluish energy at you!');
					writeln('Suddenly, you find yourself hurtling downwards through misty orange clouds.');
					writeln('Your descent slows, the smoke clears, and you find yourself in a new place...');
					xpoof(p);
					writeln;
				end else begin
					writeln(sendname,' directs a firey burst of energy at ',here.people[targ].name,'!');
					writeln('A thick burst of orange smoke results, and when it clears, you see');
					writeln('that ',here.people[targ].name,' is gone.');
				end;
			   end;
		E_WHO: begin
			case p of
				0: writeln(sendname,' produces a "who" list and reads it.');
				1: writeln(sendname,' is seeing who''s playing Monster.');
				otherwise writeln(sendname,' checks the "who" list.');
			end;
		       end;
		E_PLAYERS:begin
				writeln(sendname,' checks the "players" list.');
			  end;
		E_VIEWSELF: writeln(sendname,' is reading ',s,'''s self-description.');
		E_MIDNIGHT: show_midnight(targ,printed);

		E_ACTION:writeln(sendname,' is',desc_action(p,targ));
		otherwise writeln('*** Bad Event ***');
	end;
end;


[global]
procedure checkevents(silent: boolean := false);
var
	gotone: boolean;
	tmp,printed: boolean;

begin
	getevent;
	freeevent;

	event := eventfile^;
	gotone := false;
	printed := false;
	while myevent <> event.point do begin
		myevent := myevent + 1;
		if myevent > maxevent then
			myevent := 1;

		if debug then begin
			writeln('%checking event ',myevent);
			if event.evnt[myevent].loc = location then
				writeln('  - event here')
			else
				writeln('  - event elsewhere');
			writeln('  - event number = ',event.evnt[myevent].action:1);
		end;

		if (event.evnt[myevent].loc = location) then begin
			if (event.evnt[myevent].sender <> myslot) then begin

						{ if sent by me don't look at it }
						{ will use global record event }
				handle_event(tmp);
				if tmp then
					printed := true;

				inmem := false;	{ re-read important data that }
				gethere;	{ may have been altered }

				gotone := true;
			end;
		end;
	end;
	if (printed) and (gotone) and not(silent) then begin
		putchars(chr(10)+chr(13)+old_prompt+line);
	end;

	rnd_event(silent);
end;



{ count the number of people in this room; assumes a gethere has been done }

function find_numpeople: integer;
var
	sum,i: integer;

begin
	sum := 0;
	for i := 1 to maxpeople do
		if here.people[i].kind > 0 then
{		if here.people[i].username <> '' then	}
			sum := sum + 1;
	find_numpeople := sum;
end;



{ don't give them away, but make noise--maybe
  percent is percentage chance that they WON'T make any noise }

procedure noisehide(percent: integer);

begin
	{ assumed gethere;  }
	if (hiding) and (find_numpeople > 1) then begin
		if rnd100 > percent then
			log_event(myslot,E_REALNOISE,rnd100,0);
			{ myslot: don't tell them they made noise }
	end;
end;



function checkhide: boolean;

begin
	if (hiding) then begin
		checkhide := false;
		noisehide(50);
		writeln('You can''t do that while you''re hiding.');
	end else
		checkhide := true;
end;



procedure clear_command;

begin
	if logged_act then begin
		getroom;
		here.people[myslot].act := 0;
		putroom;
		logged_act := false;
	end;
end;

{ forward procedure take_token(aslot, roomno: integer); }
procedure take_token;
			{ remove self from a room's people list }

begin
	getroom(roomno);
	with here.people[aslot] do begin
		kind := 0;
		username:= '';
		name := '';
	end;
	putroom;
end;


{ fowrard function put_token(room: integer;var aslot:integer;
	hidelev:integer := 0):boolean;
			 put a person in a room's people list
			 returns myslot }
function put_token;
var
	i,j: integer;
	found: boolean;
	savehold: array[1..maxhold] of integer;

begin
	if first_puttoken then begin
		for i := 1 to maxhold do
			savehold[i] := 0;
		first_puttoken := false;
	end else begin
		gethere;
		for i := 1 to maxhold do
			savehold[i] := here.people[myslot].holding[i];
	end;

	getroom(room);
	i := 1;
	found := false;
	while (i <= maxpeople) and (not found) do begin
		if here.people[i].name = '' then
			found := true
		else
			i := i + 1;
	end;
	put_token := found;
	if found then begin
		here.people[i].kind := 1;	{ I'm a real player }
		here.people[i].name := myname;
		here.people[i].username := userid;
		here.people[i].hiding := hidelev;
			{ hidelev is zero for most everyone
			  unless you want to poof in and remain hidden }

		here.people[i].wearing := mywear;
		here.people[i].wielding := mywield;
		here.people[i].health := myhealth;
		here.people[i].self := myself;

		here.people[i].act := 0;

		for j := 1 to maxhold do
			here.people[i].holding[j] := savehold[j];
		putroom;

		aslot := i;
		for j := 1 to maxexit do	{ haven't found any exits in }
			found_exit[j] := false;	{ the new room }

		{ note the user's new location in the logfile }
		getint(N_LOCATION); 
		anint.int[mylog] := room;
		putint;
	end else
		freeroom;
end;

procedure log_exit(direction,room,sender_slot: integer);

begin
	log_event(sender_slot,E_EXIT,direction,0,myname,room);
end;

procedure log_entry(direction,room,sender_slot: integer);

begin
	log_event(sender_slot,E_ENTER,direction,0,myname,room);
end;

procedure log_begin(room:integer := 1);

begin
	log_event(0,E_BEGIN,0,0,myname,room);
end;

procedure log_quit(room:integer;dropped:boolean);

begin
	log_event(0,E_QUIT,0,0,myname,room);
	if dropped then
		log_event(0,E_DROPALL,0,0,myname,room);
end;




{ return the number of people you can see here }

function n_can_see: integer;
var
	sum: integer;
	i: integer;
	selfslot: integer;

begin
	if here.locnum = location then
		selfslot := myslot
	else
		selfslot := 0;

	sum := 0;
	for i := 1 to maxpeople do
		if ( i <> selfslot ) and
		   ( length(here.people[i].name) > 0 ) and
		   ( here.people[i].hiding = 0 ) then
			sum := sum + 1;
	n_can_see := sum;
	if debug then
		writeln('%n_can_see = ',sum:1);
end;



function next_can_see(var point: integer): string;
var
	found: boolean;
	selfslot: integer;

begin
	if here.locnum <> location then
		selfslot := 0
	else
		selfslot := myslot;
	found := false;
	while (not found) and (point <= maxpeople) do begin
		if (point <> selfslot) and
		   (length(here.people[point].name) > 0) and
		   (here.people[point].hiding = 0) then
			found := true
		else
			point := point + 1;
	end;

	if found then begin
		next_can_see := here.people[point].name;
		point := point + 1;
	end else begin
		next_can_see := myname;	{ error!  error! }
		writeln('%searching error in next_can_see; notify the Monster Manager');
	end;
end;


procedure niceprint(var len: integer; s: string);

begin
	if len + length(s) > 78 then begin
		len := 0;
		writeln;
	end else begin
		len := len + length(s);
	end;
	write(s);
end;


procedure people_header(where: shortstring);
var
	point: integer;
	tmp: string;
	i: integer;
	n: integer;
	len: integer;

begin
	point := 1;
	n := n_can_see;
	case n of
		0:;
		1: begin
			writeln(next_can_see(point),' is ',where);
		   end;
		2: begin
			writeln(next_can_see(point),' and ',next_can_see(point),
				' are ',where);
		   end;
		otherwise begin
			len := 0;
			for i := 1 to n - 1 do begin { at least 1 to 2 }
				tmp := next_can_see(point);
				if i <> n - 1 then
					tmp := tmp + ', ';
				niceprint(len,tmp);
			end;

			niceprint(len,' and ');
			niceprint(len,next_can_see(point));
			niceprint(len,' are ' + where);
			writeln;
		end;
	end;
end;


procedure desc_person(i: integer);
var
	pname: shortstring;

begin
	pname := here.people[i].name;

	if here.people[i].act <> 0 then begin
		write(pname,' is');
		writeln(desc_action(here.people[i].act,
			here.people[i].targ));
					{ describes what person last did }
	end;

	if here.people[i].health <> GOODHEALTH then
		desc_health(i);

	if here.people[i].wielding > 0 then
		writeln(pname,' is wielding ',obj_part(here.people[i].wielding),'.');

end;


procedure show_people;
var
	i: integer;

begin
	people_header('here.');
	for i := 1 to maxpeople do begin
		if (here.people[i].name <> '') and
		   (i <> myslot) and
		   (here.people[i].hiding = 0) then
				desc_person(i);
	end;
end;


procedure show_group;
var
	gloc1,gloc2: integer;
	gnam1,gnam2: shortstring;

begin
	gloc1 := here.grploc1;
	gloc2 := here.grploc2;
	gnam1 := here.grpnam1;
	gnam2 := here.grpnam2;

	if gloc1 <> 0 then begin
		gethere(gloc1);
		people_header(gnam1);
	end;
	if gloc2 <> 0 then begin
		gethere(gloc2);
		people_header(gnam2);
	end;
	gethere;
end;


procedure desc_obj(n: integer);

begin
	if n <> 0 then begin
		getobj(n);
		freeobj;
		if (obj.linedesc = DEFAULT_LINE) then begin
			writeln('On the ground here is ',obj_part(n,FALSE),'.');

				{ the FALSE means obj_part shouldn't do its
				  own getobj, cause we already did one }
		end else
			print_line(obj.linedesc);
	end;
end;


procedure show_objects;

var
	i: integer;

begin
	for i := 1 to maxobjs do begin
		if (here.objs[i] <> 0) and (here.objhide[i] = 0) then
			desc_obj(here.objs[i]);
	end;
end;


function lookup_detail(var n: integer;s:string): boolean;
var
	i,poss,maybe,num: integer;

begin
	n := 0;
	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to maxdetail do begin
		if s = here.detail[i] then
			num := i
		else if index(here.detail[i],s) = 1 then begin
			maybe := maybe + 1;
			poss := i;
		end;
	end;
	if num <> 0 then begin
		n := num;
		lookup_detail := true;
	end else if maybe = 1 then begin
		n := poss;
		lookup_detail := true;
	end else if maybe > 1 then begin
		lookup_detail := false;
	end else begin
		lookup_detail := false;
	end;
end;


function look_detail(s: string): boolean;
var
	n: integer;

begin
	if lookup_detail(n,s) then begin
		if here.detaildesc[n] = 0 then
			look_detail := false
		else begin
			print_desc(here.detaildesc[n]);
			log_event(myslot,E_LOOKDETAIL,0,0,here.detail[n]);
			look_detail := true;
		end;
	end else
		look_detail := false;
end;


function look_person(s: string): boolean;
var
	objnum,i,n: integer;
	first: boolean;

begin
	if parse_pers(n,s) then begin
		if n = myslot then begin
			log_event(myslot,E_LOOKSELF,n,0);
			writeln('You step outside of yourself for a moment to get an objective self-appraisal:');
			writeln;
		end else
			log_event(myslot,E_LOOKYOU,n,0);
		if here.people[n].self <> 0 then begin
			print_desc(here.people[n].self);
			writeln;
		end;

		desc_health(n);

			{ Do an inventory of person S }
		first := true;
		for i := 1 to maxhold do begin
			objnum := here.people[n].holding[i];
			if objnum <> 0 then begin
				if first then begin
					writeln(here.people[n].name,' is holding:');
					first := false;
				end;
				writeln('   ',obj_part(objnum));
			end;
		end;
		if first then
			writeln(here.people[n].name,' is empty handed.');

		look_person := true;
	end else
		look_person := false;
end;



procedure do_examine(s: string;var three: boolean;silent:boolean := false);
var
	n: integer;
	msg: string;

begin
	three := false;
	if parse_obj(n,s) then begin
		if obj_here(n) or obj_hold(n) then begin
			three := true;

			getobj(n);
			freeobj;
			msg := myname + ' is examining ' + obj_part(n) + '.';
			log_event(myslot,E_EXAMINE,0,0,msg);
			if obj.examine = 0 then
				writeln('You see nothing special about the ',
						objnam.idents[n],'.')
			else
				print_desc(obj.examine);
		end else
			if not(silent) then
				writeln('That object cannot be seen here.');
	end else
		if not(silent) then
			writeln('That object cannot be seen here.');
end;



procedure print_room;

begin
	case here.nameprint of
		0:;	{ don't print name }
		1: writeln('You''re in ',here.nicename);
		2: writeln('You''re at ',here.nicename);
	end;

	if not(brief) then begin
	case here.which of
		0: print_desc(here.primary);
		1: print_desc(here.secondary);
		2: begin
			print_desc(here.primary);
			print_desc(here.secondary);
		   end;
		3: begin
			print_desc(here.primary);
			if here.magicobj <> 0 then
				if obj_hold(here.magicobj) then
					print_desc(here.secondary);
		   end;
		4: begin
			if here.magicobj <> 0 then begin
				if obj_hold(here.magicobj) then
					print_desc(here.secondary)
				else
					print_desc(here.primary);
			end else
				print_desc(here.primary);
		   end;
	end;
	writeln;
	end;   { if not(brief) }
end;



procedure do_look(s: string := '');
var
	n: integer;
	one,two,three: boolean;

begin
	gethere;
	if s = '' then begin	{ do an ordinary top-level room look }

		if hiding then begin
			writeln('You can''t get a very good view of the details of the room from where');
			writeln('you are hiding.');
			noisehide(67);
		end else begin
			print_room;
			show_exits;
		end;		{ end of what you can't see when you're hiding }
		show_people;
		show_group;
		show_objects;
	end else begin		{ look at a detail in the room }
		one := look_detail(s);
		two := look_person(s);
		do_examine(s,three,TRUE);
		if not(one or two or three) then
			writeln('There isn''t anything here by that name to look at.');
	end;
end;


procedure init_exit(dir: integer);

begin
	with here.exits[dir] do begin
		exitdesc := DEFAULT_LINE;
		fail := DEFAULT_LINE;		{ default descriptions }
		success := 0;			{ until they customize }
		comeout := DEFAULT_LINE;
		goin := DEFAULT_LINE;
		closed := DEFAULT_LINE;

		objreq := 0;		{ not a door (yet) }
		hidden := 0;		{ not hidden }
		reqalias := false;	{ don't require alias (i.e. can use
					  direction of exit North, east, etc. }
		reqverb := false;
		autolook := true;
		alias := '';
	end;
end;



procedure remove_exit(dir: integer);
var
	targroom,targslot: integer;
	hereacc,targacc: boolean;

begin
		{ Leave residual accepts if player is not the owner of
		  the room that the exit he is deleting is in }

	getroom;
	targroom := here.exits[dir].toloc;
	targslot := here.exits[dir].slot;
	here.exits[dir].toloc := 0;
	init_exit(dir);

	if (here.owner = userid) or (privd) then
		hereacc := false
	else
		hereacc := true;

	if hereacc then
		here.exits[dir].kind := 5	{ put an "accept" in its place }
	else
		here.exits[dir].kind := 0;

	putroom;
	log_event(myslot,E_DETACH,dir,0,myname,location);

	getroom(targroom);
	here.exits[targslot].toloc := 0;

	if (here.owner = userid) or (privd) then
		targacc := false
	else
		targacc := true;

	if targacc then
		here.exits[targslot].kind := 5	{ put an "accept" in its place }
	else
		here.exits[targslot].kind := 0;

	putroom;

	if targroom <> location then
		log_event(0,E_DETACH,targslot,0,myname,targroom);
	writeln('Exit destroyed.');
end;


{
User procedure to unlink a room
}
procedure do_unlink(s: string);
var
	dir: integer;

begin
	gethere;
	if checkhide then begin
	if lookup_dir(dir,s) then begin
		if can_alter(dir) then begin
			if here.exits[dir].toloc = 0 then
				writeln('There is no exit there to unlink.')
			else
				remove_exit(dir);
		end else
			writeln('You are not allowed to remove that exit.');
	end else
		writeln('To remove an exit, type UNLINK <direction of exit>.');
	end;
end;



function desc_allowed: boolean;

begin
	if (here.owner = userid) or
	   (privd) then
		desc_allowed := true
	else begin
		writeln('Sorry, you are not allowed to alter the descriptions in this room.');
		desc_allowed := false;
	end;
end;



function slead(s: string):string;
var
	i: integer;
	going: boolean;

begin 
	if length(s) = 0 then
		slead := ''
	else begin
		i := 1;
		going := true;
		while going do begin
			if i > length(s) then
				going := false
			else if (s[i]=' ') or (s[i]=chr(9)) then
				i := i + 1
			else
				going := false;
		end;

		if i > length(s) then
			slead := ''
		else
			slead := substr(s,i,length(s)+1-i);
	end;
end;


function bite(var s: string): string;
var
	i: integer;

begin
	if length(s) = 0 then
		bite := ''
	else begin
		i := index(s,' ');
		if i = 0 then begin
			bite := s;
			s := '';
		end else begin
			bite := substr(s,1,i-1);
			s := slead(substr(s,i+1,length(s)-i));
		end;
	end;
end;

procedure edit_help;

begin
	writeln;
	writeln('A	Append text to end');
	writeln('C	Check text for correct length with parameter substitution (#)');
	writeln('D #	Delete line #');
	writeln('E	Exit & save changes');
	writeln('I #	Insert lines before line #');
	writeln('P	Print out description');
	writeln('Q	Quit: THROWS AWAY CHANGES');
	writeln('R #	Replace text of line #');
	writeln('Z	Zap all text');
	writeln('@	Throw away text & exit with the default description');
	writeln('?	This list');
	writeln;
end;

procedure edit_replace(n: integer);
var
	prompt: string;
	s: string;

begin
	if (n > heredsc.desclen) or (n < 1) then
		writeln('-- Bad line number')
	else begin
		writev(prompt,n:2,': ');
		grab_line(prompt,s);
		if s <> '**' then
			heredsc.lines[n] := s;
	end;
end;

procedure edit_insert(n: integer);
var
	i: integer;

begin
	if heredsc.desclen = descmax then
		writeln('You have already used all ',descmax:1,' lines of text.')
	else if (n < 1) or (n > heredsc.desclen) then begin
		writeln('Invalid line #; valid lines are between 1 and ',heredsc.desclen:1);
		writeln('Use A (add) to add text to the end of your description.');
	end else begin
		for i := heredsc.desclen+1 downto n + 1 do
			heredsc.lines[i] := heredsc.lines[i-1];
		heredsc.desclen := heredsc.desclen + 1;
		heredsc.lines[n] := '';
	end;
end;

procedure edit_doinsert(n: integer);
var
	s: string;
	prompt: string;

begin
	if heredsc.desclen = descmax then
		writeln('You have already used all ',descmax:1,' lines of text.')
	else if (n < 1) or (n > heredsc.desclen) then begin
		writeln('Invalid line #; valid lines are between 1 and ',heredsc.desclen:1);
		writeln('Use A (add) to add text to the end of your description.');
	end else repeat
		writev(prompt,n:1,': ');
		grab_line(prompt,s);
		if s <> '**' then begin
			edit_insert(n);		{ put the blank line in }
			heredsc.lines[n] := s;	{ copy this line onto it }
			n := n + 1;
		end;
	until (heredsc.desclen = descmax) or (s = '**');
end;

procedure edit_show;
var
	i: integer;

begin
	writeln;
	if heredsc.desclen = 0 then
		writeln('[no text]')
	else begin
		i := 1;
		while i <= heredsc.desclen do begin
			writeln(i:2,': ',heredsc.lines[i]);
			i := i + 1;
		end;
	end;
end;

procedure edit_append;
var
	prompt,s: string;
	stilladding: boolean;

begin
	if heredsc.desclen = descmax then
		writeln('You have already used all ',descmax:1,' lines of text.')
	else begin
		stilladding := true;
		writeln('Enter text.  Terminate with ** at the beginning of a line.');
		writeln('You have ',descmax:1,' lines maximum.');
		writeln;
		while (heredsc.desclen < descmax) and (stilladding) do begin
			writev(prompt,heredsc.desclen+1:2,': ');
			grab_line(prompt,s);
			if s = '**' then
				stilladding := false
			else begin
				heredsc.desclen := heredsc.desclen + 1;
				heredsc.lines[heredsc.desclen] := s;
			end;
		end;
	end;
end;

procedure edit_delete(n: integer);
var
	i: integer;

begin
	if heredsc.desclen = 0 then
		writeln('-- No lines to delete')
	else if (n > heredsc.desclen) or (n < 1) then
		writeln('-- Bad line number')
	else if (n = 1) and (heredsc.desclen = 1) then
		heredsc.desclen := 0
	else begin
		for i := n to heredsc.desclen-1 do
			heredsc.lines[i] := heredsc.lines[i + 1];
		heredsc.desclen := heredsc.desclen - 1;
	end;
end;


procedure check_subst;
var
	i: integer;

begin
	if heredsc.desclen > 0 then begin
		for i := 1 to heredsc.desclen do
			if (index(heredsc.lines[i],'#') > 0) and
			   (length(heredsc.lines[i]) > 59) then
				writeln('Warning: line ',i:1,' is too long for correct parameter substitution.');
	end;
end;


function edit_desc(var dsc: integer):boolean;
var
	cmd: char;
	s: string;
	done: boolean;
	n: integer;

begin
	if dsc = DEFAULT_LINE then begin
		heredsc.desclen := 0;
	end else if dsc > 0 then begin
		getblock(dsc);
		freeblock;
		heredsc := block;
	end else if dsc < 0 then begin
		n := (- dsc);
		getline(n);
		freeline;
		heredsc.lines[1] := oneliner.theline;
		heredsc.desclen := 1;
	end else begin
		heredsc.desclen := 0;
	end;

	edit_desc := true;
	done := false;
	if heredsc.desclen = 0 then
		edit_append;
	repeat
		writeln;
		repeat
			grab_line('* ',s);
			s := slead(s);
		until length(s) > 0;
		s := lowcase(s);
		cmd := s[1];

		if length(s)>1 then begin
			n := number(slead(substr(s,2,length(s)-1)))
		end else
			n := 0;

		case cmd of
			'h','?': edit_help;
			'a': edit_append;
			'z': heredsc.desclen := 0;
			'c': check_subst;
			'p','l','t': edit_show;
			'd': edit_delete(n);
			'e': begin
				check_subst;
				if debug then
					writeln('edit_desc: dsc is ',dsc:1);


{ what I do here may require some explanation:

	dsc is a pointer to some text structure:
		dsc = 0 :  no text
		dsc > 0 :  dsc refers to a description block (descmax lines)
		dsc < 0 :  dsc refers to a description "one liner".  abs(dsc)
			   is the actual pointer

	If there are no lines of text to be written out (heredsc.desclen = 0)
	then we deallocate whatever dsc is when edit_desc was invoked, if
	it was pointing to something;

	if there is one line of text to be written out, allocate a one liner
	record, assign the string to it, and return dsc as negative;

	if there is mmore than one line of text, allocate a description block,
	store the lines in it, and return dsc as positive.

	In all cases if there was already a record allocated to dsc then
	use it and don't reallocate a new record.
}

{ kill the default }		if (heredsc.desclen > 0) and
{ if we're gonna put real }		(dsc = DEFAULT_LINE) then
{ texty in here }				dsc := 0;

{ no lines, delete existing }	if heredsc.desclen = 0 then
{ desc, if any }			delete_block(dsc)
				else if heredsc.desclen = 1 then begin
					if (dsc = 0) then begin
						if alloc_line(dsc) then;
						dsc := (- dsc);
					end else if dsc > 0 then begin
						delete_block(dsc);
						if alloc_line(dsc) then;
						dsc := (- dsc);
					end;

					if dsc < 0 then begin
						getline( abs(dsc) );
						oneliner.theline := heredsc.lines[1];
						putline;
					end;
{ more than 1 lines }		end else begin
					if dsc = 0 then begin
						if alloc_block(dsc) then;
					end else if dsc < 0 then begin
						delete_line(dsc);
						if alloc_block(dsc) then;
					end;

					if dsc > 0 then begin
						getblock(dsc);
						block := heredsc;
{ This is a fudge }				block.descrinum := dsc;
						putblock;
					end;
				end;
				done := true;
			     end;
			'r': edit_replace(n);
			'@': begin
				delete_block(dsc);
				dsc := DEFAULT_LINE;
				done := true;
			     end;
			'i': edit_doinsert(n);
			'q': begin
				grab_line('Throw away changes, are you sure? ',s);
				s := lowcase(s);
				if (s = 'y') or (s = 'yes') then begin
					done := true;
					edit_desc := false; { signal caller not to save }
				end;
			     end;
			otherwise writeln('-- Invalid command, type ? for a list.');
		end;
	until done;
end;




function alloc_detail(var n: integer;s: string): boolean;
var
	found: boolean;

begin
	n := 1;
	found := false;
	while (n <= maxdetail) and (not found) do begin
		if here.detaildesc[n] = 0 then
			found := true
		else
			n := n + 1;
	end;
	alloc_detail := found;
	if not(found) then
		n := 0
	else begin
		getroom;
		here.detail[n] := lowcase(s);
		putroom;
	end;
end;


{
User describe procedure.  If no s then describe the room

Known problem: if two people edit the description to the same room one of their
	description blocks could be lost.
This is unlikely to happen unless the Monster Manager tries to edit a
description while the room's owner is also editing it.
}
procedure do_describe(s: string);
var
	i: integer;
	newdsc: integer;

begin
	gethere;
	if checkhide then begin
	if s = '' then begin { describe this room }
		if desc_allowed then begin
			log_action(desc,0);
			writeln('[ Editing the primary room description ]');
			newdsc := here.primary;
			if edit_desc(newdsc) then begin
				getroom;
				here.primary := newdsc;
				putroom;
			end;
			log_event(myslot,E_EDITDONE,0,0);
		end;
	end else begin{ describe a detail of this room }
		if length(s) > veryshortlen then
			writeln('Your detail keyword can only be ',veryshortlen:1,' characters.')
		else if desc_allowed then begin
			if not(lookup_detail(i,s)) then
			if not(alloc_detail(i,s)) then begin
				writeln('You have used all ',maxdetail:1,' details.');
				writeln('To delete a detail, DESCRIBE <the detail> and delete all the text.');
			end;
			if i <> 0 then begin
				log_action(e_detail,0);
				writeln('[ Editing detail "',here.detail[i],'" of this room ]');
				newdsc := here.detaildesc[i];
				if edit_desc(newdsc) then begin
					getroom;
					here.detaildesc[i] := newdsc;
					putroom;
				end;
				log_event(myslot,E_DONEDET,0,0);
			end;
		end;
	end;
{	clear_command;	}
	end;
end;




procedure del_room(n: integer);
var
	i: integer;

begin
	getnam;
	nam.idents[n] := '';	{ blank out name }
	putnam;

	getown;
	own.idents[n] := '';	{ blank out owner }
	putown;

	getroom(n);
	for i := 1 to maxexit do begin
		with here.exits[i] do begin
			delete_line(exitdesc);
			delete_line(fail);
			delete_line(success);
			delete_line(comeout);
			delete_line(goin);
		end;
	end;
	delete_block(here.primary);
	delete_block(here.secondary);
	putroom;
	delete_room(n);	{ return room to free list }
end;



procedure createroom(s: string);	{ create a room with name s }
var
	roomno: integer;
	dummy: integer;
	i:integer;
	rand_accept: integer;

begin
	if length(s) = 0 then begin
		writeln('Please specify the name of the room you wish to create as a parameter to FORM.');
	end else if length(s) > shortlen then begin
		writeln('Please limit your room name to a maximum of ',shortlen:1,' characters.');
	end else if exact_room(dummy,s) then begin
		writeln('That room name has already been used.  Please give a unique room name.');
	end else if alloc_room(roomno) then begin
		log_action(form,0);

		getnam;
		nam.idents[roomno] := lowcase(s);	{ assign room name }
		putnam;					{ case insensitivity }

		getown;
		own.idents[roomno] := userid;	{ assign room owner }
		putown;

		getroom(roomno);

		here.primary := 0;
		here.secondary := 0;
		here.which := 0;	{ print primary desc only by default }
		here.magicobj := 0;

		here.owner := userid;	{ owner and name are stored here too }
		here.nicename := s;
		here.nameprint := 1;	{ You're in ... }
		here.objdrop := 0;	{ objects dropped stay here }
		here.objdesc := 0;	{ nothing printed when they drop }
		here.magicobj := 0;	{ no magic object default }
		here.trapto := 0;	{ no trapdoor }
		here.trapchance := 0;	{ no chance }
		here.rndmsg := DEFAULT_LINE;	{ bland noises message }
		here.pile := 0;
		here.grploc1 := 0;
		here.grploc2 := 0;
		here.grpnam1 := '';
		here.grpnam2 := '';

		here.effects := 0;
		here.parm := 0;

		here.xmsg2 := 0;
		here.exp2 := 0;
		here.exp3 := 0;
		here.exp4 := 0;
		here.exitfail := DEFAULT_LINE;
		here.ofail := DEFAULT_LINE;

		for i := 1 to maxpeople do
			here.people[i].kind := 0;

		for i := 1 to maxpeople do
			here.people[i].name := '';

		for i := 1 to maxobjs do
			here.objs[i] := 0;

		for i := 1 to maxdetail do
			here.detail[i] := '';
		for i := 1 to maxdetail do
			here.detaildesc[i] := 0;

		for i := 1 to maxobjs do
			here.objhide[i] := 0;

		for i := 1 to maxexit do
			with here.exits[i] do begin
				toloc := 0;
				kind := 0;
				slot := 0;
				exitdesc := DEFAULT_LINE;
				fail := DEFAULT_LINE;
				success := 0;	{ no success desc by default }
				goin := DEFAULT_LINE;
				comeout := DEFAULT_LINE;
				closed := DEFAULT_LINE;

				objreq := 0;
				hidden := 0;
				alias := '';

				reqverb := false;
				reqalias := false;
				autolook := true;
			end;
		
{		here.exits := zero;	}

				{ random accept for this room }
		rand_accept := 1 + (rnd100 mod 6);
		here.exits[rand_accept].kind := 5;

		putroom;
	end;
end;



procedure show_help;
var
	i: integer;
	s: string;

begin
	writeln;
	writeln('Accept/Refuse #  Allow others to Link an exit here at direction # | Undo Accept');
	writeln('Brief            Toggle printing of room descriptions');
	writeln('Customize [#]    Customize this room | Customize exit # | Customize object #');
	writeln('Describe [#]     Describe this room | Describe a feature (#) in detail');
	writeln('Destroy #        Destroy an instance of object # (you must be holding it)');
	writeln('Duplicate #      Make a duplicate of an already-created object.');
	writeln('Form/Zap #       Form a new room with name # | Destroy room named #');
	writeln('Get/Drop #       Get/Drop an object');
	writeln('#,Go #           Go towards # (Some: N/North S/South E/East W/West U/Up D/Down)');
	writeln('Health           Show how healthy you are');
	writeln('Hide/Reveal [#]  Hide/Reveal yoursef | Hide object (#)');
	writeln('I,Inventory      See what you or someone else is carrying');
	writeln('Link/Unlink #    Link/Unlink this room to/from another via exit at direction #');
	writeln('Look,L [#]       Look here | Look at something or someone (#) closely');
	writeln('Make #           Make a new object named #');
	writeln('Name #           Set your game name to #');
	writeln('Players          List people who have played Monster');
	writeln('Punch #          Punch person #');
	writeln('Quit             Leave the game');
	writeln('Relink           Move an exit');
	writeln;
	grab_line('-more-',s);
	writeln;
	writeln('Rooms            Show information about rooms you have made');
	writeln('Say, '' (quote)   Say line of text following command to others in the room');
	writeln('Search           Look around the room for anything hidden');
	writeln('Self #           Edit a description of yourself | View #''s self-description');
	writeln('Show #           Show option # (type SHOW ? for a list)');
	writeln('Unmake #         Remove the form definition of object #');
	writeln('Use #            Use object #');
	writeln('Wear #           Wear the object #');
	writeln('Wield #          Wield the weapon #;  you must be holding it first');
	writeln('Whisper #        Whisper something (prompted for) to person #');
	writeln('Who              List of people playing Monster now');
	writeln('Whois #          What is a player''s username');
	writeln('?,Help           This list');
	writeln('. (period)       Repeat last command');
	writeln;
end;


function lookup_cmd(s: string):integer;
var
	i,		{ index for loop }
	poss,		{ a possible match -- only for partial matches }
	maybe,		{ number of possible matches we have: > 2 is ambig. }
	num		{ the definite match }
		: integer;


begin
	s := lowcase(s);
	i := 1;
	maybe := 0;
	num := 0;
	for i := 1 to numcmds do begin
		if s = cmds[i] then
			num := i
		else if index(cmds[i],s) = 1 then begin
			maybe := maybe + 1;
			poss := i;
		end;
	end;
	if num <> 0 then begin
		lookup_cmd := num;
	end else if maybe = 1 then begin
		lookup_cmd := poss;
	end else if maybe > 1 then
		lookup_cmd := error	{ "Ambiguous" }
	else
		lookup_cmd := error;	{ "Command not found " }
end;


procedure addrooms(n: integer);
var
	i: integer;

begin
	getindex(I_ROOM);
	for i := indx.top+1 to indx.top+n do begin
		locate(roomfile,i);
		roomfile^.valid := i;
		roomfile^.locnum := i;
		roomfile^.primary := 0;
		roomfile^.secondary := 0;
		roomfile^.which := 0;
		put(roomfile);
	end;
	indx.top := indx.top + n;
	putindex;
end;



procedure addints(n: integer);
var
	i: integer;

begin
	getindex(I_INT);
	for i := indx.top+1 to indx.top+n do begin
		locate(intfile,i);
		intfile^.intnum := i;
		put(intfile);
	end;
	indx.top := indx.top + n;
	putindex;
end;



procedure addlines(n: integer);
var
	i: integer;

begin
	getindex(I_LINE);
	for i := indx.top+1 to indx.top+n do begin
		locate(linefile,i);
		linefile^.linenum := i;
		put(linefile);
	end;
	indx.top := indx.top + n;
	putindex;
end;

procedure addblocks(n: integer);
var
	i: integer;

begin
	getindex(I_BLOCK);
	for i := indx.top+1 to indx.top+n do begin
		locate(descfile,i);
		descfile^.descrinum := i;
		put(descfile);
	end;
	indx.top := indx.top + n;
	putindex;
end;


procedure addobjects(n: integer);
var
	i: integer;

begin
	getindex(I_OBJECT);
	for i := indx.top+1 to indx.top+n do begin
		locate(objfile,i);
		objfile^.objnum := i;
		put(objfile);
	end;
	indx.top := indx.top + n;
	putindex;
end;


procedure dist_list;
var
	i,j: integer;
	f: text;
	where_they_are: intrec;

begin
	writeln('Writing distribution list . . .');
	open(f,'monsters.dis',history := new);
	rewrite(f);

	getindex(I_PLAYER);	{ Rec of valid player log records  }
	freeindex;		{ False if a valid player log }

	getuser;		{ Corresponding userids of players }
	freeuser;

	getpers;		{ Personal names of players }
	freepers;

	getdate;		{ date of last play }
	freedate;

	if privd then begin
		getint(N_LOCATION);
		freeint;
		where_they_are := anint;

		getnam;
		freenam;
	end;

	for i := 1 to maxplayers do begin
		if not(indx.free[i]) then begin
			write(f,user.idents[i]);
			for j := length(user.idents[i]) to 15 do
				write(f,' ');
			write(f,'! ',pers.idents[i]);
			for j := length(pers.idents[i]) to 21 do
				write(f,' ');

			write(f,adate.idents[i]);
				if length(adate.idents[i]) < 19 then
					for j := length(adate.idents[i]) to 18 do
						write(f,' ');
			if anint.int[i] <> 0 then
				write(f,' * ')
			else
				write(f,'   ');

			if privd then begin
				write(f,nam.idents[ where_they_are.int[i] ]);
			end;
			writeln(f);

		end;
	end;
	writeln('Done.');
end;


procedure system_view;
var
	used,free,total: integer;

begin
	writeln;
	getindex(I_BLOCK);
	freeindex;
	used := indx.inuse;
	total := indx.top;
	free := total - used;

	writeln('               used   free   total');
	writeln('Block file   ',used:5,'  ',free:5,'   ',total:5);

	getindex(I_LINE);
	freeindex;
	used := indx.inuse;
	total := indx.top;
	free := total - used;
	writeln('Line file    ',used:5,'  ',free:5,'   ',total:5);

	getindex(I_ROOM);
	freeindex;
	used := indx.inuse;
	total := indx.top;
	free := total - used;
	writeln('Room file    ',used:5,'  ',free:5,'   ',total:5);

	getindex(I_OBJECT);
	freeindex;
	used := indx.inuse;
	total := indx.top;
	free := total - used;
	writeln('Object file  ',used:5,'  ',free:5,'   ',total:5);

	getindex(I_INT);
	freeindex;
	used := indx.inuse;
	total := indx.top;
	free := total - used;
	writeln('Integer file ',used:5,'  ',free:5,'   ',total:5);

	writeln;
end;


{ remove a user from the log records (does not handle ownership) }

procedure kill_user(s:string);
var
	n: integer;

begin
	if length(s) = 0 then
		writeln('No user specified')
	else begin
		if lookup_user(n,s) then begin
			getindex(I_ASLEEP);
			freeindex;
			if indx.free[n] then begin
				delete_log(n);
				writeln('Player deleted.');
			end else
				writeln('That person is playing now.');
		end else
			writeln('No such userid found in log information.');
	end;
end;


{ disown everything a player owns }

procedure disown_user(s:string);
var
	n: integer;
	i: integer;
	tmp: string;
	theuser: string;

begin
	if length(s) > 0 then begin
		if debug then
			writeln('calling lookup_user with ',s);
		if not lookup_user(n,s) then
			writeln('User not in log info, attempting to disown anyway.');

		theuser := user.idents[n];

		{ first disown all their rooms }

		getown;
		freeown;
		for i := 1 to maxroom do
			if own.idents[i] = theuser then begin
				getown;
				own.idents[i] := '*';
				putown;

				getroom(i);
				tmp := here.nicename;
				here.owner := '*';
				putroom;

				writeln('Disowned room ',tmp);
			end;
		writeln;

		getobjown;
		freeobjown;
		getobjnam;
		freeobjnam;
		for i := 1 to maxroom do
			if objown.idents[i] = theuser then begin
				getobjown;
				objown.idents[i] := '*';
				putobjown;

				tmp := objnam.idents[i];
				writeln('Disowned object ',tmp);
			end;
	end else
		writeln('No user specified.');
end;

procedure move_asleep;
var
	pname,rname:string;	{ player & room names }
	newroom,n: integer;	{ room number & player slot number }

begin
	grab_line('Player name? ',pname);
	grab_line('Room name?   ',rname);
	if lookup_user(n,pname) then begin
		if lookup_room(newroom,rname) then begin
			getindex(I_ASLEEP);
			freeindex;
			if indx.free[n] then begin
				getint(N_LOCATION);
				anint.int[n] := newroom;
				putint;
				writeln('Player moved.');
			end else
				writeln('That player is not asleep.');
		end else
			writeln('No such room found.');
	end else
		writeln('User not found.');
end;


procedure system_help;

begin
	writeln;
	writeln('B	Add description blocks');
	writeln('D	Disown <user>');
	writeln('E	Exit (same as quit)');
	writeln('I	Add Integer records');
	writeln('K	Kill <user>');
	writeln('L	Add one liner records');
	writeln('M	Move a player who is asleep (not playing now)');
	writeln('O	Add object records');
	writeln('P	Write a distribution list of players');
	writeln('Q	Quit (same as exit)');
	writeln('R	Add rooms');
	writeln('V	View current sizes/usage');
	writeln('?	This list');
	writeln;
end;


{ *************** FIX_STUFF ******************** }

procedure fix_stuff;

begin
end;


procedure do_system(s: string);
var
	prompt: string;
	done: boolean;
	cmd: char;
	n: integer;
	p: string;

begin
	if privd then begin
		log_action(c_system,0);
		prompt := 'System> ';
		done := false;
		repeat
			repeat
				grab_line(prompt,s);
				s := slead(s);
			until length(s) > 0;
			s := lowcase(s);
			cmd := s[1];

			n := 0;
			p := '';
			if length(s) > 1 then begin
				p := slead( substr(s,2,length(s)-1) );
				n := number(p)
			end;
			if debug then begin
				writeln('p = ',p);
			end;

			case cmd of
				'h','?': system_help;
				'1': fix_stuff;
{remove a user}			'k': kill_user(p);
{disown}			'd': disown_user(p);
{dist list of players}		'p': dist_list;
{move where user will wakeup}	'm': move_asleep;
{add rooms}			'r': begin
					if n > 0 then begin
						addrooms(n);
					end else
						writeln('To add rooms, say R <# to add>');
				     end;
{add ints}			'i': begin
					if n > 0 then begin
						addints(n);
					end else
						writeln('To add integers, say I <# to add>');
				     end;
{add description blocks}	'b': begin
					if n > 0 then begin
						addblocks(n);
					end else
						writeln('To add description blocks, say B <# to add>');
				     end;
{add objects}			'o': begin
					if n > 0 then begin
						addobjects(n);
					end else
						writeln('To add object records, say O <# to add>');
				     end;
{add one-liners}		'l': begin
					if n > 0 then begin
						addlines(n);
					end else
						writeln('To add one liner records, say L <# to add>');
				     end;
{view current stats}		'v': begin
					system_view;
				     end;
{quit}				'q','e': done := true;
			otherwise writeln('-- bad command, type ? for a list.');
			end;
		until done;
		log_event(myslot,E_SYSDONE,0,0);
	end else
		writeln('Only the Monster Manger may enter system maintenance mode.');
end;


procedure do_version(s: string);

begin
	writeln('Monster, a multiplayer adventure game where the players create the world');
	writeln('and make the rules.');
	writeln;
	writeln('Written by Rich Skrenta at Northwestern University, 1988.');
end;


procedure rebuild_system;
var
	i,j: integer;

begin
	writeln('Creating index file 1-6');
	for i := 1 to 7 do begin
			{ 1 is blocklist
			  2 is linelist
			  3 is roomlist
			  4 is playeralloc
			  5 is player awake (playing game)
			  6 are objects
			  7 is intfile }

		locate(indexfile,i);
		for j := 1 to maxindex do
			indexfile^.free[j] := true;
		indexfile^.indexnum := i;
		indexfile^.top := 0; { none of each to start }
		indexfile^.inuse := 0;
		put(indexfile);
	end;


	writeln('Initializing roomfile with 10 rooms');
	addrooms(10);

	writeln('Initializing block file with 10 description blocks');
	addblocks(10);

	writeln('Initializing line file with 10 lines');
	addlines(10);

	writeln('Initializing object file with 10 objects');
	addobjects(10);


	writeln('Initializing namfile 1-8');
	for j := 1 to 8 do begin
		locate(namfile,j);
		namfile^.validate := j;
		namfile^.loctop := 0;
		for i := 1 to maxroom do begin
			namfile^.idents[i] := '';
		end;
		put(namfile);
	end;

	writeln('Initializing eventfile');
	for i := 1 to numevnts + 1 do begin
		locate(eventfile,i);
		eventfile^.validat := i;
		eventfile^.point := 1;
		put(eventfile);
	end;

	writeln('Initializing intfile');
	for i := 1 to 6 do begin
		locate(intfile,i);
		intfile^.intnum := i;
		put(intfile);
	end;

	getindex(I_INT);
	for i := 1 to 6 do
		indx.free[i] := false;
	indx.top := 6;
	indx.inuse := 6;
	putindex;

	{ Player log records should have all their slots initially,
	  they don't have to be allocated because they use namrec
	  and intfile for their storage; they don't have their own
	  file to allocate
	}
	getindex(I_PLAYER);
	indx.top := maxplayers;
	putindex;
	getindex(I_ASLEEP);
	indx.top := maxplayers;
	putindex;

	writeln('Creating the Great Hall');
	createroom('Great Hall');
	getroom(1);
	here.owner := '';
	putroom;
	getown;
	own.idents[1] := '';
	putown;

	writeln('Creating the Void');
	createroom('Void');			{ loc 2 }
	writeln('Creating the Pit of Fire');
	createroom('Pit of Fire');		{ loc 3 }
			{ note that these are NOT public locations }


	writeln('Use the SYSTEM command to view and add capacity to the database');
	writeln;
end;


procedure special(s: string);

begin
	if (s = 'rebuild') and (privd) then begin
		if REBUILD_OK then begin
			writeln('Do you really want to destroy the entire universe?');
			readln(s);
			if length(s) > 0 then
				if substr(lowcase(s),1,1) = 'y' then
					rebuild_system;
		end else
			writeln('REBUILD is disabled; you must recompile.');
	end else if s = 'version' then begin
		{ Don't take this out please... }
	  	writeln('Monster, written by Rich Skrenta at Northwestern University, 1988.');
	end else if s = 'quit' then
		done := true;
end;


{ put an object in this location
  if returns false, there were no more free object slots here:
  in other words, the room is too cluttered, and cannot hold any
  more objects
}
function place_obj(n: integer;silent:boolean := false): boolean;
var
	found: boolean;
	i: integer;

begin
	if here.objdrop = 0 then
		getroom
	else
		getroom(here.objdrop);
	i := 1;
	found := false;
	while (i <= maxobjs) and (not found) do begin
		if here.objs[i] = 0 then
			found := true
		else
			i := i + 1;
	end;
	place_obj := found;
	if found then begin
		here.objs[i] := n;
		here.objhide[i] := 0;
		putroom;

		gethere;


		{ if it bounced somewhere else then tell them }

		if (here.objdrop <> 0) and (here.objdest <> 0) then
			log_event(0,E_BOUNCEDIN,here.objdest,n,'',here.objdrop);


		if not(silent) then begin
			if here.objdesc <> 0 then
				print_subs(here.objdesc,obj_part(n))
			else
				writeln('Dropped.');
		end;
	end else
		freeroom;
end;


{ remove an object from this room }
function take_obj(objnum,slot: integer): boolean;

begin
	getroom;
	if here.objs[slot] = objnum then begin
		here.objs[slot] := 0;
		here.objhide[slot] := 0;
		take_obj := true;
	end else
		take_obj := false;
	putroom;
end;


function can_hold: boolean;

begin
	if find_numhold < maxhold then
		can_hold := true
	else
		can_hold := false;
end;


function can_drop: boolean;

begin
	if find_numobjs < maxobjs then
		can_drop := true
	else
		can_drop := false;
end;


function find_hold(objnum: integer;slot:integer := 0): integer;
var
	i: integer;

begin
	if slot = 0 then
		slot := myslot;
	i := 1;
	find_hold := 0;
	while i <= maxhold do begin
		if here.people[slot].holding[i] = objnum then
			find_hold := i;
		i := i + 1;
	end;
end;



{ put object number n into the player's inventory; returns false if
  he's holding too many things to carry another }

function hold_obj(n: integer): boolean;
var
	found: boolean;
	i: integer;

begin
	getroom;
	i := 1;
	found := false;
	while (i <= maxhold) and (not found) do begin
		if here.people[myslot].holding[i] = 0 then
			found := true
		else
			i := i + 1;
	end;
	hold_obj := found;
	if found then begin
		here.people[myslot].holding[i] := n;
		putroom;

		getobj(n);
		freeobj;
		hold_kind[i] := obj.kind;
	end else
		freeroom;
end;



{ remove an object (hold) from the player record, given the slot that
  the object is being held in }

procedure drop_obj(slot: integer;pslot: integer := 0);

begin
	if pslot = 0 then
		pslot := myslot;
	getroom;
	here.people[pslot].holding[slot] := 0;
	putroom;

	hold_kind[slot] := 0;
end;



{ maybe drop something I'm holding if I'm hit }

procedure maybe_drop;
var
	i: integer;
	objnum: integer;
	s: string;

begin
	i := 1 + (rnd100 mod maxhold);
	objnum := here.people[myslot].holding[i];

	if (objnum <> 0) and (mywield <> objnum) and (mywear <> objnum) then begin
		{ drop something }

		drop_obj(i);
		if place_obj(objnum,TRUE) then begin
			getobjnam;
			freeobjnam;
			writeln('The ',objnam.idents[objnum],' has slipped out of your hands.');

			
		s := objnam.idents[objnum];
			log_event(myslot,E_SLIPPED,0,0,s);
		end else
			writeln('%error in maybe_drop; unsuccessful place_obj; notify Monster Manager');

	end;
end;



{ return TRUE if the player is allowed to program the object n
  if checkpub is true then obj_owner will return true if the object in
  question is public }

function obj_owner(n: integer;checkpub: boolean := FALSE):boolean;

begin
	getobjown;
	freeobjown;
	if (objown.idents[n] = userid) or (privd) then begin
		obj_owner := true;
	end else if (objown.idents[n] = '') and (checkpub) then begin
		obj_owner := true;
	end else begin
		obj_owner := false;
	end;
end;


procedure do_duplicate(s: string);
var
	objnum: integer;

begin
   if length(s) > 0 then begin
	if not is_owner(location,TRUE) then begin
			{ only let them make things if they're on their home turf }
		writeln('You may only create objects when you are in one of your own rooms.');
	end else begin
		if lookup_obj(objnum,s) then begin
			if obj_owner(objnum,TRUE) then begin
				if not(place_obj(objnum,TRUE)) then
					{ put the new object here }
					writeln('There isn''t enough room here to make that.')
				else begin
{ keep track of how many there }	getobj(objnum);
{ are in existence }			obj.numexist := obj.numexist + 1;
					putobj;

					log_event(myslot,E_MADEOBJ,0,0,
						myname + ' has created an object here.');
					writeln('Object created.');
				end;
			end else
				writeln('Power to create that object belongs to someone else.');
		end else
			writeln('There is no object by that name.');
	end;
   end else
		writeln('To duplicate an object, type DUPLICATE <object name>.');
end;


{ make an object }
procedure do_makeobj(s: string);
var
	objnum: integer;

begin
	gethere;
	if checkhide then begin
	if not is_owner(location,TRUE) then begin
		writeln('You may only create objects when you are in one of your own rooms.');
	end else if s <> '' then begin
		if length(s) > shortlen then
			writeln('Please limit your object names to ',shortlen:1,' characters.')
		else if exact_obj(objnum,s) then begin	{ object already exits }
			writeln('That object already exits.  If you would like to make another copy of it,');
			writeln('use the DUPLICATE command.');
		end else begin
			if debug then
				writeln('%beggining to create object');
			if find_numobjs < maxobjs then begin
				if alloc_obj(objnum) then begin
					if debug then
						writeln('%alloc_obj successful');
					getobjnam;
					objnam.idents[objnum] := lowcase(s);
					putobjnam;
					if debug then
						writeln('%getobjnam completed');
					getobjown;
					objown.idents[objnum] := userid;
					putobjown;
					if debug then
						writeln('%getobjown completed');

					getobj(objnum);
						obj.onum := objnum;
						obj.oname := s;	{ name of object }
						obj.kind := 0; { bland object }
						obj.linedesc := DEFAULT_LINE;
						obj.actindx := 0;
						obj.examine := 0;
						obj.numexist := 1;
						obj.home := 0;
						obj.homedesc := 0;

						obj.sticky := false;
						obj.getobjreq := 0;
						obj.getfail := 0;
						obj.getsuccess := DEFAULT_LINE;

						obj.useobjreq := 0;
						obj.uselocreq := 0;
						obj.usefail := DEFAULT_LINE;
						obj.usesuccess := DEFAULT_LINE;

						obj.usealias := '';
						obj.reqalias := false;
						obj.reqverb := false;

			if s[1] in ['a','A','e','E','i','I','o','O','u','U'] then
						obj.particle := 2  { an }
			else
						obj.particle := 1; { a }

						obj.d1 := 0;
						obj.d2 := 0;
						obj.exp3 := 0;
						obj.exp4 := 0;
						obj.exp5 := DEFAULT_LINE;
						obj.exp6 := DEFAULT_LINE;
					putobj;


					if debug then
						writeln('putobj completed');
				end;
					{ else: alloc_obj prints errors by itself }
				if not(place_obj(objnum,TRUE)) then
					{ put the new object here }
					writeln('%error in makeobj - could not place object; notify the Monster Manager.')
				else begin
					log_event(myslot,E_MADEOBJ,0,0,
						myname + ' has created an object here.');
					writeln('Object created.');
				end;

			end else
				writeln('This place is too crowded to create any more objects.  Try somewhere else.');
		end;
	end else
		writeln('To create an object, type MAKE <object name>.');
	end;
end;

{ remove the type block for an object; all instances of the object must
  be destroyed first }

procedure do_unmake(s: string);
var
	n: integer;
	tmp: string;

begin
	if not(is_owner(location,TRUE)) then
		writeln('You must be in one of your own rooms to UNMAKE an object.')
	else if lookup_obj(n,s) then begin
		tmp := obj_part(n);
			{ this will do a getobj(n) for us }

		if obj.numexist = 0 then begin
			delete_obj(n);

			log_event(myslot,E_UNMAKE,0,0,tmp);
			writeln('Object removed.');
		end else
			writeln('You must DESTROY all instances of the object first.');
	end else
		writeln('There is no object here by that name.');
end;


{ destroy a copy of an object }

procedure do_destroy(s: string);
var
	slot,n: integer;

begin
	if length(s) = 0 then	
		writeln('To destroy an object you own, type DESTROY <object>.')
	else if not is_owner(location,TRUE) then
		writeln('You must be in one of your own rooms to destroy an object.')
	else if parse_obj(n,s) then begin
		getobjown;
		freeobjown;
		if (objown.idents[n] <> userid) and (objown.idents[n] <> '') and
		   (not privd) then
			writeln('You must be the owner of an object to destroy it.')
		else if obj_hold(n) then begin
			slot := find_hold(n);
			drop_obj(slot);

			log_event(myslot,E_DESTROY,0,0,
				myname + ' has destroyed ' + obj_part(n) + '.');
			writeln('Object destroyed.');

			getobj(n);
			obj.numexist := obj.numexist - 1;
			putobj;
		end else if obj_here(n) then begin
			slot := find_obj(n);
			if not take_obj(n,slot) then
				writeln('Someone picked it up before you could destroy it.')
			else begin
				log_event(myslot,E_DESTROY,0,0,
					myname + ' has destroyed ' + obj_part(n,FALSE) + '.');
				writeln('Object destroyed.');

				getobj(n);
				obj.numexist := obj.numexist - 1;
				putobj;
			end;
		end else
			writeln('Such a thing is not here.');
	end else
		writeln('No such thing can be seen here.');
end;


function links_possible: boolean;
var
	i: integer;

begin
	gethere;
	links_possible := false;
	if is_owner(location,TRUE) then
		links_possible := true
	else begin
		for i := 1 to maxexit do
			if (here.exits[i].toloc = 0) and (here.exits[i].kind = 5) then
				links_possible := true;
	end;
end;



{ make a room }
procedure do_form(s: string);

begin
	gethere;
	if checkhide then begin
		if links_possible then begin
			if s = '' then begin
				grab_line('Room name: ',s);
			end;
			s := slead(s);

			createroom(s);
		end else begin
			writeln('You may not create any new exits here.  Go to a place where you can create');
			writeln('an exit before FORMing a new room.');
		end;
	end;
end;


procedure xpoof; { loc: integer; forward }
var
	targslot: integer;

begin
	if put_token(loc,targslot,here.people[myslot].hiding) then begin
		if hiding then begin
			log_event(myslot,E_HPOOFOUT,0,0,myname,location);
			log_event(myslot,E_HPOOFIN,0,0,myname,loc);
		end else begin
			log_event(myslot,E_POOFOUT,0,0,myname,location);
			log_event(targslot,E_POOFIN,0,0,myname,loc);
		end;

		take_token(myslot,location);
		myslot := targslot;
		location := loc;
		setevent;
		do_look;
	end else
		writeln('There is a crackle of electricity, but the poof fails.');
end;


procedure do_poof(s: string);
var
	n,loc: integer;

begin
	if privd then begin
		gethere;
		if lookup_room(loc,s) then begin
			xpoof(loc);
		end else if parse_pers(n,s) then begin
			grab_line('What room? ',s);
			if lookup_room(loc,s) then begin
				log_event(myslot,E_POOFYOU,n,loc);
				writeln;
				writeln('You extend your arms, muster some energy, and ',here.people[n].name,' is');
				writeln('engulfed in a cloud of orange smoke.');
				writeln;
			end else
				writeln('There is no room named ',s,'.');
		end else
			writeln('There is no room named ',s,'.');
	end else
		writeln('Only the Monster Manager may poof.');
end;


procedure link_room(origdir,targdir,targroom: integer);

begin
	{ since exit creation involves the writing of two records,
	  perhaps there should be a global lock around this code,
	  such as a get to some obscure index field or something.
	  I haven't put this in because I don't believe that if this
	  routine fails it will seriously damage the database.

	  Actually, the lock should be on the test (do_link) but that
	  would be hard	}

	getroom;
	with here.exits[origdir] do begin
		toloc := targroom;
		kind := 1; { type of exit, they can customize later }
		slot := targdir; { exit it comes out in in target room }

		init_exit(origdir);
	end;
	putroom;

	log_event(myslot,E_NEWEXIT,0,0,myname,location);
	if location <> targroom then
		log_event(0,E_NEWEXIT,0,0,myname,targroom);

	getroom(targroom);
	with here.exits[targdir] do begin
		toloc := location;
		kind := 1;
		slot := origdir;

		init_exit(targdir);
	end;
	putroom;
	writeln('Exit created.  Use CUSTOM ',direct[origdir],' to customize your exit.');
end;


{
User procedure to link a room
}
procedure do_link(s: string);
var
	ok: boolean;
	orgexitnam,targnam,trgexitnam: string;
	targroom,	{ number of target room }
	targdir,	{ number of target exit direction }
	origdir: integer;{ number of exit direction here }
	firsttime: boolean;

begin

{	gethere;	! done in links_possible }

   if links_possible then begin
	log_action(link,0);
	if checkhide then begin
	writeln('Hit return alone at any prompt to terminate exit creation.');
	writeln;

	if s = '' then
		firsttime := false
	else begin
		orgexitnam := bite(s);
		firsttime := true;
	end;

	repeat
		if not(firsttime) then
			grab_line('Direction of exit? ',orgexitnam)
		else
			firsttime := false;

		ok :=lookup_dir(origdir,orgexitnam);
		if ok then
			ok := can_make(origdir);
	until (orgexitnam = '') or ok;

	if ok then begin
		if s = '' then
			firsttime := false
		else begin
			targnam := s;
			firsttime := true;
		end;

		repeat
			if not(firsttime) then
				grab_line('Room to link to? ',targnam)
			else
				firsttime := false;

			ok := lookup_room(targroom,targnam);
		until (targnam = '') or ok;
	end;

	if ok then begin
		repeat
			writeln('Exit comes out in target room');
			grab_line('from what direction? ',trgexitnam);
			ok := lookup_dir(targdir,trgexitnam);
			if ok then
				ok := can_make(targdir,targroom);
		until (trgexitnam='') or ok;
	end;

	if ok then begin { actually create the exit }
		link_room(origdir,targdir,targroom);
	end;
	end;
   end else
	writeln('No links are possible here.');
end;


procedure relink_room(origdir,targdir,targroom: integer);
var
	tmp: exit;
	copyslot,
	copyloc: integer;

begin
	gethere;
	tmp := here.exits[origdir];
	copyloc := tmp.toloc;
	copyslot := tmp.slot;

	getroom(targroom);
	here.exits[targdir] := tmp;
	putroom;

	getroom(copyloc);
	here.exits[copyslot].toloc := targroom;
	here.exits[copyslot].slot := targdir;
	putroom;

	getroom;
	here.exits[origdir].toloc := 0;
	init_exit(origdir);
	putroom;
end;


procedure do_relink(s: string);
var
	ok: boolean;
	orgexitnam,targnam,trgexitnam: string;
	targroom,	{ number of target room }
	targdir,	{ number of target exit direction }
	origdir: integer;{ number of exit direction here }
	firsttime: boolean;

begin
	log_action(c_relink,0);
	gethere;
	if checkhide then begin
	writeln('Hit return alone at any prompt to terminate exit relinking.');
	writeln;

	if s = '' then
		firsttime := false
	else begin
		orgexitnam := bite(s);
		firsttime := true;
	end;

	repeat
		if not(firsttime) then
			grab_line('Direction of exit to relink? ',orgexitnam)
		else
			firsttime := false;

		ok :=lookup_dir(origdir,orgexitnam);
		if ok then
			ok := can_alter(origdir);
	until (orgexitnam = '') or ok;

	if ok then begin
		if s = '' then
			firsttime := false
		else begin
			targnam := s;
			firsttime := true;
		end;

		repeat
			if not(firsttime) then
				grab_line('Room to relink exit into? ',targnam)
			else
				firsttime := false;

			ok := lookup_room(targroom,targnam);
		until (targnam = '') or ok;
	end;

	if ok then begin
		repeat
			writeln('New exit comes out in target room');
			grab_line('from what direction? ',trgexitnam);
			ok := lookup_dir(targdir,trgexitnam);
			if ok then
				ok := can_make(targdir,targroom);
		until (trgexitnam='') or ok;
	end;

	if ok then begin { actually create the exit }
		relink_room(origdir,targdir,targroom);
	end;
	end;
end;


{ print the room default no-go message if there is one;
  otherwise supply the generic "you can't go that way" }

procedure default_fail;

begin
	if (here.exitfail <> 0) and (here.exitfail <> DEFAULT_LINE) then
		print_desc(here.exitfail)
	else
		writeln('You can''t go that way.');
end;

procedure  exit_fail(dir: integer);
var
	tmp: string;

begin
	if (dir < 1) or (dir > maxexit) then
		default_fail
	else if (here.exits[dir].fail = DEFAULT_LINE) then begin
		case here.exits[dir].kind of
			5: writeln('There isn''t an exit there yet.');
			6: writeln('You don''t have the power to go there.');
			otherwise default_fail;
		end;
	end else if here.exits[dir].fail <> 0 then
		block_subs(here.exits[dir].fail,myname);


{ now print the exit failure message for everyone else in the room:
	if they tried to go through a valid exit,
	  and the exit has an other-person failure desc, then
		substitute that one & use;

	if there is a room default other-person failure desc, then
		print that;

	if they tried to go through a valid exit,
	  and the exit has no required alias, then
		print default exit fail
	else
		print generic "didn't leave room" message

cases:
1) valid/alias exit and specific fail message
2) valid/alias exit and blanket fail message
3) valid exit (no specific or blanket) "x fails to go [direct]"
4) alias exit and blanket fail
5) blanket fail
6) generic fail
}

	if dir <> 0 then
		log_event(myslot,E_FAILGO,dir,0);
end;



procedure do_exit; { (exit_slot: integer)-- declared forward }
var
	orig_slot,
	targ_slot,
	orig_room,
	enter_slot,
	targ_room: integer;
	doalook: boolean;

begin
	if (exit_slot < 1) or (exit_slot > 6) then
		exit_fail(exit_slot)
	else if here.exits[exit_slot].toloc > 0 then begin
		block_subs(here.exits[exit_slot].success,myname);

		orig_slot := myslot;
		orig_room := location;
		targ_room := here.exits[exit_slot].toloc;
		enter_slot := here.exits[exit_slot].slot;
		doalook := here.exits[exit_slot].autolook;

				{ optimization for exit that goes nowhere;
				  why go nowhere?  For special effects, we
				  don't want it to take too much time,
				  the logs are important because they force the
				  exit descriptions, but actually moving the
				  player is unnecessary }

		if orig_room = targ_room then begin
			log_exit(exit_slot,orig_room,orig_slot);
			log_entry(enter_slot,targ_room,orig_slot);
				{ orig_slot in log_entry 'cause we're not
				  really going anwhere }
			if doalook then
				do_look;
		end else begin
			take_token(orig_slot,orig_room);
			if not put_token(targ_room,targ_slot) then begin
					{ no room in room! }
{ put them back! Quick! }	if not put_token(orig_room,myslot) then begin
					writeln('%Oh no!');
					halt;
				end;
			end else begin
				log_exit(exit_slot,orig_room,orig_slot);
				log_entry(enter_slot,targ_room,targ_slot);

				myslot := targ_slot;
				location := targ_room;
				setevent;
	
				if doalook then
					do_look;
			end;
		end;
	end else
		exit_fail(exit_slot);
end;



function cycle_open: boolean;
var
	ch: char;
	s: string;

begin
	s := systime;
	ch := s[5];
	if ch in ['1','3','5','7','9'] then
		cycle_open := true
	else
		cycle_open := false;
end;


function which_dir(var dir:integer;s: string): boolean;
var
	aliasdir, exitdir: integer;
	aliasmatch,exitmatch,
	aliasexact,exitexact: boolean;
	exitreq: boolean;

begin
	s := lowcase(s);
	if lookup_alias(aliasdir,s) then
		aliasmatch := true
	else
		aliasmatch := false;
	if lookup_dir(exitdir,s) then
		exitmatch := true
	else
		exitmatch := false;
	if aliasmatch then begin
		if s = here.exits[aliasdir].alias then
			aliasexact := true
		else
			aliasexact := false;
	end else
		aliasexact := false;
	if exitmatch then begin
		if (s = direct[exitdir]) or (s = substr(direct[exitdir],1,1)) then
			exitexact := true
		else
			exitexact := false;
	end else
		exitexact := false;
	if exitmatch then
		exitreq := here.exits[exitdir].reqalias
	else
		exitreq := false;

	dir := 0;
	which_dir := true;
	if aliasexact and exitexact then
		dir := aliasdir
	else if aliasexact then
		dir := aliasdir
	else if exitexact and not exitreq then
		dir := exitdir
	else if aliasmatch then
		dir := aliasdir
	else if exitmatch and not exitreq then
		dir := exitdir
	else if exitmatch and exitreq then begin
		dir := exitdir;
		which_dir := false;
	end else begin
		which_dir := false;
	end;
end;


procedure exit_case(dir: integer);

begin
	case here.exits[dir].kind of
		0: exit_fail(dir);
		1: do_exit(dir);  { more checking goes here }

		3: if obj_hold(here.exits[dir].objreq) then
			exit_fail(dir)
		   else
			do_exit(dir);
		4: if rnd100 < 34 then
			do_exit(dir)
		   else
			exit_fail(dir);

		2: begin
			if obj_hold(here.exits[dir].objreq) then
				do_exit(dir)
			else
				exit_fail(dir);
		   end;
		6: if obj_hold(here.exits[dir].objreq) then
			do_exit(dir)
		     else
			exit_fail(dir);
		7: if cycle_open then
			do_exit(dir)
		   else
		exit_fail(dir);
	end;
end;

{
Player wants to go to s
Handle everthing, this is the top level procedure

Check that he can go to s
Put him through the exit	( in do_exit )
Do a look for him		( in do_exit )
}
procedure do_go(s: string;verb:boolean := true);
var
	dir: integer;

begin
	gethere;
	if checkhide then begin
		if length(s) = 0 then
			writeln('You must give the direction you wish to travel.')
		else begin
			if which_dir(dir,s) then begin
				if (dir >= 1) and (dir <= maxexit) then begin
					if here.exits[dir].toloc = 0 then begin
						exit_fail(dir);
					end else begin
						exit_case(dir);
					end;
				end else
					exit_fail(dir);
			end else
				exit_fail(dir);
		end;
	end;
end;


procedure nice_say(var s: string);

begin
		{ capitalize the first letter of their sentence }

	if s[1] in ['a'..'z'] then
		s[1] := chr( ord('A') + (ord(s[1]) - ord('a')) );

			{ put a period on the end of their sentence if
			  they don't use any punctuation. }

	if s[length(s)] in ['a'..'z','A'..'Z'] then
		s := s + '.';
end;


procedure do_say(s:string);

begin
	if length(s) > 0 then begin

{		if length(s) + length(myname) > 79 then begin
			s := substr(s,1,75-length(myname));
			writeln('Your message was truncated:');
			writeln('-- ',s);
		end;					}

		nice_say(s);
		if hiding then
			log_event(myslot,E_HIDESAY,0,0,s)
		else
			log_event(myslot,E_SAY,0,0,s);
	end else
		writeln('To talk to others in the room, type SAY <message>.');
end;

procedure do_setname(s: string);
var
	notice: string;
	ok: boolean;
	dummy: integer;
	sprime: string;

begin
	gethere;
	if s <> '' then begin
	if length(s) <= shortlen then begin
		sprime := lowcase(s);
		if (sprime = 'monster manager') and (userid <> MM_userid) then begin
			writeln('Only the Monster Manager can have that personal name.');
			ok := false;
		end else if (sprime = 'vice manager') and (userid <> MVM_userid) then begin
			writeln('Only the Vice Manager can have that name.');
			ok := false;
		end else if (sprime = 'faust') and (userid <> FAUST_userid) then begin
			writeln('You are not Faust!  You may not have that name.');
			ok := false;
		end else
			ok := true;

		if ok then
			if exact_pers(dummy,sprime) then begin
				if dummy = myslot then
					ok := true
				else begin
					writeln('Someone already has that name.  Your personal name must be unique.');
					ok := false;
				end;
			end;

		if ok then begin
			myname := s;
			getroom;
			notice := here.people[myslot].name;
			here.people[myslot].name := s;
			putroom;
			notice := notice + ' is now known as ' + s;

			if not(hiding) then
				log_event(0,E_SETNAM,0,0,notice);
					{ slot 0 means notify this player also }

			getpers;	{ note the new personal name in the logfile }
			pers.idents[mylog] := s; { don't lowcase it }
			putpers;
		end;
	end else
		writeln('Please limit your personal name to ',shortlen:1,' characters.');
	end else
		writeln('You are known to others as ',myname);
end;

function sysdate:string;
var
	thedate: packed array[1..11] of char;

begin
	date(thedate);
	sysdate := thedate;
end;


{
1234567890123456789012345678901234567890
example display for alignment:

       Monster Status
    19-MAR-1988 08:59pm

}

procedure do_who;
var
	i,j: integer;
	ok: boolean;
	metaok: boolean;
	roomown: veryshortstring;

begin
	log_event(myslot,E_WHO,0,(rnd100 mod 4));

	{ we need just about everything to print this list:
		player alloc index, userids, personal names,
		room names, room owners, and the log record	}

	getindex(I_ASLEEP);	{ Get index of people who are playing now }
	freeindex;
	getuser;
	freeuser;
	getpers;
	freepers;
	getnam;
	freenam;
	getown;
	freeown;
	getint(N_LOCATION);	{ get where they are }
	freeint;
	writeln('                   Monster Status');
	writeln('                ',sysdate,' ',systime);
	writeln;
	writeln('Username        Game Name                 Where');

	if (privd) { or has_kind(O_ALLSEEING) } then
		metaok := true
	else
		metaok := false;

	for i := 1 to indx.top do begin
		if not(indx.free[i]) then begin
			write(user.idents[i]);
			j := length(user.idents[i]);
			while j < 16 do begin
				write(' ');
				j := j + 1;
			end;

			write(pers.idents[i]);
			j := length(pers.idents[i]);
			while j <= 25 do begin
				write(' ');
				j := j + 1;
			end;

			if not(metaok) then begin
				roomown := own.idents[anint.int[i]];

{ if a person is in a public or disowned room, or
  if they are in the domain of the WHOer, then the player should know
  where they are  }

				if (roomown = '') or (roomown = '*') or
					(roomown = userid) then
					ok := true
				else
					ok := false;


{ the player obviously knows where he is }
				if i = mylog then
					ok := true;
			end;


			if ok or metaok then begin
				writeln(nam.idents[anint.int[i]]);
			end else
				writeln('n/a');
		end;
	end;
end;

function own_trans(s: string): string;

begin
	if s = '' then
		own_trans := '<public>'
	else if s = '*' then
		own_trans := '<disowned>'
	else
		own_trans := s;
end;


procedure list_rooms(s: shortstring);
var
	first: boolean;
	i,j,posit: integer;

begin
	first := true;
	posit := 0;
	for i := 1 to indx.top do begin
		if (not indx.free[i]) and (own.idents[i] = s) then begin
			if posit = 3 then begin
				posit := 1;
				writeln;
			end else
				posit := posit + 1;
			if first then begin
				first := false;
				writeln(own_trans(s),':');
			end;
			write('    ',nam.idents[i]);
			for j := length(nam.idents[i]) to 21 do
				write(' ');
		end;
	end;
	if posit <> 3 then
		writeln;
	if first then
		writeln('No rooms owned by ',own_trans(s))
	else
		writeln;
end;


procedure list_all_rooms;
var
	i,j: integer;
	tmp: packed array[1..maxroom] of boolean;

begin
	tmp := zero;
	list_rooms('');		{ public rooms first }
	list_rooms('*');	{ disowned rooms next }
	for i := 1 to indx.top do begin
		if not(indx.free[i]) and not(tmp[i]) and
		   (own.idents[i] <> '') and (own.idents[i] <> '*') then begin
				list_rooms(own.idents[i]);	{ player rooms }
				for j := 1 to indx.top do
					if own.idents[j] = own.idents[i] then
						tmp[j] := TRUE;
		end;
	end;
end;

procedure do_rooms(s: string);
var
	cmd: string;
	id: veryshortstring;
	listall: boolean;

begin
	getnam;
	freenam;
	getown;
	freeown;
	getindex(I_ROOM);
	freeindex;

	listall := false;
	s := lowcase(s);
	cmd := bite(s);
	if cmd = '' then
		id := userid
	else if cmd = 'public' then
		id := ''
	else if cmd = 'disowned' then
		id := '*'
	else if cmd = '<public>' then
		id := ''
	else if cmd = '<disowned>' then
		id := '*'
	else if cmd = '*' then
		listall := true
	else if length(cmd) > veryshortlen then
		id := substr(cmd,1,veryshortlen)
	else
		id := cmd;

	if listall then begin
		if privd then
			list_all_rooms
		else
			writeln('You may not obtain a list of all the rooms.');
	end else begin
		if privd or (userid = id) or (id = '') or (id = '*') then
			list_rooms(id)
		else
			writeln('You may not list rooms that belong to another player.');
	end;
end;



procedure do_objects;
var
	i: integer;
	total,public,disowned,private: integer;

begin
	getobjnam;
	freeobjnam;
	getobjown;
	freeobjown;
	getindex(I_OBJECT);
	freeindex;

	total := 0;
	public := 0;
	disowned := 0;
	private := 0;

	writeln;
	for i := 1 to indx.top do begin
		if not(indx.free[i]) then begin
			total := total + 1;
			if objown.idents[i]='' then begin
				writeln(i:4,'    ','<public>':12,'    ',objnam.idents[i]);
				public := public + 1
			end else if objown.idents[i]='*' then begin
				writeln(i:4,'    ','<disowned>':12,'    ',objnam.idents[i]);
				disowned := disowned + 1
			end else begin
				private := private + 1;

				if (objown.idents[i] = userid) or
				 (privd) then begin
{ >>>>>> }	writeln(i:4,'    ',objown.idents[i]:12,'    ',objnam.idents[i]);
				end;
			end;
		end;
	end;
	writeln;
	writeln('Public:      ',public:4);
	writeln('Disowned:    ',disowned:4);
	writeln('Private:     ',private:4);
	writeln('             ----');
	writeln('Total:       ',total:4);
end;


procedure do_claim(s: string);
var
	n: integer;
	ok: boolean;
	tmp: string;

begin
	if length(s) = 0 then begin	{ claim this room }
		getroom;
		if (here.owner = '*') or (privd) then begin
			here.owner := userid;
			putroom;
			getown;
			own.idents[location] := userid;
			putown;
			log_event(myslot,E_CLAIM,0,0);
			writeln('You are now the owner of this room.');
		end else begin
			freeroom;
			if here.owner = '' then
				writeln('This is a public room.  You may not claim it.')
			else
				writeln('This room has an owner.');
		end;
	end else if lookup_obj(n,s) then begin
		getobjown;
		freeobjown;
		if objown.idents[n] = '' then
			writeln('That is a public object.  You may DUPLICATE it, but may not CLAIM it.')
		else if objown.idents[n] <> '*' then
			writeln('That object has an owner.')
		else begin
			getobj(n);
			freeobj;
			if obj.numexist = 0 then
				ok := true
			else begin
				if obj_hold(n) or obj_here(n) then
					ok := true
				else
					ok := false;
			end;

			if ok then begin
				getobjown;
				objown.idents[n] := userid;
				putobjown;
				tmp := obj.oname;
				log_event(myslot,E_OBJCLAIM,0,0,tmp);
				writeln('You are now the owner the ',tmp,'.');
			end else
				writeln('You must have one to claim it.');
		end;
	end else
		writeln('There is nothing here by that name to claim.');
end;

procedure do_disown(s: string);
var
	n: integer;
	tmp: string;

begin
	if length(s) = 0 then begin	{ claim this room }
		getroom;
		if (here.owner = userid) or (privd) then begin
			getroom;
			here.owner := '*';
			putroom;
			getown;
			own.idents[location] := '*';
			putown;
			log_event(myslot,E_DISOWN,0,0);
			writeln('You have disowned this room.');
		end else begin
			freeroom;
			writeln('You are not the owner of this room.');
		end;
	end else begin	{ disown an object }
		if lookup_obj(n,s) then begin
			getobj(n);
			freeobj;
			tmp := obj.oname;

			getobjown;
			if objown.idents[n] = userid then begin
				objown.idents[n] := '*';
				putobjown;
				log_event(myslot,E_OBJDISOWN,0,0,tmp);
				writeln('You are no longer the owner of the ',tmp,'.');
			end else begin
				freeobjown;
				writeln('You are not the owner of any such thing.');
			end;
		end else
			writeln('You are not the owner of any such thing.');
	end;
end;


procedure do_public(s: string);
var
	ok: boolean;
	tmp: string;
	n: integer;

begin
	if privd then begin
		if length(s) = 0 then begin
			getroom;
			here.owner := '';
			putroom;
			getown;
			own.idents[location] := '';
			putown;
		end else if lookup_obj(n,s) then begin
			getobjown;
			freeobjown;
			if objown.idents[n] = '' then
				writeln('That is already public.')
			else begin
				getobj(n);
				freeobj;
				if obj.numexist = 0 then
					ok := true
				else begin
					if obj_hold(n) or obj_here(n) then
						ok := true
					else
						ok := false;
				end;

				if ok then begin
					getobjown;
					objown.idents[n] := '';
					putobjown;

					tmp := obj.oname;
					log_event(myslot,E_OBJPUBLIC,0,0,tmp);
					writeln('The ',tmp,' is now public.');
				end else
					writeln('You must have one to claim it.');
			end;
		end else
			writeln('There is nothing here by that name to claim.');
	end else
		writeln('Only the Monster Manager may make things public.');
end;



{ sum up the number of real exits in this room }

function find_numexits: integer;
var
	i: integer;
	sum: integer;

begin
	sum := 0;
	for i := 1 to maxexit do
		if here.exits[i].toloc <> 0 then
			sum := sum + 1;
	find_numexits := sum;
end;



{ clear all people who have played monster and quit in this location
  out of the room so that when they start up again they won't be here,
  because we are destroying this room }

procedure clear_people(loc: integer);
var
	i: integer;

begin
	getint(N_LOCATION);
	for i := 1 to maxplayers do
		if anint.int[i] = loc then
			anint.int[i] := 1;
	putint;
end;


procedure do_zap(s: string);
var
	loc: integer;

begin
	gethere;
	if checkhide then begin
	if lookup_room(loc,s) then begin
		gethere(loc);
		if (here.owner = userid) or (privd) then begin
			clear_people(loc);
			if find_numpeople = 0 then begin
				if find_numexits = 0 then begin
					if find_numobjs = 0 then begin
						del_room(loc);
						writeln('Room deleted.');
					end else
						writeln('You must remove all of the objects from that room first.');
				end else
					writeln('You must delete all of the exits from that room first.');
			end else
				writeln('Sorry, you cannot destroy a room if people are still in it.');
		end else
			writeln('You are not the owner of that room.');
	end else
		writeln('There is no room named ',s,'.');
	end;
end;


function room_nameinuse(num: integer; newname: string): boolean;
var
	dummy: integer;

begin
	if exact_obj(dummy,newname) then begin
		if dummy = num then
			room_nameinuse := false
		else
			room_nameinuse := true;
	end else
		room_nameinuse := false;
end;



procedure do_rename;
var
	dummy: integer;
	newname: string;
	s: string;

begin
	gethere;
	writeln('This room is named ',here.nicename);
	writeln;
	grab_line('New name: ',newname);
	if (newname = '') or (newname = '**') then
		writeln('No changes.')
	else if length(newname) > shortlen then
		writeln('Please limit your room name to ',shortlen:1,' characters.')
	else if room_nameinuse(location,newname) then
		writeln(newname,' is not a unique room name.')
	else begin
		getroom;
		here.nicename := newname;
		putroom;

		getnam;
		nam.idents[location] := lowcase(newname);
		putnam;
		writeln('Room name updated.');
	end;
end;


function obj_nameinuse(objnum: integer; newname: string): boolean;
var
	dummy: integer;

begin
	if exact_obj(dummy,newname) then begin
		if dummy = objnum then
			obj_nameinuse := false
		else
			obj_nameinuse := true;
	end else
		obj_nameinuse := false;
end;


procedure do_objrename(objnum: integer);
var
	newname: string;
	s: string;

begin
	getobj(objnum);
	freeobj;

	writeln('This object is named ',obj.oname);
	writeln;
	grab_line('New name: ',newname);
	if (newname = '') or (newname = '**') then
		writeln('No changes.')
	else if length(newname) > shortlen then
		writeln('Please limit your object name to ',shortlen:1,' characters.')
	else if obj_nameinuse(objnum,newname) then
		writeln(newname,' is not a unique object name.')
	else begin
		getobj(objnum);
		obj.oname := newname;
		putobj;

		getobjnam;
		objnam.idents[objnum] := lowcase(newname);
		putobjnam;
		writeln('Object name updated.');
	end;
end;



procedure view_room;
var
	s: string;
	i: integer;

begin
	writeln;
	getnam;
	freenam;
	getobjnam;
	freeobjnam;

	with here do begin
		writeln('Room:        ',nicename);
		case nameprint of
			0: writeln('Room name not printed');
			1: writeln('"You''re in" precedes room name');
			2: writeln('"You''re at" precedes room name');
			otherwise writeln('Room name printing is damaged.');
		end;

		write('Room owner:    ');
		if owner = '' then
			writeln('<public>')
		else if owner = '*' then
			writeln('<disowned>')
		else
			writeln(owner);

		if primary = 0 then
			writeln('There is no primary description')
		else
			writeln('There is a primary description');

		if secondary = 0 then
			writeln('There is no secondary description')
		else
			writeln('There is a secondary description');

		case which of
			0: writeln('Only the primary description will print');
			1: writeln('Only the secondary description will print');
			2: writeln('Both the primary and secondary descriptions will print');
			3: begin
				writeln('The primary description will print, followed by the seconary description');
				writeln('if the player is holding the magic object');
			   end;
			4: begin
				writeln('If the player is holding the magic object, the secondary description will print');
				writeln('Otherwise, the primary description will print');
			   end;
			otherwise writeln('The way the room description prints is damaged');
		end;

		writeln;
		if magicobj = 0 then
			writeln('There is no magic object for this room')
		else
			writeln('The magic object for this room is the ',objnam.idents[magicobj],'.');

		if objdrop = 0 then
			writeln('Dropped objects remain here')
		else begin
			writeln('Dropped objects go to ',nam.idents[objdrop],'.');
			if objdesc = 0 then
				writeln('Dropped.')
			else
				print_line(objdesc);
			if objdest = 0 then
				writeln('Nothing is printed when object "bounces in" to target room')
			else
				print_line(objdest);
		end;
		writeln;
		if trapto = 0 then
			writeln('There is no trapdoor set')
		else
			writeln('The trapdoor sends players ',direct[trapto],
				' with a chance factor of ',trapchance:1,'%');

		for i := 1 to maxdetail do begin
			if length(detail[i]) > 0 then begin
				write('Detail "',detail[i],'" ');
				if detaildesc[i] > 0 then
					writeln('has a description')
				else
					writeln('has no description');
			end;
		end;
		writeln;
	end;
end;


procedure room_help;

begin
	writeln;
	writeln('D	Alter the way the room description prints');
	writeln('N	Change how the room Name prints');
	writeln('P	Edit the Primary room description [the default one] (same as desc)');
	writeln('S	Edit the Secondary room description');
	writeln('X	Define a mystery message');
	writeln;
	writeln('G	Set the location that a dropped object really Goes to');
	writeln('O	Edit the object drop description (for drop effects)');
	writeln('B	Edit the target room (G) "bounced in" description');
	writeln;
	writeln('T	Set the direction that the Trapdoor goes to');
	writeln('C	Set the Chance of the trapdoor functioning');
	writeln;
	writeln('M	Define the magic object for this room');
	writeln('R	Rename the room');
	writeln;
	writeln('V	View settings on this room');
	writeln('E	Exit (same as quit)');
	writeln('Q	Quit (same as exit)');
	writeln('?	This list');
	writeln;
end;



procedure custom_room;
var
	done: boolean;
	prompt: string;
	n: integer;
	s: string;
	newdsc: integer;
	bool: boolean;

begin
	log_action(e_custroom,0);
	writeln;
	writeln('Customizing this room');
	writeln('If you would rather be customizing an exit, type CUSTOM <direction of exit>');
	writeln('If you would rather be customizing an object, type CUSTOM <object name>');
	writeln;
	done := false;
	prompt := 'Custom> ';

	repeat
		repeat
			grab_line(prompt,s);
			s := slead(s);
		until length(s) > 0;
		s := lowcase(s);
		case s[1] of

			'e','q': done := true;
			'?','h': room_help;
			'r': do_rename;
			'v': view_room;
{dir trapdoor goes}	't': begin
				grab_line('What direction does the trapdoor exit through? ',s);
				if length(s) > 0 then begin
					if lookup_dir(n,s) then begin
						getroom;
						here.trapto := n;
						putroom;
						writeln('Room updated.');
					end else
						writeln('No such direction.');
				end else
					writeln('No changes.');
			     end;
{chance}		'c': begin
				writeln('Enter the chance that in any given minute the player will fall through');
				writeln('the trapdoor (0-100) :');
				writeln;
				grab_line('? ',s);
				if isnum(s) then begin
					n := number(s);
					if n in [0..100] then begin
						getroom;
						here.trapchance := n;
						putroom;
					end else
						writeln('Out of range.');
				end else
					writeln('No changes.');
			     end;
			's': begin
				newdsc := here.secondary;
				writeln('[ Editing the secondary room description ]');
				if edit_desc(newdsc) then begin
					getroom;
					here.secondary := newdsc;
					putroom;
				end;
			     end;
			'p': begin
{ same as desc }		newdsc := here.primary;
				writeln('[ Editing the primary room description ]');
				if edit_desc(newdsc) then begin
					getroom;
					here.primary := newdsc;
					putroom;
				end;
			     end;
			'o': begin
				writeln('Enter the line that will be printed when someone drops an object here:');
				writeln('If dropped objects do not stay here, you may use a # for the object name.');
				writeln('Right now it says:');
				if here.objdesc = 0 then
					writeln('Dropped. [default]')
				else
					print_line(here.objdesc);

				n := here.objdesc;
				make_line(n);
				getroom;
				here.objdesc := n;
				putroom;
			     end;
			'x': begin
				writeln('Enter a line that will be randomly shown.');
				writeln('Right now it says:');
				if here.objdesc = 0 then
					writeln('[none defined]')
				else
					print_line(here.rndmsg);

				n := here.rndmsg;
				make_line(n);
				getroom;
				here.rndmsg := n;
				putroom;
			     end;
{bounced in desc}	'b': begin
				writeln('Enter the line that will be displayed in the room where an object really');
				writeln('goes when an object dropped here "bounces" there:');
				writeln('Place a # where the object name should go.');
				writeln;
				writeln('Right now it says:');
				if here.objdest = 0 then
					writeln('Something has bounced into the room.')
				else
					print_line(here.objdest);

				n := here.objdest;
				make_line(n);
				getroom;
				here.objdest := n;
				putroom;
			     end;
			'm': begin
				getobjnam;
				freeobjnam;
				if here.magicobj = 0 then
					writeln('There is currently no magic object for this room.')
				else
					writeln(objnam.idents[here.magicobj],
						' is currently the magic object for this room.');
				writeln;
				grab_line('New magic object? ',s);
				if s = '' then
					writeln('No changes.')
				else if lookup_obj(n,s) then begin
					getroom;
					here.magicobj := n;
					putroom;
					writeln('Room updated.');
				end else
					writeln('No such object found.');
			     end;
			'g': begin
				getnam;
				freenam;
				if here.objdrop = 0 then
					writeln('Objects dropped fall here.')
				else
					writeln('Objects dropped fall in ',nam.idents[here.objdrop],'.');
				writeln;
				writeln('Enter * for [this room]:');
				grab_line('Room dropped objects go to? ',s);
				if s = '' then
					writeln('No changes.')
				else if s = '*' then begin
					getroom;
					here.objdrop := 0;
					putroom;
					writeln('Room updated.');
				end else if lookup_room(n,s) then begin
					getroom;
					here.objdrop := n;
					putroom;
					writeln('Room updated.');
				end else
					writeln('No such room found.');
			     end;
			'd': begin
				writeln('Print room descriptions how?');
				writeln;
				writeln('0)  Print primary (main) description only [default]');
				writeln('1)  Print only secondary description.');
				writeln('2)  Print both primary and secondary descriptions togther.');
				writeln('3)  Print primary description first; then print secondary description only if');
				writeln('    the player is holding the magic object for this room.');
				writeln('4)  Print secondary if holding the magic obj; print primary otherwise');
				writeln;
				grab_line('? ',s);
				if isnum(s) then begin
					n := number(s);
					if n in [0..4] then begin
						getroom;
						here.which := n;
						putroom;
						writeln('Room updated.');
					end else
						writeln('Out of range.');
				end else
					writeln('No changes.');

			     end;
			'n': begin
				writeln('How would you like the room name to print?');
				writeln;
				writeln('0) No room name is shown');
				writeln('1) "You''re in ..."');
				writeln('2) "You''re at ..."');
				writeln;
				grab_line('? ',s);
				if isnum(s) then begin
					n := number(s);
					if n in [0..2] then begin
						getroom;
						here.nameprint := n;
						putroom;
					end else
						writeln('Out of range.');
				end else
					writeln('No changes.');
			     end;
			otherwise writeln('Bad command, type ? for a list');
		end;
	until done;
	log_event(myslot,E_ROOMDONE,0,0);
end;

procedure analyze_exit(dir: integer);
var
	s: string;

begin
	writeln;
	getnam;
	freenam;
	getobjnam;
	freeobjnam;
	with here.exits[dir] do begin
		s := alias;
		if s = '' then
			s := '(no alias)'
		else
			s := '(alias ' + s + ')';
		if here.exits[dir].reqalias then
			s := s + ' (required)'
		else
			s := s + ' (not required)';

		if toloc <> 0 then
			writeln('The ',direct[dir],' exit ',s,' goes to ',nam.idents[toloc])
		else
			writeln('The ',direct[dir],' exit goes nowhere.');
		if hidden <> 0 then
			writeln('Concealed.');
		write('Exit type: ');
		case kind of
			0: writeln('no exit.');
			1: writeln('Open passage.');
			2: writeln('Door, object required to pass.');
			3: writeln('No passage if holding object.');
			4: writeln('Randomly fails');
			5: writeln('Potential exit.');
			6: writeln('Only exists while holding the required object.');
			7: writeln('Timed exit');
		end;
		if objreq = 0 then
			writeln('No required object.')
		else
			writeln('Required object is: ',objnam.idents[objreq]);


		writeln;
		if exitdesc = DEFAULT_LINE then
			exit_default(dir,kind)
		else
			print_line(exitdesc);

		if success = 0 then
			writeln('(no success message)')
		else
			print_desc(success);

		if fail = DEFAULT_LINE then begin
			if kind = 5 then
				writeln('There isn'' an exit there yet.')
			else
				writeln('You can''t go that way.');
		end else
			print_desc(fail);

		if comeout = DEFAULT_LINE then
			writeln('# has come into the room from: ',direct[dir])
		else
			print_desc(comeout);
		if goin = DEFAULT_LINE then
			writeln('# has gone ',direct[dir])
		else
			print_desc(goin);

		writeln;
		if autolook then
			writeln('LOOK automatically done after exit used')
		else
			writeln('LOOK supressed on exit use');
		if reqverb then
			writeln('The alias is required to be a verb for exit use')
		else
			writeln('The exit can be used with GO or as a verb');
	end;
	writeln;
end;

procedure custom_help;

begin
	writeln;
	writeln('A	Set an Alias for the exit');
	writeln('C	Conceal an exit');
	writeln('D	Edit the exit''s main Description');
	writeln('E	EXIT custom (saves changes)');
	writeln('F	Edit the exit''s failure line');
	writeln('I	Edit the line that others see when a player goes Into an exit');
	writeln('K	Set the object that is the Key to this exit');
	writeln('L	Automatically look [default] / don''t look on exit');
	writeln('O	Edit the line that people see when a player comes Out of an exit');
	writeln('Q	QUIT Custom (saves changes)');
	writeln('R	Require/don''t require alias for exit; ignore direction');
	writeln('S	Edit the success line');
	writeln('T	Alter Type of exit (passage, door, etc)');
	writeln('V	View exit information');
	writeln('X	Require/don''t require exit name to be a verb');
	writeln('?	This list');
	writeln;
end;


procedure get_key(dir: integer);
var
	s: string;
	n: integer;

begin
	getobjnam;
	freeobjnam;
	if here.exits[dir].objreq = 0 then
		writeln('Currently there is no key set for this exit.')
	else
		writeln(objnam.idents[here.exits[dir].objreq],' is the current key for this exit.');
	writeln('Enter * for [no key]');
	writeln;

	grab_line('What object is the door key? ',s);
	if length(s) > 0 then begin
		if s = '*' then begin
			getroom;
			here.exits[dir].objreq := 0;
			putroom;
			writeln('Exit updated.');
		end else if lookup_obj(n,s) then begin
			getroom;
			here.exits[dir].objreq := n;
			putroom;
			writeln('Exit updated.');
		end else
			writeln('There is no object by that name.');
	end else
		writeln('No changes.');
end;


procedure do_custom(dirnam: string);
var
	prompt: string;
	done : boolean;
	s: string;
	dir: integer;
	n: integer;

begin
	gethere;
	if checkhide then begin
	if length(dirnam) = 0 then begin
		if is_owner(location,TRUE) then
			custom_room
		else begin
			writeln('You are not the owner of this room; you cannot customize it.');
			writeln('However, you may be able to customize some of the exits.  To customize an');
			writeln('exit, type CUSTOM <direction of exit>');
		end;
	end else if lookup_dir(dir,dirnam) then begin
	   if can_alter(dir) then begin
		log_action(c_custom,0);

		writeln('Customizing ',direct[dir],' exit');
		writeln('If you would rather be customizing this room, type CUSTOM with no arguments');
		writeln('If you would rather be customizing an object, type CUSTOM <object name>');
		writeln;
		writeln('Type ** for any line to leave it unchanged.');
		writeln('Type return for any line to select the default.');
		writeln;
		writev(prompt,'Custom ',direct[dir],'> ');
		done := false;
		repeat
			repeat
				grab_line(prompt,s);
				s := slead(s);
			until length(s) > 0;
			s := lowcase(s);
			case s[1] of
				'?','h': custom_help;
				'q','e': done := true;
				'k': get_key(dir);
				'c': begin
					writeln('Type the description that a player will see when the exit is found.');
					writeln('Make no text for description to unconceal the exit.');
					writeln;
					writeln('[ Editing the "hidden exit found" description ]');
					n := here.exits[dir].hidden;
					if edit_desc(n) then begin
						getroom;
						here.exits[dir].hidden := n;
						putroom;
					end;
				     end;
{req alias}			'r': begin
					getroom;
					here.exits[dir].reqalias :=
						not(here.exits[dir].reqalias);
					putroom;

					if here.exits[dir].reqalias then
						writeln('The alias for this exit will be required to reference it.')
					else
						writeln('The alias will not be required to reference this exit.');
				     end;
{req verb}			'x': begin
					getroom;
					here.exits[dir].reqverb :=
						not(here.exits[dir].reqverb);
					putroom;

					if here.exits[dir].reqverb then
						writeln('The exit name will be required to be used as a verb to use the exit')
					else
						writeln('The exit name may be used with GO or as a verb to use the exit');
				     end;
{autolook}			'l': begin
					getroom;
					here.exits[dir].autolook :=
						not(here.exits[dir].autolook);
					putroom;

					if here.exits[dir].autolook then
						writeln('A LOOK will be done after the player travels through this exit.')
					else
						writeln('The automatic LOOK will not be done when a player uses this exit.');
				     end;
				'a': begin
					grab_line('Alternate name for the exit? ',s);
					if length(s) > veryshortlen then
						writeln('Your alias must be less than ',veryshortlen:1,' characters.')
					else begin
						getroom;
						here.exits[dir].alias := lowcase(s);
						putroom;
					end;
				     end;
				'v': analyze_exit(dir);
				't': begin
					writeln;
					writeln('Select the type of your exit:');
					writeln;
					writeln('0) No exit');
					writeln('1) Open passage');
					writeln('2) Door (object required to pass)');
					writeln('3) No passage if holding key');
					if privd then
						writeln('4) exit randomly fails');
					writeln('6) Exit exists only when holding object');
					if privd then
						writeln('7) exit opens/closes invisibly every minute');
					writeln;
					grab_line('Which type? ',s);
					if isnum(s) then begin
						n := number(s);
						if n in [0..4,6..7] then begin
							getroom;
							here.exits[dir].kind := n;
							putroom;
							writeln('Exit type updated.');
							writeln;
							if n in [2,6] then
								get_key(dir);
						end else
							writeln('Bad exit type.');
					end else
						writeln('Exit type not changed.');
				     end;
				'f': begin
					writeln('The failure description will print if the player attempts to go through the');
					writeln('the exit but cannot for any reason.');
					writeln;
					writeln('[ Editing the exit failure description ]');

					n := here.exits[dir].fail;
					if edit_desc(n) then begin
						getroom;
						here.exits[dir].fail := n;
						putroom;
					end;
				     end;
				'i': begin
					writeln('Edit the description that other players see when someone goes into');
					writeln('the exit.  Place a # where the player''s name should appear.');
					writeln;
					writeln('[ Editing the exit "go in" description ]');
					n := here.exits[dir].goin;
					if edit_desc(n) then begin
						getroom;
						here.exits[dir].goin := n;
						putroom;
					end;
				     end;
				'o': begin
					writeln('Edit the description that other players see when someone comes out of');
					writeln('the exit.  Place a # where the player''s name should appear.');
					writeln;
					writeln('[ Editing the exit "come out of" description ]');
					n := here.exits[dir].comeout;
					if edit_desc(n) then begin
						getroom;
						here.exits[dir].comeout := n;
						putroom;
					end;
				     end;
{ main exit desc }		'd': begin
					writeln('Enter a one line description of the exit.');
					writeln;
					n := here.exits[dir].exitdesc;
					make_line(n);
					getroom;
					here.exits[dir].exitdesc := n;
					putroom;
				     end;
				's': begin
					writeln('The success description will print when the player goes through the exit.');
					writeln;
					writeln('[ Editing the exit success description ]');

					n := here.exits[dir].success;
					if edit_desc(n) then begin
						getroom;
						here.exits[dir].success := n;
						putroom;
					end;
				     end;
				otherwise writeln('-- Bad command, type ? for a list');
			end;
		until done;


		log_event(myslot,E_CUSTDONE,0,0);
	   end else
		writeln('You are not allowed to alter that exit.');
	end else if lookup_obj(n,dirnam) then
{ if lookup_obj returns TRUE then dirnam is name of object to custom }
				do_program(dirnam)	{ customize the object }
			else begin
		writeln('To customize this room, type CUSTOM');
		writeln('To customize an exits, type CUSTOM <direction>');
		writeln('To customize an object, type CUSTOM <object name>');
	end;
{	clear_command;	}
	end;
end;



procedure reveal_people(var three: boolean);
var
	retry,i: integer;

begin
	if debug then
		writeln('%revealing people');
	three := false;
	retry := 1;

	repeat
		retry := retry + 1;
		i := (rnd100 mod maxpeople) + 1;
		if (here.people[i].hiding > 0) and
				(i <> myslot) then begin
			three := true;
			writeln('You''ve found ',here.people[i].name,' hiding in the shadows!');
			log_event(myslot,E_FOUNDYOU,i,0);
		end;
	until (retry > 7) or three;
end;



procedure reveal_objects(var two: boolean);
var
	tmp: string;
	i: integer;

begin
	if debug then
		writeln('%revealing objects');
	two := false;
	for i := 1 to maxobjs do begin
		if here.objs[i] <> 0 then	{ if there is an object here }
			if (here.objhide[i] <> 0) then begin
				two := true;

				if here.objhide[i] = DEFAULT_LINE then
					writeln('You''ve found ',obj_part(here.objs[i]),'.')
				else begin
					print_desc(here.objhide[i]);
					delete_block(here.objhide[i]);
				end;
			end;
	end;
end;


procedure reveal_exits(var one: boolean);
var
	retry,i: integer;

begin
	if debug then
		writeln('%revealing exits');
	one := false;
	retry := 1;

	repeat
		retry := retry + 1;
		i := (rnd100 mod maxexit) + 1;  { a random exit }
		if (here.exits[i].hidden <> 0) and (not found_exit[i]) then begin
			one := true;
			found_exit[i] := true;	{ mark exit as found }

			if here.exits[i].hidden = DEFAULT_LINE then begin
				if here.exits[i].alias = '' then
					writeln('You''ve found a hidden exit: ',direct[i],'.')
				else
					writeln('You''ve found a hidden exit: ',here.exits[i].alias,'.');
			end else
				print_desc(here.exits[i].hidden);
		end;
	until (retry > 4) or (one);
end;


procedure do_search(s: string);
var
	chance: integer;
	found,dummy: boolean;

begin
	if checkhide then begin
		chance := rnd100;
		found := false;
		dummy := false;

		if chance in [1..20] then
			reveal_objects(found)
		else if chance in [21..40] then
			reveal_exits(found)
		else if chance in [41..60] then
			reveal_people(dummy);

		if found then begin
			log_event(myslot,E_FOUND,0,0);
		end else if not(dummy) then begin
			log_event(myslot,E_SEARCH,0,0);
			writeln('You haven''t found anything.');
		end;
	end;
end;

procedure do_unhide(s: string);

begin
	if s = '' then begin
		if hiding then begin
			hiding := false;
			log_event(myslot,E_UNHIDE,0,0);
			getroom;
			here.people[myslot].hiding := 0;
			putroom;
			writeln('You are no longer hiding.');
		end else
			writeln('You were not hiding.');
	end;
end;


procedure do_hide(s: string);
var
	slot,n: integer;
	founddsc: integer;
	tmp: string;

begin
	gethere;
	if s = '' then begin	{ hide yourself }

			{ don't let them hide (or hide better) if people
			  that they can see are in the room.  Note that the
			  use of n_can_see instead of find_numpeople will
			  let them hide if other people are hidden in the
			  room that they have not seen.  The previously hidden
			  people will see them hide }

		if n_can_see > 0 then begin
			if hiding then
				writeln('You can''t hide any better with people in the room.')
			else
				writeln('You can''t hide when people are watching you.');
		end else if (rnd100 > 25) then begin
			if here.people[myslot].hiding >= 4 then
				writeln('You''re pretty well hidden now.  I don''t think you could be any less visible.')
			else begin
				getroom;
				here.people[myslot].hiding := 
						here.people[myslot].hiding + 1;
				putroom;
				if hiding then begin
					log_event(myslot,E_NOISES,rnd100,0);
					writeln('You''ve managed to hide yourself a little better.');
				end else begin
					log_event(myslot,E_IHID,0,0);
					writeln('You''ve hidden yourself from view.');
					hiding := true;
				end;
			end;
		end else begin { unsuccessful }
			if hiding then
				writeln('You could not find a better hiding place.')
			else
				writeln('You could not find a good hiding place.');
		end;
	end else begin	{ Hide an object }
		if parse_obj(n,s) then begin
			if obj_here(n) then begin
				writeln('Enter the description the player will see when the object is found:');
				writeln('(if no description is given a default will be supplied)');
				writeln;
				writeln('[ Editing the "object found" description ]');
				founddsc := 0;
				if edit_desc(founddsc) then ;
				if founddsc = 0 then
					founddsc := DEFAULT_LINE;

				getroom;
				slot := find_obj(n);
				here.objhide[slot] := founddsc;
				putroom;

				tmp := obj_part(n);
				log_event(myslot,E_HIDOBJ,0,0,tmp);
				writeln('You have hidden ',tmp,'.');
			end else if obj_hold(n) then begin
				writeln('You''ll have to put it down before it can be hidden.');
			end else
				writeln('I see no such object here.');
		end else
			writeln('I see no such object here.');
	end;
end;


procedure do_punch(s: string);
var
	sock,n: integer;

begin
	if s <> '' then begin
		if parse_pers(n,s) then begin
			if n = myslot then
				writeln('Self-abuse will not be tolerated in the Monster universe.')
			else if protected(n) then begin
				log_event(myslot,E_TRYPUNCH,n,0);
				writeln('A mystic shield of force prevents you from attacking.');
			end else if here.people[n].username = MM_userid then begin
				log_event(myslot,E_TRYPUNCH,n,0);
				writeln('You can''t punch the Monster Manager.');
			end else begin
				if hiding then begin
					hiding := false;

					getroom;
					here.people[myslot].hiding := 0;
					putroom;

					log_event(myslot,E_HIDEPUNCH,n,0);
					writeln('You pounce unexpectedly on ',here.people[n].name,'!');
				end else begin
					sock := (rnd100 mod numpunches)+1;
					log_event(myslot,E_PUNCH,n,sock);
					put_punch(sock,here.people[n].name);
				end;
				wait(1+random*3);	{ Ha ha ha }
			end;
		end else
			writeln('That person cannot be seen in this room.');
	end else
		writeln('To punch somebody, type PUNCH <personal name>.');
end;


{ support for do_program (custom an object)
  Give the player a list of kinds of object he's allowed to make his object
  and update it }

procedure prog_kind(objnum: integer);
var
	n: integer;
	s: string;

begin
	writeln('Select the type of your object:');
	writeln;
	writeln('0	Ordinary object (good for door keys)');
	writeln('1	Weapon');
	writeln('2	Armor');
	writeln('3	Exit thruster');

	if privd then begin
	writeln;
	writeln('100	Bag');
	writeln('101	Crystal Ball');
	writeln('102	Wand of Power');
	writeln('103	Hand of Glory');
	end;
	writeln;
	grab_line('Which kind? ',s);

	if isnum(s) then begin
		n := number(s);
		if (n > 100) and (privd) then
			writeln('Out of range.')
		else if n in [0..3,100..103] then begin
			getobj(objnum);
			obj.kind := n;
			putobj;
			writeln('Object updated.');
		end else
			writeln('Out of range.');
	end;
end;



{ support for do_program (custom an object)
  Based on the kind it is allow the
  user to set the various parameters for the effects associated with that
  kind }

procedure prog_obj(objnum: integer);

begin
end;


procedure show_kind(p: integer);

begin
	case p of
		0: writeln('Ordinary object');
		1: writeln('Weapon');
		2: writeln('Armor');
		100: writeln('Bag');
		101: writeln('Crystal Ball');
		102: writeln('Wand of Power');
		103: writeln('Hand of Glory');
		otherwise writeln('Bad object type');
	end;
end;


procedure obj_view(objnum: integer);

begin
	writeln;
	getobj(objnum);
	freeobj;
	getobjown;
	freeobjown;
	writeln('Object name:    ',obj.oname);
	writeln('Owner:          ',objown.idents[objnum]);
	writeln;
	show_kind(obj.kind);
	writeln;

	if obj.linedesc = 0 then
		writeln('There is a(n) # here')
	else
		print_line(obj.linedesc);

	if obj.examine = 0 then
		writeln('No inspection description set')
	else
		print_desc(obj.examine);

{	writeln('Worth (in points) of this object: ',obj.worth:1);	}
	writeln('Number in existence: ',obj.numexist:1);
	writeln;
end;


procedure program_help;

begin
	writeln;
	writeln('A	"a", "an", "some", etc.');
	writeln('D	Edit a Description of the object');
	writeln('F	Edit the GET failure message');
	writeln('G	Set the object required to pick up this object');
	writeln('1	Set the get success message');
	writeln('K	Set the Kind of object this is');
	writeln('L	Edit the label description ("There is a ... here.")');
	writeln('P	Program the object based on the kind it is');
	writeln('R	Rename the object');
	writeln('S	Toggle the sticky bit');
	writeln;
	writeln('U	Set the object required for use');
	writeln('2	Set the place required for use');
	writeln('3	Edit the use failure description');
	writeln('4	Edit the use success description');
	writeln('V	View attributes of this object');
	writeln;
	writeln('X	Edit the extra description');
	writeln('5	Edit extra desc #2');
	writeln('E	Exit (same as Quit)');
	writeln('Q	Quit (same as Exit)');
	writeln('?	This list');
	writeln;
end;


procedure do_program;	{ (objnam: string);  declared forward }
var
	prompt: string;
	done : boolean;
	s: string;
	objnum: integer;
	n: integer;
	newdsc: integer;

begin
	gethere;
	if checkhide then begin
	if length(objnam) = 0 then begin
		writeln('To program an object, type PROGRAM <object name>.');
	end else if lookup_obj(objnum,objnam) then begin
	if not is_owner(location,TRUE) then begin
		writeln('You may only work on your objects when you are in one of your own rooms.');
	end else if obj_owner(objnum) then begin
		log_action(e_program,0);
		writeln;
		writeln('Customizing object');
		writeln('If you would rather be customizing an EXIT, type CUSTOM <direction of exit>');
		writeln('If you would rather be customizing this room, type CUSTOM');
		writeln;
		getobj(objnum);
		freeobj;
		prompt := 'Custom object> ';
		done := false;
		repeat
			repeat
				grab_line(prompt,s);
				s := slead(s);
			until length(s) > 0;
			s := lowcase(s);
			case s[1] of
				'?','h': program_help;
				'q','e': done := true;
				'v': obj_view(objnum);
				'r': do_objrename(objnum);
				'g': begin
					writeln('Enter * for no object');
					grab_line('Object required for GET? ',s);
					if s = '*' then begin
						getobj(objnum);
						obj.getobjreq := 0;
						putobj;
					end else if lookup_obj(n,s) then begin
						getobj(objnum);
						obj.getobjreq := n;
						putobj;
						writeln('Object modified.');
					end else
						writeln('No such object.');
				     end;
				'u': begin
					writeln('Enter * for no object');
					grab_line('Object required for USE? ',s);
					if s = '*' then begin
						getobj(objnum);
						obj.useobjreq := 0;
						putobj;
					end else if lookup_obj(n,s) then begin
						getobj(objnum);
						obj.useobjreq := n;
						putobj;
						writeln('Object modified.');
					end else
						writeln('No such object.');
				     end;
				'2': begin
					writeln('Enter * for no special place');
					grab_line('Place required for USE? ',s);
					if s = '*' then begin
						getobj(objnum);
						obj.uselocreq := 0;
						putobj;
					end else if lookup_room(n,s) then begin
						getobj(objnum);
						obj.uselocreq := n;
						putobj;
						writeln('Object modified.');
					end else
						writeln('No such object.');
				     end;
				's': begin
					getobj(objnum);
					obj.sticky := not(obj.sticky);
					putobj;
					if obj.sticky then
						writeln('The object will not be takeable.')
					else
						writeln('The object will be takeable.');
				     end;
				'a': begin
					writeln;
					writeln('Select the article for your object:');
					writeln;
					writeln('0)	None                ex: " You have taken Excalibur "');
					writeln('1)	"a"                 ex: " You have taken a small box "');
					writeln('2)	"an"                ex: " You have taken an empty bottle "');
					writeln('3)	"some"              ex: " You have picked up some jelly beans "');
					writeln('4)     "the"               ex: " You have picked up the Scepter of Power"');
					writeln;
					grab_line('? ',s);
					if isnum(s) then begin
						n := number(s);
						if n in [0..4] then begin
							getobj(objnum);
							obj.particle := n;
							putobj;
						end else
							writeln('Out of range.');
					end else
						writeln('No changes.');
				     end;
				'k': begin
					prog_kind(objnum);
				     end;
				'p': begin
					prog_obj(objnum);
				     end;
				'd': begin
					newdsc := obj.examine;
					writeln('[ Editing the description of the object ]');
					if edit_desc(newdsc) then begin
						getobj(objnum);
						obj.examine := newdsc;
						putobj;
					end;
				     end;
				'x': begin
					newdsc := obj.d1;
					writeln('[ Editing extra description #1 ]');
					if edit_desc(newdsc) then begin
						getobj(objnum);
						obj.d1 := newdsc;
						putobj;
					end;
				     end;
				'5': begin
					newdsc := obj.d2;
					writeln('[ Editing extra description #2 ]');
					if edit_desc(newdsc) then begin
						getobj(objnum);
						obj.d2 := newdsc;
						putobj;
					end;
				     end;
				'f': begin
					newdsc := obj.getfail;
					writeln('[ Editing the get failure description ]');
					if edit_desc(newdsc) then begin
						getobj(objnum);
						obj.getfail := newdsc;
						putobj;
					end;
				     end;
				'1': begin
					newdsc := obj.getsuccess;
					writeln('[ Editing the get success description ]');
					if edit_desc(newdsc) then begin
						getobj(objnum);
						obj.getsuccess := newdsc;
						putobj;
					end;
				     end;
				'3': begin
					newdsc := obj.usefail;
					writeln('[ Editing the use failure description ]');
					if edit_desc(newdsc) then begin
						getobj(objnum);
						obj.usefail := newdsc;
						putobj;
					end;
				     end;
				'4': begin
					newdsc := obj.usesuccess;
					writeln('[ Editing the use success description ]');
					if edit_desc(newdsc) then begin
						getobj(objnum);
						obj.usesuccess := newdsc;
						putobj;
					end;
				     end;
				'l': begin
					writeln('Enter a one line description of what the object will look like in any room.');
					writeln('Example: "There is an as unyet described object here."');
					writeln;
					getobj(objnum);
					freeobj;
					n := obj.linedesc;
					make_line(n);
					getobj(objnum);
					obj.linedesc := n;
					putobj;
				     end;
				otherwise writeln('-- Bad command, type ? for a list');
			end;
		until done;
		log_event(myslot,E_OBJDONE,objnum,0);

	end else
		writeln('You are not allowed to program that object.');
	end else
		writeln('There is no object by that name.');
	end;
end;


{ returns TRUE if anything was actually dropped }
function drop_everything;
{ forward function drop_everything(pslot: integer := 0): boolean; }
var
	i: integer;
	slot: integer;
	didone: boolean;
	theobj: integer;
	tmp: string;

begin
	if pslot = 0 then
		pslot := myslot;

	gethere;
	didone := false;

	mywield := 0;
	mywear := 0;

	for i := 1 to maxhold do begin
		if here.people[pslot].holding[i] <> 0 then begin
			didone := true;
			theobj := here.people[pslot].holding[i];
			slot := find_hold(theobj,pslot);
			if place_obj(theobj,TRUE) then begin
				drop_obj(slot,pslot);
			end else begin	{ no place to put it, it's lost .... }
				getobj(theobj);
				obj.numexist := obj.numexist - 1;
				putobj;
				tmp := obj.oname;
				writeln('The ',tmp,' was lost.');
			end;
		end;
	end;

	drop_everything := didone;
end;

procedure do_endplay(lognum: integer;ping:boolean := FALSE);

{ If update is true do_endplay will update the "last play" date & time
  we don't want to do this if this endplay is called from a ping }

begin
	if not(ping) then begin
			{ Set the "last date & time of play" }
		getdate;
		adate.idents[lognum] := sysdate + ' ' + systime;
		putdate;
	end;


	{ Put the player to sleep.  Don't delete his information,
	  so it can be restored the next time they play. }

	getindex(I_ASLEEP);
	indx.free[lognum] := true;	{ Yes, I'm asleep }
	putindex;
end;


function check_person(n: integer;id: string):boolean;

begin
	inmem := false;
	gethere;
	if here.people[n].username = id then
		check_person := true
	else
		check_person := false;
end;


function nuke_person(n: integer;id: string): boolean;
var
	lognum: integer;
	tmp: string;

begin
	getroom;
	if here.people[n].username = id then begin

			{ drop everything they're carrying }
		drop_everything(n);

		tmp := here.people[n].username;
			{ we'll need this for do_endplay }

			{ Remove the person from the room }
		here.people[n].kind := 0;
		here.people[n].username := '';
		here.people[n].name := '';
		putroom;

			{ update the log entries for them }
			{ but first we have to find their log number
			  (mylog for them).  We can do this with a lookup_user
			  give the userid we got above }

		if lookup_user(lognum,tmp) then begin
			do_endplay(lognum,TRUE);
				{ TRUE tells do_endplay not to update the
				  "time of last play" information 'cause we
				  don't know how long the "zombie" has been
				  there. }
		end else
			writeln('%error in nuke_person; can''t fing their log number; notify the Monster Manager');

		nuke_person := true;
	end else begin
		freeroom;
		nuke_person := false;
	end;
end;


function ping_player(n:integer;silent: boolean := false): boolean;
var
	retry: integer;
	id: string;
	idname: string;

begin
	ping_player := false;

	id := here.people[n].username;
	idname := here.people[n].name;

	retry := 0;
	ping_answered := false;

	repeat
		retry := retry + 1;
		if not(silent) then
			writeln('Sending ping # ',retry:1,' to ',idname,' . . .');

		log_event(myslot,E_PING,n,0,myname);
		wait(1);
		checkevents(TRUE);
				{ TRUE = don't reprint prompt }

		if not(ping_answered) then
			if check_person(n,id) then begin
				wait(1);
				checkevents(TRUE);
			end else
				ping_answered := true;

		if not(ping_answered) then
			if check_person(n,id) then begin
				wait(1);
				checkevents(TRUE);
			end else
				ping_answered := true;

	until (retry >= 3) or ping_answered;

	if not(ping_answered) then begin
		if not(silent) then
			writeln('That person is not responding to your pings . . .');

		if nuke_person(n,id) then begin
			ping_player := true;
			if not(silent) then
				writeln(idname,' shimmers and vanishes from sight.');
			log_event(myslot,E_PINGONE,n,0,idname);
		end else
			if not(silent) then
				writeln('That person is not a zombie after all.');
	end else
		if not(silent) then
			writeln('That person is alive and well.');
end;


procedure do_ping(s: string);
var
	n: integer;
	dummy: boolean;

begin
	if s <> '' then begin
		if parse_pers(n,s) then begin
			if n = myslot then
				writeln('Don''t ping yourself.')
			else
				dummy := ping_player(n);
		end else
			writeln('You see no person here by that name.');
	end else
		writeln('To see if someone is really alive, type PING <personal name>.');
end;

procedure list_get;
var
	first: boolean;
	i: integer;

begin
	first := true;
	for i := 1 to maxobjs do begin
		if (here.objs[i] <> 0) and
		   (here.objhide[i] = 0) then begin
			if first then begin
				writeln('Objects that you see here:');
				first := false;
			end;
			writeln('   ',obj_part(here.objs[i]));
		end;
	end;
	if first then
		writeln('There is nothing you see here that you can get.');
end;



{ print the get success message for object number n }

procedure p_getsucc(n: integer);

begin
	{ we assume getobj has already been done }
	if (obj.getsuccess = 0) or (obj.getsuccess = DEFAULT_LINE) then
		writeln('Taken.')
	else
		print_desc(obj.getsuccess);
end;


procedure do_meta_get(n: integer);
var
	slot: integer;

begin
	if obj_here(n) then begin
		if can_hold then begin
			slot := find_obj(n);
			if take_obj(n,slot) then begin
				hold_obj(n);
				log_event(myslot,E_GET,0,0,
{ >>> }		myname + ' has picked up ' + obj_part(n) + '.');
				p_getsucc(n);
			end else
				writeln('Someone got to it before you did.');
		end else
			writeln('Your hands are full.  You''ll have to drop something you''re carrying first.');
	end else if obj_hold(n) then
		writeln('You''re already holding that item.')
	else
		writeln('That item isn''t in an obvious place.');
end;


procedure do_get(s: string);
var
	n: integer;
	ok: boolean;

begin
	if s = '' then begin
		list_get;
	end else if parse_obj(n,s,TRUE) then begin
		getobj(n);
		freeobj;
		ok := true;

		if obj.sticky then begin
			ok := false;
			log_event(myslot,E_FAILGET,n,0);
			if (obj.getfail = 0) or (obj.getfail = DEFAULT_LINE) then
				writeln('You can''t take ',obj_part(n,FALSE),'.')
			else
				print_desc(obj.getfail);
		end else if obj.getobjreq > 0 then begin
			if not(obj_hold(obj.getobjreq)) then begin
				ok := false;
				log_event(myslot,E_FAILGET,n,0);
				if (obj.getfail = 0) or (obj.getfail = DEFAULT_LINE) then
					writeln('You''ll need something first to get the ',obj_part(n,FALSE),'.')
				else
					print_desc(obj.getfail);
			end;
		end;

		if ok then
			do_meta_get(n);		{ get the object }

	end else if lookup_detail(n,s) then begin
			writeln('That detail of this room is here for the enjoyment of all Monster players,');
			writeln('and may not be taken.');
	end else
		writeln('There is no object here by that name.');
end;


procedure do_drop(s: string);
var
	slot,n: integer;

begin
	if s = '' then begin
		writeln('To drop an object, type DROP <object name>.');
		writeln('To see what you are carrying, type INV (inventory).');
	end else if parse_obj(n,s) then begin
		if obj_hold(n) then begin
			getobj(n);
			freeobj;
			if obj.sticky then
				writeln('You can''t drop sticky objects.')
			else if can_drop then begin
				slot := find_hold(n);
				if place_obj(n) then begin
					drop_obj(slot);
					log_event(myslot,E_DROP,0,n,
						myname + ' has dropped '+obj_part(n) + '.');

					if mywield = n then begin
						mywield := 0;
						getroom;
						here.people[myslot].wielding := 0;
						putroom;
					end;
					if mywear = n then begin
						mywear := 0;
						getroom;
						here.people[myslot].wearing := 0;
						putroom;
					end;
				end else
					writeln('Someone took the spot where your were going to drop it.');
			end else
				writeln('It is too cluttered here.  Find somewhere else to drop your things.');
		end else begin
			writeln('You''re not holding that item.  To see what you''re holding, type INV.');
		end;
	end else
		writeln('You''re not holding that item.  To see what you''re holding, type INVENTORY.');
end;


procedure do_inv(s: string);
var
	first: boolean;
	i,n: integer;
	objnum: integer;

begin
	gethere;
	if s = '' then begin
		noisehide(50);
		first := true;
		log_event(myslot,E_INVENT,0,0);
		for i := 1 to maxhold do begin
			objnum := here.people[myslot].holding[i];
			if objnum <> 0 then begin
				if first then begin
					writeln('You are holding:');
					first := false;
				end;
				writeln('   ',obj_part(objnum));
			end;
		end;
		if first then
			writeln('You are empty handed.');
	end else if parse_pers(n,s) then begin
		first := true;
		log_event(myslot,E_LOOKYOU,n,0);
		for i := 1 to maxhold do begin
			objnum := here.people[n].holding[i];
			if objnum <> 0 then begin
				if first then begin
					writeln(here.people[n].name,' is holding:');
					first := false;
				end;
				writeln('   ',objnam.idents[ objnum ]);
			end;
		end;
		if first then
			writeln(here.people[n].name,' is empty handed.');
	end else
		writeln('To see what someone else is carrying, type INV <personal name>.');
end;


{ translate a personal name into a real userid on request }

procedure do_whois(s: string);
var
	n: integer;

begin
	if lookup_pers(n,s) then begin
		getuser;
		freeuser;
{		getpers;
		freepers;	! Already done in lookup_pers !		}

		writeln(pers.idents[n],' is ',user.idents[n],'.');
	end else
		writeln('There is no one playing with that personal name.');
end;


procedure do_players(s: string);
var
	i,j: integer;
	tmpasleep: indexrec;
	where_they_are: intrec;

begin
	log_event(myslot,E_PLAYERS,0,0);
	getindex(I_ASLEEP);	{ Rec of bool; False if playing now }
	freeindex;
	tmpasleep := indx;

	getindex(I_PLAYER);	{ Rec of valid player log records  }
	freeindex;		{ False if a valid player log }

	getuser;		{ Corresponding userids of players }
	freeuser;

	getpers;		{ Personal names of players }
	freepers;

	getdate;		{ date of last play }
	freedate;

	if privd then begin
		getint(N_LOCATION);
		freeint;
		where_they_are := anint;

		getnam;
		freenam;
	end;

	getint(N_SELF);
	freeint;

	writeln;
	writeln('Userid          Personal Name              Last Play');
	for i := 1 to maxplayers do begin
		if not(indx.free[i]) then begin
			write(user.idents[i]);
			for j := length(user.idents[i]) to 15 do
				write(' ');
			write(pers.idents[i]);
			for j := length(pers.idents[i]) to 21 do
				write(' ');

			if tmpasleep.free[i] then begin
				write(adate.idents[i]);
				if length(adate.idents[i]) < 19 then
					for j := length(adate.idents[i]) to 18 do
						write(' ');
			end else
				write('   -playing now-   ');

			if (anint.int[i] <> 0) and (anint.int[i] <> DEFAULT_LINE) then
				write(' * ')
			else
				write('   ');

			if privd then begin
				write(nam.idents[ where_they_are.int[i] ]);
			end;
			writeln;
		end;
	end;
	writeln;
end;


procedure do_self(s: string);
var
	n: integer;

begin
	if length(s) = 0 then begin
		log_action(c_self,0);
		writeln('[ Editing your self description ]');
		if edit_desc(myself) then begin
			getroom;
			here.people[myslot].self := myself;
			putroom;
			getint(N_SELF);
			anint.int[mylog] := myself;
			putint;
			log_event(myslot,E_SELFDONE,0,0);
		end;
	end else if lookup_pers(n,s) then begin
		getint(N_SELF);
		freeint;
		if (anint.int[n] = 0) or (anint.int[n] = DEFAULT_LINE) then
			writeln('That person has not made a self-description.')
		else begin
			print_desc(anint.int[n]);
			log_event(myslot,E_VIEWSELF,0,0,pers.idents[n]);
		end;
	end else
		writeln('There is no person by that name.');
end;


procedure do_health(s: string);

begin
	write('You ');
	case myhealth of
		9: writeln('are in exceptional health.');
		8: writeln('are in better than average condition.');
		7: writeln('are in perfect health.');
		6: writeln('feel a little bit dazed.');
		5: writeln('have some minor cuts and abrasions.');
		4: writeln('have some wounds, but are still fairly strong.');
		3: writeln('are suffering from some serious wounds.'); 
		2: writeln('are very badly wounded.');
		1: writeln('have many serious wounds, and are near death.');
		0: writeln('are dead.');
		otherwise writeln('don''t seem to be in any condition at all.');
	end;
end;


procedure crystal_look(chill_msg: integer);
var
	numobj,numppl,numsee: integer;
	i: integer;
	yes: boolean;

begin
	writeln;
	print_desc(here.primary);
	log_event(0,E_CHILL,chill_msg,0,'',here.locnum);
	numppl := find_numpeople;
	numsee := n_can_see + 1;

	if numppl > numsee then
		writeln('Someone is hiding here.')
	else if numppl = 0 then begin
		writeln('Strange, empty shadows swirl before your eyes.');
	end;
	if rnd100 > 50 then
		people_header('at this place.')
	else case numppl of
			0: writeln('Vague empty forms drift through your view.');
			1: writeln('You can make out a shadowy figure here.');
			2: writeln('There are two dark figures here.');
			3: writeln('You can see the silhouettes of three people.');
			otherwise
				writeln('Many dark figures can be seen here.');
	end;

	numobj := find_numobjs;
	if rnd100 > 50 then begin
		if rnd100 > 50 then
			show_objects
		else if numobj > 0 then
			writeln('Some objects are here.')
		else
			writeln('There are no objects here.');
	end else begin
		yes := false;
		for i := 1 to maxobjs do
			if here.objhide[i] <> 0 then
				yes := true;
		if yes then
			writeln('Something is hidden here.');
	end;
	writeln;
end;


procedure use_crystal(objnum: integer);
var
	done: boolean;
	s: string;
	n: integer;
	done_msg,chill_msg: integer;
	tmp: string;
	i: integer;

begin
	if obj_hold(objnum) then begin
		log_action(e_usecrystal,0);
		getobj(objnum);
		freeobj;
		done_msg := obj.d1;
		chill_msg := obj.d2;

		grab_line('',s);
		if lookup_room(n,s) then begin
			gethere(n);
			crystal_look(chill_msg);
			done := false;
		end else
			done := true;

		while not(done) do begin
			grab_line('',s);
			if lookup_dir(n,s) then begin
				if here.exits[n].toloc > 0 then begin
					gethere(here.exits[n].toloc);
					crystal_look(chill_msg);
				end;
			end else begin
				s := lowcase(s);
				tmp := bite(s);
				if tmp = 'poof' then begin
					if lookup_room(n,s) then begin
						gethere(n);
						crystal_look(chill_msg);
					end else
						done := true;
				end else if tmp = 'say' then begin
					i := (rnd100 mod 4) + 1;
					log_event(0,E_NOISE2,i,0,'',n);
				end else
					done := true;
			end;
		end;

		gethere;
		log_event(myslot,E_DONECRYSTALUSE,0,0);
		print_desc(done_msg);
	end else
		writeln('You must be holding it first.');
end;



procedure p_usefail(n: integer);

begin
	{ we assume getobj has already been done }
	if (obj.usefail = 0) or (obj.usefail = DEFAULT_LINE) then
		writeln('It doesn''t work for some reason.')
	else
		print_desc(obj.usefail);
end;


procedure p_usesucc(n: integer);

begin
	{ we assume getobj has already been done }
	if (obj.usesuccess = 0) or (obj.usesuccess = DEFAULT_LINE) then
		writeln('It seems to work, but nothing appears to happen.')
	else
		print_desc(obj.usesuccess);
end;


procedure do_use(s: string);
var
	n: integer;

begin
	if length(s) = 0 then
		writeln('To use an object, type USE <object name>')
	else if parse_obj(n,s) then begin
		getobj(n);
		freeobj;

		if (obj.useobjreq > 0) and not(obj_hold(obj.useobjreq)) then begin
			log_event(myslot,E_FAILUSE,n,0);
			p_usefail(n);
		end else if (obj.uselocreq > 0) and (location <> obj.uselocreq) then begin
			log_event(myslot,E_FAILUSE,n,0);
			p_usefail(n);
		end else begin
			p_usesucc(n);
			case obj.kind of
				O_BLAND:;
				O_CRYSTAL: use_crystal(n);
				otherwise ;
			end;
		end;
	end else
		writeln('There is no such object here.');
end;


procedure do_whisper(s: string);
var
	n: integer;

begin
	if length(s) = 0 then begin
		writeln('To whisper to someone, type WHISPER <personal name>.');
	end else if parse_pers(n,s) then begin
		if n = myslot then
			writeln('You can''t whisper to yourself.')
		else begin
			grab_line('>> ',s);
			if length(s) > 0 then begin
				nice_say(s);
				log_event(myslot,E_WHISPER,n,0,s);
			end else
				writeln('Nothing whispered.');
		end;
	end else
		writeln('No such person can be seen here.');
end;


procedure do_wield(s: string);
var
	tmp: string;
	slot,n: integer;

begin
	if length(s) = 0 then begin	{ no parms means unwield }
		if mywield = 0 then
			writeln('You are not wielding anything.')
		else begin
			getobj(mywield);
			freeobj;
			tmp := obj.oname;
			log_event(myslot,E_UNWIELD,0,0,tmp);
			writeln('You are no longer wielding the ',tmp,'.');

			mywield := 0;
			getroom;
			here.people[mylog].wielding := 0;
			putroom;
		end;
	end else if parse_obj(n,s) then begin
		if mywield <> 0 then begin
			writeln('You are already wielding ',obj_part(mywield),'.');
		end else begin
			getobj(n);
			freeobj;
			tmp := obj.oname;
			if obj.kind = O_WEAPON then begin
				if obj_hold(n) then begin
					mywield := n;
					getroom;
					here.people[myslot].wielding := n;
					putroom;

					log_event(myslot,E_WIELD,0,0,tmp);
					writeln('You are now wielding the ',tmp,'.');
				end else
					writeln('You must be holding it first.');
			end else
			writeln('That is not a weapon.');
		end;
	end else
		writeln('No such weapon can be seen here.');
end;


procedure do_wear(s: string);
var
	tmp: string;
	slot,n: integer;

begin
	if length(s) = 0 then begin	{ no parms means unwield }
		if mywear = 0 then
			writeln('You are not wearing anything.')
		else begin
			getobj(mywear);
			freeobj;
			tmp := obj.oname;
			log_event(myslot,E_UNWEAR,0,0,tmp);
			writeln('You are no longer wearing the ',tmp,'.');

			mywear := 0;
			getroom;
			here.people[mylog].wearing := 0;
			putroom;
		end;
	end else if parse_obj(n,s) then begin
		getobj(n);
		freeobj;
		tmp := obj.oname;
		if (obj.kind = O_ARMOR) or (obj.kind = O_CLOAK) then begin
			if obj_hold(n) then begin
				mywear := n;
				getroom;
				here.people[mylog].wearing := n;
				putroom;

				log_event(myslot,E_WEAR,0,0,tmp);
				writeln('You are now wearing the ',tmp,'.');
			end else
				writeln('You must be holding it first.');
		end else
			writeln('That cannot be worn.');
	end else
		writeln('No such thing can be seen here.');
end;


procedure do_brief;

begin
	brief := not(brief);
	if brief then
		writeln('Brief descriptions.')
	else
		writeln('Verbose descriptions.');
end;


function p_door_key(n: integer): string;

begin
	if n = 0 then
		p_door_key := '<none>'
	else
		p_door_key := objnam.idents[n];
end;



procedure anal_exit(dir: integer);

begin
	if (here.exits[dir].toloc = 0) and (here.exits[dir].kind <> 5) then
		{ no exit here, don't print anything }
	else with here.exits[dir] do begin
		write(direct[dir]);
		if length(alias) > 0 then begin
			write('(',alias);
			if reqalias then
				write(' required): ')
			else
				write('): ');
		end else
			write(': ');

		if (toloc = 0) and (kind = 5) then
			write('accept, no exit yet')
		else if toloc > 0 then begin
			write('to ',nam.idents[toloc],', ');
			case kind of
				0: write('no exit');
				1: write('open passage');
				2: write('door, key=',p_door_key(objreq));
				3: write('~door, ~key=',p_door_key(objreq));
				4: write('exit open randomly');
				5: write('potential exit');
				6: write('xdoor, key=',p_door_key(objreq));
				7: begin
					write('timed exit, now ');
					if cycle_open then
						write('open')
					else
						write('closed');
				   end;
			end;
			if hidden <> 0 then
				write(', hidden');
			if reqverb then
				write(', reqverb');
			if not(autolook) then
				write(', autolook off');
			if here.trapto = dir then
				write(', trapdoor (',here.trapchance:1,'%)');
		end;
		writeln;
	end;
end;


procedure do_s_exits;
var
	i: integer;
	accept,one: boolean;	{ accept is true if the particular exit is
				  an "accept" (other players may link there)
				  one means at least one exit was shown }

begin
	one := false;
	gethere;

	for i := 1 to maxexit do begin
		if (here.exits[i].toloc = 0) and (here.exits[i].kind = 5) then
			accept := true
		else
			accept := false;

		if (can_alter(i)) or (accept) then begin
			if not(one) then begin	{ first time we do this then }
				getnam;		{ read room name list in }
				freenam;
				getobjnam;
				freeobjnam;
			end;
			one := true;
			anal_exit(i);
		end;
	end;

	if not(one) then
		writeln('There are no exits here which you may inspect.');
end;


procedure do_s_object(s: string);
var
	n: integer;
	x: objectrec;

begin
	if length(s) = 0 then begin
		grab_line('Object? ',s);
	end;

	if lookup_obj(n,s) then begin
		if obj_owner(n,TRUE) then begin
			write(obj_part(n),': ');
			write(objown.idents[n],' is owner');
			x := obj;

			if x.sticky then
				write(', sticky');
			if x.getobjreq > 0 then
				write(', ',obj_part(x.getobjreq),' required to get');
			if x.useobjreq > 0 then
				write(', ',obj_part(x.useobjreq),' required to use');
			if x.uselocreq > 0 then begin
				getnam;
				freenam;
				write(', used only in ',nam.idents[x.uselocreq]);
			end;
			if x.usealias <> '' then begin
				write(', use="',x.usealias,'"');
				if x.reqalias then
					write(' (required)');
			end;

			writeln;
		end else
			writeln('You are not allowed to see the internals of that object.');
	end else
		writeln('There is no such object.');
end;


procedure do_s_details;
var
	i: integer;
	one: boolean;

begin
	gethere;
	one := false;
	for i := 1 to maxdetail do
		if (here.detail[i] <> '') and (here.detaildesc[i] <> 0) then begin
			if not(one) then begin
				one := true;
				writeln('Details here that you may inspect:');
			end;
			writeln('    ',here.detail[i]);
		end;
	if not(one) then
		writeln('There are no details of this room that you can inspect.');
end;

procedure do_s_help;

begin
	writeln;
	writeln('Exits             Lists exits you can inspect here');
	writeln('Object            Show internals of an object');
	writeln('Details           Show details you can look at in this room');
	writeln;
end;


procedure s_show(n: integer;s: string);

begin
	case n of
		s_exits: do_s_exits;
		s_object: do_s_object(s);
		s_quest: do_s_help;
		s_details: do_s_details;
	end;
end;


procedure do_y_altmsg;
var
	newdsc: integer;

begin
	if is_owner then begin
		gethere;
		newdsc := here.xmsg2;
		writeln('[ Editing the alternate mystery message for this room ]');
		if edit_desc(newdsc) then begin
			getroom;
			here.xmsg2 := newdsc;
			putroom;
		end;
	end;
end;


procedure do_y_help;

begin
	writeln;
	writeln('Altmsg        Set the alternate mystery message block');
	writeln;
end;


procedure do_group1;
var
	grpnam: string;
	loc: integer;
	tmp: string;
	
begin
	if is_owner then begin
		gethere;
		if here.grploc1 = 0 then
			writeln('No primary group location set')
		else begin
			getnam;
			freenam;
			writeln('The primary group location is ',nam.idents[here.grploc1],'.');
			writeln('Descriptor string: [',here.grpnam1,']');
		end;
		writeln;
		writeln('Type * to turn off the primary group location');
		grab_line('Room name of primary group? ',grpnam);
		if length(grpnam) = 0 then
			writeln('No changes.')
		else if grpnam = '*' then begin
			getroom;
			here.grploc1 := 0;
			putroom;
		end else if lookup_room(loc,grpnam) then begin
			writeln('Enter the descriptive string.  It will be placed after player names.');
			writeln('Example:  Monster Manager is [descriptive string, instead of "here."]');
			writeln;
			grab_line('Enter string? ',tmp);
			if length(tmp) > shortlen then begin
				writeln('Your string was truncated to ',shortlen:1,' characters.');
				tmp := substr(tmp,1,shortlen);
			end;
			getroom;
			here.grploc1 := loc;
			here.grpnam1 := tmp;
			putroom;
		end else
			writeln('No such room.');
	end;
end;



procedure do_group2;
var
	grpnam: string;
	loc: integer;
	tmp: string;
	
begin
	if is_owner then begin
		gethere;
		if here.grploc2 = 0 then
			writeln('No secondary group location set')
		else begin
			getnam;
			freenam;
			writeln('The secondary group location is ',nam.idents[here.grploc1],'.');
			writeln('Descriptor string: [',here.grpnam1,']');
		end;
		writeln;
		writeln('Type * to turn off the secondary group location');
		grab_line('Room name of secondary group? ',grpnam);
		if length(grpnam) = 0 then
			writeln('No changes.')
		else if grpnam = '*' then begin
			getroom;
			here.grploc2 := 0;
			putroom;
		end else if lookup_room(loc,grpnam) then begin
			writeln('Enter the descriptive string.  It will be placed after player names.');
			writeln('Example:  Monster Manager is [descriptive string, instead of "here."]');
			writeln;
			grab_line('Enter string? ',tmp);
			if length(tmp) > shortlen then begin
				writeln('Your string was truncated to ',shortlen:1,' characters.');
				tmp := substr(tmp,1,shortlen);
			end;
			getroom;
			here.grploc2 := loc;
			here.grpnam2 := tmp;
			putroom;
		end else
			writeln('No such room.');
	end;
end;


procedure s_set(n: integer;s: string);

begin
	case n of
		y_quest: do_y_help;
		y_altmsg: do_y_altmsg;
		y_group1: do_group1;
		y_group2: do_group2;
	end;
end;


procedure do_show(s: string);
var
	n: integer;
	cmd: string;

begin
	cmd := bite(s);
	if length(cmd) = 0 then
		grab_line('Show what attribute? (type ? for a list) ',cmd);

	if length(cmd) = 0 then
	else if lookup_show(n,cmd) then
		s_show(n,s)
	else
		writeln('Invalid show option, type SHOW ? for a list.');
end;


procedure do_set(s: string);
var
	n: integer;
	cmd: string;

begin
	cmd := bite(s);
	if length(cmd) = 0 then
		grab_line('Set what attribute? (type ? for a list) ',cmd);

	if length(cmd) = 0 then
	else if lookup_set(n,cmd) then
		s_set(n,s)
	else
		writeln('Invalid set option, type SET ? for a list.');
end;


procedure parser;
var
	s: string;
	cmd: string;
	n: integer;
	dummybool: boolean;

begin
   repeat
	grab_line('> ',s);
	s := slead(s);
   until length(s) > 0;

	if s = '.' then
		s := oldcmd
	else
		oldcmd := s;

	if (s[1]='''') and (length(s) > 1) then
		do_say(substr(s,2,length(s)-1))
	else begin
		cmd := bite(s);
		case lookup_cmd(cmd) of
{ try exit alias }	error:begin
				if (lookup_alias(n,cmd)) or
				   (lookup_dir(n,cmd)) then begin
					do_go(cmd);
				end else
					writeln('Bad command, type ? for a list.');
			end;

			setnam: do_setname(s);
			help,quest: show_help;
			quit: done := true;
			c_l,look: do_look(s);
			go: do_go(s,FALSE);	{ FALSE = dir not a verb }
			form: do_form(s);
			link: do_link(s);
			unlink: do_unlink(s);
			poof: do_poof(s);
			desc: do_describe(s);
			say: do_say(s);
			c_rooms: do_rooms(s);
			c_claim: do_claim(s);
			c_disown: do_disown(s);
			c_public: do_public(s);
			c_accept: do_accept(s);
			c_refuse: do_refuse(s);
			c_zap: do_zap(s);

			c_north,c_n,
			c_south,c_s,
			c_east,c_e,
			c_west,c_w,
			c_up,c_u,
			c_down,c_d: do_go(cmd);

			c_who: do_who;
			c_custom: do_custom(s);
			c_search: do_search(s);
			c_system: do_system(s);
			c_hide: do_hide(s);
			c_unhide: do_unhide(s);
			c_punch: do_punch(s);
			c_ping: do_ping(s);
			c_create: do_makeobj(s);
			c_get: do_get(s);
			c_drop: do_drop(s);
			c_i,c_inv: do_inv(s);
			c_whois: do_whois(s);
			c_players: do_players(s);
			c_health: do_health(s);
			c_duplicate: do_duplicate(s);
			c_version: do_version(s);
			c_objects: do_objects;
			c_self: do_self(s);
			c_use: do_use(s);
			c_whisper: do_whisper(s);
			c_wield: do_wield(s);
			c_brief: do_brief;
			c_wear: do_wear(s);
			c_destroy: do_destroy(s);
			c_relink: do_relink(s);
			c_unmake: do_unmake(s);
			c_show: do_show(s);
			c_set: do_set(s);

			dbg: begin
				debug := not(debug);
				if debug then
					writeln('Debugging is on.')
				else
					writeln('Debugging is off.');
			     end;
			otherwise begin
				writeln('%Parser error, bad return from lookup');
			end;
		end;
		clear_command;
	end;
end;



procedure init;
var
	i: integer;

begin
	rndcycle := 0;
	location := 1;		{ Great Hall }
        
	mywield := 0;		{ not initially wearing or weilding any weapon }
	mywear := 0;
	myhealth := 7;		{ how healthy they are to start }
	healthcycle := 0;	{ pretty much meaningless at the start }

	userid := lowcase(get_userid);
	if (userid = MM_userid) then begin
		myname := 'Monster Manager';
		privd := true;
	end else if (userid = MVM_userid) then begin
		privd := true;
		myname := 'Vice Manager';
	end else if (userid = FAUST_userid) then begin
		privd := true;
	end else begin
		myname := lowcase(userid);
		myname[1] := chr( ord('A') + (ord(myname[1]) - ord('a'))   );
		privd := false;
	end;

	numcmds:= 66;

	show[s_exits] := 'exits';
	show[s_object] := 'object';
	show[s_quest] := '?';
	show[s_details] := 'details';
	numshow := 4;

	setkey[y_quest] := '?';
	setkey[y_altmsg] := 'altmsg';
	setkey[y_group1] := 'group1';
	setkey[y_group2] := 'group2';
	numset := 4;

	numspells := 0;

	open(roomfile,root+'ROOMS.MON',access_method := direct,
		sharing := readwrite,
		history := unknown);
	open(namfile,root+'NAMS.MON',access_method := direct,
		sharing := readwrite,
		history := unknown);
	open(eventfile,root+'EVENTS.MON',access_method := direct,
		sharing := readwrite,
		history := unknown);
	open(descfile,root+'DESC.MON',access_method := direct,
		sharing := readwrite,
		history := unknown);
	open(indexfile,root+'INDEX.MON',access_method := direct,
		sharing := readwrite,
		history := unknown);
	open(linefile,root+'LINE.MON',access_method := direct,
		sharing := readwrite,
		history := unknown);
	open(intfile,root+'INTFILE.MON',access_method := direct,
		sharing := readwrite,
		history := unknown);
	open(objfile,root+'OBJECTS.MON',access_method := direct,
		sharing := readwrite,
		history := unknown);
	open(spellfile,root+'SPELLS.MON',access_method := direct,
		sharing := readwrite,
		history := unknown);
end;


procedure prestart;
var
	s: string;

begin
	write('Welcome to Monster!  Hit return to start: ');
	readln(s);
	writeln;
	writeln;
	if length(s) > 0 then
		special(lowcase(s));
end;


procedure welcome_back(var mylog: integer);
var
	tmp: string;
	sdate,stime: shortstring;

begin
	getdate;
	freedate;

	write('Welcome back, ',myname,'.');
	if length(myname) > 18 then
		writeln;

	write('  Your last play was on');

	if length(adate.idents[mylog]) < 11 then begin
		writeln(' ???');
	end else begin
		sdate := substr(adate.idents[mylog],1,11);	{ extract the date }
		if length(adate.idents[mylog]) = 19 then
			stime := substr(adate.idents[mylog],13,7)
		else
			stime := '???';

		if sdate[1] = ' ' then
			tmp := sdate
		else
			tmp := ' ' + sdate;

		if stime[1] = ' ' then
			tmp := tmp + ' at' + stime
		else
			tmp := tmp + ' at ' + stime;
		writeln(tmp,'.');
	end;
	writeln;
end;


function loc_ping:boolean;
var
	i: integer;
	found: boolean;

begin
	inmem := false;
	gethere;

	i := 1;
	found := false;

		{ first get the slot that the supposed "zombie" is in }
	while (not found) and (i <= maxpeople) do begin
		if here.people[i].name = myname then
			found := true
		else
			i := i + 1;
	end;

	myslot := 0;	{ setup for ping_player }

	if found then begin
		setevent;
		loc_ping := ping_player(i,TRUE);  { TRUE = silent operation }
	end else
		loc_ping := true;
			{ well, if we can't find them, let's assume
			  that they're not in any room records, so they're
			  ok . . . Let's hope... }
end;



{ attempt to fix the player using loc_ping if the database incorrectly
  shows someone playing who isn' playing }

function fix_player:boolean;
var
	ok: boolean;

begin
	writeln('There may have been some trouble the last time you played.');
	writeln('Trying to fix it . . .');
	if loc_ping then begin
		writeln('All should be fixed now.');
		writeln;
		fix_player := true;
	end else begin
		writeln('Either someone else is playing Monster on your account, or something is');
		writeln('very wrong with the database.');
		writeln;
		fix_player := false;
	end;
end;


function revive_player(var mylog: integer): boolean;
var
	ok: boolean;
	i,n: integer;

begin
	if exact_user(mylog,userid) then begin	{ player has played before }
		getint(N_LOCATION);
		freeint;
		location := anint.int[mylog];	{ Retrieve their old loc }

		getpers;
		freepers;
		myname := pers.idents[mylog];	{ Retrieve old personal name }

		getint(N_EXPERIENCE);
		freeint;
		myexperience := anint.int[mylog];

		getint(N_SELF);
		freeint;
		myself := anint.int[mylog];

		getindex(I_ASLEEP);
		freeindex;

		if indx.free[mylog] then begin
				{ if player is asleep, all is well }
			ok := true;
		end else begin
				{ otherwise, there is one of two possibilities:
					1) someone on the same account is
					   playing Monster
					2) his last play terminated abnormally
				}
			ok := fix_player;
		end;

		if ok then
			welcome_back(mylog);

	end else begin	{ must allocate a log block for the player }
		if alloc_log(mylog) then begin

			writeln('Welcome to Monster, ',myname,'!');
			writeln('You will start in the Great Hall.');
			writeln;

			{ Store their userid }
			getuser;
			user.idents[mylog] := lowcase(userid);
			putuser;

			{ Set their initial location }
			getint(N_LOCATION);
			anint.int[mylog] := 1;	{ Start out in Great Hall }
			putint;
			location := 1;

			getint(N_EXPERIENCE);
			anint.int[mylog] := 0;
			putint;
			myexperience := 0;

			getint(N_SELF);
			anint.int[mylog] := 0;
			putint;
			myself := 0;

				{ initialize the record containing the
				  level of each spell they have to start;
				  all start at zero; since the spellfile is
				  directly parallel with mylog, we can hack
				  init it here without dealing with SYSTEM }

			locate(spellfile,mylog);
			for i := 1 to maxspells do
				spellfile^.level[i] := 0;
			spellfile^.recnum := mylog;
			put(spellfile);

			ok := true;
		end else
			ok := false;
	end;

	if ok then begin { Successful, MYLOG is my log slot }

		{ Wake up the player }
		getindex(I_ASLEEP);
		indx.free[mylog] := false;	{ I'm NOT asleep now }
		putindex;

		{ Set the "last date of play" }
		getdate;
		adate.idents[mylog] := sysdate + ' ' + systime;
		putdate;
	end else
		writeln('There is no place for you in Monster.  Contact the Monster Manager.');
	revive_player := ok;
end;


function enter_universe:boolean;
var
	orignam: string;
	dummy,i: integer;
	ok: boolean;

begin


		{ take MYNAME given to us by init or revive_player and make
		  sure it's unique.  If it isn't tack _1, _2, etc onto it 
		  until it is.  Code must come before alloc_log, or there
		  will be an invalid pers record in there cause we aren't in yet
		}
		orignam := myname;
		i := 0;
		repeat	{ tack _n onto pers name until a unique one is found }
			ok := true;

{*** Should this use exact_pers instead?  Is this a copy of exact_pers code? }

			if lookup_pers(dummy,myname) then
				if lowcase(pers.idents[dummy]) = lowcase(myname) then begin
					ok := false;
					i := i + 1;
					writev(myname,orignam,'_',i:1);
				end;
		until ok;



	if revive_player(mylog) then begin
	if put_token(location,myslot) then begin
		getpers;
		pers.idents[mylog] := myname;
		putpers;

		enter_universe := true;
		log_begin(location);
		setevent;
		do_look;
	end else begin
		writeln('put_token failed.');
		enter_universe := false;
	end;
	end else begin
		writeln('revive_player failed.');
		enter_universe := false;
	end;
end;

procedure leave_universe;
var
	diddrop: boolean;

begin
	diddrop := drop_everything;
	take_token(myslot,location);
	log_quit(location,diddrop);
	do_endplay(mylog);

	writeln('You vanish in a brilliant burst of multicolored light.');
	if diddrop then
		writeln('All of your belongings drop to the ground.');
end;


begin
	done := false;
	setup_guts;
	init;
	prestart;
	if not(done) then begin
		if enter_universe then begin
			repeat
				parser;
			until done;
			leave_universe;
		end else
			writeln('You attempt to enter the Monster universe, but a strange force repels you.');
	end;
	finish_guts;
end.


{ Notes to other who may inherit this program:

	Change all occurances in this file of dolpher to the account which
	you will use for maintenance of this program.  That account will
	have special administrative powers.

	This program uses several data files.  These files are in a directory
	specified by the variable root in procedure init.  In my implementation,
	I have a default ACL on the directory allowing everyone READ and WRITE
	access to the files created in that directory.  Whoever plays the game
	must be able to write to these data files.


Written by Rich Skrenta, 1988.




Brief program organization overview:
------------------------------------

Monster's Shared Files:

Monster uses several shared files for communication.
Each shared file is accessed within Monster by a group of 3 procedures of the
form:	getX(), freeX and putX.

getX takes an integer and attempts to get and lock that record from the
appropriate data file.  If it encounters a "collision", it waits a short
random amount of time and tries again.  After maxerr collisions it prints
a deadlock warning message.

If data is to be read but not changed, a freeX should immediately follow
the getX so that other Monster processes can access the record.  If the
record is to be written then a putX must eventually follow the getX.


Monster's Record Allocation:

Monster dynamically allocates some resources such as description blocks and
lines and player log entries.  The allocation is from a bitmap.  I chose a
bitmap over a linked list to make the multiuser access to the database
more stable.  A particular resource (such as log entries) will have a
particular bitmap in the file INDEXFILE.  A getindex(I_LOG) will retrieve
the bitmap for it.  

Actually allocation and deallocation is done through the group of functions
alloc_X and delete_X.  If alloc_X returns true, the allocation was successful,
and the integer parameter is the number of the block allocated.

The top available record in each group is stored in indexrec.  To increase
the top, the new records must be initially written so that garbage data is
not in them and the getX routines can locate them.  This can be done with
the addX(n) group of routines, which add capacity to resources.



Parsing in Monster:

The main parser(s) use a first-unique-characters method to lookup command
keywords and parameters.  The format of these functions is lookup_x(n,s).
If it returns true, it successfully found an unambiguous match to string s.
The integer index will be in n.

If an unambiguating match is needed (for example, if someone makes a new room,
the match to see if the name exists shouldn't disambiguate), the group of
routines exact_X(n,s) are called.  They function similarly to lookup_x(n,s).

The customization subsystems and the editor use very primitive parsers
which only use first character match and integer arguments.



Asynchronous events in Monster:

When someone comes into a room, the other players in that room need
to be notified, even if they might be typing a command on their terminal.

This is done in a two part process (producer/consumer problem):

When an event takes place, the player's Monster that caused the event
makes a call to log_event.  Parameters include the slot of the sender (which
person in the room caused the event), the actual event that occurred
(E_something) and parameters.  Log_event works by sticking the event
into a circular buffer associated with the room (room may be specified on
log_event).

Note: there is not an event record for every room; instead, the event
      record used is  ROOM # mod ACTUAL NUMBER of EVENT RECORDS

The other half of the process occurrs when a player's Monster calls
grab_line to get some input.  Grab line looks for keystrokes, and if
there are none, it calls checkevent and then sleeps for a short time
(.1 - .2 seconds).  Checkevent loads the event record associated with this
room and compare's the player's buffer pointer with the record's buffer
pointer.  If they are different, checkevent bites off events and sends them
to handle_event until there are no more events to be processed.  Checkevent
ignores events logged by it's own player.


}