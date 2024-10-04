/*
 *  This is a port of VMS Monster to C
 *
 *  VMS Monster was originally written in VMS Pascal.
 *  It ran on Northwestern University's VAX in 1988.
 *  See http://www.skrenta.com/monster/ for more information.
 *
 *  port version: Sat Dec 22 20:05:25 PST 2001
 * 
 *  This is a more-or-less straight port of the VMS program, and
 *  as such uses shared files for IPC between monster clients.
 *  Users must have an account on the local machine.  Each runs
 *  a separate copy of the program.  Each client spins looking at
 *  the shared files for communication; short sleeps are used in the
 *  line parser to avoid excessively slowing down the system.
 *
 *  Author: Rich Skrenta <skrenta@rt.com>
 *  December, 2001
 *
 *
 *  Installation instructions:
 *
 *	If you want to install monster for shared use on a system, then
 *		1) set the #define for root to point to the shared file
 * 		   directory
 *		2) set the #define for MONSTER_FILE to be 0660
 *		3) install monster setuid or setgid to give it access to
 *		   the files in the shared directory
 *
 *	Monster will automatically create the proto database the
 *	first time its run.  You can type "rebuild" at the first
 *	prompt to re-create the database if necessary.
 */


/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "mon.pas" */


/*

        This is Monster, a multiuser adventure game system
        where the players create the universe.

        Written by Rich Skrenta at Northwestern University, 1988.

                skrenta@nuacc.acns.nwu.edu
                skrenta@nuacc.bitnet

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <pwd.h>
#include <termio.h>


/* p2c definitions */

typedef unsigned char boolean;
typedef unsigned char uchar;

#ifndef true
# define true    1
# define false   0
#endif

#ifndef TRUE
# define TRUE    1
# define FALSE   0
#endif


/* These are PRIVILEDGED users.  The Monster Manager has the most power;
   this should be the game administrator.  The Monster Vice Manager can help
   the MM in his adminstrative duties.  Faust is another person who can do
   anything but is generally incognito. */

#define MM_userid       "dolpher"   /* Monster Manager*/
#define MVM_userid      "gary"   /* Monster Vice Manager*/
#define FAUST_userid    "skrenta"   /* Dr. Faustus*/

#define MONSTER_FILE	0666

#define REBUILD_OK      true
/* if this is TRUE, the MM can blow away
                                  and reformat the entire universe.  It's
                                  a good idea to set this to FALSE and
                                  recompile after you've got your world
                                  going */


/* #define root            "/usr/local/lib/monster" */
#define root            "."

/* this is where the Monster database goes
   This directory and the datafiles Monster
   creates in it must be world:rw for
   people to be able to play.  This sucks,
   but we don't have setgid to games on VMS */

#define veryshortlen    12   /* very short string length for userid's etc */
#define shortlen        20   /* ordinary short string */

#define maxobjs         15   /* max objects allow on floor in a room */
#define maxpeople       10   /* max people allowed in a room */
#define maxplayers      300   /* max log entries to make for players */
#define maxcmds         75   /* top value for cmd keyword slots */
#define maxshow         50   /* top value for set/show keywords */
#define maxexit         6   /* 6 exits from each loc: NSEWUD */
#define maxroom         1000   /* Total maximum ever possible*/
#define maxdetail       5   /* max num of detail keys/descriptions per room */
#define maxevent        15   /* event slots per event block */
#define maxindex        10000   /* top value for bitmap allocation */
#define maxhold         6   /* max # of things a player can be holding */
#define maxerr          15
/* # of consecutive record collisions before the
                          the deadlock error message is printed */
#define numevnts        10
    /* # of different event records to be maintained */
#define numpunches      12   /* # of different kinds of punches there are */
#define maxparm         20   /* parms for object USEs */
#define maxspells       50   /* total number of spells available */

#define descmax         10   /* lines per description block */


#define DEFAULT_LINE    32000
/* A virtual one liner record number that
                          really means "use the default one liner
                          description instead of reading one from
                          the file" */

/* Mnemonics for directions */

#define north           1
#define south           2
#define east            3
#define west            4
#define up              5
#define down            6


/* Index record mnemonics */

#define I_BLOCK         1   /* True if description block is not used*/
#define I_LINE          2   /* True if line slot is not used*/
#define I_ROOM          3   /* True if room slot is not in use*/
#define I_PLAYER        4   /* True if slot is not occupied by a player*/
#define I_ASLEEP        5   /* True if player is not playing*/
#define I_OBJECT        6   /* True if object record is not being used*/
#define I_INT           7   /* True if int record is not being used*/

/* Integer record mnemonics */

#define N_LOCATION      1   /* Player's location */
#define N_NUMROOMS      2   /* How many rooms they've made */
#define N_ALLOW         3   /* How many rooms they're allowed to make */
#define N_ACCEPT        4   /* Number of open accept exits they have */
#define N_EXPERIENCE    5   /* How "good" they are */
#define N_SELF          6   /* player's self descriptions */

/* object kind mnemonics */

#define O_BLAND         0   /* bland object, good for keys */
#define O_WEAPON        1
#define O_ARMOR         2
#define O_THRUSTER      3   /* use puts player through an exit */
#define O_CLOAK         4

#define O_BAG           100
#define O_CRYSTAL       101
#define O_WAND          102
#define O_HAND          103


/* Command Mnemonics */
#define error           0
#define setnam          1
#define help            2
#define quest           3
#define quit            4
#define look            5
#define go              6
#define form            7
#define link            8
#define unlink          9
#define c_whisper       10
#define poof            11
#define desc            12
#define dbg             14
#define say             15

#define c_rooms         17
#define c_system        18
#define c_disown        19
#define c_claim         20
#define c_create        21
#define c_public        22
#define c_accept        23
#define c_refuse        24
#define c_zap           25
#define c_hide          26
#define c_l             27
#define c_north         28
#define c_south         29
#define c_east          30
#define c_west          31
#define c_up            32
#define c_down          33
#define c_n             34
#define c_s             35
#define c_e             36
#define c_w             37
#define c_u             38
#define c_d             39
#define c_custom        40
#define c_who           41
#define c_players       42
#define c_search        43
#define c_unhide        44
#define c_punch         45
#define c_ping          46
#define c_health        47
#define c_get           48
#define c_drop          49
#define c_inv           50
#define c_i             51
#define c_self          52
#define c_whois         53
#define c_duplicate     54

#define c_version       56
#define c_objects       57
#define c_use           58
#define c_wield         59
#define c_brief         60
#define c_wear          61
#define c_relink        62
#define c_unmake        63
#define c_destroy       64
#define c_show          65
#define c_set           66

#define e_detail        100   /* pseudo command for log_action of desc exit */
#define e_custroom      101   /* customizing this room */
#define e_program       102   /* customizing (programming) an object */
#define e_usecrystal    103   /* using a crystal ball */


/* Show Mnemonics */

#define s_exits         1
#define s_object        2
#define s_quest         3
#define s_details       4


/* Set Mnemonics */

#define y_quest         1
#define y_altmsg        2
#define y_group1        3
#define y_group2        4


/* Event Mnemonics */

#define E_EXIT          1   /* player left room*/
#define E_ENTER         2   /* player entered room*/
#define E_BEGIN         3   /* player joined game here*/
#define E_QUIT          4   /* player here quit game*/

#define E_SAY           5   /* someone said something*/
#define E_SETNAM        6   /* player set his personal name*/
#define E_POOFIN        8   /* someone poofed into this room*/
#define E_POOFOUT       9   /* someone poofed out of this room*/
#define E_DETACH        10   /* a link has been destroyed*/
#define E_EDITDONE      11   /* someone is finished editing a desc*/
#define E_NEWEXIT       12   /* someone made an exit here*/
#define E_BOUNCEDIN     13   /* an object "bounced" into the room*/
#define E_EXAMINE       14   /* someone is examining something*/
#define E_CUSTDONE      15   /* someone is done customizing an exit*/
#define E_FOUND         16   /* player found something*/
#define E_SEARCH        17   /* player is searching room*/
#define E_DONEDET       18   /* done adding details to a room*/
#define E_HIDOBJ        19   /* someone hid an object here*/
#define E_UNHIDE        20   /* voluntarily revealed themself*/
#define E_FOUNDYOU      21   /* someone found someone else hiding*/
#define E_PUNCH         22   /* someone has punched someone else*/
#define E_MADEOBJ       23   /* someone made an object here*/
#define E_GET           24   /* someone picked up an object*/
#define E_DROP          25   /* someone dropped an object*/
#define E_DROPALL       26   /* quit & dropped stuff on way out*/
#define E_IHID          27   /* tell others that I have hidden (!)*/
#define E_NOISES        28   /* strange noises from hidden people*/
#define E_PING          29   /* send a ping to a potential zombie*/
#define E_PONG          30   /* ping answered*/
#define E_HIDEPUNCH     31   /* someone hidden is attacking*/
#define E_SLIPPED       32   /* attack caused obj to drop unwillingly */
#define E_ROOMDONE      33   /* done customizing this room*/
#define E_OBJDONE       34   /* done programming an object*/
#define E_HPOOFOUT      35   /* someone hiding poofedout*/
#define E_FAILGO        36   /* a player failed to go through an exit */
#define E_HPOOFIN       37   /* someone poofed into a room hidden*/
#define E_TRYPUNCH      38   /* someone failed to punch someone else*/
#define E_PINGONE       39   /* someone was pinged away . . .*/
#define E_CLAIM         40   /* someone claimed this room*/
#define E_DISOWN        41   /* owner of this room has disowned it*/
#define E_WEAKER        42   /* person is weaker from battle*/
#define E_OBJCLAIM      43   /* someone claimed an object*/
#define E_OBJDISOWN     44   /* someone disowned an object*/
#define E_SELFDONE      45   /* done editing self description*/
#define E_WHISPER       46   /* someone whispers to someone else*/
#define E_WIELD         47   /* player wields a weapon*/
#define E_UNWIELD       48   /* player puts a weapon away*/
#define E_DONECRYSTALUSE  49   /* done using the crystal ball*/
#define E_WEAR          50   /* someone has put on something*/
#define E_UNWEAR        51   /* someone has taken off something*/
#define E_DESTROY       52   /* someone has destroyed an object*/
#define E_HIDESAY       53   /* anonymous say*/
#define E_OBJPUBLIC     54   /* someone made an object public*/
#define E_SYSDONE       55   /* done with system maint. mode*/
#define E_UNMAKE        56   /* remove typedef for object*/
#define E_LOOKDETAIL    57   /* looking at a detail of this room*/
#define E_ACCEPT        58   /* made an "accept" exit here*/
#define E_REFUSE        59   /* got rid of an "accept" exit here*/
#define E_DIED          60   /* someone died and evaporated*/
#define E_LOOKYOU       61   /* someone is looking at you*/
#define E_FAILGET       62   /* someone can't get something*/
#define E_FAILUSE       63   /* someone can't use something*/
#define E_CHILL         64   /* someone scrys you*/
#define E_NOISE2        65   /* say while in crystal ball*/
#define E_LOOKSELF      66   /* someone looks at themself*/
#define E_INVENT        67   /* someone takes inventory*/
#define E_POOFYOU       68   /* MM poofs someone away . . . .*/
#define E_WHO           69   /* someone does a who*/
#define E_PLAYERS       70   /* someone does a players*/
#define E_VIEWSELF      71   /* someone views a self description*/
#define E_REALNOISE     72   /* make the real noises message print*/
#define E_ALTNOISE      73   /* alternate mystery message*/
#define E_MIDNIGHT      74   /* it's midnight now, tell everyone*/

#define E_ACTION        100   /* base command action event */


/* Misc. */

#define GOODHEALTH      7


typedef char string[81];

typedef char veryshortstring[veryshortlen + 1];

typedef char shortstring[shortlen + 1];


/* This is a list of description block numbers;
   If a number is zero, there is no text for that block */


/* exit kinds:
        0: no way - blocked exit
        1: open passageway
        2: object required

        6: exit only exists if player is holding the key
*/

typedef struct exit_ {
  int toloc;   /* location exit goes to */
  int kind;   /* type of the exit */
  int slot;   /* exit slot of toloc target */

  int exitdesc;   /* one liner description of exit  */
  int closed;   /* desc of a closed door */
  int fail;   /* description if can't go thru   */
  int success;   /* desc while going thru exit     */
  int goin;   /* what others see when you go into the exit */
  /*ofail,*/
  /* what others see when you come out of the exit */
  int comeout;   /* all refer to the liner file */
  /* if zero defaults will be printed */

  int hidden;   /* **** about to change this **** */
  int objreq;   /* object required to pass this exit */

  veryshortstring alias;   /* alias for the exit dir, a keyword */

  boolean reqverb;   /* require alias as a verb to work */
  boolean reqalias;
  /* require alias only (no direction) to
                            pass through the exit */
  boolean autolook;   /* do a look when user comes out of exit */
} exit_;


/* index record # 1 is block index */
/* index record # 2 is line index */
/* index record # 3 is room index */
/* index record # 4 is player alloc index */
/* index record # 5 is player awake (in game) index */

typedef struct indexrec {
  int indexnum;   /* validation number */
  boolean free[maxindex];
  int top;   /* max records available */
  int inuse;   /* record #s in use */
} indexrec;


/* names are record #1   */
/* owners are record # 2 */
/* player pers_names are record # 3 */
/* userids are record # 4 */
/* object names are record # 5 */
/* object creators are record # 6 */
/* date of last play is # 7 */
/* time of last play is # 8 */

typedef struct namrec {
  int validate, loctop;
  shortstring idents[maxroom];
} namrec;

typedef struct objectrec {
  int objnum;   /* allocation number for the object */
  int onum;   /* number index to objnam/objown */
  shortstring oname;   /* duplicate of name of object */
  int kind;   /* what kind of object this is */
  int linedesc;   /* liner desc of object Here */

  int home;   /* if object at home, then print the */
  int homedesc;   /* home description */

  int actindx;   /* action index -- programs for the future */
  int examine;   /* desc block for close inspection */
  int worth;   /* how much it cost to make (in gold) */
  int numexist;   /* number in existence */

  boolean sticky;   /* can they ever get it? */
  int getobjreq;   /* object required to get this object */
  int getfail;   /* fail-to-get description */
  int getsuccess;   /* successful picked up description */

  int useobjreq;   /* object require to use this object */
  int uselocreq;   /* place have to be to use this object */
  int usefail;   /* fail-to-use description */
  int usesuccess;   /* successful use of object description */

  veryshortstring usealias;
  boolean reqalias, reqverb;

  int particle;
  /* a,an,some, etc... "particle" is not
                            be right, but hey, it's in the code */

  int parms[maxparm];

  int d1;   /* extra description # 1 */
  int d2;   /* extra description # 2 */
  int exp3, exp4, exp5, exp6;
} objectrec;

typedef struct anevent {
  int sender;   /* slot of sender */
  int action;   /* what event this is, E_something */
  int target;   /* opt target of action */
  int parm;   /* expansion parm */
  string msg;   /* string for SAY and other cmds */
  int loc;   /* room that event is targeted for */
} anevent;

typedef struct eventrec {
  int validat;   /* validation number for record locking */
  anevent evnt[maxevent];
  int point;   /* circular buffer pointer */
} eventrec;

typedef struct peoplerec {
  int kind;   /* 0=none,1=player,2=npc */
  int parm;   /* index to npc controller (object?) */

  veryshortstring username;   /* actual userid of person */
  shortstring name;   /* chosen name of person */
  int hiding;   /* degree to which they're hiding */
  int act, targ;   /* last thing that this person did */

  int holding[maxhold];   /* objects being held */
  int experience;

  int wearing;   /* object that they're wearing */
  int wielding;   /* weapon they're wielding */
  int health;   /* how healthy they are */

  int self;   /* self description */

  int ex1, ex2, ex3, ex4, ex5;
} peoplerec;

typedef struct spellrec {
  int recnum;
  int level[maxspells];
} spellrec;

typedef struct descrec {
  int descrinum;
  string lines[descmax];
  int desclen;   /* number used in this block */
} descrec;

typedef struct linerec {
  int linenum;
  string theline;
} linerec;

typedef struct room {
  int valid;   /* validation number for record locking */
  int locnum;
  veryshortstring owner;
  /* who owns the room: userid if private
                                               '' if public
                                               '*' if disowned */
  string nicename;   /* pretty name for location */
  int nameprint;
  /* code for printing name:
                                  0: don't print it
                                  1: You're in
                                  2: You're at
                          */

  int primary;   /* room descriptions */
  int secondary, which;
  /* 0 = only print primary room desc.
                            1 = only print secondary room desc.
                            2 = print both
                            3 = print primary then secondary
                                  if has magic object */

  int magicobj;   /* special object for this room */
  int effects, parm;

  exit_ exits[maxexit];

  int pile;   /* if more than maxobjs objects here */
  int objs[maxobjs];   /* refs to object file */
  int objhide[maxobjs];
  /* how much each object
                                            is hidden */
  /* see hidden on exitrec
     above */

  int objdrop;   /* where objects go when they're dropped */
  int objdesc;   /* what it says when they're dropped */
  int objdest;
  /* what it says in target room when
                            "bounced" object comes in */

  peoplerec people[maxpeople];

  int grploc1, grploc2;
  shortstring grpnam1, grpnam2;

  veryshortstring detail[maxdetail];
  int detaildesc[maxdetail];

  int trapto;   /* where the "trapdoor" goes */
  int trapchance;   /* how often the trapdoor works */

  int rndmsg;   /* message that randomly prints */

  int xmsg2;   /* another random block */
  int exp2, exp3, exp4, exitfail;   /* default fail description for exits */
  int ofail;   /* what other's see when you fail, default */
} room;


typedef struct intrec {
  int intnum;
  int int_[maxplayers];
} intrec;


extern string old_prompt, line;
string oldcmd;   /* string for '.' command to do last command */

boolean inmem;
/* Is this rooms roomrec (here....) in memory?
                   We call gethere many times to make sure
                   here is current.  However, we only want to
                   actually do a getroom if the roomrec has been
                   modified*/
boolean brief = false;
/* brief/verbose descriptions */

int rndcycle;   /* integer for rnd_event */
boolean debug, ping_answered;   /* flag for ping answers */
boolean hiding = false;
/* is player hiding? */
boolean midnight_notyet = true;
/* hasn't been midnight yet */
boolean first_puttoken = true;
/* flag for first place into world */
boolean logged_act = false;
/* flag to indicate that a log_action
                                  has been called, and the next call
                                  to clear_command needs to clear the
                                  action parms in the here roomrec */

string line;
string old_prompt;

int roomfile;
int eventfile;
int namfile;
int descfile;
int linefile;
int indexfile;
int intfile;
int objfile;
int spellfile;

room      roomfile_hat;
eventrec  eventfile_hat;
namrec    namfile_hat;
descrec   descfile_hat;
linerec	  linefile_hat;
indexrec  indexfile_hat;
intrec    intfile_hat;
objectrec objfile_hat;
spellrec  spellfile_hat;

shortstring cmds[maxcmds] = {
  "name", "help", "?", "quit", "look", "go", "form", "link", "unlink",
  "whisper", "poof", "describe", "", "debug", "say", "", "rooms", "system",
  "disown", "claim", "make", "public", "accept", "refuse", "zap", "hide", "l",
  "north", "south", "east", "west", "up", "down", "n", "s", "e", "w", "u",
  "d", "customize", "who", "players", "search", "reveal", "punch", "ping",
  "health", "get", "drop", "inventory", "i", "self", "whois", "duplicate", "",
  "version", "objects", "use", "wield", "brief", "wear", "relink", "unmake",
  "destroy", "show", "set", "", "", "", "", "", "", "", "", ""
/* p2c: mon.pas, line 667: 
 * Note: Line breaker spent 0.0 seconds, 5000 tries on line 572 [251] */
};

/* setnam = 1*/
/* help = 2*/
/* quest = 3*/
/* quit = 4*/
/* look = 5*/
/* go = 6*/
/* form = 7*/
/* link = 8*/
/* unlink = 9*/
/* c_whisper = 10*/
/* poof = 11*/
/* desc = 12*/
/* dbg = 14*/
/* say = 15*/
/**/
/* c_rooms = 17*/
/* c_system = 18*/
/* c_disown = 19*/
/* c_claim = 20*/
/* c_create = 21*/
/* c_public = 22*/
/* c_accept = 23*/
/* c_refuse = 24*/
/* c_zap = 25*/
/* c_hide = 26*/
/* c_l = 27*/
/* c_north = 28*/
/* c_south = 29*/
/* c_east = 30*/
/* c_west = 31*/
/* c_up = 32*/
/* c_down = 33*/
/* c_n = 34*/
/* c_s = 35*/
/* c_e = 36*/
/* c_w = 37*/
/* c_u = 38*/
/* c_d = 39*/
/* c_custom = 40*/
/* c_who = 41*/
/* c_players = 42*/
/* c_search = 43*/
/* c_unhide = 44*/
/* c_punch = 45*/
/* c_ping = 46*/
/* c_health = 47*/
/* c_get = 48*/
/* c_drop = 49*/
/* c_inv = 50*/
/* c_i = 51*/
/* c_self = 52*/
/* c_whois = 53*/
/* c_duplicate = 54 */
/* c_version = 56*/
/* c_objects = 57*/
/* c_use = 58*/
/* c_wield = 59*/
/* c_brief = 60*/
/* c_wear = 61*/
/* c_relink = 62*/
/* c_unmake = 63*/
/* c_destroy = 64*/
/* c_show = 65*/
/* c_set = 66*/


int numcmds;   /* number of main level commands there are */
shortstring show[maxshow];
int numshow;
shortstring setkey[maxshow];
int numset;

shortstring direct[maxexit] = {
  "north", "south", "east", "west", "up", "down"
};

string spells[maxspells];   /* names of spells */
int numspells;   /* number of spells there actually are */

boolean done;   /* flag for QUIT */
veryshortstring userid;   /* userid of this player */
int location;   /* current place number */

int hold_kind[maxhold];
/* kinds of the objects i'm
                                           holding */

int myslot = 1;
/* here.people[myslot]... is this player */
shortstring myname;   /* personal name this player chose (setname) */
int myevent;   /* which point in event buffer we are at */

boolean found_exit[maxexit];
/* has exit i been found by the player? */

int mylog;   /* which log entry this player is */
int mywear;   /* what I'm wearing */
int mywield;   /* weapon I'm wielding */
int myhealth;   /* how well I'm feeling */
int myexperience;   /* how experienced I am */
int myself;   /* self description block */

int healthcycle;
/* used in rnd_event to control how quickly a
                          player heals */

room here;   /* current room record */
eventrec event;
boolean privd;

namrec objnam;   /* object names */
namrec objown;   /* object owners */
namrec nam;   /* record 1 is room names */
namrec own;   /* rec 2 is room owners */
namrec pers;   /* 3 is player personal names */
namrec user;   /* 4 is player userid*/
namrec adate;   /* 5 is date of last play */
/* 6 is time of last play */
namrec atime;

intrec anint;   /* info about game players */
objectrec obj;
spellrec spell;

descrec block;   /* a text block of descmax lines */
indexrec indx;   /* an record allocation record */
linerec oneliner;   /* a line record */

descrec heredsc;



double mrandom() {
	return random() / RAND_MAX;
}


#if defined(BSD)

#define		TCGETA	TIOCGETP
#define		TCSETAW	TIOCSETP
struct		sgttyb	raw_tty, original_tty;

#else

struct		termio raw_tty, original_tty;
#endif

void setup_guts() {

	  ioctl(0, TCGETA, &original_tty);
	  ioctl(0, TCGETA, &raw_tty);

#if defined(BSD)
	  raw_tty.sg_flags &= ~(ECHO | CRMOD);	/* echo off */
	  raw_tty.sg_flags |= CBREAK;	/* raw on    */
#else
	  raw_tty.c_lflag &= ~(ICANON | ECHO);	/* noecho raw mode        */

	  raw_tty.c_cc[VMIN] = '\01';	/* minimum # of chars to queue    */
	  raw_tty.c_cc[VTIME] = '\0';	/* minimum time to wait for input */

#endif

	  ioctl(0, TCSETAW, &raw_tty);
}

void finish_guts() {

	  ioctl(0, TCSETAW, &original_tty);
}


extern void checkevents(boolean);

#define SHORT_WAIT      0.1
#define LONG_WAIT       0.2

void doawait(t)
double t;
{
	struct timespec ts = { 0, 100000000 };

	ts.tv_sec = t;
	ts.tv_nsec = 1000000000 * (t - (double)((int)t));

	nanosleep(&ts, 0);
}

int in_grab_line = 0;

void pchars(s)
char *s;
{
	printf("%s", s);
	fflush(stdout);
}

void putchars(s)
char *s;
{
	if (in_grab_line) {
		pchars("\n");
		in_grab_line = 0;
	}

	printf("%s", s);
	fflush(stdout);
}

mprintf(format, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac)
char *format;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac;
{
	char buf[256];

	sprintf(buf, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac);
	putchars(buf);
}


char keyget() {
	fd_set ibits, obits, xbits;
	int hifd = 0;
	int ret;
	struct timeval tv = { 0, 200000};

	FD_ZERO(&ibits);
	FD_ZERO(&obits);
	FD_ZERO(&xbits);

	FD_SET(0, &ibits);
	hifd = 1;

	ret = select(hifd, &ibits, &obits, &xbits, &tv);

	if (ret < 0)
		perror("select");

	if (FD_ISSET(0, &ibits)) {
		unsigned char c;
		int nread;

		nread = read(0, &c, 1);

		if (nread <= 0)
			return 0;

		return c;
	}

	checkevents(false);

	return 0;
}


void grab_line(prompt, s, echo)
char *prompt, *s;
boolean echo;
{
  char ch;
  long pos;
  char STR1[82];
  char STR2[256];

  in_grab_line = 1;

  strcpy(old_prompt, prompt);
  pchars(prompt);
  *line = '\0';
  ch = keyget();
  while ((ch != 13) && (ch != 10)) {
    /* del char */
    if (ch == '\b' || ch == '\177') {
      switch (strlen(line)) {

      case 0:
	ch = keyget();
	break;

      case 1:
	*line = '\0';
	if (echo)
	  pchars("\b \b");
	ch = keyget();
	break;

      case 2:
	sprintf(STR2, "%c", line[0]);
	strcpy(line, STR2);
	if (echo)
	  pchars("\b \b");
	ch = keyget();
	break;

      default:
	sprintf(STR2, "%.*s", (int)(strlen(line) - 1L), line);
	strcpy(line, STR2);
	if (echo)
	  pchars("\b \b");
	ch = keyget();
	break;
      }
      /* cancel line */
      continue;
    }
    /* if */
    if (ch == '\025') {
      pchars("\015\033[K");
      /*                       pos := length(prompt) + length(line);
                               if pos > 0 then
                                       for i := 1 to pos do
                                               pchars(chr(8)+' '+chr(8));    */
      pchars(prompt);
      *line = '\0';
      ch = keyget();
      /*line too long*/
      continue;
    }
    if (strlen(line) > 76) {
      pchars("\007");
      ch = keyget();
      /* no ctrls */
      continue;
    }
    if (ch > 31) {
      sprintf(line + strlen(line), "%c", ch);
      if (echo) {
	sprintf(STR2, "%c", ch);
	pchars(STR2);
      }
      ch = keyget();
    } else
      ch = keyget();
  }
  /* ***           ch := nextkey;           *** */
  pos = strlen(prompt) + strlen(line);

  /*       if pos > 0 then
                  for i := 1 to pos do
                          pchars(chr(8));       */

  pchars("\r\n");

  strcpy(s, line);

  in_grab_line = 0;
}


/* returns a random # between 0-100 */
int rnd100()
{
  return random() % 101;
}



/* Return the index of the first occurrence of "pat" as a substring of "s",
   starting at index "pos" (1-based).  Result is 1-based, 0 if not found. */

int strpos2(s, pat, pos)
char *s;
register char *pat;
register int pos;
{
    register char *cp, ch;
    register int slen;

    if (--pos < 0)
        return 0;
    slen = strlen(s) - pos;
    cp = s + pos;
    if (!(ch = *pat++))
        return 0;
    pos = strlen(pat);
    slen -= pos;
    while (--slen >= 0) {
        if (*cp++ == ch && !strncmp(cp, pat, pos))
            return cp - s;
    }
    return 0;
}

int locked[256];

void lock(int fdes) {
	if (!locked[fdes])
		flock(fdes, LOCK_EX);
	locked[fdes]++;
}

void unlock(int fdes) {
	locked[fdes]--;
	if (locked[fdes] == 0)
		flock(fdes, LOCK_UN);
	assert(locked[fdes] >= 0);
}




/* necessary to prevent ZOMBIES in the world */


char *get_userid(Result)
char *Result;
{
	struct passwd *myentry;

	myentry = getpwuid(getuid());
	strcpy(Result, myentry->pw_name);
}


extern char *trim();

/* Input routine.   Gets a line of text from user which checking
   for async events */
extern void grab_line();

extern void putchars();

void xpoof();

void do_exit();

boolean put_token();

void take_token();

void maybe_drop();

void do_program();

boolean drop_everything();


void collision_wait()
{
  double wait_time;

  wait_time = mrandom();
  if (wait_time < 0.001)
    wait_time = 0.001;
  doawait(wait_time);
}


/* increment err; if err is too high, suspect deadlock */
/* this is called by all getX procedures to ease deadlock checking */
void deadcheck(err, s)
int *err;
char *s;
{
  (*err)++;
  if (*err <= maxerr)
    return;
  mprintf("?warning- %s seems to be deadlocked; notify the Monster Manager\n",
	 s);
  finish_guts();
  exit(0);
}



/* first procedure of form getX
   attempts to get given room record
   resolves record access conflicts, checks for deadlocks
   Locks record; use freeroom immediately after getroom if data is
   for read-only
*/
void getroom(n)
int n;
{
  int err = 0;

  if (n == 0)
    n = location;

  lock(roomfile);
  lseek(roomfile, (n - 1L) * sizeof(room), 0);
  read(roomfile, &roomfile_hat, sizeof(room));
  memcpy(&here, &roomfile_hat, sizeof(room));

  inmem = false;
  /* since this getroom could be doing anything, we will
     assume that it is bozoing the correct here record for
     this room.  If this getroom called by gethere, then
     gethere will correct inmem immediately.  Otherwise
     the next gethere will restore the correct here record. */
}


void putroom()
{
  lseek(roomfile, (here.valid - 1L) * sizeof(room), 0);
  memcpy(&roomfile_hat, &here, sizeof(room));
  write(roomfile, &roomfile_hat, sizeof(room));
  unlock(roomfile);
}


void freeroom()
{
  unlock(roomfile);
}


void gethere(n)
int n;
{
  if (n != 0 && n != location) {
    getroom(n);
    freeroom();
    return;
  }
  if (inmem) {
    if (debug)
      mprintf("?gethere - here already in memory\n");
    return;
  }
  getroom(0);   /* getroom(n) okay here also */
  freeroom();
  inmem = true;
}


void getown()
{
  int err = 0;

  lock(namfile);
  lseek(namfile, (2 - 1L) * sizeof(namrec), 0);
  read(namfile, &namfile_hat, sizeof(namrec));
  memcpy(&own, &namfile_hat, sizeof(namrec));
}


void getnam()
{
  lock(namfile);
  lseek(namfile, (1 - 1L) * sizeof(namrec), 0);
  read(namfile, &namfile_hat, sizeof(namrec));
  memcpy(&nam, &namfile_hat, sizeof(namrec));
}


void freenam()
{
  unlock(namfile);
}


void freeown()
{
  unlock(namfile);
}


void putnam()
{
  lseek(namfile, 0L, 0);
  memcpy(&namfile_hat, &nam, sizeof(namrec));
  write(namfile, &namfile_hat, sizeof(namrec));
  unlock(namfile);
}


void putown()
{
  lseek(namfile, sizeof(namrec), 0);
  memcpy(&namfile_hat, &own, sizeof(namrec));
  write(namfile, &namfile_hat, sizeof(namrec));
  unlock(namfile);
}


void getobj(n)
int n;
{
  lock(objfile);
  lseek(objfile, (n - 1L) * sizeof(objectrec), 0);
  read(objfile, &objfile_hat, sizeof(objectrec));
  memcpy(&obj, &objfile_hat, sizeof(objectrec));
}

void putobj()
{
  lseek(objfile, (obj.objnum - 1L) * sizeof(objectrec), 0);
  memcpy(&objfile_hat, &obj, sizeof(objectrec));
  write(objfile, &objfile_hat, sizeof(objectrec));
  unlock(objfile);
}


void freeobj()
{
  unlock(objfile);
}



void getint(n)
int n;
{
  lock(intfile);
  lseek(intfile, (n - 1L) * sizeof(intrec), 0);
  read(intfile, &intfile_hat, sizeof(intrec));
  memcpy(&anint, &intfile_hat, sizeof(intrec));
}


void freeint()
{
  unlock(intfile);
}


void putint()
{
  int n = anint.intnum;

  lseek(intfile, (n - 1L) * sizeof(intrec), 0);
  memcpy(&intfile_hat, &anint, sizeof(intrec));
  write(intfile, &intfile_hat, sizeof(intrec));
  unlock(intfile);
}


void getspell(n)
int n;
{
  if (n == 0)
    n = mylog;

  lock(spellfile);
  lseek(spellfile, (n - 1L) * sizeof(spellrec), 0);
  read(spellfile, &spellfile_hat, sizeof(spellrec));
  memcpy(&spell, &spellfile_hat, sizeof(spellrec));
}


void freespell()
{
  unlock(spellfile);
}


void putspell()
{
  int n;

  n = spell.recnum;

  lseek(spellfile, (n - 1L) * sizeof(spellrec), 0);
  memcpy(&spellfile_hat, &spell, sizeof(spellrec));
  write(spellfile, &spellfile_hat, sizeof(spellrec));
  unlock(spellfile);
}

void getuser()
{

  lock(namfile);
  lseek(namfile, (4 - 1L) * sizeof(namrec), 0);
  read(namfile, &namfile_hat, sizeof(namrec));
  memcpy(&user, &namfile_hat, sizeof(namrec));
}

void freeuser()
{
  unlock(namfile);
}


void putuser()
{
  lseek(namfile, 3L * sizeof(namrec), 0);
  memcpy(&namfile_hat, &user, sizeof(namrec));
  write(namfile, &namfile_hat, sizeof(namrec));
  unlock(namfile);
}



void getdate()
{
  lock(namfile);
  lseek(namfile, (7 - 1L) * sizeof(namrec), 0);
  read(namfile, &namfile_hat, sizeof(namrec));
  memcpy(&adate, &namfile_hat, sizeof(namrec));
}

void freedate()
{
  unlock(namfile);
}


void putdate()
{
  lseek(namfile, sizeof(namrec) * 6L, 0);
  memcpy(&namfile_hat, &adate, sizeof(namrec));
  write(namfile, &namfile_hat, sizeof(namrec));
  unlock(namfile);
}


void gettime()
{
  lock(namfile);
  lseek(namfile, (8 - 1L) * sizeof(namrec), 0);
  read(namfile, &namfile_hat, sizeof(namrec));
  memcpy(&atime, &namfile_hat, sizeof(namrec));
}


void freetime()
{
  unlock(namfile);
}

void puttime()
{
  lseek(namfile, sizeof(namrec) * 7L, 0);
  memcpy(&namfile_hat, &atime, sizeof(namrec));
  write(namfile, &namfile_hat, sizeof(namrec));
  unlock(namfile);
}


void getobjnam()
{
  lock(namfile);
  lseek(namfile, (5 - 1L) * sizeof(namrec), 0);
  read(namfile, &namfile_hat, sizeof(namrec));
  memcpy(&objnam, &namfile_hat, sizeof(namrec));
}


void freeobjnam()
{
  unlock(namfile);
}


void putobjnam()
{
  lseek(namfile, sizeof(namrec) * 4L, 0);
  memcpy(&namfile_hat, &objnam, sizeof(namrec));
  write(namfile, &namfile_hat, sizeof(namrec));
  unlock(namfile);
}


void getobjown()
{
  lock(namfile);
  lseek(namfile, (6 - 1L) * sizeof(namrec), 0);
  read(namfile, &namfile_hat, sizeof(namrec));
  memcpy(&objown, &namfile_hat, sizeof(namrec));
}


void freeobjown()
{
  unlock(namfile);
}


void putobjown()
{
  lseek(namfile, sizeof(namrec) * 5L, 0);
  memcpy(&namfile_hat, &objown, sizeof(namrec));
  write(namfile, &namfile_hat, sizeof(namrec));
  unlock(namfile);
}



void getpers()
{
  lock(namfile);
  lseek(namfile, (3 - 1L) * sizeof(namrec), 0);
  read(namfile, &namfile_hat, sizeof(namrec));
  memcpy(&pers, &namfile_hat, sizeof(namrec));
}


void freepers()
{
  unlock(namfile);
}


void putpers()
{
  lseek(namfile, sizeof(namrec) * 2L, 0);
  memcpy(&namfile_hat, &pers, sizeof(namrec));
  write(namfile, &namfile_hat, sizeof(namrec));
  unlock(namfile);
}



void getevent(n)
int n;
{
  int err = 0;

  if (n == 0)
    n = location;

  n = n % numevnts + 1;

  lock(eventfile);
  lseek(eventfile, (n - 1L) * sizeof(eventrec), 0);
  read(eventfile, &eventfile_hat, sizeof(eventrec));
  memcpy(&event, &eventfile_hat, sizeof(eventrec));
}


void freeevent()
{
  unlock(eventfile);
}


void putevent()
{
  lseek(eventfile, (event.validat - 1L) * sizeof(eventrec), 0);
  memcpy(&eventfile_hat, &event, sizeof(eventrec));
  write(eventfile, &eventfile_hat, sizeof(eventrec));
  unlock(eventfile);
}


void getblock(n)
int n;
{
  lock(descfile);
  lseek(descfile, (n - 1L) * sizeof(descrec), 0);
  read(descfile, &descfile_hat, sizeof(descrec));
  memcpy(&block, &descfile_hat, sizeof(descrec));
}


void putblock()
{
  int n;

  n = block.descrinum;
  if (debug)
    mprintf("?putblock: %d\n", n);
  if (n == 0)
    return;

  lseek(descfile, (n - 1L) * sizeof(descrec), 0);
  memcpy(&descfile_hat, &block, sizeof(descrec));
  write(descfile, &descfile_hat, sizeof(descrec));
  unlock(descfile);
}


void freeblock()
{
  unlock(descfile);
}


void getline(n)
int n;
{

  if (n == -1) {
    *oneliner.theline = '\0';
    return;
  }

  lock(linefile);
  lseek(linefile, (n - 1L) * sizeof(linerec), 0);
  read(linefile, &linefile_hat, sizeof(linerec));
  memcpy(&oneliner, &linefile_hat, sizeof(linerec));
}


void putline()
{
  if (oneliner.linenum <= 0)
    return;

  lseek(linefile, (oneliner.linenum - 1L) * sizeof(linerec), 0);
  memcpy(&linefile_hat, &oneliner, sizeof(linerec));
  write(linefile, &linefile_hat, sizeof(linerec));
  unlock(linefile);
}


void freeline()
{
  unlock(linefile);
}




/*
Index record 1 -- Description blocks that are free
Index record 2 -- One liners that are free
*/


void getindex(n)
int n;
{
  lock(indexfile);
  lseek(indexfile, (n - 1L) * sizeof(indexrec), 0);
  read(indexfile, &indexfile_hat, sizeof(indexrec));
  memcpy(&indx, &indexfile_hat, sizeof(indexrec));
}


void putindex()
{
  lseek(indexfile, (indx.indexnum - 1L) * sizeof(indexrec), 0);
  memcpy(&indexfile_hat, &indx, sizeof(indexrec));
  write(indexfile, &indexfile_hat, sizeof(indexrec));
  unlock(indexfile);
}

void freeindex()
{
  unlock(indexfile);
}



/*
First procedure of form alloc_X
Allocates the oneliner resource using the indexrec bitmaps

Return the number of a one liner if one is available
and remove it from the free list
*/
boolean alloc_line(n)
int *n;
{
  boolean Result, found;

  getindex(I_LINE);
  if (indx.inuse == indx.top) {
    freeindex();
    *n = 0;
    Result = false;
    mprintf("There are no available one line descriptions.\n");
    return false;
  } else {
    *n = 1;
    found = false;
    while (!found && *n <= indx.top) {
      if (indx.free[*n - 1])
	found = true;
      else
	(*n)++;
    }
    if (found) {
      indx.free[*n - 1] = false;
      Result = true;
      indx.inuse++;
      putindex();
      return true;
    } else {
      freeindex();
      mprintf("?serious error in alloc_line; notify Monster Manager\n");

      return false;
    }
  }
  return Result;
}


/*
put the line specified by n back on the free list
zeroes n also, for convenience
*/
void delete_line(n)
int *n;
{
  if (*n == DEFAULT_LINE)
    *n = 0;
  else if (*n > 0) {
    getindex(I_LINE);
    indx.inuse--;
    indx.free[*n - 1] = true;
    putindex();
  }
  *n = 0;
}



boolean alloc_int(n)
int *n;
{
  boolean Result, found;

  getindex(I_INT);
  if (indx.inuse == indx.top) {
    freeindex();
    *n = 0;
    Result = false;
    mprintf("There are no available integer records.\n");
    return false;
  } else {
    *n = 1;
    found = false;
    while (!found && *n <= indx.top) {
      if (indx.free[*n - 1])
	found = true;
      else
	(*n)++;
    }
    if (found) {
      indx.free[*n - 1] = false;
      Result = true;
      indx.inuse++;
      putindex();
      return true;
    } else {
      freeindex();
      mprintf("?serious error in alloc_int; notify Monster Manager\n");

      return false;
    }
  }
  return Result;
}


void delete_int(n)
int *n;
{
  if (*n > 0) {
    getindex(I_INT);
    indx.inuse--;
    indx.free[*n - 1] = true;
    putindex();
  }
  *n = 0;
}



/*
Return the number of a description block if available and
remove it from the free list
*/
boolean alloc_block(n)
int *n;
{
  boolean Result, found;

  if (debug)
    mprintf("?alloc_block entry\n");
  getindex(I_BLOCK);
  if (indx.inuse == indx.top) {
    freeindex();
    *n = 0;
    Result = false;
    mprintf("There are no available description blocks.\n");
    return false;
  } else {
    *n = 1;
    found = false;
    while (!found && *n <= indx.top) {
      if (indx.free[*n - 1])
	found = true;
      else
	(*n)++;
    }
    if (found) {
      indx.free[*n - 1] = false;
      Result = true;
      indx.inuse++;
      putindex();
      if (debug)
	mprintf("?alloc_block successful\n");
      return true;
    } else {
      freeindex();
      mprintf("?serious error in alloc_block; notify Monster Manager\n");
      return false;
    }
  }
  return Result;
}




/*
puts a description block back on the free list
zeroes n for convenience
*/
void delete_block(n)
int *n;
{
  if (*n == DEFAULT_LINE) {
    *n = 0;   /* no line really exists in the file */
    return;
  }
  if (*n > 0) {
    getindex(I_BLOCK);
    indx.inuse--;
    indx.free[*n - 1] = true;
    putindex();
    *n = 0;
    return;
  }
  if (*n < 0) {
    *n = -*n;
    delete_line(n);
  }
}



/*
Return the number of a room if one is available
and remove it from the free list
*/
boolean alloc_room(n)
int *n;
{
  boolean Result, found;

  getindex(I_ROOM);
  if (indx.inuse == indx.top) {
    freeindex();
    *n = 0;
    Result = false;
    mprintf("There are no available free rooms.\n");
    return false;
  } else {
    *n = 1;
    found = false;
    while (!found && *n <= indx.top) {
      if (indx.free[*n - 1])
	found = true;
      else
	(*n)++;
    }
    if (found) {
      indx.free[*n - 1] = false;
      Result = true;
      indx.inuse++;
      putindex();
      return true;
    } else {
      freeindex();
      mprintf("?serious error in alloc_room; notify Monster Manager\n");
      return false;
    }
  }
  return Result;
}


/*
Called by DEL_ROOM()
put the room specified by n back on the free list
zeroes n also, for convenience
*/
void delete_room(n)
int *n;
{
  if (*n == 0)
    return;
  getindex(I_ROOM);
  indx.inuse--;
  indx.free[*n - 1] = true;
  putindex();
  *n = 0;
}



boolean alloc_log(n)
int *n;
{
  boolean Result, found;

  getindex(I_PLAYER);
  if (indx.inuse == indx.top) {
    freeindex();
    *n = 0;
    Result = false;
    mprintf("There are too many monster players, you can't find a space.\n");
    return false;
  } else {
    *n = 1;
    found = false;
    while (!found && *n <= indx.top) {
      if (indx.free[*n - 1])
	found = true;
      else
	(*n)++;
    }
    if (found) {
      indx.free[*n - 1] = false;
      Result = true;
      indx.inuse++;
      putindex();
      return true;
    } else {
      freeindex();
      mprintf("?serious error in alloc_log; notify Monster Manager\n");
      return false;
    }
  }
  return Result;
}


void delete_log(n)
int *n;
{
  if (*n == 0)
    return;
  getindex(I_PLAYER);
  indx.inuse--;
  indx.free[*n - 1] = true;
  putindex();
  *n = 0;
}


char *lowcase(Result, s)
char *Result, *s;
{
  string sprime;
  int i, FORLIM;

  if (*s == '\0')
    return strcpy(Result, "");
  else {
    strcpy(sprime, s);
    FORLIM = strlen(s);
    for (i = 0; i < FORLIM; i++) {
      if (isupper(sprime[i]))
	sprime[i] = _tolower(sprime[i]);
    }
    return strcpy(Result, sprime);
  }
}


/* lookup a spell with disambiguation in the spell list */

boolean lookup_spell(n, s_)
int *n;
char *s_;
{
  string s;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string STR1;
  int FORLIM;

  strcpy(s, s_);
  strcpy(s, lowcase(STR1, s));
  FORLIM = numspells;
  for (i = 1; i <= FORLIM; i++) {
    if (!strcmp(s, spells[i-1]))
      num = i;
    else if (strncmp(s, spells[i-1], strlen(s)) == 0) {
      maybe++;
      poss = i;
    }
  }
  if (num != 0) {
    *n = num;
    return true;
  } else if (maybe == 1) {
    *n = poss;
    return true;
  } else if (maybe > 1)
    return false;
  else
    return false;
}


boolean lookup_user(pnum, s_)
int *pnum;
char *s_;
{
  string s;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string STR1;
  int FORLIM;

  strcpy(s, s_);
  getuser();
  freeuser();
  getindex(I_PLAYER);
  freeindex();

  strcpy(s, lowcase(STR1, s));
  FORLIM = indx.top;
  for (i = 1; i <= FORLIM; i++) {
    if (!indx.free[i-1]) {
      if (!strcmp(s, user.idents[i-1]))
	num = i;
      else if (strncmp(s, user.idents[i-1], strlen(s)) == 0) {
	maybe++;
	poss = i;
      }
    }
  }
  if (num != 0) {
    *pnum = num;
    return true;
  } else if (maybe == 1) {
    *pnum = poss;
    return true;
  } else if (maybe > 1) {
    /*writeln('-- Ambiguous direction');*/
    return false;
  } else {
    return false;
    /*writeln('-- Unknown direction');*/
  }
}


boolean alloc_obj(n)
int *n;
{
  boolean Result, found;

  getindex(I_OBJECT);
  if (indx.inuse == indx.top) {
    freeindex();
    *n = 0;
    Result = false;
    mprintf("All of the possible objects have been made.\n");
    return false;
  } else {
    *n = 1;
    found = false;
    while (!found && *n <= indx.top) {
      if (indx.free[*n - 1])
	found = true;
      else
	(*n)++;
    }
    if (found) {
      indx.free[*n - 1] = false;
      Result = true;
      indx.inuse++;
      putindex();
      return true;
    } else {
      freeindex();
      mprintf("?serious error in alloc_obj; notify Monster Manager\n");
      return false;
    }
  }
  return Result;
}


void delete_obj(n)
int *n;
{
  if (*n == 0)
    return;
  getindex(I_OBJECT);
  indx.inuse--;
  indx.free[*n - 1] = true;
  putindex();
  *n = 0;
}




boolean lookup_obj(pnum, s_)
int *pnum;
char *s_;
{
  string s;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string STR1;
  int FORLIM;

  strcpy(s, s_);
  getobjnam();
  freeobjnam();
  getindex(I_OBJECT);
  freeindex();

  strcpy(s, lowcase(STR1, s));
  FORLIM = indx.top;
  for (i = 1; i <= FORLIM; i++) {
    if (!indx.free[i-1]) {
      if (!strcmp(s, objnam.idents[i-1]))
	num = i;
      else if (strncmp(s, objnam.idents[i-1], strlen(s)) == 0) {
	maybe++;
	poss = i;
      }
    }
  }
  if (num != 0) {
    *pnum = num;
    return true;
  } else if (maybe == 1) {
    *pnum = poss;
    return true;
  } else if (maybe > 1) {
    /*writeln('-- Ambiguous direction');*/
    return false;
  } else {
    return false;
    /*writeln('-- Unknown direction');*/
  }
}



/* returns true if object N is in this room */

boolean obj_here(n)
int n;
{
  int i = 1;
  boolean found = false;

  while (i <= maxobjs && !found) {
    if (here.objs[i-1] == n)
      found = true;
    else
      i++;
  }
  return found;
}




/* returns true if object N is being held by the player */

boolean obj_hold(n)
int n;
{
  int i;
  boolean found;

  if (n == 0)
    return false;
  else {
    i = 1;
    found = false;
    while (i <= maxhold && !found) {
      if (here.people[myslot-1].holding[i-1] == n)
	found = true;
      else
	i++;
    }
    return found;
  }
}



/* return the slot of an object that is HERE */
int find_obj(objnum)
int objnum;
{
  int Result = 0, i = 1;

  while (i <= maxobjs) {
    if (here.objs[i-1] == objnum)
      Result = i;
    i++;
  }
  return Result;
}



/* similar to lookup_obj, but only returns true if the object is in
   this room or is being held by the player */

boolean parse_obj(n, s, override)
int *n;
char *s;
boolean override;
{
  if (!lookup_obj(n, s))
    return false;
  if (obj_here(*n) || obj_hold(*n)) {
    /* took out a great block of code that wouldn't let
       parse_obj work if player couldn't see object */

    return true;
  }

}




boolean lookup_pers(pnum, s_)
int *pnum;
char *s_;
{
  string s;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string pname, STR1;
  int FORLIM;

  strcpy(s, s_);
  getpers();
  freepers();
  getindex(I_PLAYER);
  freeindex();

  strcpy(s, lowcase(STR1, s));
  FORLIM = indx.top;
  for (i = 1; i <= FORLIM; i++) {
    if (!indx.free[i-1]) {
      lowcase(pname, pers.idents[i-1]);

      if (!strcmp(s, pname))
	num = i;
      else if (strncmp(s, pname, strlen(s)) == 0) {
	maybe++;
	poss = i;
      }
    }
  }
  if (num != 0) {
    *pnum = num;
    return true;
  } else if (maybe == 1) {
    *pnum = poss;
    return true;
  } else if (maybe > 1) {
    /*writeln('-- Ambiguous direction');*/
    return false;
  } else {
    return false;
    /*writeln('-- Unknown direction');*/
  }
}



boolean parse_pers(pnum, s_)
int *pnum;
char *s_;
{
  boolean Result;
  string s;
  int persnum;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string pname, STR1;

  strcpy(s, s_);
  gethere(0);
  strcpy(s, lowcase(STR1, s));
  for (i = 1; i <= maxpeople; i++) {
    if (here.people[i-1].kind > 0) {
      lowcase(pname, here.people[i-1].name);

      if (!strcmp(s, pname))
	num = i;
      else if (strncmp(s, pname, strlen(s)) == 0) {
	maybe++;
	poss = i;
      }
    }
  }
  /*if here.people[i].username <> '' then begin*/

  if (num != 0) {
    persnum = num;
    Result = true;
  } else if (maybe == 1) {
    persnum = poss;
    Result = true;
  } else if (maybe > 1) {
    persnum = 0;
    Result = false;
  } else {
    persnum = 0;
    Result = false;
  }
  if (persnum <= 0)
    return Result;
  if (here.people[persnum-1].hiding > 0)
    return false;
  Result = true;
  *pnum = persnum;
  return true;
}





/*
Returns TRUE if player is owner of room n
If no n is given default will be this room (location)
*/
boolean is_owner(n, surpress)
int n;
boolean surpress;
{
  boolean Result = false;

  gethere(n);
  if (!strcmp(here.owner, userid) || privd)
    return true;
  if (!surpress)
    mprintf("You are not the owner of this room.\n");
  return false;
}


char *room_owner(Result, n)
char *Result;
int n;
{
  if (n == 0)
    return strcpy(Result, "no room");
  gethere(n);
  strcpy(Result, here.owner);
  gethere(0);   /* restore old state! */
  return Result;
}


/*
Returns TRUE if player is allowed to alter the exit
TRUE if either this room or if target room is owned by player
*/

boolean can_alter(dir, room_)
int dir, room_;
{
  string STR1;

  gethere(0);
  if (!strcmp(here.owner, userid) || privd)
    return true;
  else {
    if (here.exits[dir-1].toloc > 0) {
      if (!strcmp(room_owner(STR1, here.exits[dir-1].toloc), userid))
	return true;
      else
	return false;
    } else
      return false;
  }
}


boolean can_make(dir, room_)
int dir, room_;
{
  boolean Result = false;

  gethere(room_);   /* 5 is accept door */
  if (here.exits[dir-1].toloc != 0) {
    mprintf("There is already an exit there.  Use UNLINK or RELINK.\n");
    return false;
  }
  if (!strcmp(here.owner, userid) || here.exits[dir-1].kind == 5 || privd ||
      !strcmp(here.owner, "*"))
	/* I'm the owner */
	  return true;
  /* there's an accept */
  /* Monster Manager */
  /* disowned room */
  mprintf("You are not allowed to create an exit there.\n");
  return false;
}


/*
print a one liner
*/
void print_line(n)
int n;
{
  if (n == DEFAULT_LINE) {
    mprintf("<default line>\n");
    return;
  }
  if (n <= 0)
    return;
  getline(n);
  freeline();
  puts(oneliner.theline);
}



void print_desc(dsc, default_)
int dsc;
char *default_;
{
  int i = 1;

  if (dsc == DEFAULT_LINE) {
    puts(default_);
    return;
  }
  if (dsc <= 0) {
    if (dsc < 0)
      print_line(abs(dsc));
    return;
  }
  getblock(dsc);
  freeblock();
  while (i <= block.desclen) {
    puts(block.lines[i-1]);
    i++;
  }
}




void make_line(n, prompt, limit)
int *n;
char *prompt;
int limit;
{
  string s;
  boolean ok;

  mprintf("Type ** to leave line unchanged, * to make [no line]\n");
  grab_line(prompt, s, true);
  if (!strcmp(s, "**")) {
    mprintf("No changes.\n");
    return;
  }
  if (!strcmp(s, "***")) {
    *n = DEFAULT_LINE;
    return;
  }
  if (!strcmp(s, "*")) {
    if (debug)
      mprintf("?deleting line %d\n", *n);
    delete_line(n);
    return;
  }
  if (*s == '\0') {
    if (debug)
      mprintf("?deleting line %d\n", *n);
    delete_line(n);
    return;
  }
  if (strlen(s) > limit) {
    mprintf("Please limit your string to %d characters.\n", limit);
    return;
  }
  if (*n == 0 || *n == DEFAULT_LINE) {
    if (debug)
      mprintf("?makeline: allocating line\n");
    ok = alloc_line(n);
  } else
    ok = true;

  if (!ok)
    return;
  if (debug)
    mprintf("?ok in makeline\n");
  getline(*n);
  strcpy(oneliner.theline, s);
  putline();

  if (debug)
    mprintf("?completed putline in makeline\n");
}


/* translate a direction s [north, south, etc...] into the integer code */

boolean lookup_dir(dir, s_)
int *dir;
char *s_;
{
  string s;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string STR1;

  strcpy(s, s_);
  strcpy(s, lowcase(STR1, s));
  for (i = 1; i <= maxexit; i++) {
    if (!strcmp(s, direct[i-1]))
      num = i;
    else if (strncmp(s, direct[i-1], strlen(s)) == 0) {
      maybe++;
      poss = i;
    }
  }
  if (num != 0) {
    *dir = num;
    return true;
  } else if (maybe == 1) {
    *dir = poss;
    return true;
  } else if (maybe > 1) {
    return false;
    /*writeln('-- Ambiguous direction');*/
  } else {
    return false;
    /*writeln('-- Unknown direction');*/
  }
}


boolean lookup_show(n, s_)
int *n;
char *s_;
{
  string s;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string STR1;
  int FORLIM;

  strcpy(s, s_);
  strcpy(s, lowcase(STR1, s));
  FORLIM = numshow;
  for (i = 1; i <= FORLIM; i++) {
    if (!strcmp(s, show[i-1]))
      num = i;
    else if (strncmp(s, show[i-1], strlen(s)) == 0) {
      maybe++;
      poss = i;
    }
  }
  if (num != 0) {
    *n = num;
    return true;
  } else if (maybe == 1) {
    *n = poss;
    return true;
  } else if (maybe > 1) {
    return false;
    /*writeln('-- Ambiguous direction');*/
  } else {
    return false;
    /*writeln('-- Unknown direction');*/
  }
}


boolean lookup_set(n, s_)
int *n;
char *s_;
{
  string s;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string STR1;
  int FORLIM;

  strcpy(s, s_);
  strcpy(s, lowcase(STR1, s));
  FORLIM = numset;
  for (i = 1; i <= FORLIM; i++) {
    if (!strcmp(s, setkey[i-1]))
      num = i;
    else if (strncmp(s, setkey[i-1], strlen(s)) == 0) {
      maybe++;
      poss = i;
    }
  }
  if (num != 0) {
    *n = num;
    return true;
  } else if (maybe == 1) {
    *n = poss;
    return true;
  } else if (maybe > 1)
    return false;
  else
    return false;
}


boolean lookup_room(n, s_)
int *n;
char *s_;
{
  boolean Result;
  string s;
  int top;

  int i, poss, maybe, num;
  string STR1;

  strcpy(s, s_);
  if (*s != '\0') {
    strcpy(s, lowcase(STR1, s));   /* case insensitivity */
    getnam();
    freenam();
    getindex(I_ROOM);
    freeindex();
    top = indx.top;


    i = 1;
    maybe = 0;
    num = 0;
    for (i = 1; i <= top; i++) {
      if (!strcmp(s, nam.idents[i-1]))
	num = i;
      else if (strncmp(s, nam.idents[i-1], strlen(s)) == 0) {
	maybe++;
	poss = i;
      }
    }
    if (num != 0) {
      Result = true;
      *n = num;
      return true;
    } else if (maybe == 1) {
      Result = true;
      *n = poss;
      return true;
    } else if (maybe > 1)
      return false;
    else
      return false;
  } else {

    return false;
  }
  return Result;
}


boolean exact_room(n, s)
int *n;
char *s;
{
  string STR2;

  if (debug)
    mprintf("?exact room: s = %s\n", s);
  if (lookup_room(n, s)) {
    if (!strcmp(nam.idents[*n - 1], lowcase(STR2, s)))
      return true;
    else
      return false;
  } else
    return false;
}


boolean exact_pers(n, s)
int *n;
char *s;
{
  string STR1, STR2;

  if (lookup_pers(n, s)) {
    if (!strcmp(lowcase(STR1, pers.idents[*n - 1]), lowcase(STR2, s)))
      return true;
    else
      return false;
  } else
    return false;
}


boolean exact_user(n, s)
int *n;
char *s;
{
  string STR1, STR2;

  if (lookup_user(n, s)) {
    if (!strcmp(lowcase(STR1, user.idents[*n - 1]), lowcase(STR2, s)))
      return true;
    else
      return false;
  } else
    return false;
}


boolean exact_obj(n, s)
int *n;
char *s;
{
  string STR1;

  if (lookup_obj(n, s)) {
    if (!strcmp(objnam.idents[*n - 1], lowcase(STR1, s)))
      return true;
    else
      return false;
  } else
    return false;
}



/*
Return n as the direction number if s is a valid alias for an exit
*/
boolean lookup_alias(n, s_)
int *n;
char *s_;
{
  string s;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string STR1;

  strcpy(s, s_);
  gethere(0);
  strcpy(s, lowcase(STR1, s));
  for (i = 1; i <= maxexit; i++) {
    if (!strcmp(s, here.exits[i-1].alias))
      num = i;
    else if (strncmp(s, here.exits[i-1].alias, strlen(s)) == 0) {
      maybe++;
      poss = i;
    }
  }
  if (num != 0) {
    *n = num;
    return true;
  } else if (maybe == 1) {
    *n = poss;
    return true;
  } else if (maybe > 1)
    return false;
  else
    return false;
}


void exit_default(dir, kind)
int dir, kind;
{
  switch (kind) {

  case 1:
    mprintf("There is a passage leading %s.\n", direct[dir-1]);
    break;

  case 2:
    mprintf("There is a locked door leading %s.\n", direct[dir-1]);
    break;

  case 5:
    switch (dir) {

    case north:
    case south:
    case east:
    case west:
      mprintf("A note on the %s wall says \"Your exit here.\"\n",
	     direct[dir-1]);
      break;

    case up:
      mprintf("A note on the ceiling says \"Your exit here.\"\n");
      break;

    case down:
      mprintf("A note on the floor says \"Your exit here.\"\n");
      break;
    }
    break;

  default:
    mprintf("There is an exit: %s\n", direct[dir-1]);
    break;
  }
}


/*
Prints out the exits here for DO_LOOK()
*/
void show_exits()
{
  int i;
  boolean one = false;
  boolean cansee;

  for (i = 1; i <= maxexit; i++) {
    if (here.exits[i-1].toloc != 0 || here.exits[i-1].kind == 5)
	/* there is an exit */
	{  /* there could be an exit */
      if (here.exits[i-1].hidden == 0 || found_exit[i-1])
	cansee = true;
      else
	cansee = false;

      if (here.exits[i-1].kind == 6) {
	/* door kind only visible with object */
	if (obj_hold(here.exits[i-1].objreq))
	  cansee = true;
	else
	  cansee = false;
      }

      if (cansee) {
	if (here.exits[i-1].exitdesc == DEFAULT_LINE) {
	  exit_default(i, here.exits[i-1].kind);
	  /* give it direction and type */
	  one = true;
	} else if (here.exits[i-1].exitdesc > 0) {
	  print_line(here.exits[i-1].exitdesc);
	  one = true;
	}
      }
    }

  }
  if (one)
    putchar('\n');
}


void setevent()
{
  getevent(0);
  freeevent();
  myevent = event.point;
}



boolean isnum(s)
char *s;
{
  boolean Result = true;
  int i = 1;

  if (strlen(s) < 1)
    return false;
  while (i <= strlen(s)) {
    if (!isdigit(s[i-1]))
      Result = false;
    i++;
  }
  return Result;
}


int number(s)
char *s;
{
  int i;

  if (strlen(s) < 1 || !isdigit(s[0]))
    return 0;
  else {
    i = atoi(s);
    return i;
  }
}



void log_event(send, act, targ, p, s, room_)
int send, act, targ, p;
char *s;
int room_;
{
  /* slot of sender */
  /* what event occurred */
  /* target of event */
  /* expansion parameter */
  /* string for messages */
  /* room to log event in */
  anevent *WITH;

  if (room_ == 0)
    room_ = location;
  getevent(room_);
  event.point++;
  if (debug)
    mprintf("?logging event %d to point %d\n", act, event.point);
  if (event.point > maxevent)
    event.point = 1;
  WITH = &event.evnt[event.point - 1];
  WITH->sender = send;
  WITH->action = act;
  WITH->target = targ;
  WITH->parm = p;
  strcpy(WITH->msg, s);
  WITH->loc = room_;
  putevent();
}


void log_action(theaction, thetarget)
int theaction, thetarget;
{
  if (debug)
    mprintf("?log_action(%d,%d)\n", theaction, thetarget);
  getroom(0);
  here.people[myslot-1].act = theaction;
  here.people[myslot-1].targ = thetarget;
  putroom();

  logged_act = true;
  log_event(myslot, E_ACTION, thetarget, theaction, myname, 0);
}


char *desc_action(Result, theaction, thetarget)
char *Result;
int theaction, thetarget;
{
  string s;

  switch (theaction) {   /* use command mnemonics */

  case look:
    strcpy(s, " looking around the room.");
    break;

  case form:
    strcpy(s, " creating a new room.");
    break;

  case desc:
    strcpy(s, " editing the description to this room.");
    break;

  case e_detail:
    strcpy(s, " adding details to the room.");
    break;

  case c_custom:
    strcpy(s, " customizing an exit here.");
    break;

  case e_custroom:
    strcpy(s, " customizing this room.");
    break;

  case e_program:
    strcpy(s, " customizing an object.");
    break;

  case c_self:
    strcpy(s, " editing a self-description.");
    break;

  case e_usecrystal:
    strcpy(s, " hunched over a crystal orb, immersed in its glow.");
    break;

  case link:
    strcpy(s, " creating an exit here.");
    break;

  case c_system:
    strcpy(s, " in system maintenance mode.");
    break;

  default:
    strcpy(s, " here.");
    break;
  }
  return strcpy(Result, s);
}


boolean protected_(n)
int n;
{
  if (n == 0)
    n = myslot;
  if (here.people[n-1].act == c_system || here.people[n-1].act == c_self ||
      here.people[n-1].act == e_program ||
      here.people[n-1].act == e_custroom ||
      here.people[n-1].act == c_custom || here.people[n-1].act == e_detail)
    return true;
  else
    return false;
}



/*
user procedure to designate an exit for acceptance of links
*/
void do_accept(s)
char *s;
{
  int dir;

  if (!lookup_dir(&dir, s)) {
    mprintf("To allow others to make an exit, type ACCEPT <direction of exit>.\n");
    return;
  }
  if (!can_make(dir, 0))
    return;
  getroom(0);
  here.exits[dir-1].kind = 5;
  putroom();

  log_event(myslot, E_ACCEPT, 0, 0, "", 0);
  mprintf("Someone will be able to make an exit %s.\n", direct[dir-1]);
}


/*
User procedure to refuse an exit for links
Note: may be unlink
*/
void do_refuse(s)
char *s;
{
  int dir;
  boolean ok;
  exit_ *WITH;

  if (!is_owner(0, false))
    return;
  /* is_owner prints error message itself */
  if (!lookup_dir(&dir, s)) {
    mprintf("To undo an Accept, type REFUSE <direction>.\n");
    return;
  }
  getroom(0);
  WITH = &here.exits[dir-1];
  if (WITH->toloc == 0 && WITH->kind == 5) {
    WITH->kind = 0;
    ok = true;
  } else
    ok = false;
  putroom();
  if (ok) {
    log_event(myslot, E_REFUSE, 0, 0, "", 0);
    mprintf("Exits %s will be refused.\n", direct[dir-1]);
  } else
    mprintf("Exits were not being accepted there.\n");
}


nicedate(timestr, newstr)
char *timestr, *newstr;
{

/* Thu Dec 20 00:06:14 2001 --> 20-Dec-2001 */

	*newstr++ = timestr[8];
	*newstr++ = timestr[9];
	*newstr++ = '-';
	*newstr++ = timestr[4];
	*newstr++ = timestr[5];
	*newstr++ = timestr[6];
	*newstr++ = '-';
	*newstr++ = timestr[20];
	*newstr++ = timestr[21];
	*newstr++ = timestr[22];
	*newstr++ = timestr[23];
	*newstr++ = '\0';
}

nicetime(timestr, newstr)
char *timestr, *newstr;
{
	int hours;
	char dayornite[3];

	if (timestr[11] == ' ')
		hours = timestr[12] - '0';
	else
		hours = (timestr[11]-'0')*10 + (timestr[12]-'0');
	if (hours < 12)
		strcpy(dayornite, "am");
	else
		strcpy(dayornite, "pm");
	if (hours >= 13)
		hours -= 12;
	if (!hours)
		hours = 12;
	sprintf(newstr, "%d:%c%c%s", hours, timestr[14],
					timestr[15], dayornite);
}

char *nice_time() {
	char *timestr;
	char the_date[17];
	char the_time[8];
	extern char *ctime();
	long time_now;
	static char buf[25];

	time(&time_now);
	timestr = ctime(&time_now);
	nicedate(timestr, the_date);
	nicetime(timestr, the_time);
	sprintf(buf,"%s  %s", the_date, the_time);
	return(buf);
}



char *sysdate(Result)
char *Result;
{
	char *timestr;
	time_t time_now;

	time(&time_now);
	timestr = ctime(&time_now);
	nicedate(timestr, Result);
	return Result;
}


char *systime(Result)
char *Result;
{
	char *timestr;
	time_t time_now;

	time(&time_now);
	timestr = ctime(&time_now);
	nicetime(timestr, Result);
	return Result;
}



/* substitute a parameter string for the # sign in the source string */
char *subs_parm(Result, s, parm)
char *Result, *s, *parm;
{
  string right, left;
  int i;   /* i is point to break at */
  char STR1[256];

  i = strpos2("#", s, 1);
  if (i > 0 && strlen(s) + strlen(parm) <= 80) {
    if (i >= strlen(s)) {
      *right = '\0';
      strcpy(left, s);
    } else if (i < 1) {
      strcpy(right, s);
      *left = '\0';
    } else {
      sprintf(right, "%.*s", strlen(s) - i, s + i);
      sprintf(left, "%.*s", i, s);
    }
    if (strlen(left) <= 1)
      *left = '\0';
    else {
      sprintf(STR1, "%.*s", (int)(strlen(left) - 1), left);
      strcpy(left, STR1);
    }
    sprintf(Result, "%s%s%s", left, parm, right);

    return Result;
  } else
    return strcpy(Result, s);
}


void time_health()
{
  char STR2[162];

  if (healthcycle <= 0) {  /* how quickly they heal */
    healthcycle++;
    return;
  }
  if (myhealth < 7) {  /* heal a little bit */
    myhealth++;

    getroom(0);
    here.people[myslot-1].health = myhealth;
    putroom();

    /*show new health rating */
    switch (myhealth) {

    case 9:
      mprintf("You are now in exceptional health.\n");
      break;

    case 8:
      mprintf("You feel much stronger.  You are in better than average condition.\n");
      break;

    case 7:
      mprintf("You are now in perfect health.\n");
      break;

    case 6:
      mprintf("You only feel a little bit dazed now.\n");
      break;

    case 5:
      mprintf(
	"You only have some minor cuts and abrasions now.  Most of your serious wounds\n");
      mprintf("have healed.\n");
      break;

    case 4:
      mprintf("You are only suffering from some minor wounds now.\n");
      break;

    case 3:
      mprintf(
	"Your most serious wounds have healed, but you are still in bad shape.\n");
      break;

    case 2:
      mprintf("You have healed somewhat, but are still very badly wounded.\n");
      break;

    case 1:
      mprintf("You are in critical condition, but there may be hope.\n");
      break;

    case 0:
      mprintf("are still dead.\n");
      break;

    default:
      mprintf("You don't seem to be in any condition at all.\n");
      break;
    }

    sprintf(STR2, "\n%s%s", old_prompt, line);
    putchars(STR2);

  }
  healthcycle = 0;
}


void time_noises()
{
  int n;

  if (rnd100() > 2)
    return;
  n = rnd100();
  if ((unsigned)n <= 40)
    log_event(0, E_NOISES, rnd100(), 0, "", 0);
  else if (n >= 41 && n <= 60)
    log_event(0, E_ALTNOISE, rnd100(), 0, "", 0);
}


void time_trapdoor(silent)
boolean silent;
{
  boolean fall;
  char STR2[162];

  if (rnd100() >= here.trapchance)
    return;
  /* trapdoor fires! */

  if (here.trapto > 0) {   /*(protected) or*/
    if (logged_act)
      fall = false;
    else if (here.magicobj == 0)
      fall = true;
    else if (obj_hold(here.magicobj))
      fall = false;
    else
      fall = true;
  }
  /* logged action should cover {protected) */
  else
    fall = false;

  if (!fall)
    return;
  do_exit(here.trapto);
  if (!silent) {
    sprintf(STR2, "\n%s%s", old_prompt, line);
    putchars(STR2);
  }
}


void time_midnight()
{
  string STR1;

  if (!strcmp(systime(STR1), "12:00am"))
    log_event(0, E_MIDNIGHT, rnd100(), 0, "", 0);
}


/* cause random events to occurr (ha ha ha) */

void rnd_event(silent)
boolean silent;
{
  if (rndcycle != 200) {  /* inside here 3 times/min */
    rndcycle++;
    return;
  }

  time_noises();
  time_health();
  time_trapdoor(silent);
  time_midnight();

  rndcycle = 0;
}


void do_die()
{
  boolean some;

  mprintf("\n        *** You have died ***\n\n");
  some = drop_everything(0);
  myhealth = 7;
  take_token(myslot, location);
  log_event(0, E_DIED, 0, 0, myname, 0);
  if (put_token(2, &myslot, 0)) {
    location = 2;
    inmem = false;
    setevent();
    /* log entry to death loc */
    /* perhaps turn off refs to other people */
    return;
  } else {
    mprintf(
      "The Monster universe regrets to inform you that you cannot be ressurected at\n");
    mprintf("the moment.\n");
    exit(0);
  }
}


void poor_health(p)
int p;
{
  if (myhealth <= p) {
    do_die();
    /* they died */
    return;
  }
  myhealth--;
  getroom(0);
  here.people[myslot-1].health = myhealth;
  putroom();
  log_event(myslot, E_WEAKER, myhealth, 0, "", 0);

  /* show new health rating */
  mprintf("You ");
  switch (here.people[myslot-1].health) {

  case 9:
    mprintf("are still in exceptional health.\n");
    break;

  case 8:
    mprintf("feel weaker, but are in better than average condition.\n");
    break;

  case 7:
    mprintf("are somewhat weaker, but are in perfect health.\n");
    break;

  case 6:
    mprintf("feel a little bit dazed.\n");
    break;

  case 5:
    mprintf("have some minor cuts and abrasions.\n");
    break;

  case 4:
    mprintf("have some wounds, but are still fairly strong.\n");
    break;

  case 3:
    mprintf("are suffering from some serious wounds.\n");
    break;

  case 2:
    mprintf("are very badly wounded.\n");
    break;

  case 1:
    mprintf("have many serious wounds, and are near death.\n");
    break;

  case 0:
    mprintf("are dead.\n");
    break;

  default:
    mprintf("don't seem to be in any condition at all.\n");
    break;
  }
}



/* count objects here */

int find_numobjs()
{
  int sum = 0;
  int i;

  for (i = 0; i < maxobjs; i++) {
    if (here.objs[i] != 0)
      sum++;
  }
  return sum;
}



/* optional parameter is slot of player's objects to count */

int find_numhold(player)
int player;
{
  int sum = 0;
  int i;

  if (player == 0)
    player = myslot;

  for (i = 0; i < maxhold; i++) {
    if (here.people[player-1].holding[i] != 0)
      sum++;
  }
  return sum;
}




void take_hit(p)
int p;
{
  int i;

  if (p <= 0)
    return;
  if (rnd100() < (p - 1) * 30 + 55)   /* chance that they're hit */
    poor_health(p);

  if (find_numobjs() <= maxobjs) {
    /* maybe they drop something if they're hit */
    for (i = 1; i <= p; i++)
      maybe_drop();
  }
}


int punch_force(sock)
int sock;
{
  int p;

  if ((unsigned)sock < 32 && ((1L << sock) & 0x19cc) != 0)
  {   /* no punch or a graze */
    p = 0;
    return p;
  }
  if ((unsigned)sock < 32 && ((1L << sock) & 0x610) != 0)   /* hard punches */
    p = 2;
  else {
    p = 1;
    /* 1,5,13,14,15 */
  }
  /* all others are medium punches */
  return p;
}


void put_punch(sock, s)
int sock;
char *s;
{
  switch (sock) {

  case 1:
    mprintf("You deliver a quick jab to %s's jaw.\n", s);
    break;

  case 2:
    mprintf("You swing at %s and miss.\n", s);
    break;

  case 3:
    mprintf("A quick punch, but it only grazes %s.\n", s);
    break;

  case 4:
    mprintf("%s doubles over after your jab to the stomach.\n", s);
    break;

  case 5:
    mprintf("Your punch lands square on %s's face!\n", s);
    break;

  case 6:
    mprintf("You swing wild and miss.\n");
    break;

  case 7:
    mprintf("A good swing, but it misses %s by a mile!\n", s);
    break;

  case 8:
    mprintf("Your punch is blocked by %s.\n", s);
    break;

  case 9:
    mprintf("Your roundhouse blow sends %s reeling.\n", s);
    break;

  case 10:
    mprintf("You land a solid uppercut on %s's chin.\n", s);
    break;

  case 11:
    mprintf("%s fends off your blow.\n", s);
    break;

  case 12:
    mprintf("%s ducks and avoids your punch.\n", s);
    break;

  case 13:
    mprintf("You thump %s in the ribs.\n", s);
    break;

  case 14:
    mprintf("You catch %s's face on your elbow.\n", s);
    break;

  case 15:
    mprintf("You knock the wind out of %s with a punch to the chest.\n", s);
    break;
  }
}


void get_punch(sock, s)
int sock;
char *s;
{
  switch (sock) {

  case 1:
    mprintf("%s delivers a quick jab to your jaw!\n", s);
    break;

  case 2:
    mprintf("%s swings at you but misses.\n", s);
    break;

  case 3:
    mprintf("%s's fist grazes you.\n", s);
    break;

  case 4:
    mprintf("You double over after %s lands a mean jab to your stomach!\n", s);
    break;

  case 5:
    mprintf("You see stars as %s bashes you in the face.\n", s);
    break;

  case 6:
    mprintf("You only feel the breeze as %s swings wildly.\n", s);
    break;

  case 7:
    mprintf("%s's swing misses you by a yard.\n", s);
    break;

  case 8:
    mprintf("With lightning reflexes you block %s's punch.\n", s);
    break;

  case 9:
    mprintf("%s's blow sends you reeling.\n", s);
    break;

  case 10:
    mprintf("Your head snaps back from %s's uppercut!\n", s);
    break;

  case 11:
    mprintf("You parry %s's attack.\n", s);
    break;

  case 12:
    mprintf("You duck in time to avoid %s's punch.\n", s);
    break;

  case 13:
    mprintf("%s thumps you hard in the ribs.\n", s);
    break;

  case 14:
    mprintf("Your vision blurs as %s elbows you in the head.\n", s);
    break;

  case 15:
    mprintf("%s knocks the wind out of you with a punch to your chest.\n", s);
    break;
  }
}


void view_punch(a, b, p)
char *a, *b;
int p;
{
  switch (p) {

  case 1:
    mprintf("%s jabs %s in the jaw.\n", a, b);
    break;

  case 2:
    mprintf("%s throws a wild punch at the air.\n", a);
    break;

  case 3:
    mprintf("%s's fist barely grazes %s.\n", a, b);
    break;

  case 4:
    mprintf("%s doubles over in pain with %s's punch\n", b, a);
    break;

  case 5:
    mprintf("%s bashes %s in the face.\n", a, b);
    break;

  case 6:
    mprintf("%s takes a wild swing at %s and misses.\n", a, b);
    break;

  case 7:
    mprintf("%s swings at %s and misses by a yard.\n", a, b);
    break;

  case 8:
    mprintf("%s's punch is blocked by %s's quick reflexes.\n", b, a);
    break;

  case 9:
    mprintf("%s is sent reeling from a punch by %s.\n", b, a);
    break;

  case 10:
    mprintf("%s lands an uppercut on %s's head.\n", a, b);
    break;

  case 11:
    mprintf("%s parrys %s's attack.\n", b, a);
    break;

  case 12:
    mprintf("%s ducks to avoid %s's punch.\n", b, a);
    break;

  case 13:
    mprintf("%s thumps %s hard in the ribs.\n", a, b);
    break;

  case 14:
    mprintf("%s's elbow connects with %s's head.\n", a, b);
    break;

  case 15:
    mprintf("%s knocks the wind out of %s.\n", a, b);
    break;
  }
}




void desc_health(n, header)
int n;
char *header;
{
  if (*header == '\0')
    mprintf("%s ", here.people[n-1].name);
  else
    fputs(header, stdout);

  switch (here.people[n-1].health) {

  case 9:
    mprintf("is in exceptional health, and looks very strong.\n");
    break;

  case 8:
    mprintf("is in better than average condition.\n");
    break;

  case 7:
    mprintf("is in perfect health.\n");
    break;

  case 6:
    mprintf("looks a little dazed.\n");
    break;

  case 5:
    mprintf("has some minor cuts and abrasions.\n");
    break;

  case 4:
    mprintf("has some minor wounds.\n");
    break;

  case 3:
    mprintf("is suffering from some serious wounds.\n");
    break;

  case 2:
    mprintf("is very badly wounded.\n");
    break;

  case 1:
    mprintf("has many serious wounds, and is near death.\n");
    break;

  case 0:
    mprintf("is dead.\n");
    break;

  default:
    mprintf("doesn't seem to be in any condition at all.\n");
    break;
  }
}


char *obj_part(Result, objnum, doread)
char *Result;
int objnum;
boolean doread;
{
  string s;
  char STR1[84];
  char STR2[86];

  if (doread) {
    getobj(objnum);
    freeobj();
  }
  strcpy(s, obj.oname);
  switch (obj.particle) {

  case 0:
    /* blank case */
    break;

  case 1:
    sprintf(s, "a %s", strcpy(STR1, s));
    break;

  case 2:
    sprintf(s, "an %s", strcpy(STR1, s));
    break;

  case 3:
    sprintf(s, "some %s", strcpy(STR2, s));
    break;

  case 4:
    sprintf(s, "the %s", strcpy(STR2, s));
    break;
  }
  return strcpy(Result, s);
}


void print_subs(n, s)
int n;
char *s;
{
  string STR1;

  if (n <= 0 || n == DEFAULT_LINE) {
    if (n == DEFAULT_LINE)
      mprintf("?<default line> in print_subs\n");
    return;
  }
  getline(n);
  freeline();
  puts(subs_parm(STR1, oneliner.theline, s));
}



/* print out a (up to) 10 line description block, substituting string s for
   up to one occurance of # per line */

void block_subs(n, s)
int n;
char *s;
{
  int p;
  int i = 1;
  string STR1;

  if (n < 0) {
    print_subs(abs(n), s);
    return;
  }
  if (n <= 0 || n == DEFAULT_LINE)
    return;
  getblock(n);
  freeblock();
  while (i <= block.desclen) {
    p = strpos2("#", block.lines[i-1], 1);
    if (p > 0)
      puts(subs_parm(STR1, block.lines[i-1], s));
    else
      puts(block.lines[i-1]);
    i++;
  }
}


void show_noises(n)
int n;
{
  if (n < 33) {
    mprintf("There are strange noises coming from behind you.\n");
    return;
  }
  if (n < 66)
    mprintf("You hear strange rustling noises behind you.\n");
  else
    mprintf("There are faint noises coming from behind you.\n");
}


void show_altnoise(n)
int n;
{
  if (n < 33) {
    mprintf("A chill wind blows, ruffling your clothes and chilling your bones.\n");
    return;
  }
  if (n < 66)
    mprintf("Muffled scuffling sounds can be heard behind you.\n");
  else
    mprintf("A loud crash can be heard in the distance.\n");
}


void show_midnight(n, printed)
int n;
boolean *printed;
{
  if (!midnight_notyet) {
    *printed = false;
    return;
  }
  if (n < 50) {
    mprintf("A voice booms out of the air from all around you!\n");
    mprintf("The voice says,  \" It is now midnight. \"\n");
  } else {
    mprintf("You hear a clock chiming in the distance.\n");
    mprintf("It rings twelve times for midnight.\n");
  }
  midnight_notyet = false;
}




void handle_event(printed)
boolean *printed;
{
  int send, act, targ, p;
  string s, sendname, STR3;
  anevent *WITH;

  *printed = true;
  if (debug)
    mprintf("?handling event %12d\n", myevent);
  WITH = &event.evnt[myevent-1];
  send = WITH->sender;
  act = WITH->action;
  targ = WITH->target;
  p = WITH->parm;
  strcpy(s, WITH->msg);
  if (send != 0)
    strcpy(sendname, here.people[send-1].name);
  else
    strcpy(sendname, "<Unknown>");

  switch (act) {

  case E_EXIT:
    if (here.exits[targ-1].goin == DEFAULT_LINE)
      mprintf("%s has gone %s.\n", s, direct[targ-1]);
    else if (here.exits[targ-1].goin != 0 &&
	     here.exits[targ-1].goin != DEFAULT_LINE)
      block_subs(here.exits[targ-1].goin, s);
    else
      *printed = false;
    break;

  case E_ENTER:
    if (here.exits[targ-1].comeout == DEFAULT_LINE)
      mprintf("%s has come into the room from: %s\n", s, direct[targ-1]);
    else if (here.exits[targ-1].comeout != 0 &&
	     here.exits[targ-1].comeout != DEFAULT_LINE)
      block_subs(here.exits[targ-1].comeout, s);
    else
      *printed = false;
    break;

  case E_BEGIN:
    mprintf("%s appears in a brilliant burst of multicolored light.\n", s);
    break;

  case E_QUIT:
    mprintf("%s vanishes in a brilliant burst of multicolored light.\n", s);
    break;

  case E_SAY:
    if (strlen(s) + strlen(sendname) > 73) {
      mprintf("%s says,\n", sendname);
      mprintf("\"%s\"\n", s);
    } else {
      if (rnd100() < 50 || strlen(s) > 50)
	mprintf("%s: \"%s\"\n", sendname, s);
      else
	mprintf("%s says, \"%s\"\n", sendname, s);
    }
    break;

  case E_HIDESAY:
    mprintf("An unidentified voice speaks to you:\n");
    mprintf("\"%s\"\n", s);
    break;

  case E_SETNAM:
    puts(s);
    break;

  case E_POOFIN:
    mprintf("In an explosion of orange smoke %s poofs into the room.\n", s);
    break;

  case E_POOFOUT:
    mprintf("%s vanishes from the room in a cloud of orange smoke.\n", s);
    break;

  case E_DETACH:
    mprintf("%s has destroyed the exit %s.\n", s, direct[targ-1]);
    break;

  case E_EDITDONE:
    mprintf("%s is done editing the room description.\n", sendname);
    break;

  case E_NEWEXIT:
    mprintf("%s has created an exit here.\n", s);
    break;

  case E_CUSTDONE:
    mprintf("%s is done customizing an exit here.\n", sendname);
    break;

  case E_SEARCH:
    mprintf("%s seems to be looking for something.\n", sendname);
    break;

  case E_FOUND:
    mprintf("%s appears to have found something.\n", sendname);
    break;

  case E_DONEDET:
    mprintf("%s is done adding details to the room.\n", sendname);
    break;

  case E_ROOMDONE:
    mprintf("%s is finished customizing this room.\n", sendname);
    break;

  case E_OBJDONE:
    mprintf("%s is finished customizing an object.\n", sendname);
    break;

  case E_UNHIDE:
    mprintf("%s has stepped out of the shadows.\n", sendname);
    break;

  case E_FOUNDYOU:
    if (targ == myslot) {  /* found me! */
      mprintf("You've been discovered by %s!\n", sendname);
      hiding = false;
      getroom(0);
      /* they're not hidden anymore */
      here.people[myslot-1].hiding = 0;
      putroom();
    } else
      mprintf("%s has found %s hiding in the shadows!\n",
	     sendname, here.people[targ-1].name);
    break;

  case E_PUNCH:
    if (targ == myslot) {  /* punched me! */
      get_punch(p, sendname);
      take_hit(punch_force(p));
      /* relic, but not harmful */
      ping_answered = true;
      healthcycle = 0;
    } else
      view_punch(sendname, here.people[targ-1].name, p);
    break;

  case E_MADEOBJ:
    puts(s);
    break;

  case E_GET:
    puts(s);
    break;

  case E_DROP:
    puts(s);
    if (here.objdesc != 0)
      print_subs(here.objdesc, obj_part(STR3, p, true));
    break;

  case E_BOUNCEDIN:
    if (targ == 0 || targ == DEFAULT_LINE)
      mprintf("%s has bounced into the room.\n", obj_part(STR3, p, true));
    else
      print_subs(targ, obj_part(STR3, p, true));
    break;

  case E_DROPALL:
    mprintf("Some objects drop to the ground.\n");
    break;

  case E_EXAMINE:
    puts(s);
    break;

  case E_IHID:
    mprintf("%s has hidden in the shadows.\n", sendname);
    break;

  case E_NOISES:
    if (here.rndmsg == 0 || here.rndmsg == DEFAULT_LINE)
      show_noises(targ);
    else
      print_line(here.rndmsg);
    break;

  case E_ALTNOISE:
    if (here.xmsg2 == 0 || here.xmsg2 == DEFAULT_LINE)
      show_altnoise(targ);
    else
      block_subs(here.xmsg2, myname);
    break;

  case E_REALNOISE:
    show_noises(targ);
    break;

  case E_HIDOBJ:
    mprintf("%s has hidden the %s.\n", sendname, s);
    break;

  case E_PING:
    if (targ == myslot) {
      mprintf("%s is trying to ping you.\n", sendname);
      log_event(myslot, E_PONG, send, 0, "", 0);
    } else
      mprintf("%s is pinging %s.\n", sendname, here.people[targ-1].name);
    break;

  case E_PONG:
    ping_answered = true;
    break;

  case E_HIDEPUNCH:
    if (targ == myslot) {
      mprintf("%s pounces on you from the shadows!\n", sendname);
      take_hit(2);
    } else
      mprintf("%s jumps out of the shadows and attacks %s.\n",
	     sendname, here.people[targ-1].name);
    break;

  case E_SLIPPED:
    mprintf("The %s has slipped from %s's hands.\n", s, sendname);
    break;

  case E_HPOOFOUT:
    if (rnd100() > 50)
      mprintf("Great wisps of orange smoke drift out of the shadows.\n");
    else
      *printed = false;
    break;

  case E_HPOOFIN:
    if (rnd100() > 50)
      mprintf("Some wisps of orange smoke drift about in the shadows.\n");
    else
      *printed = false;
    break;

  case E_FAILGO:
    if (targ > 0) {
      mprintf("%s has failed to go ", sendname);
      mprintf("%s.\n", direct[targ-1]);
    }
    break;

  case E_TRYPUNCH:
    if (targ == myslot)
      mprintf("%s fails to punch you.\n", sendname);
    else
      mprintf("%s fails to punch %s.\n", sendname, here.people[targ-1].name);
    break;

  case E_PINGONE:
    if (targ == myslot) {  /* ohoh---pinged away */
      mprintf(
	"The Monster program regrets to inform you that a destructive ping has\n");
      mprintf("destroyed your existence.  Please accept our apologies.\n");
      exit(0);
    }
    mprintf("%s shimmers and vanishes from sight.\n", s);
    break;

  case E_CLAIM:
    mprintf("%s has claimed this room.\n", sendname);
    break;

  case E_DISOWN:
    mprintf("%s has disowned this room.\n", sendname);
    break;

  case E_WEAKER:
    here.people[send-1].health = targ;

    /* This is a hack for efficiency so we don't read the room record twice;
       we need the current data now for desc_health, but checkevents, our caller,
       is about to re-read it anyway; we make an incremental fix here so desc_health
       is happy, then checkevents will do the real read later */

    desc_health(send, "");
    break;
    /*inmem := false;
                                    gethere;*/

  case E_OBJCLAIM:
    mprintf("%s is now the owner of the %s.\n", sendname, s);
    break;

  case E_OBJDISOWN:
    mprintf("%s has disowned the object %s.\n", sendname, s);
    break;

  case E_SELFDONE:
    mprintf("%s's self-description is finished.\n", sendname);
    break;

  case E_WHISPER:
    if (targ == myslot) {
      if (strlen(s) < 39)
	mprintf("%s whispers to you, \"%s\"\n", sendname, s);
      else {
	mprintf("%s whispers something to you:\n", sendname);
	mprintf("%s whispers, ", sendname);
	if (strlen(s) > 50)
	  putchar('\n');
	mprintf("\"%s\"\n", s);
      }
    } else if (privd || rnd100() > 85) {
      mprintf("You overhear %s whispering to %s!\n",
	     sendname, here.people[targ-1].name);
      mprintf("%s whispers, ", sendname);
      if (strlen(s) > 50)
	putchar('\n');
      mprintf("\"%s\"\n", s);
    } else
      mprintf("%s is whispering to %s.\n", sendname, here.people[targ-1].name);
    break;

  case E_WIELD:
    mprintf("%s is now wielding the %s.\n", sendname, s);
    break;

  case E_UNWIELD:
    mprintf("%s is no longer wielding the %s.\n", sendname, s);
    break;

  case E_WEAR:
    mprintf("%s is now wearing the %s.\n", sendname, s);
    break;

  case E_UNWEAR:
    mprintf("%s has taken off the %s.\n", sendname, s);
    break;

  case E_DONECRYSTALUSE:
    mprintf("%s emerges from the glow of the crystal.\n", sendname);
    mprintf("The orb becomes dark.\n");
    break;

  case E_DESTROY:
    puts(s);
    break;

  case E_OBJPUBLIC:
    mprintf("The object %s is now public.\n", s);
    break;

  case E_SYSDONE:
    mprintf("%s is no longer in system maintenance mode.\n", sendname);
    break;

  case E_UNMAKE:
    mprintf("%s has unmade %s.\n", sendname, s);
    break;

  case E_LOOKDETAIL:
    mprintf("%s is looking at the %s.\n", sendname, s);
    break;

  case E_ACCEPT:
    mprintf("%s has accepted an exit here.\n", sendname);
    break;

  case E_REFUSE:
    mprintf("%s has refused an Accept here.\n", sendname);
    break;

  case E_DIED:
    mprintf("%s expires and vanishes in a cloud of greasy black smoke.\n", s);
    break;

  case E_LOOKYOU:
    if (targ == myslot)
      mprintf("%s is looking at you.\n", sendname);
    else
      mprintf("%s looks at %s.\n", sendname, here.people[targ-1].name);
    break;

  case E_LOOKSELF:
    mprintf("%s is making a self-appraisal.\n", sendname);
    break;

  case E_FAILGET:
    mprintf("%s fails to get %s.\n", sendname, obj_part(STR3, targ, true));
    break;

  case E_FAILUSE:
    mprintf("%s fails to use %s.\n", sendname, obj_part(STR3, targ, true));
    break;

  case E_CHILL:
    if (targ == 0 || targ == DEFAULT_LINE)
      mprintf("A chill wind blows over you.\n");
    else
      print_desc(targ, "<no default supplied>");
    break;

  case E_NOISE2:
    switch (targ) {

    case 1:
      mprintf("Strange, gutteral noises sound from everywhere.\n");
      break;

    case 2:
      mprintf(
	"A chill wind blows past you, almost whispering as it ruffles your clothes.\n");
      break;

    case 3:
      mprintf("Muffled voices speak to you from the air!\n");
      break;

    default:
      mprintf("The air vibrates with a chill shudder.\n");
      break;
    }
    break;

  case E_INVENT:
    mprintf("%s is taking inventory.\n", sendname);
    break;

  case E_POOFYOU:
    if (targ == myslot) {
      mprintf("\n%s directs a firey burst of bluish energy at you!\n",
	     sendname);
      mprintf(
	"Suddenly, you find yourself hurtling downwards through misty orange clouds.\n");
      mprintf(
	"Your descent slows, the smoke clears, and you find yourself in a new place...\n");
      xpoof(p);
      putchar('\n');
    } else {
      mprintf("%s directs a firey burst of energy at %s!\n",
	     sendname, here.people[targ-1].name);
      mprintf("A thick burst of orange smoke results, and when it clears, you see\n");
      mprintf("that %s is gone.\n", here.people[targ-1].name);
    }
    break;

  case E_WHO:
    switch (p) {

    case 0:
      mprintf("%s produces a \"who\" list and reads it.\n", sendname);
      break;

    case 1:
      mprintf("%s is seeing who's playing Monster.\n", sendname);
      break;

    default:
      mprintf("%s checks the \"who\" list.\n", sendname);
      break;
    }
    break;

  case E_PLAYERS:
    mprintf("%s checks the \"players\" list.\n", sendname);
    break;

  case E_VIEWSELF:
    mprintf("%s is reading %s's self-description.\n", sendname, s);
    break;

  case E_MIDNIGHT:
    show_midnight(targ, printed);
    break;

  case E_ACTION:
    mprintf("%s is%s\n", sendname, desc_action(STR3, p, targ));
    break;

  default:
    mprintf("*** Bad Event ***\n");
    break;
  }
}
/* p2c: mon.pas, line 3429: Warning: Type attribute GLOBAL ignored [128] */


void checkevents(silent)
boolean silent;
{
  boolean gotone = false;
  boolean tmp;
  boolean printed = false;
  char STR1[164];

  getevent(0);
  freeevent();

  memcpy(&event, &eventfile_hat, sizeof(eventrec));
  while (myevent != event.point) {
    myevent++;
    if (myevent > maxevent)
      myevent = 1;

    if (debug) {
      mprintf("?checking event %12d\n", myevent);
      if (event.evnt[myevent-1].loc == location)
	mprintf("  - event here\n");
      else
	mprintf("  - event elsewhere\n");
      mprintf("  - event number = %d\n", event.evnt[myevent-1].action);
    }

    if (event.evnt[myevent-1].loc != location)
      continue;
    if (event.evnt[myevent-1].sender == myslot)
      continue;

    /* if sent by me don't look at it */
    /* will use global record event */
    putchars("\015\033[K");
    handle_event(&tmp);
    if (tmp)
      printed = true;

    inmem = false;   /* re-read important data that */
    gethere(0);   /* may have been altered */

    gotone = true;
  }
  if (printed && gotone && !silent) {
    mprintf("%s%s", old_prompt, line);
    in_grab_line = 1;
  }

  rnd_event(silent);
}



/* count the number of people in this room; assumes a gethere has been done */

int find_numpeople()
{
  int sum = 0;
  int i;

  for (i = 0; i < maxpeople; i++) {
    if (here.people[i].kind > 0) {
      /*if here.people[i].username <> '' then*/
      sum++;
    }
  }
  return sum;
}



/* don't give them away, but make noise--maybe
   percent is percentage chance that they WON'T make any noise */

void noisehide(percent)
int percent;
{
  /* assumed gethere;  */
  if (hiding && find_numpeople() > 1) {
    if (rnd100() > percent)
      log_event(myslot, E_REALNOISE, rnd100(), 0, "", 0);
    /* myslot: don't tell them they made noise */
  }
}



boolean checkhide()
{
  boolean Result = false;

  if (!hiding)
    return true;
  noisehide(50);
  mprintf("You can't do that while you're hiding.\n");
  return false;
}



void clear_command()
{
  if (!logged_act)
    return;
  getroom(0);
  here.people[myslot-1].act = 0;
  putroom();
  logged_act = false;
}


/* forward procedure take_token(aslot, roomno: integer); */
void take_token(aslot, roomno)
int aslot, roomno;
{
  /* remove self from a room's people list */
  peoplerec *WITH;

  getroom(roomno);
  WITH = &here.people[aslot-1];
  WITH->kind = 0;
  *WITH->username = '\0';
  *WITH->name = '\0';
  putroom();
}


/* fowrard function put_token(room: integer;var aslot:integer;
        hidelev:integer := 0):boolean;
                         put a person in a room's people list
                         returns myslot */
boolean put_token(room_, aslot, hidelev)
int room_, *aslot, hidelev;
{
  boolean Result;
  int i, j;
  boolean found = false;
  int savehold[maxhold];

  if (first_puttoken) {
    for (i = 0; i < maxhold; i++)
      savehold[i] = 0;
    first_puttoken = false;
  } else {
    gethere(0);
    for (i = 0; i < maxhold; i++)
      savehold[i] = here.people[myslot-1].holding[i];
  }

  getroom(room_);
  i = 1;
  while (i <= maxpeople && !found) {
    if (*here.people[i-1].name == '\0')
      found = true;
    else
      i++;
  }
  Result = found;
  if (!found) {
    freeroom();
    return Result;
  }
  here.people[i-1].kind = 1;   /* I'm a real player */
  strcpy(here.people[i-1].name, myname);
  strcpy(here.people[i-1].username, userid);
  here.people[i-1].hiding = hidelev;
  /* hidelev is zero for most everyone
     unless you want to poof in and remain hidden */

  here.people[i-1].wearing = mywear;
  here.people[i-1].wielding = mywield;
  here.people[i-1].health = myhealth;
  here.people[i-1].self = myself;

  here.people[i-1].act = 0;

  for (j = 0; j < maxhold; j++)
    here.people[i-1].holding[j] = savehold[j];
  putroom();

  *aslot = i;
  for (j = 0; j < maxexit; j++)   /* haven't found any exits in */
    found_exit[j] = false;
  /* the new room */

  /* note the user's new location in the logfile */
  getint(N_LOCATION);
  anint.int_[mylog-1] = room_;
  putint();
  return Result;
}


void log_exit(direction, room_, sender_slot)
int direction, room_, sender_slot;
{
  log_event(sender_slot, E_EXIT, direction, 0, myname, room_);
}


void log_entry(direction, room_, sender_slot)
int direction, room_, sender_slot;
{
  log_event(sender_slot, E_ENTER, direction, 0, myname, room_);
}


void log_begin(room_)
int room_;
{
  log_event(0, E_BEGIN, 0, 0, myname, room_);
}


void log_quit(room_, dropped)
int room_;
boolean dropped;
{
  log_event(0, E_QUIT, 0, 0, myname, room_);
  if (dropped)
    log_event(0, E_DROPALL, 0, 0, myname, room_);
}




/* return the number of people you can see here */

int n_can_see()
{
  int Result;
  int sum = 0;
  int i, selfslot;

  if (here.locnum == location)
    selfslot = myslot;
  else {
    selfslot = 0;

  }
  for (i = 1; i <= maxpeople; i++) {
    if (i != selfslot && *here.people[i-1].name != '\0' &&
	here.people[i-1].hiding == 0)
      sum++;
  }
  Result = sum;
  if (debug)
    mprintf("?n_can_see = %d\n", sum);
  return Result;
}



char *next_can_see(Result, point)
char *Result;
int *point;
{
  boolean found = false;
  int selfslot;

  if (here.locnum != location)
    selfslot = 0;
  else
    selfslot = myslot;
  while (!found && *point <= maxpeople) {
    if (*point != selfslot && *here.people[*point - 1].name != '\0' &&
	here.people[*point - 1].hiding == 0)
      found = true;
    else
      (*point)++;
  }

  if (found) {
    strcpy(Result, here.people[*point - 1].name);
    (*point)++;
  } else {
    strcpy(Result, myname);   /* error!  error! */
    mprintf("?searching error in next_can_see; notify the Monster Manager\n");
  }
  return Result;
}


void niceprint(len, s)
int *len;
char *s;
{
  if (*len + strlen(s) > 78) {
    *len = 0;
    putchar('\n');
  } else
    *len += strlen(s);
  fputs(s, stdout);
}


void people_header(where)
char *where;
{
  int point = 1;
  string tmp;
  int i, n, len;
  string STR1, STR2;
  char STR3[26];

  n = n_can_see();
  switch (n) {

  case 0:
    /* blank case */
    break;

  case 1:
    mprintf("%s is %s\n", next_can_see(STR1, &point), where);
    break;

  case 2:
    mprintf("%s and %s are %s\n",
	   next_can_see(STR1, &point), next_can_see(STR2, &point), where);
    break;

  default:
    len = 0;
    for (i = 1; i < n; i++) {  /* at least 1 to 2 */
      next_can_see(tmp, &point);
      if (i != n - 1)
	strcat(tmp, ", ");
      niceprint(&len, tmp);
    }

    niceprint(&len, " and ");
    niceprint(&len, next_can_see(STR1, &point));
    sprintf(STR3, " are %s", where);
    niceprint(&len, STR3);
    putchar('\n');
    break;
  }
}


void desc_person(i)
int i;
{
  shortstring pname;
  string STR3;

  strcpy(pname, here.people[i-1].name);

  if (here.people[i-1].act != 0) {
    mprintf("%s is", pname);
    puts(desc_action(STR3, here.people[i-1].act, here.people[i-1].targ));
    /* describes what person last did */
  }

  if (here.people[i-1].health != GOODHEALTH)
    desc_health(i, "");

  if (here.people[i-1].wielding > 0)
    mprintf("%s is wielding %s.\n",
	   pname, obj_part(STR3, here.people[i-1].wielding, true));

}


void show_people()
{
  int i;

  people_header("here.");
  for (i = 1; i <= maxpeople; i++) {
    if (*here.people[i-1].name != '\0' && i != myslot &&
	here.people[i-1].hiding == 0)
      desc_person(i);
  }
}


void show_group()
{
  int gloc1, gloc2;
  shortstring gnam1, gnam2;

  gloc1 = here.grploc1;
  gloc2 = here.grploc2;
  strcpy(gnam1, here.grpnam1);
  strcpy(gnam2, here.grpnam2);

  if (gloc1 != 0) {
    gethere(gloc1);
    people_header(gnam1);
  }
  if (gloc2 != 0) {
    gethere(gloc2);
    people_header(gnam2);
  }
  gethere(0);
}


void desc_obj(n)
int n;
{
  string STR2;

  if (n == 0)
    return;
  getobj(n);
  freeobj();
  if (obj.linedesc == DEFAULT_LINE) {
    mprintf("On the ground here is %s.\n", obj_part(STR2, n, false));

    /* the FALSE means obj_part shouldn't do its
       own getobj, cause we already did one */
  } else
    print_line(obj.linedesc);
}


void show_objects()
{
  int i;

  for (i = 0; i < maxobjs; i++) {
    if (here.objs[i] != 0 && here.objhide[i] == 0)
      desc_obj(here.objs[i]);
  }
}


boolean lookup_detail(n, s_)
int *n;
char *s_;
{
  string s;
  int i = 1;
  int poss;
  int maybe = 0, num = 0;
  string STR1;

  strcpy(s, s_);
  *n = 0;
  strcpy(s, lowcase(STR1, s));
  for (i = 1; i <= maxdetail; i++) {
    if (!strcmp(s, here.detail[i-1]))
      num = i;
    else if (strncmp(s, here.detail[i-1], strlen(s)) == 0) {
      maybe++;
      poss = i;
    }
  }
  if (num != 0) {
    *n = num;
    return true;
  } else if (maybe == 1) {
    *n = poss;
    return true;
  } else if (maybe > 1)
    return false;
  else
    return false;
}


boolean look_detail(s)
char *s;
{
  int n;

  if (lookup_detail(&n, s)) {
    if (here.detaildesc[n-1] == 0)
      return false;
    else {
      print_desc(here.detaildesc[n-1], "<no default supplied>");
      log_event(myslot, E_LOOKDETAIL, 0, 0, here.detail[n-1], 0);
      return true;
    }
  } else
    return false;
}


boolean look_person(s)
char *s;
{
  int objnum, i, n;
  boolean first;
  string STR3;

  if (parse_pers(&n, s)) {
    if (n == myslot) {
      log_event(myslot, E_LOOKSELF, n, 0, "", 0);
      mprintf(
	"You step outside of yourself for a moment to get an objective self-appraisal:\n\n");
    } else
      log_event(myslot, E_LOOKYOU, n, 0, "", 0);
    if (here.people[n-1].self != 0) {
      print_desc(here.people[n-1].self, "<no default supplied>");
      putchar('\n');
    }

    desc_health(n, "");

    /* Do an inventory of person S */
    first = true;
    for (i = 0; i < maxhold; i++) {
      objnum = here.people[n-1].holding[i];
      if (objnum != 0) {
	if (first) {
	  mprintf("%s is holding:\n", here.people[n-1].name);
	  first = false;
	}
	mprintf("   %s\n", obj_part(STR3, objnum, true));
      }
    }
    if (first)
      mprintf("%s is empty handed.\n", here.people[n-1].name);

    return true;
  } else
    return false;
}



void do_examine(s, three, silent)
char *s;
boolean *three, silent;
{
  int n;
  string msg, STR2;

  *three = false;
  if (!parse_obj(&n, s, false)) {
    if (!silent)
      mprintf("That object cannot be seen here.\n");
    return;
  }
  if (!(obj_here(n) || obj_hold(n))) {
    if (!silent)
      mprintf("That object cannot be seen here.\n");
    return;
  }
  *three = true;

  getobj(n);
  freeobj();
  sprintf(msg, "%s is examining %s.", myname, obj_part(STR2, n, true));
  log_event(myslot, E_EXAMINE, 0, 0, msg, 0);
  if (obj.examine == 0)
    mprintf("You see nothing special about the %s.\n", objnam.idents[n-1]);
  else
    print_desc(obj.examine, "<no default supplied>");
}



void print_room()
{
  switch (here.nameprint) {

  case 0:   /* don't print name */
    break;

  case 1:
    mprintf("You're in %s\n", here.nicename);
    break;

  case 2:
    mprintf("You're at %s\n", here.nicename);
    break;
  }

  if (brief) {
    return;
  }  /* if not(brief) */
  switch (here.which) {

  case 0:
    print_desc(here.primary, "<no default supplied>");
    break;

  case 1:
    print_desc(here.secondary, "<no default supplied>");
    break;

  case 2:
    print_desc(here.primary, "<no default supplied>");
    print_desc(here.secondary, "<no default supplied>");
    break;

  case 3:
    print_desc(here.primary, "<no default supplied>");
    if (here.magicobj != 0) {
      if (obj_hold(here.magicobj))
	print_desc(here.secondary, "<no default supplied>");
    }
    break;

  case 4:
    if (here.magicobj != 0) {
      if (obj_hold(here.magicobj))
	print_desc(here.secondary, "<no default supplied>");
      else
	print_desc(here.primary, "<no default supplied>");
    } else
      print_desc(here.primary, "<no default supplied>");
    break;
  }
  putchar('\n');
}



void do_look(s)
char *s;
{
  boolean one, two, three;

  gethere(0);
  if (*s == '\0') {  /* do an ordinary top-level room look */
    if (hiding) {
      mprintf("You can't get a very good view of the details of the room from where\n");
      mprintf("you are hiding.\n");
      noisehide(67);
    } else {
      print_room();
      show_exits();
    }
    /* end of what you can't see when you're hiding */
    show_people();
    show_group();
    show_objects();
    return;
  }

  one = look_detail(s);
  two = look_person(s);
  do_examine(s, &three, true);
  if (!(one || two || three)) {
    mprintf("There isn't anything here by that name to look at.\n");
    /* look at a detail in the room */
  }
}


void init_exit(dir)
int dir;
{
  exit_ *WITH;

  WITH = &here.exits[dir-1];
  WITH->exitdesc = DEFAULT_LINE;
  WITH->fail = DEFAULT_LINE;   /* default descriptions */
  WITH->success = 0;   /* until they customize */
  WITH->comeout = DEFAULT_LINE;
  WITH->goin = DEFAULT_LINE;
  WITH->closed = DEFAULT_LINE;

  WITH->objreq = 0;   /* not a door (yet) */
  WITH->hidden = 0;   /* not hidden */
  WITH->reqalias = false;
  /* don't require alias (i.e. can use
                            direction of exit North, east, etc. */
  WITH->reqverb = false;
  WITH->autolook = true;
  *WITH->alias = '\0';
}



void remove_exit(dir)
int dir;
{
  int targroom, targslot;
  boolean hereacc, targacc;

  /* Leave residual accepts if player is not the owner of
     the room that the exit he is deleting is in */

  getroom(0);
  targroom = here.exits[dir-1].toloc;
  targslot = here.exits[dir-1].slot;
  here.exits[dir-1].toloc = 0;
  init_exit(dir);

  if (!strcmp(here.owner, userid) || privd)
    hereacc = false;
  else
    hereacc = true;

  if (hereacc)
    here.exits[dir-1].kind = 5;   /* put an "accept" in its place */
  else
    here.exits[dir-1].kind = 0;

  putroom();
  log_event(myslot, E_DETACH, dir, 0, myname, location);

  getroom(targroom);
  here.exits[targslot-1].toloc = 0;

  if (!strcmp(here.owner, userid) || privd)
    targacc = false;
  else
    targacc = true;

  if (targacc)
    here.exits[targslot-1].kind = 5;   /* put an "accept" in its place */
  else
    here.exits[targslot-1].kind = 0;

  putroom();

  if (targroom != location)
    log_event(0, E_DETACH, targslot, 0, myname, targroom);
  mprintf("Exit destroyed.\n");
}


/*
User procedure to unlink a room
*/
void do_unlink(s)
char *s;
{
  int dir;

  gethere(0);
  if (!checkhide())
    return;
  if (!lookup_dir(&dir, s)) {
    mprintf("To remove an exit, type UNLINK <direction of exit>.\n");
    return;
  }
  if (!can_alter(dir, 0)) {
    mprintf("You are not allowed to remove that exit.\n");
    return;
  }
  if (here.exits[dir-1].toloc == 0)
    mprintf("There is no exit there to unlink.\n");
  else
    remove_exit(dir);
}



boolean desc_allowed()
{
  if (!strcmp(here.owner, userid) || privd)
    return true;
  else {
    mprintf("Sorry, you are not allowed to alter the descriptions in this room.\n");
    return false;
  }
}



char *slead(Result, s)
char *Result, *s;
{
  int i;
  boolean going;

  if (*s == '\0')
    return strcpy(Result, "");
  else {
    i = 1;
    going = true;
    while (going) {
      if (i > strlen(s)) {
	going = false;
	break;
      }
      if (s[i-1] == ' ' || s[i-1] == '\t')
	i++;
      else
	going = false;
    }

    if (i > strlen(s))
      return strcpy(Result, "");
    else {
      sprintf(Result, "%.*s", (int)(strlen(s) - i + 1), s + i - 1);
      return Result;
    }
  }
}


char *bite(Result, s)
char *Result, *s;
{
  int i, orig_i;
  string STR1;
  char STR2[256];

  if (*s == '\0')
    return strcpy(Result, "");

  for (i = 0; i < strlen(s); i++)
    if (s[i] == ' ')
	break;
  if (i >= strlen(s))
	i = 0;

  if (i == 0) {
    strcpy(Result, s);
    *s = '\0';
   return Result;
  }

  if (s[i] == ' ') {
	  s[i] = '\0';
	  i++;
  }

strcpy(Result, s);
strcpy(s, &s[i]);
  return Result;
}


void edit_help()
{
  mprintf("\nA\tAppend text to end\n");
  mprintf("C\tCheck text for correct length with parameter substitution (#)\n");
  mprintf("D #\tDelete line #\n");
  mprintf("E\tExit & save changes\n");
  mprintf("I #\tInsert lines before line #\n");
  mprintf("P\tPrint out description\n");
  mprintf("Q\tQuit: THROWS AWAY CHANGES\n");
  mprintf("R #\tReplace text of line #\n");
  mprintf("Z\tZap all text\n");
  mprintf("@\tThrow away text & exit with the default description\n");
  mprintf("?\tThis list\n\n");
}


void edit_replace(n)
int n;
{
  string prompt, s;

  if (n > heredsc.desclen || n < 1) {
    mprintf("-- Bad line number\n");
    return;
  }
  sprintf(prompt, "%2d: ", n);
  grab_line(prompt, s, true);
  if (strcmp(s, "**"))
    strcpy(heredsc.lines[n-1], s);
}


void edit_insert(n)
int n;
{
  int i;

  if (heredsc.desclen == descmax) {
    mprintf("You have already used all %ld lines of text.\n", (long)descmax);
    return;
  }
  if (n < 1 || n > heredsc.desclen) {
    mprintf("Invalid line #; valid lines are between 1 and %d\n",
	   heredsc.desclen);
    mprintf("Use A (add) to add text to the end of your description.\n");
    return;
  }
  for (i = heredsc.desclen + 1; i > n; i--)
    strcpy(heredsc.lines[i-1], heredsc.lines[i-2]);
  heredsc.desclen++;
  *heredsc.lines[n-1] = '\0';
}


void edit_doinsert(n)
int n;
{
  string s, prompt;

  if (heredsc.desclen == descmax) {
    mprintf("You have already used all %ld lines of text.\n", (long)descmax);
    return;
  }
  if (n < 1 || n > heredsc.desclen) {
    mprintf("Invalid line #; valid lines are between 1 and %d\n",
	   heredsc.desclen);
    mprintf("Use A (add) to add text to the end of your description.\n");
    return;
  }
  do {
    sprintf(prompt, "%d: ", n);
    grab_line(prompt, s, true);
    if (strcmp(s, "**")) {
      edit_insert(n);   /* put the blank line in */
      strcpy(heredsc.lines[n-1], s);   /* copy this line onto it */
      n++;
    }
  } while (heredsc.desclen != descmax && strcmp(s, "**"));
}


void edit_show()
{
  int i = 1;

  putchar('\n');
  if (heredsc.desclen == 0) {
    mprintf("[no text]\n");
    return;
  }
  while (i <= heredsc.desclen) {
    mprintf("%2d: %s\n", i, heredsc.lines[i-1]);
    i++;
  }
}


void edit_append()
{
  string prompt, s;
  boolean stilladding = true;

  if (heredsc.desclen == descmax) {
    mprintf("You have already used all %ld lines of text.\n", (long)descmax);
    return;
  }
  mprintf("Enter text.  Terminate with ** at the beginning of a line.\n");
  mprintf("You have %ld lines maximum.\n\n", (long)descmax);
  while (heredsc.desclen < descmax && stilladding) {
    sprintf(prompt, "%2d: ", heredsc.desclen + 1);
    grab_line(prompt, s, true);
    if (!strcmp(s, "**"))
      stilladding = false;
    else {
      heredsc.desclen++;
      strcpy(heredsc.lines[heredsc.desclen - 1], s);
    }
  }
}


void edit_delete(n)
int n;
{
  int i, FORLIM;

  if (heredsc.desclen == 0) {
    mprintf("-- No lines to delete\n");
    return;
  }
  if (n > heredsc.desclen || n < 1) {
    mprintf("-- Bad line number\n");
    return;
  }
  if (n == 1 && heredsc.desclen == 1) {
    heredsc.desclen = 0;
    return;
  }
  FORLIM = heredsc.desclen;
  for (i = n; i < FORLIM; i++)
    strcpy(heredsc.lines[i-1], heredsc.lines[i]);
  heredsc.desclen--;
}


void check_subst()
{
  int i, FORLIM;

  if (heredsc.desclen <= 0)
    return;
  FORLIM = heredsc.desclen;
  for (i = 1; i <= FORLIM; i++) {
    if (strpos2("#", heredsc.lines[i-1], 1) > 0 &&
	strlen(heredsc.lines[i-1]) > 59)
      mprintf("Warning: line %d is too long for correct parameter substitution.\n",
	     i);
  }
}


boolean edit_desc(dsc)
int *dsc;
{
  boolean Result = true;
  char cmd;
  string s;
  boolean done = false;
  int n;
  char STR1[256];
  string STR2;

  if (*dsc == DEFAULT_LINE)
    heredsc.desclen = 0;
  else if (*dsc > 0) {
    getblock(*dsc);
    freeblock();
    memcpy(&heredsc, &block, sizeof(descrec));
  } else if (*dsc < 0) {
    n = -*dsc;
    getline(n);
    freeline();
    strcpy(heredsc.lines[0], oneliner.theline);
    heredsc.desclen = 1;
  } else
    heredsc.desclen = 0;

  if (heredsc.desclen == 0)
    edit_append();
  do {
    putchar('\n');
    do {
      grab_line("* ", s, true);
      strcpy(s, slead(STR2, s));
    } while (*s == '\0');
    strcpy(s, lowcase(STR2, s));
    cmd = s[0];

    if (strlen(s) > 1) {
      sprintf(STR1, "%.*s", (int)(strlen(s) - 1), s + 1);
      n = number(slead(STR2, STR1));
    } else
      n = 0;

    switch (cmd) {

    case 'h':
    case '?':
      edit_help();
      break;

    case 'a':
      edit_append();
      break;

    case 'z':
      heredsc.desclen = 0;
      break;

    case 'c':
      check_subst();
      break;

    case 'p':
    case 'l':
    case 't':
      edit_show();
      break;

    case 'd':
      edit_delete(n);
      break;

    case 'e':
      check_subst();
      if (debug)
	mprintf("edit_desc: dsc is %d\n", *dsc);


      /* what I do here may require some explanation:

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
      */

      /* kill the default */
      if (heredsc.desclen > 0 && *dsc == DEFAULT_LINE) {
	/* if we're gonna put real */
	/* texty in here */
	*dsc = 0;
      }

      /* no lines, delete existing */
      if (heredsc.desclen == 0) {
	/* desc, if any */
	delete_block(dsc);
      } else if (heredsc.desclen == 1) {
	if (*dsc == 0) {
	  alloc_line(dsc);
	  *dsc = -*dsc;
	} else if (*dsc > 0) {
	  delete_block(dsc);
	  alloc_line(dsc);
	  *dsc = -*dsc;
	}

	if (*dsc < 0) {
	  getline(abs(*dsc));
	  strcpy(oneliner.theline, heredsc.lines[0]);
	  putline();
	}
	/* more than 1 lines */
      } else {
	if (*dsc == 0)
	  alloc_block(dsc);
	else if (*dsc < 0) {
	  delete_line(dsc);
	  alloc_block(dsc);
	}

	if (*dsc > 0) {
	  getblock(*dsc);
	  memcpy(&block, &heredsc, sizeof(descrec));
	  /* This is a fudge */
	  block.descrinum = *dsc;
	  putblock();
	}
      }
      done = true;
      break;

    case 'r':
      edit_replace(n);
      break;

    case '@':
      delete_block(dsc);
      *dsc = DEFAULT_LINE;
      done = true;
      break;

    case 'i':
      edit_doinsert(n);
      break;

    case 'q':
      grab_line("Throw away changes, are you sure? ", s, true);
      strcpy(s, lowcase(STR2, s));
      if (!strcmp(s, "y") || !strcmp(s, "yes")) {
	done = true;
	Result = false;   /* signal caller not to save */
      }
      break;

    default:
      mprintf("-- Invalid command, type ? for a list.\n");
      break;
    }
  } while (!done);
  return Result;
}




boolean alloc_detail(n, s)
int *n;
char *s;
{
  boolean Result;
  boolean found = false;

  *n = 1;
  while (*n <= maxdetail && !found) {
    if (here.detaildesc[*n - 1] == 0)
      found = true;
    else
      (*n)++;
  }
  Result = found;
  if (!found) {
    *n = 0;
    return Result;
  }
  getroom(0);
  lowcase(here.detail[*n - 1], s);
  putroom();
  return Result;
}


/*
User describe procedure.  If no s then describe the room

Known problem: if two people edit the description to the same room one of their
        description blocks could be lost.
This is unlikely to happen unless the Monster Manager tries to edit a
description while the room's owner is also editing it.
*/
void do_describe(s)
char *s;
{
  int i, newdsc;

  gethere(0);
  if (!checkhide())
    return;
  if (*s == '\0') {  /* describe this room */
    if (!desc_allowed()) {
      /* describe a detail of this room */
      /*clear_command;*/
      return;
    }
    log_action(desc, 0);
    mprintf("[ Editing the primary room description ]\n");
    newdsc = here.primary;
    if (edit_desc(&newdsc)) {
      getroom(0);
      here.primary = newdsc;
      putroom();
    }
    log_event(myslot, E_EDITDONE, 0, 0, "", 0);
    return;
  }
  if (strlen(s) > veryshortlen) {
    mprintf("Your detail keyword can only be %ld characters.\n",
	   (long)veryshortlen);
    return;
  }
  if (!desc_allowed())
    return;
  if (!lookup_detail(&i, s)) {
    if (!alloc_detail(&i, s)) {
      mprintf("You have used all %ld details.\n", (long)maxdetail);
      mprintf("To delete a detail, DESCRIBE <the detail> and delete all the text.\n");
    }
  }
  if (i == 0)
    return;
  log_action(e_detail, 0);
  mprintf("[ Editing detail \"%s\" of this room ]\n", here.detail[i-1]);
  newdsc = here.detaildesc[i-1];
  if (edit_desc(&newdsc)) {
    getroom(0);
    here.detaildesc[i-1] = newdsc;
    putroom();
  }
  log_event(myslot, E_DONEDET, 0, 0, "", 0);
}




void del_room(n)
int n;
{
  int i;
  exit_ *WITH;

  getnam();
  *nam.idents[n-1] = '\0';   /* blank out name */
  putnam();

  getown();
  *own.idents[n-1] = '\0';   /* blank out owner */
  putown();

  getroom(n);
  for (i = 0; i < maxexit; i++) {
    WITH = &here.exits[i];
    delete_line(&WITH->exitdesc);
    delete_line(&WITH->fail);
    delete_line(&WITH->success);
    delete_line(&WITH->comeout);
    delete_line(&WITH->goin);
  }
  delete_block(&here.primary);
  delete_block(&here.secondary);
  putroom();
  delete_room(&n);   /* return room to free list */
}



void createroom(s)
char *s;
{
  /* create a room with name s */
  int roomno, dummy, i, rand_accept;
  exit_ *WITH;

  if (*s == '\0') {
    mprintf(
      "Please specify the name of the room you wish to create as a parameter to FORM.\n");
    return;
  }
  if (strlen(s) > shortlen) {
    mprintf("Please limit your room name to a maximum of %ld characters.\n",
	   (long)shortlen);
    return;
  }
  if (exact_room(&dummy, s)) {
    mprintf("That room name has already been used.  Please give a unique room name.\n");
    return;
  }
  if (!alloc_room(&roomno))
    return;
  log_action(form, 0);

  getnam();
  lowcase(nam.idents[roomno-1], s);   /* assign room name */
  putnam();   /* case insensitivity */

  getown();
  strcpy(own.idents[roomno-1], userid);   /* assign room owner */
  putown();

  getroom(roomno);

  here.primary = 0;
  here.secondary = 0;
  here.which = 0;   /* print primary desc only by default */
  here.magicobj = 0;

  strcpy(here.owner, userid);   /* owner and name are stored here too */
  strcpy(here.nicename, s);
  here.nameprint = 1;   /* You're in ... */
  here.objdrop = 0;   /* objects dropped stay here */
  here.objdesc = 0;   /* nothing printed when they drop */
  here.magicobj = 0;   /* no magic object default */
  here.trapto = 0;   /* no trapdoor */
  here.trapchance = 0;   /* no chance */
  here.rndmsg = DEFAULT_LINE;   /* bland noises message */
  here.pile = 0;
  here.grploc1 = 0;
  here.grploc2 = 0;
  *here.grpnam1 = '\0';
  *here.grpnam2 = '\0';

  here.effects = 0;
  here.parm = 0;

  here.xmsg2 = 0;
  here.exp2 = 0;
  here.exp3 = 0;
  here.exp4 = 0;
  here.exitfail = DEFAULT_LINE;
  here.ofail = DEFAULT_LINE;

  for (i = 0; i < maxpeople; i++)
    here.people[i].kind = 0;

  for (i = 0; i < maxpeople; i++)
    *here.people[i].name = '\0';

  for (i = 0; i < maxobjs; i++)
    here.objs[i] = 0;

  for (i = 0; i < maxdetail; i++)
    *here.detail[i] = '\0';
  for (i = 0; i < maxdetail; i++)
    here.detaildesc[i] = 0;

  for (i = 0; i < maxobjs; i++)
    here.objhide[i] = 0;

  for (i = 0; i < maxexit; i++) {
    WITH = &here.exits[i];
    WITH->toloc = 0;
    WITH->kind = 0;
    WITH->slot = 0;
    WITH->exitdesc = DEFAULT_LINE;
    WITH->fail = DEFAULT_LINE;
    WITH->success = 0;   /* no success desc by default */
    WITH->goin = DEFAULT_LINE;
    WITH->comeout = DEFAULT_LINE;
    WITH->closed = DEFAULT_LINE;

    WITH->objreq = 0;
    WITH->hidden = 0;
    *WITH->alias = '\0';

    WITH->reqverb = false;
    WITH->reqalias = false;
    WITH->autolook = true;
  }

  /*here.exits := zero;*/

  /* random accept for this room */
  rand_accept = rnd100() % 6 + 1;
/* p2c: mon.pas, line 4689:
 * Note: Using % for possibly-negative arguments [317] */
  here.exits[rand_accept-1].kind = 5;

  putroom();
}



void show_help()
{
  string s;

  mprintf(
    "\nAccept/Refuse #  Allow others to Link an exit here at direction # | Undo Accept\n");
  mprintf("Brief            Toggle printing of room descriptions\n");
  mprintf(
    "Customize [#]    Customize this room | Customize exit # | Customize object #\n");
  mprintf("Describe [#]     Describe this room | Describe a feature (#) in detail\n");
  mprintf(
    "Destroy #        Destroy an instance of object # (you must be holding it)\n");
  mprintf("Duplicate #      Make a duplicate of an already-created object.\n");
  mprintf("Form/Zap #       Form a new room with name # | Destroy room named #\n");
  mprintf("Get/Drop #       Get/Drop an object\n");
  mprintf(
    "#,Go #           Go towards # (Some: N/North S/South E/East W/West U/Up D/Down)\n");
  mprintf("Health           Show how healthy you are\n");
  mprintf("Hide/Reveal [#]  Hide/Reveal yoursef | Hide object (#)\n");
  mprintf("I,Inventory      See what you or someone else is carrying\n");
  mprintf(
    "Link/Unlink #    Link/Unlink this room to/from another via exit at direction #\n");
  mprintf("Look,L [#]       Look here | Look at something or someone (#) closely\n");
  mprintf("Make #           Make a new object named #\n");
  mprintf("Name #           Set your game name to #\n");
  mprintf("Players          List people who have played Monster\n");
  mprintf("Punch #          Punch person #\n");
  mprintf("Quit             Leave the game\n");
  mprintf("Relink           Move an exit\n\n");
  grab_line("-more-", s, true);
  mprintf("\nRooms            Show information about rooms you have made\n");
  mprintf(
    "Say, ' (quote)   Say line of text following command to others in the room\n");
  mprintf("Search           Look around the room for anything hidden\n");
  mprintf(
    "Self #           Edit a description of yourself | View #'s self-description\n");
  mprintf("Show #           Show option # (type SHOW ? for a list)\n");
  mprintf("Unmake #         Remove the form definition of object #\n");
  mprintf("Use #            Use object #\n");
  mprintf("Wear #           Wear the object #\n");
  mprintf("Wield #          Wield the weapon #;  you must be holding it first\n");
  mprintf("Whisper #        Whisper something (prompted for) to person #\n");
  mprintf("Who              List of people playing Monster now\n");
  mprintf("Whois #          What is a player's username\n");
  mprintf("?,Help           This list\n");
  mprintf(". (period)       Repeat last command\n\n");
}


int lookup_cmd(s_)
char *s_;
{
  string s;
  int i = 1;   /* index for loop */
  int poss;   /* a possible match -- only for partial matches */
  int maybe = 0;   /* number of possible matches we have: > 2 is ambig. */
  /* the definite match */
  int num = 0;
  string STR1;
  int FORLIM;


  /* "Command not found " */
  strcpy(s, s_);
  strcpy(s, lowcase(STR1, s));
  FORLIM = numcmds;
  for (i = 1; i <= FORLIM; i++) {
    if (!strcmp(s, cmds[i-1]))
      num = i;
    else if (strncmp(s, cmds[i-1], strlen(s)) == 0) {
      maybe++;
      poss = i;
    }
  }
  if (num != 0)
    return num;
  else if (maybe == 1)
    return poss;
  else if (maybe > 1)
    return error;   /* "Ambiguous" */
  else
    return error;
}


void addrooms(n)
int n;
{
  int i, FORLIM;

  getindex(I_ROOM);
  FORLIM = indx.top + n;
  for (i = indx.top + 1; i <= FORLIM; i++) {
    lseek(roomfile, (i - 1L) * sizeof(room), 0);
    bzero(&roomfile_hat, sizeof(roomfile_hat));
    roomfile_hat.valid = i;
    roomfile_hat.locnum = i;
    roomfile_hat.primary = 0;
    roomfile_hat.secondary = 0;
    roomfile_hat.which = 0;
    write(roomfile, &roomfile_hat, sizeof(roomfile_hat));
  }
  indx.top += n;
  putindex();
}



void addints(n)
int n;
{
  int i, FORLIM;

  getindex(I_INT);
  FORLIM = indx.top + n;
  for (i = indx.top + 1; i <= FORLIM; i++) {
    lseek(intfile, (i - 1L) * sizeof(intrec), 0);
    bzero(&intfile_hat, sizeof(intfile_hat));
    intfile_hat.intnum = i;
    write(intfile, &intfile_hat, sizeof(intfile_hat));
  }
  indx.top += n;
  putindex();
}



void addlines(n)
int n;
{
  int i, FORLIM;

  getindex(I_LINE);
  FORLIM = indx.top + n;
  for (i = indx.top + 1; i <= FORLIM; i++) {
    lseek(linefile, (i - 1L) * sizeof(linerec), 0);
    bzero(&linefile_hat, sizeof(linefile_hat));
    linefile_hat.linenum = i;
    write(linefile, &linefile_hat, sizeof(linefile_hat));
  }
  indx.top += n;
  putindex();
}


void addblocks(n)
int n;
{
  int i, FORLIM;

  getindex(I_BLOCK);
  FORLIM = indx.top + n;
  for (i = indx.top + 1; i <= FORLIM; i++) {
    lseek(descfile, (i - 1L) * sizeof(descrec), 0);
    bzero(&descfile_hat, sizeof(descfile_hat));
    descfile_hat.descrinum = i;
    write(descfile, &descfile_hat, sizeof(descfile_hat));
  }
  indx.top += n;
  putindex();
}


void addobjects(n)
int n;
{
  int i, FORLIM;

  getindex(I_OBJECT);
  FORLIM = indx.top + n;
  for (i = indx.top + 1; i <= FORLIM; i++) {
    lseek(objfile, (i - 1L) * sizeof(objectrec), 0);
    bzero(&objfile_hat, sizeof(objfile_hat));
    objfile_hat.objnum = i;
    write(objfile, &objfile_hat, sizeof(objfile_hat));
  }
  indx.top += n;
  putindex();
}


void dist_list()
{
  int i, j;
  FILE *f;
  intrec where_they_are;

  mprintf("Writing distribution list . . .\n");
  f = fopen("monsters.dis", "w");

  getindex(I_PLAYER);   /* Rec of valid player log records  */
  freeindex();   /* False if a valid player log */

  getuser();   /* Corresponding userids of players */
  freeuser();

  getpers();   /* Personal names of players */
  freepers();

  getdate();   /* date of last play */
  freedate();

  if (privd) {
    getint(N_LOCATION);
    freeint();
    memcpy(&where_they_are, &anint, sizeof(intrec));

    getnam();
    freenam();
  }

  for (i = 0; i < maxplayers; i++) {
    if (!indx.free[i]) {
      fputs(user.idents[i], f);
      for (j = strlen(user.idents[i]); j <= 15; j++)
	putc(' ', f);
      fprintf(f, "! %s", pers.idents[i]);
      for (j = strlen(pers.idents[i]); j <= 21; j++)
	putc(' ', f);

      fputs(adate.idents[i], f);
      if (strlen(adate.idents[i]) < 19) {
	for (j = strlen(adate.idents[i]); j <= 18; j++)
	  putc(' ', f);
      }
      if (anint.int_[i] != 0)
	fprintf(f, " * ");
      else
	fprintf(f, "   ");

      if (privd)
	fputs(nam.idents[where_they_are.int_[i] - 1], f);
      putc('\n', f);

    }
  }
  mprintf("Done.\n");
  fclose(f);
}


void system_view()
{
  int used, free, total;

  putchar('\n');
  getindex(I_BLOCK);
  freeindex();
  used = indx.inuse;
  total = indx.top;
  free = total - used;

  mprintf("               used   free   total\n");
  mprintf("Block file   %5d  %5d   %5d\n", used, free, total);

  getindex(I_LINE);
  freeindex();
  used = indx.inuse;
  total = indx.top;
  free = total - used;
  mprintf("Line file    %5d  %5d   %5d\n", used, free, total);

  getindex(I_ROOM);
  freeindex();
  used = indx.inuse;
  total = indx.top;
  free = total - used;
  mprintf("Room file    %5d  %5d   %5d\n", used, free, total);

  getindex(I_OBJECT);
  freeindex();
  used = indx.inuse;
  total = indx.top;
  free = total - used;
  mprintf("Object file  %5d  %5d   %5d\n", used, free, total);

  getindex(I_INT);
  freeindex();
  used = indx.inuse;
  total = indx.top;
  free = total - used;
  mprintf("Integer file %5d  %5d   %5d\n\n", used, free, total);

}


/* remove a user from the log records (does not handle ownership) */

void kill_user(s)
char *s;
{
  int n;

  if (*s == '\0') {
    mprintf("No user specified\n");
    return;
  }
  if (!lookup_user(&n, s)) {
    mprintf("No such userid found in log information.\n");
    return;
  }
  getindex(I_ASLEEP);
  freeindex();
  if (indx.free[n-1]) {
    delete_log(&n);
    mprintf("Player deleted.\n");
  } else
    mprintf("That person is playing now.\n");
}


/* disown everything a player owns */

void disown_user(s)
char *s;
{
  int n, i;
  string tmp, theuser;

  if (*s == '\0') {
    mprintf("No user specified.\n");
    return;
  }
  if (debug)
    mprintf("calling lookup_user with %s\n", s);
  if (!lookup_user(&n, s))
    mprintf("User not in log info, attempting to disown anyway.\n");

  strcpy(theuser, user.idents[n-1]);

  /* first disown all their rooms */

  getown();
  freeown();
  for (i = 1; i <= maxroom; i++) {
    if (!strcmp(own.idents[i-1], theuser)) {
      getown();
      strcpy(own.idents[i-1], "*");
      putown();

      getroom(i);
      strcpy(tmp, here.nicename);
      strcpy(here.owner, "*");
      putroom();

      mprintf("Disowned room %s\n", tmp);
    }
  }
  putchar('\n');

  getobjown();
  freeobjown();
  getobjnam();
  freeobjnam();
  for (i = 0; i < maxroom; i++) {
    if (!strcmp(objown.idents[i], theuser)) {
      getobjown();
      strcpy(objown.idents[i], "*");
      putobjown();

      strcpy(tmp, objnam.idents[i]);
      mprintf("Disowned object %s\n", tmp);
    }
  }
}


void move_asleep()
{
  string pname, rname;   /* player & room names */
  int newroom, n;   /* room number & player slot number */

  grab_line("Player name? ", pname, true);
  grab_line("Room name?   ", rname, true);
  if (!lookup_user(&n, pname)) {
    mprintf("User not found.\n");
    return;
  }
  if (!lookup_room(&newroom, rname)) {
    mprintf("No such room found.\n");
    return;
  }
  getindex(I_ASLEEP);
  freeindex();
  if (!indx.free[n-1]) {
    mprintf("That player is not asleep.\n");
    return;
  }
  getint(N_LOCATION);
  anint.int_[n-1] = newroom;
  putint();
  mprintf("Player moved.\n");
}


void system_help()
{
  mprintf("\nB\tAdd description blocks\n");
  mprintf("D\tDisown <user>\n");
  mprintf("E\tExit (same as quit)\n");
  mprintf("I\tAdd Integer records\n");
  mprintf("K\tKill <user>\n");
  mprintf("L\tAdd one liner records\n");
  mprintf("M\tMove a player who is asleep (not playing now)\n");
  mprintf("O\tAdd object records\n");
  mprintf("P\tWrite a distribution list of players\n");
  mprintf("Q\tQuit (same as exit)\n");
  mprintf("R\tAdd rooms\n");
  mprintf("V\tView current sizes/usage\n");
  mprintf("?\tThis list\n\n");
}


/* *************** FIX_STUFF ******************** */

void fix_stuff()
{
}


void do_system(s_)
char *s_;
{
  string s, prompt;
  boolean done = false;
  char cmd;
  int n;
  string p, STR1;
  char STR2[256];

  strcpy(s, s_);
  if (!privd) {
    mprintf("Only the Monster Manger may enter system maintenance mode.\n");
    return;
  }
  log_action(c_system, 0);
  strcpy(prompt, "System> ");
  do {
    do {
      grab_line(prompt, s, true);
      strcpy(s, slead(STR1, s));
    } while (*s == '\0');
    strcpy(s, lowcase(STR1, s));
    cmd = s[0];

    n = 0;
    *p = '\0';
    if (strlen(s) > 1) {
      sprintf(STR2, "%.*s", (int)(strlen(s) - 1), s + 1);
      slead(p, STR2);
      n = number(p);
    }
    if (debug)
      mprintf("p = %s\n", p);

    switch (cmd) {

    case 'h':
    case '?':
      system_help();
      break;

    case '1':
      fix_stuff();
      break;

    /*remove a user*/
    case 'k':
      kill_user(p);
      break;

    /*disown*/
    case 'd':
      disown_user(p);
      break;

    /*dist list of players*/
    case 'p':
      dist_list();
      break;

    /*move where user will wakeup*/
    case 'm':
      move_asleep();
      break;

    /*add rooms*/
    case 'r':
      if (n > 0)
	addrooms(n);
      else
	mprintf("To add rooms, say R <# to add>\n");
      break;

    /*add ints*/
    case 'i':
      if (n > 0)
	addints(n);
      else
	mprintf("To add integers, say I <# to add>\n");
      break;

    /*add description blocks*/
    case 'b':
      if (n > 0)
	addblocks(n);
      else
	mprintf("To add description blocks, say B <# to add>\n");
      break;

    /*add objects*/
    case 'o':
      if (n > 0)
	addobjects(n);
      else
	mprintf("To add object records, say O <# to add>\n");
      break;

    /*add one-liners*/
    case 'l':
      if (n > 0)
	addlines(n);
      else
	mprintf("To add one liner records, say L <# to add>\n");
      break;

    /*view current stats*/
    case 'v':
      system_view();
      break;

    /*quit*/
    case 'q':
    case 'e':
      done = true;
      break;

    default:
      mprintf("-- bad command, type ? for a list.\n");
      break;
    }
  } while (!done);
  log_event(myslot, E_SYSDONE, 0, 0, "", 0);
}


void do_version(s)
char *s;
{
  mprintf("Monster, a multiplayer adventure game where the players create the world\n");
  mprintf("and make the rules.\n\n");
  mprintf("Written by Rich Skrenta at Northwestern University, 1988.\n");
}


void rebuild_system()
{
  int i, j;

  mprintf("Creating index file 1-6\n");
  for (i = 1; i <= 7; i++) {
    lseek(indexfile, (i - 1L) * sizeof(indexrec), 0);
    bzero(&indexfile_hat, sizeof(indexfile_hat));
    for (j = 0; j < maxindex; j++)
      indexfile_hat.free[j] = true;
    indexfile_hat.indexnum = i;
    indexfile_hat.top = 0;   /* none of each to start */
    indexfile_hat.inuse = 0;
    write(indexfile, &indexfile_hat, sizeof(indexfile_hat));
  }
  /* 1 is blocklist
     2 is linelist
     3 is roomlist
     4 is playeralloc
     5 is player awake (playing game)
     6 are objects
     7 is intfile */



  mprintf("Initializing roomfile with 10 rooms\n");
  addrooms(10);

  mprintf("Initializing block file with 10 description blocks\n");
  addblocks(10);

  mprintf("Initializing line file with 10 lines\n");
  addlines(10);

  mprintf("Initializing object file with 10 objects\n");
  addobjects(10);


  mprintf("Initializing namfile 1-8\n");
  for (j = 1; j <= 8; j++) {
    lseek(namfile, (j - 1L) * sizeof(namrec), 0);
    bzero(&namfile_hat, sizeof(namfile_hat));
    namfile_hat.validate = j;
    namfile_hat.loctop = 0;
    for (i = 0; i < maxroom; i++)
      *namfile_hat.idents[i] = '\0';
    write(namfile, &namfile_hat, sizeof(namfile_hat));
  }

  mprintf("Initializing eventfile\n");
  for (i = 1; i <= numevnts + 1; i++) {
    lseek(eventfile, (i - 1L) * sizeof(eventrec), 0);
    bzero(&eventfile_hat, sizeof(eventfile_hat));
    eventfile_hat.validat = i;
    eventfile_hat.point = 1;
    write(eventfile, &eventfile_hat, sizeof(eventfile_hat));
  }

  mprintf("Initializing intfile\n");
  for (i = 1; i <= 6; i++) {
    lseek(intfile, (i - 1L) * sizeof(intrec), 0);
    bzero(&intfile_hat, sizeof(intfile_hat));
    intfile_hat.intnum = i;
    write(intfile, &intfile_hat, sizeof(intfile_hat));
  }

  getindex(I_INT);
  for (i = 0; i <= 5; i++)
    indx.free[i] = false;
  indx.top = 6;
  indx.inuse = 6;
  putindex();

  /* Player log records should have all their slots initially,
     they don't have to be allocated because they use namrec
     and intfile for their storage; they don't have their own
     file to allocate
   */
  getindex(I_PLAYER);
  indx.top = maxplayers;
  putindex();
  getindex(I_ASLEEP);
  indx.top = maxplayers;
  putindex();

  mprintf("Creating the Great Hall\n");
  createroom("Great Hall");
  getroom(1);
  *here.owner = '\0';
  putroom();
  getown();
  *own.idents[0] = '\0';
  putown();

  mprintf("Creating the Void\n");
  createroom("Void");   /* loc 2 */
  mprintf("Creating the Pit of Fire\n");
  createroom("Pit of Fire");   /* loc 3 */
  /* note that these are NOT public locations */


  mprintf("Use the SYSTEM command to view and add capacity to the database\n\n");
}


void tests() {
	string s;

	mprintf("Running tests...\n");

	mprintf("date: ");
	sysdate(s);
	mprintf("%s\n", s);

	mprintf("time: ");
	systime(s);
	mprintf("%s\n", s);
}


void special(s_)
char *s_;
{
  string s, STR1;
  char STR2[256];
  char *TEMP;

  strcpy(s, s_);
  if (!strcmp(s, "test")) {
	tests();
	exit(0);
  }
  if (!strcmp(s, "rebuild") && privd) {
    if (!REBUILD_OK) {
      mprintf("REBUILD is disabled; you must recompile.\n");
      return;
    }
    mprintf("Do you really want to destroy the entire universe?\n");
    fgets(s, 81, stdin);
    TEMP = strchr(s, '\n');
    if (TEMP != NULL)
      *TEMP = 0;
    if (*s != '\0') {
      sprintf(STR2, "%.1s", lowcase(STR1, s));
      if (!strcmp(STR2, "y"))
	rebuild_system();
    }
    return;
  }
  if (!strcmp(s, "version"))
    mprintf("Monster, written by Rich Skrenta at Northwestern University, 1988.\n");
  /* Don't take this out please... */
  else if (!strcmp(s, "quit"))
    done = true;
}


/* put an object in this location
   if returns false, there were no more free object slots here:
   in other words, the room is too cluttered, and cannot hold any
   more objects
*/
boolean place_obj(n, silent)
int n;
boolean silent;
{
  boolean Result;
  boolean found = false;
  int i = 1;
  string STR1;

  if (here.objdrop == 0)
    getroom(0);
  else
    getroom(here.objdrop);
  while (i <= maxobjs && !found) {
    if (here.objs[i-1] == 0)
      found = true;
    else
      i++;
  }
  Result = found;
  if (!found) {
    freeroom();
    return Result;
  }
  here.objs[i-1] = n;
  here.objhide[i-1] = 0;
  putroom();

  gethere(0);


  /* if it bounced somewhere else then tell them */

  if (here.objdrop != 0 && here.objdest != 0)
    log_event(0, E_BOUNCEDIN, here.objdest, n, "", here.objdrop);


  if (silent)
    return Result;
  if (here.objdesc != 0)
    print_subs(here.objdesc, obj_part(STR1, n, true));
  else
    mprintf("Dropped.\n");
  return Result;
}


/* remove an object from this room */
boolean take_obj(objnum, slot)
int objnum, slot;
{
  boolean Result;

  getroom(0);
  if (here.objs[slot-1] == objnum) {
    here.objs[slot-1] = 0;
    here.objhide[slot-1] = 0;
    Result = true;
  } else
    Result = false;
  putroom();
  return Result;
}


boolean can_hold()
{
  if (find_numhold(0) < maxhold)
    return true;
  else
    return false;
}


boolean can_drop()
{
  if (find_numobjs() < maxobjs)
    return true;
  else
    return false;
}


int find_hold(objnum, slot)
int objnum, slot;
{
  int Result = 0, i = 1;

  if (slot == 0)
    slot = myslot;
  while (i <= maxhold) {
    if (here.people[slot-1].holding[i-1] == objnum)
      Result = i;
    i++;
  }
  return Result;
}



/* put object number n into the player's inventory; returns false if
   he's holding too many things to carry another */

boolean hold_obj(n)
int n;
{
  boolean Result;
  boolean found = false;
  int i = 1;

  getroom(0);
  while (i <= maxhold && !found) {
    if (here.people[myslot-1].holding[i-1] == 0)
      found = true;
    else
      i++;
  }
  Result = found;
  if (!found) {
    freeroom();
    return Result;
  }
  here.people[myslot-1].holding[i-1] = n;
  putroom();

  getobj(n);
  freeobj();
  hold_kind[i-1] = obj.kind;
  return Result;
}



/* remove an object (hold) from the player record, given the slot that
   the object is being held in */

void drop_obj(slot, pslot)
int slot, pslot;
{
  if (pslot == 0)
    pslot = myslot;
  getroom(0);
  here.people[pslot-1].holding[slot-1] = 0;
  putroom();

  hold_kind[slot-1] = 0;
}



/* maybe drop something I'm holding if I'm hit */

void maybe_drop()
{
  int i, objnum;
  string s;

  i = rnd100() % maxhold + 1;
/* p2c: mon.pas, line 5480:
 * Note: Using % for possibly-negative arguments [317] */
  objnum = here.people[myslot-1].holding[i-1];

  if (objnum == 0 || mywield == objnum || mywear == objnum)
    return;
  /* drop something */

  drop_obj(i, 0);
  if (!place_obj(objnum, true)) {
    mprintf("?error in maybe_drop; unsuccessful place_obj; notify Monster Manager\n");

    return;
  }
  getobjnam();
  freeobjnam();
  mprintf("The %s has slipped out of your hands.\n", objnam.idents[objnum-1]);


  strcpy(s, objnam.idents[objnum-1]);
  log_event(myslot, E_SLIPPED, 0, 0, s, 0);
}



/* return TRUE if the player is allowed to program the object n
   if checkpub is true then obj_owner will return true if the object in
   question is public */

boolean obj_owner(n, checkpub)
int n;
boolean checkpub;
{
  getobjown();
  freeobjown();
  if (!strcmp(objown.idents[n-1], userid) || privd)
    return true;
  else if (*objown.idents[n-1] == '\0' && checkpub)
    return true;
  else
    return false;
}


void do_duplicate(s)
char *s;
{
  int objnum;
  char STR1[50];

  if (*s == '\0') {
    mprintf("To duplicate an object, type DUPLICATE <object name>.\n");
    return;
  }
  if (!is_owner(location, true)) {
    mprintf("You may only create objects when you are in one of your own rooms.\n");
    return;
  }
  /* only let them make things if they're on their home turf */
  if (!lookup_obj(&objnum, s)) {
    mprintf("There is no object by that name.\n");
    return;
  }
  if (!obj_owner(objnum, true)) {
    mprintf("Power to create that object belongs to someone else.\n");
    return;
  }
  if (!place_obj(objnum, true)) {
    mprintf("There isn't enough room here to make that.\n");
    return;
  }
  /* put the new object here */
  /* keep track of how many there */
  getobj(objnum);
  /* are in existence */
  obj.numexist++;
  putobj();

  sprintf(STR1, "%s has created an object here.", myname);
  log_event(myslot, E_MADEOBJ, 0, 0, STR1, 0);
  mprintf("Object created.\n");
}


/* make an object */
void do_makeobj(s)
char *s;
{
  int objnum;
  char STR1[50];

  gethere(0);
  if (!checkhide())
    return;
  if (!is_owner(location, true)) {
    mprintf("You may only create objects when you are in one of your own rooms.\n");
    return;
  }
  if (*s == '\0') {
    mprintf("To create an object, type MAKE <object name>.\n");
    return;
  }
  if (strlen(s) > shortlen) {
    mprintf("Please limit your object names to %ld characters.\n",
	   (long)shortlen);
    return;
  }
  if (exact_obj(&objnum, s)) {  /* object already exits */
    mprintf(
      "That object already exits.  If you would like to make another copy of it,\n");
    mprintf("use the DUPLICATE command.\n");
    return;
  }
  if (debug)
    mprintf("?beggining to create object\n");
  if (find_numobjs() >= maxobjs) {

    mprintf(
      "This place is too crowded to create any more objects.  Try somewhere else.\n");
    return;
  }
  if (alloc_obj(&objnum)) {
    if (debug)
      mprintf("?alloc_obj successful\n");
    getobjnam();
    lowcase(objnam.idents[objnum-1], s);
    putobjnam();
    if (debug)
      mprintf("?getobjnam completed\n");
    getobjown();
    strcpy(objown.idents[objnum-1], userid);
    putobjown();
    if (debug)
      mprintf("?getobjown completed\n");

    getobj(objnum);
    obj.onum = objnum;
    strcpy(obj.oname, s);   /* name of object */
    obj.kind = 0;   /* bland object */
    obj.linedesc = DEFAULT_LINE;
    obj.actindx = 0;
    obj.examine = 0;
    obj.numexist = 1;
    obj.home = 0;
    obj.homedesc = 0;

    obj.sticky = false;
    obj.getobjreq = 0;
    obj.getfail = 0;
    obj.getsuccess = DEFAULT_LINE;

    obj.useobjreq = 0;
    obj.uselocreq = 0;
    obj.usefail = DEFAULT_LINE;
    obj.usesuccess = DEFAULT_LINE;

    *obj.usealias = '\0';
    obj.reqalias = false;
    obj.reqverb = false;

    if (s[0] == 'U' || s[0] == 'u' || s[0] == 'O' || s[0] == 'o' ||
	s[0] == 'I' || s[0] == 'i' || s[0] == 'E' || s[0] == 'e' ||
	s[0] == 'A' || s[0] == 'a')
      obj.particle = 2;   /* an */
    else
      obj.particle = 1;
    /* a */

    obj.d1 = 0;
    obj.d2 = 0;
    obj.exp3 = 0;
    obj.exp4 = 0;
    obj.exp5 = DEFAULT_LINE;
    obj.exp6 = DEFAULT_LINE;
    putobj();


    if (debug)
      mprintf("putobj completed\n");
  }
  /* else: alloc_obj prints errors by itself */
  if (!place_obj(objnum, true))
    mprintf(
      "?error in makeobj - could not place object; notify the Monster Manager.\n");
  /* put the new object here */
  else {
    sprintf(STR1, "%s has created an object here.", myname);
    log_event(myslot, E_MADEOBJ, 0, 0, STR1, 0);
    mprintf("Object created.\n");
  }
}


/* remove the type block for an object; all instances of the object must
   be destroyed first */

void do_unmake(s)
char *s;
{
  int n;
  string tmp;

  if (!is_owner(location, true)) {
    mprintf("You must be in one of your own rooms to UNMAKE an object.\n");
    return;
  }
  if (!lookup_obj(&n, s)) {
    mprintf("There is no object here by that name.\n");
    return;
  }
  obj_part(tmp, n, true);
  /* this will do a getobj(n) for us */

  if (obj.numexist != 0) {
    mprintf("You must DESTROY all instances of the object first.\n");
    return;
  }
  delete_obj(&n);

  log_event(myslot, E_UNMAKE, 0, 0, tmp, 0);
  mprintf("Object removed.\n");
}


/* destroy a copy of an object */

void do_destroy(s)
char *s;
{
  int slot, n;
  string STR1;
  char STR2[118];

  if (*s == '\0') {
    mprintf("To destroy an object you own, type DESTROY <object>.\n");
    return;
  }
  if (!is_owner(location, true)) {
    mprintf("You must be in one of your own rooms to destroy an object.\n");
    return;
  }
  if (!parse_obj(&n, s, false)) {
    mprintf("No such thing can be seen here.\n");
    return;
  }
  getobjown();
  freeobjown();
  if (strcmp(objown.idents[n-1], userid) && *objown.idents[n-1] != '\0' &&
      !privd) {
    mprintf("You must be the owner of an object to destroy it.\n");
    return;
  }
  if (obj_hold(n)) {
    slot = find_hold(n, 0);
    drop_obj(slot, 0);

    sprintf(STR2, "%s has destroyed %s.", myname, obj_part(STR1, n, true));
    log_event(myslot, E_DESTROY, 0, 0, STR2, 0);
    mprintf("Object destroyed.\n");

    getobj(n);
    obj.numexist--;
    putobj();
    return;
  }
  if (!obj_here(n)) {
    mprintf("Such a thing is not here.\n");
    return;
  }
  slot = find_obj(n);
  if (!take_obj(n, slot)) {
    mprintf("Someone picked it up before you could destroy it.\n");
    return;
  }
  sprintf(STR2, "%s has destroyed %s.", myname, obj_part(STR1, n, false));
  log_event(myslot, E_DESTROY, 0, 0, STR2, 0);
  mprintf("Object destroyed.\n");

  getobj(n);
  obj.numexist--;
  putobj();
}


boolean links_possible()
{
  boolean Result = false;
  int i;

  gethere(0);
  if (is_owner(location, true))
    return true;
  for (i = 0; i < maxexit; i++) {
    if (here.exits[i].toloc == 0 && here.exits[i].kind == 5)
      Result = true;
  }
  return Result;
}



/* make a room */
void do_form(s_)
char *s_;
{
  string s, STR1;

  strcpy(s, s_);
  gethere(0);
  if (!checkhide())
    return;
  if (!links_possible()) {
    mprintf(
      "You may not create any new exits here.  Go to a place where you can create\n");
    mprintf("an exit before FORMing a new room.\n");
    return;
  }
  if (*s == '\0')
    grab_line("Room name: ", s, true);
  strcpy(s, slead(STR1, s));

  createroom(s);
}


void xpoof(loc)
int loc;
{
  /* loc: integer; forward */
  int targslot;

  if (!put_token(loc, &targslot, here.people[myslot-1].hiding)) {
    mprintf("There is a crackle of electricity, but the poof fails.\n");
    return;
  }
  if (hiding) {
    log_event(myslot, E_HPOOFOUT, 0, 0, myname, location);
    log_event(myslot, E_HPOOFIN, 0, 0, myname, loc);
  } else {
    log_event(myslot, E_POOFOUT, 0, 0, myname, location);
    log_event(targslot, E_POOFIN, 0, 0, myname, loc);
  }

  take_token(myslot, location);
  myslot = targslot;
  location = loc;
  setevent();
  do_look("");
}


void do_poof(s_)
char *s_;
{
  string s;
  int n, loc;

  strcpy(s, s_);
  if (!privd) {
    mprintf("Only the Monster Manager may poof.\n");
    return;
  }
  gethere(0);
  if (lookup_room(&loc, s)) {
    xpoof(loc);
    return;
  }
  if (!parse_pers(&n, s)) {
    mprintf("There is no room named %s.\n", s);
    return;
  }
  grab_line("What room? ", s, true);
  if (!lookup_room(&loc, s)) {
    mprintf("There is no room named %s.\n", s);
    return;
  }
  log_event(myslot, E_POOFYOU, n, loc, "", 0);
  mprintf("\nYou extend your arms, muster some energy, and %s is\n",
	 here.people[n-1].name);
  mprintf("engulfed in a cloud of orange smoke.\n\n");
}


void link_room(origdir, targdir, targroom)
int origdir, targdir, targroom;
{
  exit_ *WITH, *WITH1;

  /* since exit creation involves the writing of two records,
     perhaps there should be a global lock around this code,
     such as a get to some obscure index field or something.
     I haven't put this in because I don't believe that if this
     routine fails it will seriously damage the database.

     Actually, the lock should be on the test (do_link) but that
     would be hard*/

  getroom(0);
  WITH = &here.exits[origdir-1];
  WITH->toloc = targroom;
  WITH->kind = 1;   /* type of exit, they can customize later */
  WITH->slot = targdir;   /* exit it comes out in in target room */

  init_exit(origdir);
  putroom();

  log_event(myslot, E_NEWEXIT, 0, 0, myname, location);
  if (location != targroom)
    log_event(0, E_NEWEXIT, 0, 0, myname, targroom);

  getroom(targroom);
  WITH1 = &here.exits[targdir-1];
  WITH1->toloc = location;
  WITH1->kind = 1;
  WITH1->slot = origdir;

  init_exit(targdir);
  putroom();
  mprintf("Exit created.  Use CUSTOM %s to customize your exit.\n",
	 direct[origdir-1]);
}


/*
User procedure to link a room
*/
void do_link(s_)
char *s_;
{
  string s;
  boolean ok;
  string orgexitnam, targnam, trgexitnam;
  int targroom;   /* number of target room */
  int targdir;   /* number of target exit direction */
  int origdir;   /* number of exit direction here */
  boolean firsttime;


  strcpy(s, s_);
  /*gethere;! done in links_possible */

  if (!links_possible()) {
    mprintf("No links are possible here.\n");
    return;
  }
  log_action(link, 0);
  if (!checkhide())
    return;
  mprintf("Hit return alone at any prompt to terminate exit creation.\n\n");

  if (*s == '\0')
    firsttime = false;
  else {
    bite(orgexitnam, s);
    firsttime = true;
  }

  do {
    if (!firsttime)
      grab_line("Direction of exit? ", orgexitnam, true);
    else
      firsttime = false;

    ok = lookup_dir(&origdir, orgexitnam);
    if (ok)
      ok = can_make(origdir, 0);
  } while (!(*orgexitnam == '\0' || ok));

  if (ok) {
    if (*s == '\0')
      firsttime = false;
    else {
      strcpy(targnam, s);
      firsttime = true;
    }

    do {
      if (!firsttime)
	grab_line("Room to link to? ", targnam, true);
      else
	firsttime = false;

      ok = lookup_room(&targroom, targnam);
    } while (!(*targnam == '\0' || ok));
  }

  if (ok) {
    do {
      mprintf("Exit comes out in target room\n");
      grab_line("from what direction? ", trgexitnam, true);
      ok = lookup_dir(&targdir, trgexitnam);
      if (ok)
	ok = can_make(targdir, targroom);
    } while (!(*trgexitnam == '\0' || ok));
  }

  if (ok)  /* actually create the exit */
    link_room(origdir, targdir, targroom);
}


void relink_room(origdir, targdir, targroom)
int origdir, targdir, targroom;
{
  exit_ tmp;
  int copyslot, copyloc;

  gethere(0);
  memcpy(&tmp, &here.exits[origdir-1], sizeof(exit_));
  copyloc = tmp.toloc;
  copyslot = tmp.slot;

  getroom(targroom);
  memcpy(&here.exits[targdir-1], &tmp, sizeof(exit_));
  putroom();

  getroom(copyloc);
  here.exits[copyslot-1].toloc = targroom;
  here.exits[copyslot-1].slot = targdir;
  putroom();

  getroom(0);
  here.exits[origdir-1].toloc = 0;
  init_exit(origdir);
  putroom();
}


void do_relink(s_)
char *s_;
{
  string s;
  boolean ok;
  string orgexitnam, targnam, trgexitnam;
  int targroom;   /* number of target room */
  int targdir;   /* number of target exit direction */
  int origdir;   /* number of exit direction here */
  boolean firsttime;

  strcpy(s, s_);
  log_action(c_relink, 0);
  gethere(0);
  if (!checkhide())
    return;
  mprintf("Hit return alone at any prompt to terminate exit relinking.\n\n");

  if (*s == '\0')
    firsttime = false;
  else {
    bite(orgexitnam, s);
    firsttime = true;
  }

  do {
    if (!firsttime)
      grab_line("Direction of exit to relink? ", orgexitnam, true);
    else
      firsttime = false;

    ok = lookup_dir(&origdir, orgexitnam);
    if (ok)
      ok = can_alter(origdir, 0);
  } while (!(*orgexitnam == '\0' || ok));

  if (ok) {
    if (*s == '\0')
      firsttime = false;
    else {
      strcpy(targnam, s);
      firsttime = true;
    }

    do {
      if (!firsttime)
	grab_line("Room to relink exit into? ", targnam, true);
      else
	firsttime = false;

      ok = lookup_room(&targroom, targnam);
    } while (!(*targnam == '\0' || ok));
  }

  if (ok) {
    do {
      mprintf("New exit comes out in target room\n");
      grab_line("from what direction? ", trgexitnam, true);
      ok = lookup_dir(&targdir, trgexitnam);
      if (ok)
	ok = can_make(targdir, targroom);
    } while (!(*trgexitnam == '\0' || ok));
  }

  if (ok)  /* actually create the exit */
    relink_room(origdir, targdir, targroom);
}


/* print the room default no-go message if there is one;
   otherwise supply the generic "you can't go that way" */

void default_fail()
{
  if (here.exitfail != 0 && here.exitfail != DEFAULT_LINE)
    print_desc(here.exitfail, "<no default supplied>");
  else
    mprintf("You can't go that way.\n");
}


void exit_fail(dir)
int dir;
{
  if (dir < 1 || dir > maxexit)
    default_fail();
  else if (here.exits[dir-1].fail == DEFAULT_LINE) {
    switch (here.exits[dir-1].kind) {

    case 5:
      mprintf("There isn't an exit there yet.\n");
      break;

    case 6:
      mprintf("You don't have the power to go there.\n");
      break;

    default:
      default_fail();
      break;
    }
  } else if (here.exits[dir-1].fail != 0)
    block_subs(here.exits[dir-1].fail, myname);


  /* now print the exit failure message for everyone else in the room:
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
  */

  if (dir != 0)
    log_event(myslot, E_FAILGO, dir, 0, "", 0);
}



void do_exit(exit_slot)
int exit_slot;
{
  /* (exit_slot: integer)-- declared forward */
  int orig_slot, targ_slot, orig_room, enter_slot, targ_room;
  boolean doalook;

  if (exit_slot < 1 || exit_slot > 6) {
    exit_fail(exit_slot);
    return;
  }
  if (here.exits[exit_slot-1].toloc <= 0) {
    exit_fail(exit_slot);
    return;
  }
  block_subs(here.exits[exit_slot-1].success, myname);

  orig_slot = myslot;
  orig_room = location;
  targ_room = here.exits[exit_slot-1].toloc;
  enter_slot = here.exits[exit_slot-1].slot;
  doalook = here.exits[exit_slot-1].autolook;

  /* optimization for exit that goes nowhere;
     why go nowhere?  For special effects, we
     don't want it to take too much time,
     the logs are important because they force the
     exit descriptions, but actually moving the
     player is unnecessary */

  if (orig_room == targ_room) {
    log_exit(exit_slot, orig_room, orig_slot);
    log_entry(enter_slot, targ_room, orig_slot);
    /* orig_slot in log_entry 'cause we're not
       really going anwhere */
    if (doalook)
      do_look("");
    return;
  }
  take_token(orig_slot, orig_room);
  if (!put_token(targ_room, &targ_slot, 0)) {
    /* no room in room! */
    /* put them back! Quick! */
    if (!put_token(orig_room, &myslot, 0)) {
      mprintf("?Oh no!\n");
      exit(0);
    }
    return;
  }
  log_exit(exit_slot, orig_room, orig_slot);
  log_entry(enter_slot, targ_room, targ_slot);

  myslot = targ_slot;
  location = targ_room;
  setevent();

  if (doalook)
    do_look("");
}



boolean cycle_open()
{
  char ch;
  string s;

  systime(s);
  ch = s[4];
  if (ch == '9' || ch == '7' || ch == '5' || ch == '3' || ch == '1')
    return true;
  else
    return false;
}


boolean which_dir(dir, s_)
int *dir;
char *s_;
{
  boolean Result = true;
  string s;
  int aliasdir, exitdir;
  boolean aliasmatch, exitmatch, aliasexact, exitexact, exitreq;
  string STR1;
  char STR2[256];

  strcpy(s, s_);
  strcpy(s, lowcase(STR1, s));
  if (lookup_alias(&aliasdir, s))
    aliasmatch = true;
  else
    aliasmatch = false;
  if (lookup_dir(&exitdir, s))
    exitmatch = true;
  else
    exitmatch = false;
  if (aliasmatch) {
    if (!strcmp(s, here.exits[aliasdir-1].alias))
      aliasexact = true;
    else
      aliasexact = false;
  } else
    aliasexact = false;
  if (exitmatch) {
    if (!strcmp(s, direct[exitdir-1]) ||
	!strcmp(s, (sprintf(STR2, "%.1s", direct[exitdir-1]), STR2)))
      exitexact = true;
    else
      exitexact = false;
  } else
    exitexact = false;
  if (exitmatch)
    exitreq = here.exits[exitdir-1].reqalias;
  else
    exitreq = false;

  *dir = 0;
  if (aliasexact && exitexact) {
    *dir = aliasdir;
    return true;
  } else if (aliasexact) {
    *dir = aliasdir;
    return true;
  } else if (exitexact && !exitreq) {
    *dir = exitdir;
    return true;
  } else if (aliasmatch) {
    *dir = aliasdir;
    return true;
  } else if (exitmatch && !exitreq) {
    *dir = exitdir;
    return true;
  } else if (exitmatch && exitreq) {
    *dir = exitdir;
    return false;
  } else
    return false;
  return true;
}


void exit_case(dir)
int dir;
{
  switch (here.exits[dir-1].kind) {

  case 0:
    exit_fail(dir);
    break;

  case 1:   /* more checking goes here */
    do_exit(dir);
    break;

  case 3:
    if (obj_hold(here.exits[dir-1].objreq))
      exit_fail(dir);
    else
      do_exit(dir);
    break;

  case 4:
    if (rnd100() < 34)
      do_exit(dir);
    else
      exit_fail(dir);
    break;

  case 2:
    if (obj_hold(here.exits[dir-1].objreq))
      do_exit(dir);
    else
      exit_fail(dir);
    break;

  case 6:
    if (obj_hold(here.exits[dir-1].objreq))
      do_exit(dir);
    else
      exit_fail(dir);
    break;

  case 7:
    if (cycle_open())
      do_exit(dir);
    else
      exit_fail(dir);
    break;
  }
}


/*
Player wants to go to s
Handle everthing, this is the top level procedure

Check that he can go to s
Put him through the exit( in do_exit )
Do a look for him( in do_exit )
*/
void do_go(s, verb)
char *s;
boolean verb;
{
  int dir;

  gethere(0);
  if (!checkhide())
    return;
  if (*s == '\0') {
    mprintf("You must give the direction you wish to travel.\n");
    return;
  }
  if (!which_dir(&dir, s)) {
    exit_fail(dir);
    return;
  }
  if (dir < 1 || dir > maxexit) {
    exit_fail(dir);
    return;
  }
  if (here.exits[dir-1].toloc == 0)
    exit_fail(dir);
  else
    exit_case(dir);
}


void nice_say(s)
char *s;
{
  /* capitalize the first letter of their sentence */

  if (islower(s[0]))
    s[0] = _toupper(s[0]);

  /* put a period on the end of their sentence if
     they don't use any punctuation. */

  if (isalpha(s[strlen(s) - 1]))
    strcat(s, ".");
}


void do_say(s_)
char *s_;
{
  string s;

  strcpy(s, s_);
  if (*s == '\0') {
    mprintf("To talk to others in the room, type SAY <message>.\n");
    return;
  }

  /*if length(s) + length(myname) > 79 then begin
                          s := substr(s,1,75-length(myname));
                          writeln('Your message was truncated:');
                          writeln('-- ',s);
                  end;*/

  nice_say(s);
  if (hiding)
    log_event(myslot, E_HIDESAY, 0, 0, s, 0);
  else
    log_event(myslot, E_SAY, 0, 0, s, 0);
}


void do_setname(s)
char *s;
{
  string notice;
  boolean ok;
  int dummy;
  string sprime;

  gethere(0);
  if (*s == '\0') {
    mprintf("You are known to others as %s\n", myname);
    return;
  }
  if (strlen(s) > shortlen) {
    mprintf("Please limit your personal name to %ld characters.\n",
	   (long)shortlen);
    return;
  }
  lowcase(sprime, s);
  if (!strcmp(sprime, "monster manager") && strcmp(userid, MM_userid)) {
    mprintf("Only the Monster Manager can have that personal name.\n");
    ok = false;
  } else if (!strcmp(sprime, "vice manager") && strcmp(userid, MVM_userid)) {
    mprintf("Only the Vice Manager can have that name.\n");
    ok = false;
  } else if (!strcmp(sprime, "faust") && strcmp(userid, FAUST_userid)) {
    mprintf("You are not Faust!  You may not have that name.\n");
    ok = false;
  } else
    ok = true;

  if (ok) {
    if (exact_pers(&dummy, sprime)) {
      if (dummy != myslot) {
	mprintf("Someone already has that name.  Your personal name must be unique.\n");
	ok = false;
      }
    }
  }

  if (!ok)
    return;
  strcpy(myname, s);
  getroom(0);
  strcpy(notice, here.people[myslot-1].name);
  strcpy(here.people[myslot-1].name, s);
  putroom();
  sprintf(notice + strlen(notice), " is now known as %s", s);

  if (!hiding)
    log_event(0, E_SETNAM, 0, 0, notice, 0);
  /* slot 0 means notify this player also */

  getpers();   /* note the new personal name in the logfile */
  strcpy(pers.idents[mylog-1], s);   /* don't lowcase it */
  putpers();
}



/*
1234567890123456789012345678901234567890
example display for alignment:

       Monster Status
    19-MAR-1988 08:59pm

*/

void do_who()
{
  int i, j;
  boolean ok, metaok;
  veryshortstring roomown;
  string STR1, STR3;
  int FORLIM;

  log_event(myslot, E_WHO, 0, rnd100() & 3, "", 0);

  /* we need just about everything to print this list:
          player alloc index, userids, personal names,
          room names, room owners, and the log record*/

  getindex(I_ASLEEP);   /* Get index of people who are playing now */
  freeindex();
  getuser();
  freeuser();
  getpers();
  freepers();
  getnam();
  freenam();
  getown();
  freeown();
  getint(N_LOCATION);   /* get where they are */
  freeint();
  mprintf("                   Monster Status\n");
  mprintf("                %s %s\n\n", sysdate(STR1), systime(STR3));
  mprintf("Username        Game Name                 Where\n");

  if (privd)   /* or has_kind(O_ALLSEEING) */
    metaok = true;
  else
    metaok = false;
  FORLIM = indx.top;

  for (i = 0; i < FORLIM; i++) {
    if (!indx.free[i]) {
      fputs(user.idents[i], stdout);
      j = strlen(user.idents[i]);
      while (j < 16) {
	putchar(' ');
	j++;
      }

      fputs(pers.idents[i], stdout);
      j = strlen(pers.idents[i]);
      while (j <= 25) {
	putchar(' ');
	j++;
      }

      if (!metaok) {
	strcpy(roomown, own.idents[anint.int_[i] - 1]);

	/* if a person is in a public or disowned room, or
	   if they are in the domain of the WHOer, then the player should know
	   where they are  */

	if (*roomown == '\0' || !strcmp(roomown, "*") ||
	    !strcmp(roomown, userid))
	  ok = true;
	else
	  ok = false;


	/* the player obviously knows where he is */
	if (i + 1 == mylog)
	  ok = true;
      }


      if (ok || metaok)
	puts(nam.idents[anint.int_[i] - 1]);
      else
	mprintf("n/a\n");
    }
  }
}


char *own_trans(Result, s)
char *Result, *s;
{
  if (*s == '\0')
    return strcpy(Result, "<public>");
  else if (!strcmp(s, "*"))
    return strcpy(Result, "<disowned>");
  else
    return strcpy(Result, s);
}


void list_rooms(s)
char *s;
{
  boolean first = true;
  int i, j;
  int posit = 0;
  int FORLIM;
  string STR2;

  FORLIM = indx.top;
  for (i = 0; i < FORLIM; i++) {
    if (!indx.free[i] && !strcmp(own.idents[i], s)) {
      if (posit == 3) {
	posit = 1;
	putchar('\n');
      } else
	posit++;
      if (first) {
	first = false;
	mprintf("%s:\n", own_trans(STR2, s));
      }
      mprintf("    %s", nam.idents[i]);
      for (j = strlen(nam.idents[i]); j <= 21; j++)
	putchar(' ');
    }
  }
  if (posit != 3)
    putchar('\n');
  if (first)
    mprintf("No rooms owned by %s\n", own_trans(STR2, s));
  else
    putchar('\n');
}


static void list_all_rooms()
{
  int i, j;
  boolean tmp[maxroom];
  int FORLIM, FORLIM1;

  memset(tmp, 0, maxroom);
  list_rooms("");   /* public rooms first */
  list_rooms("*");   /* disowned rooms next */
  FORLIM = indx.top;
  for (i = 0; i < FORLIM; i++) {
    if (!indx.free[i] && !tmp[i] && *own.idents[i] != '\0' &&
	strcmp(own.idents[i], "*")) {
      list_rooms(own.idents[i]);   /* player rooms */
      FORLIM1 = indx.top;
      for (j = 0; j < FORLIM1; j++) {
	if (!strcmp(own.idents[j], own.idents[i]))
	  tmp[j] = true;
      }
    }
  }
}

void do_rooms(s_)
char *s_;
{
  string s, cmd;
  veryshortstring id;
  boolean listall = false;
  string STR1;

  strcpy(s, s_);
  getnam();
  freenam();
  getown();
  freeown();
  getindex(I_ROOM);
  freeindex();

  strcpy(s, lowcase(STR1, s));
  bite(cmd, s);
  if (*cmd == '\0')
    strcpy(id, userid);
  else if (!strcmp(cmd, "public"))
    *id = '\0';
  else if (!strcmp(cmd, "disowned"))
    strcpy(id, "*");
  else if (!strcmp(cmd, "<public>"))
    *id = '\0';
  else if (!strcmp(cmd, "<disowned>"))
    strcpy(id, "*");
  else if (!strcmp(cmd, "*"))
    listall = true;
  else if (strlen(cmd) > veryshortlen)
    sprintf(id, "%.*s", veryshortlen, cmd);
  else
    strcpy(id, cmd);

  if (listall) {
    if (privd)
      list_all_rooms();
    else
      mprintf("You may not obtain a list of all the rooms.\n");
    return;
  }
  if (privd || !strcmp(userid, id) || *id == '\0' || !strcmp(id, "*"))
    list_rooms(id);
  else
    mprintf("You may not list rooms that belong to another player.\n");
}



void do_objects()
{
  int i;
  int total = 0, public_ = 0, disowned = 0, private_ = 0;
  int FORLIM;

  getobjnam();
  freeobjnam();
  getobjown();
  freeobjown();
  getindex(I_OBJECT);
  freeindex();


  putchar('\n');
  FORLIM = indx.top;
  for (i = 1; i <= FORLIM; i++) {
    if (!indx.free[i-1]) {
      total++;
      if (*objown.idents[i-1] == '\0') {
	mprintf("%4d    %12s    %s\n", i, "<public>", objnam.idents[i-1]);
	public_++;
      } else if (!strcmp(objown.idents[i-1], "*")) {
	mprintf("%4d    %12s    %s\n", i, "<disowned>", objnam.idents[i-1]);
	disowned++;
      } else {
	private_++;

	if (!strcmp(objown.idents[i-1], userid) || privd)
	  mprintf("%4d    %12s    %s\n",
		 i, objown.idents[i-1], objnam.idents[i-1]);
	/* >>>>>> */
      }
    }
  }
  mprintf("\nPublic:      %4d\n", public_);
  mprintf("Disowned:    %4d\n", disowned);
  mprintf("Private:     %4d\n", private_);
  mprintf("             ----\n");
  mprintf("Total:       %4d\n", total);
}


void do_claim(s)
char *s;
{
  int n;
  boolean ok;
  string tmp;

  if (*s == '\0') {  /* claim this room */
    getroom(0);
    if (!strcmp(here.owner, "*") || privd) {
      strcpy(here.owner, userid);
      putroom();
      getown();
      strcpy(own.idents[location-1], userid);
      putown();
      log_event(myslot, E_CLAIM, 0, 0, "", 0);
      mprintf("You are now the owner of this room.\n");
      return;
    }
    freeroom();
    if (*here.owner == '\0')
      mprintf("This is a public room.  You may not claim it.\n");
    else
      mprintf("This room has an owner.\n");
    return;
  }
  if (!lookup_obj(&n, s)) {
    mprintf("There is nothing here by that name to claim.\n");
    return;
  }
  getobjown();
  freeobjown();
  if (*objown.idents[n-1] == '\0') {
    mprintf("That is a public object.  You may DUPLICATE it, but may not CLAIM it.\n");
    return;
  }
  if (strcmp(objown.idents[n-1], "*")) {
    mprintf("That object has an owner.\n");
    return;
  }
  getobj(n);
  freeobj();
  if (obj.numexist == 0)
    ok = true;
  else {
    if (obj_hold(n) || obj_here(n))
      ok = true;
    else
      ok = false;
  }

  if (!ok) {
    mprintf("You must have one to claim it.\n");
    return;
  }
  getobjown();
  strcpy(objown.idents[n-1], userid);
  putobjown();
  strcpy(tmp, obj.oname);
  log_event(myslot, E_OBJCLAIM, 0, 0, tmp, 0);
  mprintf("You are now the owner the %s.\n", tmp);
}


void do_disown(s)
char *s;
{
  int n;
  string tmp;

  if (*s == '\0') {  /* claim this room */
    getroom(0);
    if (strcmp(here.owner, userid) && !privd) {
      freeroom();
      mprintf("You are not the owner of this room.\n");
      /* disown an object */
      return;
    }
    getroom(0);
    strcpy(here.owner, "*");
    putroom();
    getown();
    strcpy(own.idents[location-1], "*");
    putown();
    log_event(myslot, E_DISOWN, 0, 0, "", 0);
    mprintf("You have disowned this room.\n");
    return;
  }
  if (!lookup_obj(&n, s)) {
    mprintf("You are not the owner of any such thing.\n");
    return;
  }
  getobj(n);
  freeobj();
  strcpy(tmp, obj.oname);

  getobjown();
  if (strcmp(objown.idents[n-1], userid)) {
    freeobjown();
    mprintf("You are not the owner of any such thing.\n");
    return;
  }
  strcpy(objown.idents[n-1], "*");
  putobjown();
  log_event(myslot, E_OBJDISOWN, 0, 0, tmp, 0);
  mprintf("You are no longer the owner of the %s.\n", tmp);
}


void do_public(s)
char *s;
{
  boolean ok;
  string tmp;
  int n;

  if (!privd) {
    mprintf("Only the Monster Manager may make things public.\n");
    return;
  }
  if (*s == '\0') {
    getroom(0);
    *here.owner = '\0';
    putroom();
    getown();
    *own.idents[location-1] = '\0';
    putown();
    return;
  }
  if (!lookup_obj(&n, s)) {
    mprintf("There is nothing here by that name to claim.\n");
    return;
  }
  getobjown();
  freeobjown();
  if (*objown.idents[n-1] == '\0') {
    mprintf("That is already public.\n");
    return;
  }
  getobj(n);
  freeobj();
  if (obj.numexist == 0)
    ok = true;
  else {
    if (obj_hold(n) || obj_here(n))
      ok = true;
    else
      ok = false;
  }

  if (!ok) {
    mprintf("You must have one to claim it.\n");
    return;
  }
  getobjown();
  *objown.idents[n-1] = '\0';
  putobjown();

  strcpy(tmp, obj.oname);
  log_event(myslot, E_OBJPUBLIC, 0, 0, tmp, 0);
  mprintf("The %s is now public.\n", tmp);
}



/* sum up the number of real exits in this room */

int find_numexits()
{
  int i;
  int sum = 0;

  for (i = 0; i < maxexit; i++) {
    if (here.exits[i].toloc != 0)
      sum++;
  }
  return sum;
}



/* clear all people who have played monster and quit in this location
   out of the room so that when they start up again they won't be here,
   because we are destroying this room */

void clear_people(loc)
int loc;
{
  int i;

  getint(N_LOCATION);
  for (i = 0; i < maxplayers; i++) {
    if (anint.int_[i] == loc)
      anint.int_[i] = 1;
  }
  putint();
}


void do_zap(s)
char *s;
{
  int loc;

  gethere(0);
  if (!checkhide())
    return;
  if (!lookup_room(&loc, s)) {
    mprintf("There is no room named %s.\n", s);
    return;
  }
  gethere(loc);
  if (strcmp(here.owner, userid) && !privd) {
    mprintf("You are not the owner of that room.\n");
    return;
  }
  clear_people(loc);
  if (find_numpeople() != 0) {
    mprintf("Sorry, you cannot destroy a room if people are still in it.\n");
    return;
  }
  if (find_numexits() != 0) {
    mprintf("You must delete all of the exits from that room first.\n");
    return;
  }
  if (find_numobjs() == 0) {
    del_room(loc);
    mprintf("Room deleted.\n");
  } else
    mprintf("You must remove all of the objects from that room first.\n");
}


boolean room_nameinuse(num, newname)
int num;
char *newname;
{
  int dummy;

  if (exact_obj(&dummy, newname)) {
    if (dummy == num)
      return false;
    else
      return true;
  } else
    return false;
}



void do_rename()
{
  string newname;

  gethere(0);
  mprintf("This room is named %s\n\n", here.nicename);
  grab_line("New name: ", newname, true);
  if (*newname == '\0' || !strcmp(newname, "**")) {
    mprintf("No changes.\n");
    return;
  }
  if (strlen(newname) > shortlen) {
    mprintf("Please limit your room name to %ld characters.\n", (long)shortlen);
    return;
  }
  if (room_nameinuse(location, newname)) {
    mprintf("%s is not a unique room name.\n", newname);
    return;
  }
  getroom(0);
  strcpy(here.nicename, newname);
  putroom();

  getnam();
  lowcase(nam.idents[location-1], newname);
  putnam();
  mprintf("Room name updated.\n");
}


boolean obj_nameinuse(objnum, newname)
int objnum;
char *newname;
{
  int dummy;

  if (exact_obj(&dummy, newname)) {
    if (dummy == objnum)
      return false;
    else
      return true;
  } else
    return false;
}


void do_objrename(objnum)
int objnum;
{
  string newname;

  getobj(objnum);
  freeobj();

  mprintf("This object is named %s\n\n", obj.oname);
  grab_line("New name: ", newname, true);
  if (*newname == '\0' || !strcmp(newname, "**")) {
    mprintf("No changes.\n");
    return;
  }
  if (strlen(newname) > shortlen) {
    mprintf("Please limit your object name to %ld characters.\n",
	   (long)shortlen);
    return;
  }
  if (obj_nameinuse(objnum, newname)) {
    mprintf("%s is not a unique object name.\n", newname);
    return;
  }
  getobj(objnum);
  strcpy(obj.oname, newname);
  putobj();

  getobjnam();
  lowcase(objnam.idents[objnum-1], newname);
  putobjnam();
  mprintf("Object name updated.\n");
}



void view_room()
{
  int i;

  putchar('\n');
  getnam();
  freenam();
  getobjnam();
  freeobjnam();

  mprintf("Room:        %s\n", here.nicename);
  switch (here.nameprint) {

  case 0:
    mprintf("Room name not printed\n");
    break;

  case 1:
    mprintf("\"You're in\" precedes room name\n");
    break;

  case 2:
    mprintf("\"You're at\" precedes room name\n");
    break;

  default:
    mprintf("Room name printing is damaged.\n");
    break;
  }

  mprintf("Room owner:    ");
  if (*here.owner == '\0')
    mprintf("<public>\n");
  else if (!strcmp(here.owner, "*"))
    mprintf("<disowned>\n");
  else
    puts(here.owner);

  if (here.primary == 0)
    mprintf("There is no primary description\n");
  else
    mprintf("There is a primary description\n");

  if (here.secondary == 0)
    mprintf("There is no secondary description\n");
  else
    mprintf("There is a secondary description\n");

  switch (here.which) {

  case 0:
    mprintf("Only the primary description will print\n");
    break;

  case 1:
    mprintf("Only the secondary description will print\n");
    break;

  case 2:
    mprintf("Both the primary and secondary descriptions will print\n");
    break;

  case 3:
    mprintf(
      "The primary description will print, followed by the seconary description\n");
    mprintf("if the player is holding the magic object\n");
    break;

  case 4:
    mprintf(
      "If the player is holding the magic object, the secondary description will print\n");
    mprintf("Otherwise, the primary description will print\n");
    break;

  default:
    mprintf("The way the room description prints is damaged\n");
    break;
  }

  putchar('\n');
  if (here.magicobj == 0)
    mprintf("There is no magic object for this room\n");
  else
    mprintf("The magic object for this room is the %s.\n",
	   objnam.idents[here.magicobj - 1]);

  if (here.objdrop == 0)
    mprintf("Dropped objects remain here\n");
  else {
    mprintf("Dropped objects go to %s.\n", nam.idents[here.objdrop - 1]);
    if (here.objdesc == 0)
      mprintf("Dropped.\n");
    else
      print_line(here.objdesc);
    if (here.objdest == 0)
      mprintf("Nothing is printed when object \"bounces in\" to target room\n");
    else
      print_line(here.objdest);
  }
  putchar('\n');
  if (here.trapto == 0)
    mprintf("There is no trapdoor set\n");
  else
    mprintf("The trapdoor sends players %s with a chance factor of %d%%\n",
	   direct[here.trapto - 1], here.trapchance);

  for (i = 0; i < maxdetail; i++) {
    if (*here.detail[i] != '\0') {
      mprintf("Detail \"%s\" ", here.detail[i]);
      if (here.detaildesc[i] > 0)
	mprintf("has a description\n");
      else
	mprintf("has no description\n");
    }
  }
  putchar('\n');
}


void room_help()
{
  mprintf("\nD\tAlter the way the room description prints\n");
  mprintf("N\tChange how the room Name prints\n");
  mprintf("P\tEdit the Primary room description [the default one] (same as desc)\n");
  mprintf("S\tEdit the Secondary room description\n");
  mprintf("X\tDefine a mystery message\n\n");
  mprintf("G\tSet the location that a dropped object really Goes to\n");
  mprintf("O\tEdit the object drop description (for drop effects)\n");
  mprintf("B\tEdit the target room (G) \"bounced in\" description\n\n");
  mprintf("T\tSet the direction that the Trapdoor goes to\n");
  mprintf("C\tSet the Chance of the trapdoor functioning\n\n");
  mprintf("M\tDefine the magic object for this room\n");
  mprintf("R\tRename the room\n\n");
  mprintf("V\tView settings on this room\n");
  mprintf("E\tExit (same as quit)\n");
  mprintf("Q\tQuit (same as exit)\n");
  mprintf("?\tThis list\n\n");
}



void custom_room()
{
  boolean done = false;
  string prompt;
  int n;
  string s;
  int newdsc;
  string STR2;

  log_action(e_custroom, 0);
  mprintf("\nCustomizing this room\n");
  mprintf(
    "If you would rather be customizing an exit, type CUSTOM <direction of exit>\n");
  mprintf(
    "If you would rather be customizing an object, type CUSTOM <object name>\n\n");
  strcpy(prompt, "Custom> ");

  do {
    do {
      grab_line(prompt, s, true);
      strcpy(s, slead(STR2, s));
    } while (*s == '\0');
    strcpy(s, lowcase(STR2, s));
    switch (s[0]) {

    case 'e':
    case 'q':
      done = true;
      break;

    case '?':
    case 'h':
      room_help();
      break;

    case 'r':
      do_rename();
      break;

    case 'v':
      view_room();
      break;

    /*dir trapdoor goes*/
    case 't':
      grab_line("What direction does the trapdoor exit through? ", s, true);
      if (*s != '\0') {
	if (lookup_dir(&n, s)) {
	  getroom(0);
	  here.trapto = n;
	  putroom();
	  mprintf("Room updated.\n");
	} else
	  mprintf("No such direction.\n");
      } else
	mprintf("No changes.\n");
      break;

    /*chance*/
    case 'c':
      mprintf(
	"Enter the chance that in any given minute the player will fall through\n");
      mprintf("the trapdoor (0-100) :\n\n");
      grab_line("? ", s, true);
      if (isnum(s)) {
	n = number(s);
	if ((unsigned)n <= 100) {
	  getroom(0);
	  here.trapchance = n;
	  putroom();
	} else
	  mprintf("Out of range.\n");
      } else
	mprintf("No changes.\n");
      break;

    case 's':
      newdsc = here.secondary;
      mprintf("[ Editing the secondary room description ]\n");
      if (edit_desc(&newdsc)) {
	getroom(0);
	here.secondary = newdsc;
	putroom();
      }
      break;

    case 'p':
      /* same as desc */
      newdsc = here.primary;
      mprintf("[ Editing the primary room description ]\n");
      if (edit_desc(&newdsc)) {
	getroom(0);
	here.primary = newdsc;
	putroom();
      }
      break;

    case 'o':
      mprintf(
	"Enter the line that will be printed when someone drops an object here:\n");
      mprintf(
	"If dropped objects do not stay here, you may use a # for the object name.\n");
      mprintf("Right now it says:\n");
      if (here.objdesc == 0)
	mprintf("Dropped. [default]\n");
      else
	print_line(here.objdesc);

      n = here.objdesc;
      make_line(&n, "", 79);
      getroom(0);
      here.objdesc = n;
      putroom();
      break;

    case 'x':
      mprintf("Enter a line that will be randomly shown.\n");
      mprintf("Right now it says:\n");
      if (here.objdesc == 0)
	mprintf("[none defined]\n");
      else
	print_line(here.rndmsg);

      n = here.rndmsg;
      make_line(&n, "", 79);
      getroom(0);
      here.rndmsg = n;
      putroom();
      break;

    /*bounced in desc*/
    case 'b':
      mprintf(
	"Enter the line that will be displayed in the room where an object really\n");
      mprintf("goes when an object dropped here \"bounces\" there:\n");
      mprintf("Place a # where the object name should go.\n\n");
      mprintf("Right now it says:\n");
      if (here.objdest == 0)
	mprintf("Something has bounced into the room.\n");
      else
	print_line(here.objdest);

      n = here.objdest;
      make_line(&n, "", 79);
      getroom(0);
      here.objdest = n;
      putroom();
      break;

    case 'm':
      getobjnam();
      freeobjnam();
      if (here.magicobj == 0)
	mprintf("There is currently no magic object for this room.\n");
      else
	mprintf("%s is currently the magic object for this room.\n",
	       objnam.idents[here.magicobj - 1]);
      putchar('\n');
      grab_line("New magic object? ", s, true);
      if (*s == '\0')
	mprintf("No changes.\n");
      else if (lookup_obj(&n, s)) {
	getroom(0);
	here.magicobj = n;
	putroom();
	mprintf("Room updated.\n");
      } else
	mprintf("No such object found.\n");
      break;

    case 'g':
      getnam();
      freenam();
      if (here.objdrop == 0)
	mprintf("Objects dropped fall here.\n");
      else
	mprintf("Objects dropped fall in %s.\n", nam.idents[here.objdrop - 1]);
      mprintf("\nEnter * for [this room]:\n");
      grab_line("Room dropped objects go to? ", s, true);
      if (*s == '\0')
	mprintf("No changes.\n");
      else if (!strcmp(s, "*")) {
	getroom(0);
	here.objdrop = 0;
	putroom();
	mprintf("Room updated.\n");
      } else if (lookup_room(&n, s)) {
	getroom(0);
	here.objdrop = n;
	putroom();
	mprintf("Room updated.\n");
      } else
	mprintf("No such room found.\n");
      break;

    case 'd':
      mprintf("Print room descriptions how?\n\n");
      mprintf("0)  Print primary (main) description only [default]\n");
      mprintf("1)  Print only secondary description.\n");
      mprintf("2)  Print both primary and secondary descriptions togther.\n");
      mprintf(
	"3)  Print primary description first; then print secondary description only if\n");
      mprintf("    the player is holding the magic object for this room.\n");
      mprintf(
	"4)  Print secondary if holding the magic obj; print primary otherwise\n\n");
      grab_line("? ", s, true);
      if (isnum(s)) {
	n = number(s);
	if ((unsigned)n < 32 && ((1L << n) & 0x1f) != 0) {
	  getroom(0);
	  here.which = n;
	  putroom();
	  mprintf("Room updated.\n");
	} else
	  mprintf("Out of range.\n");
      } else
	mprintf("No changes.\n");
      break;

    case 'n':
      mprintf("How would you like the room name to print?\n\n");
      mprintf("0) No room name is shown\n");
      mprintf("1) \"You're in ...\"\n");
      mprintf("2) \"You're at ...\"\n\n");
      grab_line("? ", s, true);
      if (isnum(s)) {
	n = number(s);
	if ((unsigned)n < 32 && ((1L << n) & 0x7) != 0) {
	  getroom(0);
	  here.nameprint = n;
	  putroom();
	} else
	  mprintf("Out of range.\n");
      } else
	mprintf("No changes.\n");
      break;

    default:
      mprintf("Bad command, type ? for a list\n");
      break;
    }
  } while (!done);
  log_event(myslot, E_ROOMDONE, 0, 0, "", 0);
}


void analyze_exit(dir)
int dir;
{
  string s;
  char STR1[90];
  exit_ *WITH;

  putchar('\n');
  getnam();
  freenam();
  getobjnam();
  freeobjnam();
  WITH = &here.exits[dir-1];
  strcpy(s, WITH->alias);
  if (*s == '\0')
    strcpy(s, "(no alias)");
  else
    sprintf(s, "(alias %s)", strcpy(STR1, s));
  if (here.exits[dir-1].reqalias)
    strcat(s, " (required)");
  else
    strcat(s, " (not required)");

  if (WITH->toloc != 0)
    mprintf("The %s exit %s goes to %s\n",
	   direct[dir-1], s, nam.idents[WITH->toloc - 1]);
  else
    mprintf("The %s exit goes nowhere.\n", direct[dir-1]);
  if (WITH->hidden != 0)
    mprintf("Concealed.\n");
  mprintf("Exit type: ");
  switch (WITH->kind) {

  case 0:
    mprintf("no exit.\n");
    break;

  case 1:
    mprintf("Open passage.\n");
    break;

  case 2:
    mprintf("Door, object required to pass.\n");
    break;

  case 3:
    mprintf("No passage if holding object.\n");
    break;

  case 4:
    mprintf("Randomly fails\n");
    break;

  case 5:
    mprintf("Potential exit.\n");
    break;

  case 6:
    mprintf("Only exists while holding the required object.\n");
    break;

  case 7:
    mprintf("Timed exit\n");
    break;
  }
  if (WITH->objreq == 0)
    mprintf("No required object.\n");
  else
    mprintf("Required object is: %s\n", objnam.idents[WITH->objreq - 1]);


  putchar('\n');
  if (WITH->exitdesc == DEFAULT_LINE)
    exit_default(dir, WITH->kind);
  else
    print_line(WITH->exitdesc);

  if (WITH->success == 0)
    mprintf("(no success message)\n");
  else
    print_desc(WITH->success, "<no default supplied>");

  if (WITH->fail == DEFAULT_LINE) {
    if (WITH->kind == 5)
      mprintf("There isn' an exit there yet.\n");
    else
      mprintf("You can't go that way.\n");
  } else
    print_desc(WITH->fail, "<no default supplied>");

  if (WITH->comeout == DEFAULT_LINE)
    mprintf("# has come into the room from: %s\n", direct[dir-1]);
  else
    print_desc(WITH->comeout, "<no default supplied>");
  if (WITH->goin == DEFAULT_LINE)
    mprintf("# has gone %s\n", direct[dir-1]);
  else
    print_desc(WITH->goin, "<no default supplied>");

  putchar('\n');
  if (WITH->autolook)
    mprintf("LOOK automatically done after exit used\n");
  else
    mprintf("LOOK supressed on exit use\n");
  if (WITH->reqverb)
    mprintf("The alias is required to be a verb for exit use\n");
  else
    mprintf("The exit can be used with GO or as a verb\n");
  putchar('\n');
}


void custom_help()
{
  mprintf("\nA\tSet an Alias for the exit\n");
  mprintf("C\tConceal an exit\n");
  mprintf("D\tEdit the exit's main Description\n");
  mprintf("E\tEXIT custom (saves changes)\n");
  mprintf("F\tEdit the exit's failure line\n");
  mprintf("I\tEdit the line that others see when a player goes Into an exit\n");
  mprintf("K\tSet the object that is the Key to this exit\n");
  mprintf("L\tAutomatically look [default] / don't look on exit\n");
  mprintf("O\tEdit the line that people see when a player comes Out of an exit\n");
  mprintf("Q\tQUIT Custom (saves changes)\n");
  mprintf("R\tRequire/don't require alias for exit; ignore direction\n");
  mprintf("S\tEdit the success line\n");
  mprintf("T\tAlter Type of exit (passage, door, etc)\n");
  mprintf("V\tView exit information\n");
  mprintf("X\tRequire/don't require exit name to be a verb\n");
  mprintf("?\tThis list\n\n");
}


void get_key(dir)
int dir;
{
  string s;
  int n;

  getobjnam();
  freeobjnam();
  if (here.exits[dir-1].objreq == 0)
    mprintf("Currently there is no key set for this exit.\n");
  else
    mprintf("%s is the current key for this exit.\n",
	   objnam.idents[here.exits[dir-1].objreq - 1]);
  mprintf("Enter * for [no key]\n\n");

  grab_line("What object is the door key? ", s, true);
  if (*s == '\0') {
    mprintf("No changes.\n");
    return;
  }
  if (!strcmp(s, "*")) {
    getroom(0);
    here.exits[dir-1].objreq = 0;
    putroom();
    mprintf("Exit updated.\n");
    return;
  }
  if (!lookup_obj(&n, s)) {
    mprintf("There is no object by that name.\n");
    return;
  }
  getroom(0);
  here.exits[dir-1].objreq = n;
  putroom();
  mprintf("Exit updated.\n");
}


void do_custom(dirnam)
char *dirnam;
{
  string prompt;
  boolean done;
  string s;
  int dir, n;
  string STR1;

  gethere(0);
  if (!checkhide())
    return;
  if (*dirnam == '\0') {
    if (is_owner(location, true)) {
      custom_room();
      /*clear_command;*/
      return;
    }
    mprintf("You are not the owner of this room; you cannot customize it.\n");
    mprintf(
      "However, you may be able to customize some of the exits.  To customize an\n");
    mprintf("exit, type CUSTOM <direction of exit>\n");
    return;
  }
  if (lookup_dir(&dir, dirnam)) {
    if (!can_alter(dir, 0)) {
      mprintf("You are not allowed to alter that exit.\n");
      return;
    }
    log_action(c_custom, 0);

    mprintf("Customizing %s exit\n", direct[dir-1]);
    mprintf(
      "If you would rather be customizing this room, type CUSTOM with no arguments\n");
    mprintf(
      "If you would rather be customizing an object, type CUSTOM <object name>\n\n");
    mprintf("Type ** for any line to leave it unchanged.\n");
    mprintf("Type return for any line to select the default.\n\n");
    sprintf(prompt, "Custom %s> ", direct[dir-1]);
    done = false;
    do {
      do {
	grab_line(prompt, s, true);
	strcpy(s, slead(STR1, s));
      } while (*s == '\0');
      strcpy(s, lowcase(STR1, s));
      switch (s[0]) {

      case '?':
      case 'h':
	custom_help();
	break;

      case 'q':
      case 'e':
	done = true;
	break;

      case 'k':
	get_key(dir);
	break;

      case 'c':
	mprintf(
	  "Type the description that a player will see when the exit is found.\n");
	mprintf("Make no text for description to unconceal the exit.\n\n");
	mprintf("[ Editing the \"hidden exit found\" description ]\n");
	n = here.exits[dir-1].hidden;
	if (edit_desc(&n)) {
	  getroom(0);
	  here.exits[dir-1].hidden = n;
	  putroom();
	}
	break;

      /*req alias*/
      case 'r':
	getroom(0);
	here.exits[dir-1].reqalias = !here.exits[dir-1].reqalias;
	putroom();

	if (here.exits[dir-1].reqalias)
	  mprintf("The alias for this exit will be required to reference it.\n");
	else
	  mprintf("The alias will not be required to reference this exit.\n");
	break;

      /*req verb*/
      case 'x':
	getroom(0);
	here.exits[dir-1].reqverb = !here.exits[dir-1].reqverb;
	putroom();

	if (here.exits[dir-1].reqverb)
	  mprintf(
	    "The exit name will be required to be used as a verb to use the exit\n");
	else
	  mprintf("The exit name may be used with GO or as a verb to use the exit\n");
	break;

      /*autolook*/
      case 'l':
	getroom(0);
	here.exits[dir-1].autolook = !here.exits[dir-1].autolook;
	putroom();

	if (here.exits[dir-1].autolook)
	  mprintf("A LOOK will be done after the player travels through this exit.\n");
	else
	  mprintf(
	    "The automatic LOOK will not be done when a player uses this exit.\n");
	break;

      case 'a':
	grab_line("Alternate name for the exit? ", s, true);
	if (strlen(s) > veryshortlen)
	  mprintf("Your alias must be less than %ld characters.\n",
		 (long)veryshortlen);
	else {
	  getroom(0);
	  lowcase(here.exits[dir-1].alias, s);
	  putroom();
	}
	break;

      case 'v':
	analyze_exit(dir);
	break;

      case 't':
	mprintf("\nSelect the type of your exit:\n\n");
	mprintf("0) No exit\n");
	mprintf("1) Open passage\n");
	mprintf("2) Door (object required to pass)\n");
	mprintf("3) No passage if holding key\n");
	if (privd)
	  mprintf("4) exit randomly fails\n");
	mprintf("6) Exit exists only when holding object\n");
	if (privd)
	  mprintf("7) exit opens/closes invisibly every minute\n");
	putchar('\n');
	grab_line("Which type? ", s, true);
	if (isnum(s)) {
	  n = number(s);
	  if ((unsigned)n < 32 && ((1L << n) & 0xdf) != 0) {
	    getroom(0);
	    here.exits[dir-1].kind = n;
	    putroom();
	    mprintf("Exit type updated.\n\n");
	    if ((unsigned)n < 32 && ((1L << n) & 0x44) != 0)
	      get_key(dir);
	  } else
	    mprintf("Bad exit type.\n");
	} else
	  mprintf("Exit type not changed.\n");
	break;

      case 'f':
	mprintf(
	  "The failure description will print if the player attempts to go through the\n");
	mprintf("the exit but cannot for any reason.\n\n");
	mprintf("[ Editing the exit failure description ]\n");

	n = here.exits[dir-1].fail;
	if (edit_desc(&n)) {
	  getroom(0);
	  here.exits[dir-1].fail = n;
	  putroom();
	}
	break;

      case 'i':
	mprintf("Edit the description that other players see when someone goes into\n");
	mprintf("the exit.  Place a # where the player's name should appear.\n\n");
	mprintf("[ Editing the exit \"go in\" description ]\n");
	n = here.exits[dir-1].goin;
	if (edit_desc(&n)) {
	  getroom(0);
	  here.exits[dir-1].goin = n;
	  putroom();
	}
	break;

      case 'o':
	mprintf(
	  "Edit the description that other players see when someone comes out of\n");
	mprintf("the exit.  Place a # where the player's name should appear.\n\n");
	mprintf("[ Editing the exit \"come out of\" description ]\n");
	n = here.exits[dir-1].comeout;
	if (edit_desc(&n)) {
	  getroom(0);
	  here.exits[dir-1].comeout = n;
	  putroom();
	}
	break;

      /* main exit desc */
      case 'd':
	mprintf("Enter a one line description of the exit.\n\n");
	n = here.exits[dir-1].exitdesc;
	make_line(&n, "", 79);
	getroom(0);
	here.exits[dir-1].exitdesc = n;
	putroom();
	break;

      case 's':
	mprintf(
	  "The success description will print when the player goes through the exit.\n\n");
	mprintf("[ Editing the exit success description ]\n");

	n = here.exits[dir-1].success;
	if (edit_desc(&n)) {
	  getroom(0);
	  here.exits[dir-1].success = n;
	  putroom();
	}
	break;

      default:
	mprintf("-- Bad command, type ? for a list\n");
	break;
      }
    } while (!done);


    log_event(myslot, E_CUSTDONE, 0, 0, "", 0);
    return;
  }
  if (lookup_obj(&n, dirnam)) {   /* customize the object */
    do_program(dirnam);
    return;
  }
  /* if lookup_obj returns TRUE then dirnam is name of object to custom */
  mprintf("To customize this room, type CUSTOM\n");
  mprintf("To customize an exits, type CUSTOM <direction>\n");
  mprintf("To customize an object, type CUSTOM <object name>\n");
}



void reveal_people(three)
boolean *three;
{
  int retry = 1;
  int i;

  if (debug)
    mprintf("?revealing people\n");
  *three = false;

  do {
    retry++;
    i = rnd100() % maxpeople + 1;
/* p2c: mon.pas, line 7649:
 * Note: Using % for possibly-negative arguments [317] */
    if (here.people[i-1].hiding > 0 && i != myslot) {
      *three = true;
      mprintf("You've found %s hiding in the shadows!\n",
	     here.people[i-1].name);
      log_event(myslot, E_FOUNDYOU, i, 0, "", 0);
    }
  } while (!(retry > 7 || *three));
}



void reveal_objects(two)
boolean *two;
{
  int i;
  string STR1;

  if (debug)
    mprintf("?revealing objects\n");
  *two = false;
  for (i = 0; i < maxobjs; i++) {
    if (here.objs[i] != 0) {   /* if there is an object here */
      if (here.objhide[i] != 0) {
	*two = true;

	if (here.objhide[i] == DEFAULT_LINE)
	  mprintf("You've found %s.\n", obj_part(STR1, here.objs[i], true));
	else {
	  print_desc(here.objhide[i], "<no default supplied>");
	  delete_block(&here.objhide[i]);
	}
      }
    }
  }
}


void reveal_exits(one)
boolean *one;
{
  int retry = 1;
  int i;

  if (debug)
    mprintf("?revealing exits\n");
  *one = false;

  do {
    retry++;
    i = rnd100() % maxexit + 1;   /* a random exit */
/* p2c: mon.pas, line 7698:
 * Note: Using % for possibly-negative arguments [317] */
    if (here.exits[i-1].hidden != 0 && !found_exit[i-1]) {
      *one = true;
      found_exit[i-1] = true;   /* mark exit as found */

      if (here.exits[i-1].hidden == DEFAULT_LINE) {
	if (*here.exits[i-1].alias == '\0')
	  mprintf("You've found a hidden exit: %s.\n", direct[i-1]);
	else
	  mprintf("You've found a hidden exit: %s.\n", here.exits[i-1].alias);
      } else
	print_desc(here.exits[i-1].hidden, "<no default supplied>");
    }
  } while (!(retry > 4 || *one));
}


void do_search(s)
char *s;
{
  int chance;
  boolean found = false, dummy = false;

  if (!checkhide())
    return;
  chance = rnd100();

  if ((unsigned)chance < 32 && ((1L << chance) & 0x1ffffeL) != 0)
    reveal_objects(&found);
  else if (chance >= 21 && chance <= 40)
    reveal_exits(&found);
  else if (chance >= 41 && chance <= 60)
    reveal_people(&dummy);

  if (found) {
    log_event(myslot, E_FOUND, 0, 0, "", 0);
    return;
  }
  if (!dummy) {
    log_event(myslot, E_SEARCH, 0, 0, "", 0);
    mprintf("You haven't found anything.\n");
  }
}


void do_unhide(s)
char *s;
{
  if (*s != '\0')
    return;
  if (!hiding) {
    mprintf("You were not hiding.\n");
    return;
  }
  hiding = false;
  log_event(myslot, E_UNHIDE, 0, 0, "", 0);
  getroom(0);
  here.people[myslot-1].hiding = 0;
  putroom();
  mprintf("You are no longer hiding.\n");
}


void do_hide(s)
char *s;
{
  int slot, n, founddsc;
  string tmp;

  gethere(0);
  if (*s == '\0') {  /* hide yourself */
    /* don't let them hide (or hide better) if people
       that they can see are in the room.  Note that the
       use of n_can_see instead of find_numpeople will
       let them hide if other people are hidden in the
       room that they have not seen.  The previously hidden
       people will see them hide */

    if (n_can_see() > 0) {
      if (hiding)
	mprintf("You can't hide any better with people in the room.\n");
      else
	mprintf("You can't hide when people are watching you.\n");
      return;
    }
    if (rnd100() > 25) {
      if (here.people[myslot-1].hiding >= 4) {
	mprintf(
	  "You're pretty well hidden now.  I don't think you could be any less visible.\n");
	return;
      }
      getroom(0);
      here.people[myslot-1].hiding++;
      putroom();
      if (hiding) {
	log_event(myslot, E_NOISES, rnd100(), 0, "", 0);
	mprintf("You've managed to hide yourself a little better.\n");
	/* Hide an object */
	return;
      }
      log_event(myslot, E_IHID, 0, 0, "", 0);
      mprintf("You've hidden yourself from view.\n");
      hiding = true;
      return;
    }
    if (hiding)
      mprintf("You could not find a better hiding place.\n");
    else
      mprintf("You could not find a good hiding place.\n");
    return;
  }

  /* unsuccessful */
  if (!parse_obj(&n, s, false)) {
    mprintf("I see no such object here.\n");
    return;
  }
  if (obj_here(n)) {
    mprintf("Enter the description the player will see when the object is found:\n");
    mprintf("(if no description is given a default will be supplied)\n\n");
    mprintf("[ Editing the \"object found\" description ]\n");
    founddsc = 0;
    edit_desc(&founddsc);
    if (founddsc == 0)
      founddsc = DEFAULT_LINE;

    getroom(0);
    slot = find_obj(n);
    here.objhide[slot-1] = founddsc;
    putroom();

    obj_part(tmp, n, true);
    log_event(myslot, E_HIDOBJ, 0, 0, tmp, 0);
    mprintf("You have hidden %s.\n", tmp);
    return;
  }
  if (obj_hold(n))
    mprintf("You'll have to put it down before it can be hidden.\n");
  else
    mprintf("I see no such object here.\n");
}


void do_punch(s)
char *s;
{
  int sock, n;

  if (*s == '\0') {
    mprintf("To punch somebody, type PUNCH <personal name>.\n");
    return;
  }
  if (!parse_pers(&n, s)) {
    mprintf("That person cannot be seen in this room.\n");
    return;
  }
  if (n == myslot) {
    mprintf("Self-abuse will not be tolerated in the Monster universe.\n");
    return;
  }
  if (protected_(n)) {
    log_event(myslot, E_TRYPUNCH, n, 0, "", 0);
    mprintf("A mystic shield of force prevents you from attacking.\n");
    return;
  }
  if (!strcmp(here.people[n-1].username, MM_userid)) {
    log_event(myslot, E_TRYPUNCH, n, 0, "", 0);
    mprintf("You can't punch the Monster Manager.\n");
    return;
  }
  if (hiding) {
    hiding = false;

    getroom(0);
    here.people[myslot-1].hiding = 0;
    putroom();

    log_event(myslot, E_HIDEPUNCH, n, 0, "", 0);
    mprintf("You pounce unexpectedly on %s!\n", here.people[n-1].name);
  } else {
    sock = rnd100() % numpunches + 1;
/* p2c: mon.pas, line 7860:
 * Note: Using % for possibly-negative arguments [317] */
    log_event(myslot, E_PUNCH, n, sock, "", 0);
    put_punch(sock, here.people[n-1].name);
  }
  doawait(1 + mrandom() * 3);   /* Ha ha ha */
}


/* support for do_program (custom an object)
   Give the player a list of kinds of object he's allowed to make his object
   and update it */

void prog_kind(objnum)
int objnum;
{
  int n;
  string s;

  mprintf("Select the type of your object:\n\n");
  mprintf("0\tOrdinary object (good for door keys)\n");
  mprintf("1\tWeapon\n");
  mprintf("2\tArmor\n");
  mprintf("3\tExit thruster\n");

  if (privd) {
    mprintf("\n100\tBag\n");
    mprintf("101\tCrystal Ball\n");
    mprintf("102\tWand of Power\n");
    mprintf("103\tHand of Glory\n");
  }
  putchar('\n');
  grab_line("Which kind? ", s, true);

  if (!isnum(s))
    return;
  n = number(s);
  if (n > 100 && !privd) {
    mprintf("Out of range.\n");
    return;
  }
  if ((unsigned)n > 3 && (n < 100 || n > 103)) {
    mprintf("Out of range.\n");
    return;
  }
  getobj(objnum);
  obj.kind = n;
  putobj();
  mprintf("Object updated.\n");
}



/* support for do_program (custom an object)
   Based on the kind it is allow the
   user to set the various parameters for the effects associated with that
   kind */

void prog_obj(objnum)
int objnum;
{
}


void show_kind(p)
int p;
{
  switch (p) {

  case 0:
    mprintf("Ordinary object\n");
    break;

  case 1:
    mprintf("Weapon\n");
    break;

  case 2:
    mprintf("Armor\n");
    break;

  case 100:
    mprintf("Bag\n");
    break;

  case 101:
    mprintf("Crystal Ball\n");
    break;

  case 102:
    mprintf("Wand of Power\n");
    break;

  case 103:
    mprintf("Hand of Glory\n");
    break;

  default:
    mprintf("Bad object type\n");
    break;
  }
}


void obj_view(objnum)
int objnum;
{
  putchar('\n');
  getobj(objnum);
  freeobj();
  getobjown();
  freeobjown();
  mprintf("Object name:    %s\n", obj.oname);
  mprintf("Owner:          %s\n\n", objown.idents[objnum-1]);
  show_kind(obj.kind);
  putchar('\n');

  if (obj.linedesc == 0)
    mprintf("There is a(n) # here\n");
  else
    print_line(obj.linedesc);

  if (obj.examine == 0)
    mprintf("No inspection description set\n");
  else
    print_desc(obj.examine, "<no default supplied>");

  /*writeln('Worth (in points) of this object: ',obj.worth:1);*/
  mprintf("Number in existence: %d\n\n", obj.numexist);
}


void program_help()
{
  mprintf("\nA\t\"a\", \"an\", \"some\", etc.\n");
  mprintf("D\tEdit a Description of the object\n");
  mprintf("F\tEdit the GET failure message\n");
  mprintf("G\tSet the object required to pick up this object\n");
  mprintf("1\tSet the get success message\n");
  mprintf("K\tSet the Kind of object this is\n");
  mprintf("L\tEdit the label description (\"There is a ... here.\")\n");
  mprintf("P\tProgram the object based on the kind it is\n");
  mprintf("R\tRename the object\n");
  mprintf("S\tToggle the sticky bit\n\n");
  mprintf("U\tSet the object required for use\n");
  mprintf("2\tSet the place required for use\n");
  mprintf("3\tEdit the use failure description\n");
  mprintf("4\tEdit the use success description\n");
  mprintf("V\tView attributes of this object\n\n");
  mprintf("X\tEdit the extra description\n");
  mprintf("5\tEdit extra desc #2\n");
  mprintf("E\tExit (same as Quit)\n");
  mprintf("Q\tQuit (same as Exit)\n");
  mprintf("?\tThis list\n\n");
}


void do_program(objnam)
char *objnam;
{
  /* (objnam: string);  declared forward */
  string prompt;
  boolean done = false;
  string s;
  int objnum, n, newdsc;
  string STR2;

  gethere(0);
  if (!checkhide())
    return;
  if (*objnam == '\0') {
    mprintf("To program an object, type PROGRAM <object name>.\n");
    return;
  }
  if (!lookup_obj(&objnum, objnam)) {
    mprintf("There is no object by that name.\n");
    return;
  }
  if (!is_owner(location, true)) {
    mprintf(
      "You may only work on your objects when you are in one of your own rooms.\n");
    return;
  }
  if (!obj_owner(objnum, false)) {
    mprintf("You are not allowed to program that object.\n");
    return;
  }
  log_action(e_program, 0);
  mprintf("\nCustomizing object\n");
  mprintf(
    "If you would rather be customizing an EXIT, type CUSTOM <direction of exit>\n");
  mprintf("If you would rather be customizing this room, type CUSTOM\n\n");
  getobj(objnum);
  freeobj();
  strcpy(prompt, "Custom object> ");
  do {
    do {
      grab_line(prompt, s, true);
      strcpy(s, slead(STR2, s));
    } while (*s == '\0');
    strcpy(s, lowcase(STR2, s));
    switch (s[0]) {

    case '?':
    case 'h':
      program_help();
      break;

    case 'q':
    case 'e':
      done = true;
      break;

    case 'v':
      obj_view(objnum);
      break;

    case 'r':
      do_objrename(objnum);
      break;

    case 'g':
      mprintf("Enter * for no object\n");
      grab_line("Object required for GET? ", s, true);
      if (!strcmp(s, "*")) {
	getobj(objnum);
	obj.getobjreq = 0;
	putobj();
      } else if (lookup_obj(&n, s)) {
	getobj(objnum);
	obj.getobjreq = n;
	putobj();
	mprintf("Object modified.\n");
      } else
	mprintf("No such object.\n");
      break;

    case 'u':
      mprintf("Enter * for no object\n");
      grab_line("Object required for USE? ", s, true);
      if (!strcmp(s, "*")) {
	getobj(objnum);
	obj.useobjreq = 0;
	putobj();
      } else if (lookup_obj(&n, s)) {
	getobj(objnum);
	obj.useobjreq = n;
	putobj();
	mprintf("Object modified.\n");
      } else
	mprintf("No such object.\n");
      break;

    case '2':
      mprintf("Enter * for no special place\n");
      grab_line("Place required for USE? ", s, true);
      if (!strcmp(s, "*")) {
	getobj(objnum);
	obj.uselocreq = 0;
	putobj();
      } else if (lookup_room(&n, s)) {
	getobj(objnum);
	obj.uselocreq = n;
	putobj();
	mprintf("Object modified.\n");
      } else
	mprintf("No such object.\n");
      break;

    case 's':
      getobj(objnum);
      obj.sticky = !obj.sticky;
      putobj();
      if (obj.sticky)
	mprintf("The object will not be takeable.\n");
      else
	mprintf("The object will be takeable.\n");
      break;

    case 'a':
      mprintf("\nSelect the article for your object:\n\n");
      mprintf("0)\tNone                ex: \" You have taken Excalibur \"\n");
      mprintf("1)\t\"a\"                 ex: \" You have taken a small box \"\n");
      mprintf("2)\t\"an\"                ex: \" You have taken an empty bottle \"\n");
      mprintf(
	"3)\t\"some\"              ex: \" You have picked up some jelly beans \"\n");
      mprintf(
	"4)     \"the\"               ex: \" You have picked up the Scepter of Power\"\n\n");
      grab_line("? ", s, true);
      if (isnum(s)) {
	n = number(s);
	if ((unsigned)n < 32 && ((1L << n) & 0x1f) != 0) {
	  getobj(objnum);
	  obj.particle = n;
	  putobj();
	} else
	  mprintf("Out of range.\n");
      } else
	mprintf("No changes.\n");
      break;

    case 'k':
      prog_kind(objnum);
      break;

    case 'p':
      prog_obj(objnum);
      break;

    case 'd':
      newdsc = obj.examine;
      mprintf("[ Editing the description of the object ]\n");
      if (edit_desc(&newdsc)) {
	getobj(objnum);
	obj.examine = newdsc;
	putobj();
      }
      break;

    case 'x':
      newdsc = obj.d1;
      mprintf("[ Editing extra description #1 ]\n");
      if (edit_desc(&newdsc)) {
	getobj(objnum);
	obj.d1 = newdsc;
	putobj();
      }
      break;

    case '5':
      newdsc = obj.d2;
      mprintf("[ Editing extra description #2 ]\n");
      if (edit_desc(&newdsc)) {
	getobj(objnum);
	obj.d2 = newdsc;
	putobj();
      }
      break;

    case 'f':
      newdsc = obj.getfail;
      mprintf("[ Editing the get failure description ]\n");
      if (edit_desc(&newdsc)) {
	getobj(objnum);
	obj.getfail = newdsc;
	putobj();
      }
      break;

    case '1':
      newdsc = obj.getsuccess;
      mprintf("[ Editing the get success description ]\n");
      if (edit_desc(&newdsc)) {
	getobj(objnum);
	obj.getsuccess = newdsc;
	putobj();
      }
      break;

    case '3':
      newdsc = obj.usefail;
      mprintf("[ Editing the use failure description ]\n");
      if (edit_desc(&newdsc)) {
	getobj(objnum);
	obj.usefail = newdsc;
	putobj();
      }
      break;

    case '4':
      newdsc = obj.usesuccess;
      mprintf("[ Editing the use success description ]\n");
      if (edit_desc(&newdsc)) {
	getobj(objnum);
	obj.usesuccess = newdsc;
	putobj();
      }
      break;

    case 'l':
      mprintf(
	"Enter a one line description of what the object will look like in any room.\n");
      mprintf("Example: \"There is an as unyet described object here.\"\n\n");
      getobj(objnum);
      freeobj();
      n = obj.linedesc;
      make_line(&n, "", 79);
      getobj(objnum);
      obj.linedesc = n;
      putobj();
      break;

    default:
      mprintf("-- Bad command, type ? for a list\n");
      break;
    }
  } while (!done);
  log_event(myslot, E_OBJDONE, objnum, 0, "", 0);

}


/* returns TRUE if anything was actually dropped */
boolean drop_everything(pslot)
int pslot;
{
  /* forward function drop_everything(pslot: integer := 0): boolean; */
  int i, slot;
  boolean didone = false;
  int theobj;
  string tmp;

  if (pslot == 0)
    pslot = myslot;

  gethere(0);

  mywield = 0;
  mywear = 0;

  for (i = 0; i < maxhold; i++) {
    if (here.people[pslot-1].holding[i] != 0) {
      didone = true;
      theobj = here.people[pslot-1].holding[i];
      slot = find_hold(theobj, pslot);
      if (place_obj(theobj, true))
	drop_obj(slot, pslot);
      else {
	getobj(theobj);
	obj.numexist--;
	putobj();
	strcpy(tmp, obj.oname);
	mprintf("The %s was lost.\n", tmp);
      }
    }
  }

  /* no place to put it, it's lost .... */
  return didone;
}


void do_endplay(lognum, ping)
int lognum;
boolean ping;
{

  /* If update is true do_endplay will update the "last play" date & time
     we don't want to do this if this endplay is called from a ping */
  string STR1, STR3;

  if (!ping) {
    /* Set the "last date & time of play" */
    getdate();
    sprintf(adate.idents[lognum-1], "%s %s", sysdate(STR1), systime(STR3));
    putdate();
  }


  /* Put the player to sleep.  Don't delete his information,
     so it can be restored the next time they play. */

  getindex(I_ASLEEP);
  indx.free[lognum-1] = true;   /* Yes, I'm asleep */
  putindex();
}


boolean check_person(n, id)
int n;
char *id;
{
  inmem = false;
  gethere(0);
  if (!strcmp(here.people[n-1].username, id))
    return true;
  else
    return false;
}


boolean nuke_person(n, id)
int n;
char *id;
{
  int lognum;
  string tmp;

  getroom(0);
  if (!strcmp(here.people[n-1].username, id)) {
    /* drop everything they're carrying */
    drop_everything(n);

    strcpy(tmp, here.people[n-1].username);
    /* we'll need this for do_endplay */

    /* Remove the person from the room */
    here.people[n-1].kind = 0;
    *here.people[n-1].username = '\0';
    *here.people[n-1].name = '\0';
    putroom();

    /* update the log entries for them */
    /* but first we have to find their log number
       (mylog for them).  We can do this with a lookup_user
       give the userid we got above */

    if (lookup_user(&lognum, tmp)) {
      do_endplay(lognum, true);
      /* TRUE tells do_endplay not to update the
         "time of last play" information 'cause we
         don't know how long the "zombie" has been
         there. */
    } else
      mprintf(
	"?error in nuke_person; can't find their log number; notify the Monster Manager\n");

    return true;
  }

  else {
    freeroom();
    return false;
  }
}


boolean ping_player(n, silent)
int n;
boolean silent;
{
  boolean Result = false;
  int retry = 0;
  string id, idname;


  strcpy(id, here.people[n-1].username);
  strcpy(idname, here.people[n-1].name);

  ping_answered = false;

  do {
    retry++;
    if (!silent)
      mprintf("Sending ping # %d to %s . . .\n", retry, idname);

    log_event(myslot, E_PING, n, 0, myname, 0);
    doawait(1.0);
    checkevents(true);
    /* TRUE = don't reprint prompt */

    if (!ping_answered) {
      if (check_person(n, id)) {
	doawait(1.0);
	checkevents(true);
      } else
	ping_answered = true;
    }

    if (!ping_answered) {
      if (check_person(n, id)) {
	doawait(1.0);
	checkevents(true);
      } else
	ping_answered = true;
    }

  } while (!(retry >= 3 || ping_answered));

  if (ping_answered) {
    if (!silent)
      mprintf("That person is alive and well.\n");
    return false;
  }
  if (!silent)
    mprintf("That person is not responding to your pings . . .\n");

  if (!nuke_person(n, id)) {
    if (!silent)
      mprintf("That person is not a zombie after all.\n");
    return false;
  }
  Result = true;
  if (!silent)
    mprintf("%s shimmers and vanishes from sight.\n", idname);
  log_event(myslot, E_PINGONE, n, 0, idname, 0);
  return true;
}


void do_ping(s)
char *s;
{
  int n;
  boolean dummy;

  if (*s == '\0') {
    mprintf("To see if someone is really alive, type PING <personal name>.\n");
    return;
  }
  if (!parse_pers(&n, s)) {
    mprintf("You see no person here by that name.\n");
    return;
  }
  if (n == myslot)
    mprintf("Don't ping yourself.\n");
  else
    dummy = ping_player(n, false);
}


void list_get()
{
  boolean first = true;
  int i;
  string STR1;

  for (i = 0; i < maxobjs; i++) {
    if (here.objs[i] != 0 && here.objhide[i] == 0) {
      if (first) {
	mprintf("Objects that you see here:\n");
	first = false;
      }
      mprintf("   %s\n", obj_part(STR1, here.objs[i], true));
    }
  }
  if (first)
    mprintf("There is nothing you see here that you can get.\n");
}



/* print the get success message for object number n */

void p_getsucc(n)
int n;
{
  /* we assume getobj has already been done */
  if (obj.getsuccess == 0 || obj.getsuccess == DEFAULT_LINE)
    mprintf("Taken.\n");
  else
    print_desc(obj.getsuccess, "<no default supplied>");
}


void do_meta_get(n)
int n;
{
  int slot;
  char STR1[118];
  string STR2;

  if (obj_here(n)) {
    if (!can_hold()) {
      mprintf(
	"Your hands are full.  You'll have to drop something you're carrying first.\n");
      return;
    }
    slot = find_obj(n);
    if (!take_obj(n, slot)) {
      mprintf("Someone got to it before you did.\n");
      return;
    }
    hold_obj(n);
    sprintf(STR1, "%s has picked up %s.", myname, obj_part(STR2, n, true));
    /* >>> */
    log_event(myslot, E_GET, 0, 0, STR1, 0);
    p_getsucc(n);
    return;
  }
  if (obj_hold(n))
    mprintf("You're already holding that item.\n");
  else
    mprintf("That item isn't in an obvious place.\n");
}


void do_get(s)
char *s;
{
  int n;
  boolean ok;
  string STR2;

  if (*s == '\0') {
    list_get();
    return;
  }
  if (parse_obj(&n, s, true)) {
    getobj(n);
    freeobj();
    ok = true;

    if (obj.sticky) {
      ok = false;
      log_event(myslot, E_FAILGET, n, 0, "", 0);
      if (obj.getfail == 0 || obj.getfail == DEFAULT_LINE)
	mprintf("You can't take %s.\n", obj_part(STR2, n, false));
      else
	print_desc(obj.getfail, "<no default supplied>");
    } else if (obj.getobjreq > 0) {
      if (!obj_hold(obj.getobjreq)) {
	ok = false;
	log_event(myslot, E_FAILGET, n, 0, "", 0);
	if (obj.getfail == 0 || obj.getfail == DEFAULT_LINE)
	  mprintf("You'll need something first to get the %s.\n",
		 obj_part(STR2, n, false));
	else
	  print_desc(obj.getfail, "<no default supplied>");
      }
    }

    if (ok)   /* get the object */
      do_meta_get(n);

    return;
  }
  if (lookup_detail(&n, s)) {
    mprintf(
      "That detail of this room is here for the enjoyment of all Monster players,\n");
    mprintf("and may not be taken.\n");
  } else
    mprintf("There is no object here by that name.\n");
}


void do_drop(s)
char *s;
{
  int slot, n;
  string STR1;
  char STR2[116];

  if (*s == '\0') {
    mprintf("To drop an object, type DROP <object name>.\n");
    mprintf("To see what you are carrying, type INV (inventory).\n");
    return;
  }
  if (!parse_obj(&n, s, false)) {
    mprintf(
      "You're not holding that item.  To see what you're holding, type INVENTORY.\n");
    return;
  }
  if (!obj_hold(n)) {
    mprintf("You're not holding that item.  To see what you're holding, type INV.\n");
    return;
  }
  getobj(n);
  freeobj();
  if (obj.sticky) {
    mprintf("You can't drop sticky objects.\n");
    return;
  }
  if (!can_drop()) {
    mprintf("It is too cluttered here.  Find somewhere else to drop your things.\n");
    return;
  }
  slot = find_hold(n, 0);
  if (!place_obj(n, false)) {
    mprintf("Someone took the spot where your were going to drop it.\n");
    return;
  }
  drop_obj(slot, 0);
  sprintf(STR2, "%s has dropped %s.", myname, obj_part(STR1, n, true));
  log_event(myslot, E_DROP, 0, n, STR2, 0);

  if (mywield == n) {
    mywield = 0;
    getroom(0);
    here.people[myslot-1].wielding = 0;
    putroom();
  }
  if (mywear != n)
    return;
  mywear = 0;
  getroom(0);
  here.people[myslot-1].wearing = 0;
  putroom();
}


void do_inv(s)
char *s;
{
  boolean first;
  int i, n, objnum;
  string STR1;

  gethere(0);
  if (*s == '\0') {
    noisehide(50);
    first = true;
    log_event(myslot, E_INVENT, 0, 0, "", 0);
    for (i = 0; i < maxhold; i++) {
      objnum = here.people[myslot-1].holding[i];
      if (objnum != 0) {
	if (first) {
	  mprintf("You are holding:\n");
	  first = false;
	}
	mprintf("   %s\n", obj_part(STR1, objnum, true));
      }
    }
    if (first)
      mprintf("You are empty handed.\n");
    return;
  }
  if (!parse_pers(&n, s)) {
    mprintf("To see what someone else is carrying, type INV <personal name>.\n");
    return;
  }
  first = true;
  log_event(myslot, E_LOOKYOU, n, 0, "", 0);
  for (i = 0; i < maxhold; i++) {
    objnum = here.people[n-1].holding[i];
    if (objnum != 0) {
      if (first) {
	mprintf("%s is holding:\n", here.people[n-1].name);
	first = false;
      }
      mprintf("   %s\n", objnam.idents[objnum-1]);
    }
  }
  if (first)
    mprintf("%s is empty handed.\n", here.people[n-1].name);
}


/* translate a personal name into a real userid on request */

void do_whois(s)
char *s;
{
  int n;

  if (!lookup_pers(&n, s)) {
    mprintf("There is no one playing with that personal name.\n");
    return;
  }
  getuser();
  freeuser();
  /*getpers;
                  freepers;! Already done in lookup_pers !*/

  mprintf("%s is %s.\n", pers.idents[n-1], user.idents[n-1]);
}


void do_players(s)
char *s;
{
  int i, j;
  indexrec tmpasleep;
  intrec where_they_are;

  log_event(myslot, E_PLAYERS, 0, 0, "", 0);
  getindex(I_ASLEEP);   /* Rec of bool; False if playing now */
  freeindex();
  memcpy(&tmpasleep, &indx, sizeof(indexrec));

  getindex(I_PLAYER);   /* Rec of valid player log records  */
  freeindex();   /* False if a valid player log */

  getuser();   /* Corresponding userids of players */
  freeuser();

  getpers();   /* Personal names of players */
  freepers();

  getdate();   /* date of last play */
  freedate();

  if (privd) {
    getint(N_LOCATION);
    freeint();
    memcpy(&where_they_are, &anint, sizeof(intrec));

    getnam();
    freenam();
  }

  getint(N_SELF);
  freeint();

  mprintf("\nUserid          Personal Name              Last Play\n");
  for (i = 0; i < maxplayers; i++) {
    if (!indx.free[i]) {
      fputs(user.idents[i], stdout);
      for (j = strlen(user.idents[i]); j <= 15; j++)
	putchar(' ');
      fputs(pers.idents[i], stdout);
      for (j = strlen(pers.idents[i]); j <= 21; j++)
	putchar(' ');

      if (tmpasleep.free[i]) {
	fputs(adate.idents[i], stdout);
	if (strlen(adate.idents[i]) < 19) {
	  for (j = strlen(adate.idents[i]); j <= 18; j++)
	    putchar(' ');
	}
      } else
	mprintf("   -playing now-   ");

      if (anint.int_[i] != 0 && anint.int_[i] != DEFAULT_LINE)
	mprintf(" * ");
      else
	mprintf("   ");

      if (privd)
	fputs(nam.idents[where_they_are.int_[i] - 1], stdout);
      putchar('\n');
    }
  }
  putchar('\n');
}


void do_self(s)
char *s;
{
  int n;

  if (*s == '\0') {
    log_action(c_self, 0);
    mprintf("[ Editing your self description ]\n");
    if (!edit_desc(&myself))
      return;
    getroom(0);
    here.people[myslot-1].self = myself;
    putroom();
    getint(N_SELF);
    anint.int_[mylog-1] = myself;
    putint();
    log_event(myslot, E_SELFDONE, 0, 0, "", 0);
    return;
  }
  if (!lookup_pers(&n, s)) {
    mprintf("There is no person by that name.\n");
    return;
  }
  getint(N_SELF);
  freeint();
  if (anint.int_[n-1] == 0 || anint.int_[n-1] == DEFAULT_LINE)
    mprintf("That person has not made a self-description.\n");
  else {
    print_desc(anint.int_[n-1], "<no default supplied>");
    log_event(myslot, E_VIEWSELF, 0, 0, pers.idents[n-1], 0);
  }
}


void do_health(s)
char *s;
{
  mprintf("You ");
  switch (myhealth) {

  case 9:
    mprintf("are in exceptional health.\n");
    break;

  case 8:
    mprintf("are in better than average condition.\n");
    break;

  case 7:
    mprintf("are in perfect health.\n");
    break;

  case 6:
    mprintf("feel a little bit dazed.\n");
    break;

  case 5:
    mprintf("have some minor cuts and abrasions.\n");
    break;

  case 4:
    mprintf("have some wounds, but are still fairly strong.\n");
    break;

  case 3:
    mprintf("are suffering from some serious wounds.\n");
    break;

  case 2:
    mprintf("are very badly wounded.\n");
    break;

  case 1:
    mprintf("have many serious wounds, and are near death.\n");
    break;

  case 0:
    mprintf("are dead.\n");
    break;

  default:
    mprintf("don't seem to be in any condition at all.\n");
    break;
  }
}


void crystal_look(chill_msg)
int chill_msg;
{
  int numobj, numppl, numsee, i;
  boolean yes;

  putchar('\n');
  print_desc(here.primary, "<no default supplied>");
  log_event(0, E_CHILL, chill_msg, 0, "", here.locnum);
  numppl = find_numpeople();
  numsee = n_can_see() + 1;

  if (numppl > numsee)
    mprintf("Someone is hiding here.\n");
  else if (numppl == 0)
    mprintf("Strange, empty shadows swirl before your eyes.\n");
  if (rnd100() > 50)
    people_header("at this place.");
  else {
    switch (numppl) {

    case 0:
      mprintf("Vague empty forms drift through your view.\n");
      break;

    case 1:
      mprintf("You can make out a shadowy figure here.\n");
      break;

    case 2:
      mprintf("There are two dark figures here.\n");
      break;

    case 3:
      mprintf("You can see the silhouettes of three people.\n");
      break;

    default:
      mprintf("Many dark figures can be seen here.\n");
      break;
    }
  }

  numobj = find_numobjs();
  if (rnd100() > 50) {
    if (rnd100() > 50)
      show_objects();
    else if (numobj > 0)
      mprintf("Some objects are here.\n");
    else
      mprintf("There are no objects here.\n");
  } else {
    yes = false;
    for (i = 0; i < maxobjs; i++) {
      if (here.objhide[i] != 0)
	yes = true;
    }
    if (yes)
      mprintf("Something is hidden here.\n");
  }
  putchar('\n');
}


void use_crystal(objnum)
int objnum;
{
  boolean done;
  string s;
  int n, done_msg, chill_msg;
  string tmp;
  int i;
  string STR1;

  if (!obj_hold(objnum)) {
    mprintf("You must be holding it first.\n");
    return;
  }
  log_action(e_usecrystal, 0);
  getobj(objnum);
  freeobj();
  done_msg = obj.d1;
  chill_msg = obj.d2;

  grab_line("", s, true);
  if (lookup_room(&n, s)) {
    gethere(n);
    crystal_look(chill_msg);
    done = false;
  } else
    done = true;

  while (!done) {
    grab_line("", s, true);
    if (lookup_dir(&n, s)) {
      if (here.exits[n-1].toloc > 0) {
	gethere(here.exits[n-1].toloc);
	crystal_look(chill_msg);
      }
      continue;
    }
    strcpy(s, lowcase(STR1, s));
    bite(tmp, s);
    if (!strcmp(tmp, "poof")) {
      if (lookup_room(&n, s)) {
	gethere(n);
	crystal_look(chill_msg);
      } else
	done = true;
    } else if (!strcmp(tmp, "say")) {
      i = (rnd100() & 3) + 1;
      log_event(0, E_NOISE2, i, 0, "", n);
    } else
      done = true;
  }

  gethere(0);
  log_event(myslot, E_DONECRYSTALUSE, 0, 0, "", 0);
  print_desc(done_msg, "<no default supplied>");
}



void p_usefail(n)
int n;
{
  /* we assume getobj has already been done */
  if (obj.usefail == 0 || obj.usefail == DEFAULT_LINE)
    mprintf("It doesn't work for some reason.\n");
  else
    print_desc(obj.usefail, "<no default supplied>");
}


void p_usesucc(n)
int n;
{
  /* we assume getobj has already been done */
  if (obj.usesuccess == 0 || obj.usesuccess == DEFAULT_LINE)
    mprintf("It seems to work, but nothing appears to happen.\n");
  else
    print_desc(obj.usesuccess, "<no default supplied>");
}


void do_use(s)
char *s;
{
  int n;

  if (*s == '\0') {
    mprintf("To use an object, type USE <object name>\n");
    return;
  }
  if (!parse_obj(&n, s, false)) {
    mprintf("There is no such object here.\n");
    return;
  }
  getobj(n);
  freeobj();

  if (obj.useobjreq > 0 && !obj_hold(obj.useobjreq)) {
    log_event(myslot, E_FAILUSE, n, 0, "", 0);
    p_usefail(n);
    return;
  }
  if (obj.uselocreq > 0 && location != obj.uselocreq) {
    log_event(myslot, E_FAILUSE, n, 0, "", 0);
    p_usefail(n);
    return;
  }
  p_usesucc(n);
  switch (obj.kind) {

  case O_BLAND:
    /* blank case */
    break;

  case O_CRYSTAL:
    use_crystal(n);
    break;
  }
}


void do_whisper(s_)
char *s_;
{
  string s;
  int n;

  strcpy(s, s_);
  if (*s == '\0') {
    mprintf("To whisper to someone, type WHISPER <personal name>.\n");
    return;
  }
  if (!parse_pers(&n, s)) {
    mprintf("No such person can be seen here.\n");
    return;
  }
  if (n == myslot) {
    mprintf("You can't whisper to yourself.\n");
    return;
  }
  grab_line(">> ", s, true);
  if (*s != '\0') {
    nice_say(s);
    log_event(myslot, E_WHISPER, n, 0, s, 0);
  } else
    mprintf("Nothing whispered.\n");
}


void do_wield(s)
char *s;
{
  string tmp;
  int n;
  string STR2;

  if (*s == '\0') {  /* no parms means unwield */
    if (mywield == 0) {
      mprintf("You are not wielding anything.\n");
      return;
    }
    getobj(mywield);
    freeobj();
    strcpy(tmp, obj.oname);
    log_event(myslot, E_UNWIELD, 0, 0, tmp, 0);
    mprintf("You are no longer wielding the %s.\n", tmp);

    mywield = 0;
    getroom(0);
    here.people[mylog-1].wielding = 0;
    putroom();
    return;
  }
  if (!parse_obj(&n, s, false)) {
    mprintf("No such weapon can be seen here.\n");
    return;
  }
  if (mywield != 0) {
    mprintf("You are already wielding %s.\n", obj_part(STR2, mywield, true));
    return;
  }
  getobj(n);
  freeobj();
  strcpy(tmp, obj.oname);
  if (obj.kind != O_WEAPON) {
    mprintf("That is not a weapon.\n");
    return;
  }
  if (!obj_hold(n)) {
    mprintf("You must be holding it first.\n");
    return;
  }
  mywield = n;
  getroom(0);
  here.people[myslot-1].wielding = n;
  putroom();

  log_event(myslot, E_WIELD, 0, 0, tmp, 0);
  mprintf("You are now wielding the %s.\n", tmp);
}


void do_wear(s)
char *s;
{
  string tmp;
  int n;

  if (*s == '\0') {  /* no parms means unwield */
    if (mywear == 0) {
      mprintf("You are not wearing anything.\n");
      return;
    }
    getobj(mywear);
    freeobj();
    strcpy(tmp, obj.oname);
    log_event(myslot, E_UNWEAR, 0, 0, tmp, 0);
    mprintf("You are no longer wearing the %s.\n", tmp);

    mywear = 0;
    getroom(0);
    here.people[mylog-1].wearing = 0;
    putroom();
    return;
  }
  if (!parse_obj(&n, s, false)) {
    mprintf("No such thing can be seen here.\n");
    return;
  }
  getobj(n);
  freeobj();
  strcpy(tmp, obj.oname);
  if (obj.kind != O_ARMOR && obj.kind != O_CLOAK) {
    mprintf("That cannot be worn.\n");
    return;
  }
  if (!obj_hold(n)) {
    mprintf("You must be holding it first.\n");
    return;
  }
  mywear = n;
  getroom(0);
  here.people[mylog-1].wearing = n;
  putroom();

  log_event(myslot, E_WEAR, 0, 0, tmp, 0);
  mprintf("You are now wearing the %s.\n", tmp);
}


void do_brief()
{
  brief = !brief;
  if (brief)
    mprintf("Brief descriptions.\n");
  else
    mprintf("Verbose descriptions.\n");
}


char *p_door_key(Result, n)
char *Result;
int n;
{
  if (n == 0)
    return strcpy(Result, "<none>");
  else
    return strcpy(Result, objnam.idents[n-1]);
}



void anal_exit(dir)
int dir;
{
  exit_ *WITH;
  string STR2;

  if (here.exits[dir-1].toloc == 0 && here.exits[dir-1].kind != 5)
    return;
  /* no exit here, don't print anything */
  WITH = &here.exits[dir-1];
  fputs(direct[dir-1], stdout);
  if (*WITH->alias != '\0') {
    mprintf("(%s", WITH->alias);
    if (WITH->reqalias)
      mprintf(" required): ");
    else
      mprintf("): ");
  } else
    mprintf(": ");

  if (WITH->toloc == 0 && WITH->kind == 5)
    mprintf("accept, no exit yet");
  else if (WITH->toloc > 0) {
    mprintf("to %s, ", nam.idents[WITH->toloc - 1]);
    switch (WITH->kind) {

    case 0:
      mprintf("no exit");
      break;

    case 1:
      mprintf("open passage");
      break;

    case 2:
      mprintf("door, key=%s", p_door_key(STR2, WITH->objreq));
      break;

    case 3:
      mprintf("~door, ~key=%s", p_door_key(STR2, WITH->objreq));
      break;

    case 4:
      mprintf("exit open randomly");
      break;

    case 5:
      mprintf("potential exit");
      break;

    case 6:
      mprintf("xdoor, key=%s", p_door_key(STR2, WITH->objreq));
      break;

    case 7:
      mprintf("timed exit, now ");
      if (cycle_open())
	mprintf("open");
      else
	mprintf("closed");
      break;
    }
    if (WITH->hidden != 0)
      mprintf(", hidden");
    if (WITH->reqverb)
      mprintf(", reqverb");
    if (!WITH->autolook)
      mprintf(", autolook off");
    if (here.trapto == dir)
      mprintf(", trapdoor (%d%%)", here.trapchance);
  }
  putchar('\n');
}


void do_s_exits()
{
  int i;
  boolean accept;
  boolean one = false;

  /* accept is true if the particular exit is
                            an "accept" (other players may link there)
                            one means at least one exit was shown */

  gethere(0);

  for (i = 1; i <= maxexit; i++) {
    if (here.exits[i-1].toloc == 0 && here.exits[i-1].kind == 5)
      accept = true;
    else
      accept = false;

    if (can_alter(i, 0) || accept) {
      if (!one) {  /* first time we do this then */
	getnam();   /* read room name list in */
	freenam();
	getobjnam();
	freeobjnam();
      }
      one = true;
      anal_exit(i);
    }
  }

  if (!one)
    mprintf("There are no exits here which you may inspect.\n");
}


void do_s_object(s_)
char *s_;
{
  string s;
  int n;
  objectrec x;
  string STR1;

  strcpy(s, s_);
  if (*s == '\0')
    grab_line("Object? ", s, true);

  if (!lookup_obj(&n, s)) {
    mprintf("There is no such object.\n");
    return;
  }
  if (!obj_owner(n, true)) {
    mprintf("You are not allowed to see the internals of that object.\n");
    return;
  }
  mprintf("%s: ", obj_part(STR1, n, true));
  mprintf("%s is owner", objown.idents[n-1]);
  memcpy(&x, &obj, sizeof(objectrec));

  if (x.sticky)
    mprintf(", sticky");
  if (x.getobjreq > 0)
    mprintf(", %s required to get", obj_part(STR1, x.getobjreq, true));
  if (x.useobjreq > 0)
    mprintf(", %s required to use", obj_part(STR1, x.useobjreq, true));
  if (x.uselocreq > 0) {
    getnam();
    freenam();
    mprintf(", used only in %s", nam.idents[x.uselocreq - 1]);
  }
  if (*x.usealias != '\0') {
    mprintf(", use=\"%s\"", x.usealias);
    if (x.reqalias)
      mprintf(" (required)");
  }

  putchar('\n');
}


void do_s_details()
{
  int i;
  boolean one = false;

  gethere(0);
  for (i = 0; i < maxdetail; i++) {
    if (*here.detail[i] != '\0' && here.detaildesc[i] != 0) {
      if (!one) {
	one = true;
	mprintf("Details here that you may inspect:\n");
      }
      mprintf("    %s\n", here.detail[i]);
    }
  }
  if (!one)
    mprintf("There are no details of this room that you can inspect.\n");
}


void do_s_help()
{
  mprintf("\nExits             Lists exits you can inspect here\n");
  mprintf("Object            Show internals of an object\n");
  mprintf("Details           Show details you can look at in this room\n\n");
}


void s_show(n, s)
int n;
char *s;
{
  switch (n) {

  case s_exits:
    do_s_exits();
    break;

  case s_object:
    do_s_object(s);
    break;

  case s_quest:
    do_s_help();
    break;

  case s_details:
    do_s_details();
    break;
  }
}


void do_y_altmsg()
{
  int newdsc;

  if (!is_owner(0, false))
    return;
  gethere(0);
  newdsc = here.xmsg2;
  mprintf("[ Editing the alternate mystery message for this room ]\n");
  if (!edit_desc(&newdsc))
    return;
  getroom(0);
  here.xmsg2 = newdsc;
  putroom();
}


void do_y_help()
{
  mprintf("\nAltmsg        Set the alternate mystery message block\n\n");
}


void do_group1()
{
  string grpnam;
  int loc;
  string tmp;
  char STR1[256];

  if (!is_owner(0, false))
    return;
  gethere(0);
  if (here.grploc1 == 0)
    mprintf("No primary group location set\n");
  else {
    getnam();
    freenam();
    mprintf("The primary group location is %s.\n",
	   nam.idents[here.grploc1 - 1]);
    mprintf("Descriptor string: [%s]\n", here.grpnam1);
  }
  mprintf("\nType * to turn off the primary group location\n");
  grab_line("Room name of primary group? ", grpnam, true);
  if (*grpnam == '\0') {
    mprintf("No changes.\n");
    return;
  }
  if (!strcmp(grpnam, "*")) {
    getroom(0);
    here.grploc1 = 0;
    putroom();
    return;
  }
  if (!lookup_room(&loc, grpnam)) {
    mprintf("No such room.\n");
    return;
  }
  mprintf("Enter the descriptive string.  It will be placed after player names.\n");
  mprintf(
    "Example:  Monster Manager is [descriptive string, instead of \"here.\"]\n\n");
  grab_line("Enter string? ", tmp, true);
  if (strlen(tmp) > shortlen) {
    mprintf("Your string was truncated to %ld characters.\n", (long)shortlen);
    sprintf(tmp, "%.*s", shortlen, strcpy(STR1, tmp));
  }
  getroom(0);
  here.grploc1 = loc;
  strcpy(here.grpnam1, tmp);
  putroom();
}



void do_group2()
{
  string grpnam;
  int loc;
  string tmp;
  char STR1[256];

  if (!is_owner(0, false))
    return;
  gethere(0);
  if (here.grploc2 == 0)
    mprintf("No secondary group location set\n");
  else {
    getnam();
    freenam();
    mprintf("The secondary group location is %s.\n",
	   nam.idents[here.grploc1 - 1]);
    mprintf("Descriptor string: [%s]\n", here.grpnam1);
  }
  mprintf("\nType * to turn off the secondary group location\n");
  grab_line("Room name of secondary group? ", grpnam, true);
  if (*grpnam == '\0') {
    mprintf("No changes.\n");
    return;
  }
  if (!strcmp(grpnam, "*")) {
    getroom(0);
    here.grploc2 = 0;
    putroom();
    return;
  }
  if (!lookup_room(&loc, grpnam)) {
    mprintf("No such room.\n");
    return;
  }
  mprintf("Enter the descriptive string.  It will be placed after player names.\n");
  mprintf(
    "Example:  Monster Manager is [descriptive string, instead of \"here.\"]\n\n");
  grab_line("Enter string? ", tmp, true);
  if (strlen(tmp) > shortlen) {
    mprintf("Your string was truncated to %ld characters.\n", (long)shortlen);
    sprintf(tmp, "%.*s", shortlen, strcpy(STR1, tmp));
  }
  getroom(0);
  here.grploc2 = loc;
  strcpy(here.grpnam2, tmp);
  putroom();
}


void s_set(n, s)
int n;
char *s;
{
  switch (n) {

  case y_quest:
    do_y_help();
    break;

  case y_altmsg:
    do_y_altmsg();
    break;

  case y_group1:
    do_group1();
    break;

  case y_group2:
    do_group2();
    break;
  }
}


void do_show(s_)
char *s_;
{
  string s;
  int n;
  string cmd;

  strcpy(s, s_);
  bite(cmd, s);
  if (*cmd == '\0')
    grab_line("Show what attribute? (type ? for a list) ", cmd, true);

  if (*cmd == '\0')
    return;
  if (lookup_show(&n, cmd))
    s_show(n, s);
  else
    mprintf("Invalid show option, type SHOW ? for a list.\n");
}


void do_set(s_)
char *s_;
{
  string s;
  int n;
  string cmd;

  strcpy(s, s_);
  bite(cmd, s);
  if (*cmd == '\0')
    grab_line("Set what attribute? (type ? for a list) ", cmd, true);

  if (*cmd == '\0')
    return;
  if (lookup_set(&n, cmd))
    s_set(n, s);
  else
    mprintf("Invalid set option, type SET ? for a list.\n");
}


void parser()
{
  string s, cmd;
  int n;
  string STR1;
  char STR2[256];

  do {
    grab_line("> ", s, true);
    strcpy(s, slead(STR1, s));
  } while (*s == '\0');

  if (!strcmp(s, "."))
    strcpy(s, oldcmd);
  else
    strcpy(oldcmd, s);

  if (s[0] == '\'' && strlen(s) > 1) {
    sprintf(STR2, "%.*s", (int)(strlen(s) - 1), s + 1);
    do_say(STR2);
    return;
  }
  bite(cmd, s);
  switch (lookup_cmd(cmd)) {

  /* try exit alias */
  case error:
    if (lookup_alias(&n, cmd) || lookup_dir(&n, cmd))
      do_go(cmd, true);
    else
      mprintf("Bad command, type ? for a list.\n");
    break;

  case setnam:
    do_setname(s);
    break;

  case help:
  case quest:
    show_help();
    break;

  case quit:
    done = true;
    break;

  case c_l:
  case look:
    do_look(s);
    break;

  case go:   /* FALSE = dir not a verb */
    do_go(s, false);
    break;

  case form:
    do_form(s);
    break;

  case link:
    do_link(s);
    break;

  case unlink:
    do_unlink(s);
    break;

  case poof:
    do_poof(s);
    break;

  case desc:
    do_describe(s);
    break;

  case say:
    do_say(s);
    break;

  case c_rooms:
    do_rooms(s);
    break;

  case c_claim:
    do_claim(s);
    break;

  case c_disown:
    do_disown(s);
    break;

  case c_public:
    do_public(s);
    break;

  case c_accept:
    do_accept(s);
    break;

  case c_refuse:
    do_refuse(s);
    break;

  case c_zap:
    do_zap(s);
    break;

  case c_north:
  case c_n:
  case c_south:
  case c_s:
  case c_east:
  case c_e:
  case c_west:
  case c_w:
  case c_up:
  case c_u:
  case c_down:
  case c_d:
    do_go(cmd, true);
    break;

  case c_who:
    do_who();
    break;

  case c_custom:
    do_custom(s);
    break;

  case c_search:
    do_search(s);
    break;

  case c_system:
    do_system(s);
    break;

  case c_hide:
    do_hide(s);
    break;

  case c_unhide:
    do_unhide(s);
    break;

  case c_punch:
    do_punch(s);
    break;

  case c_ping:
    do_ping(s);
    break;

  case c_create:
    do_makeobj(s);
    break;

  case c_get:
    do_get(s);
    break;

  case c_drop:
    do_drop(s);
    break;

  case c_i:
  case c_inv:
    do_inv(s);
    break;

  case c_whois:
    do_whois(s);
    break;

  case c_players:
    do_players(s);
    break;

  case c_health:
    do_health(s);
    break;

  case c_duplicate:
    do_duplicate(s);
    break;

  case c_version:
    do_version(s);
    break;

  case c_objects:
    do_objects();
    break;

  case c_self:
    do_self(s);
    break;

  case c_use:
    do_use(s);
    break;

  case c_whisper:
    do_whisper(s);
    break;

  case c_wield:
    do_wield(s);
    break;

  case c_brief:
    do_brief();
    break;

  case c_wear:
    do_wear(s);
    break;

  case c_destroy:
    do_destroy(s);
    break;

  case c_relink:
    do_relink(s);
    break;

  case c_unmake:
    do_unmake(s);
    break;

  case c_show:
    do_show(s);
    break;

  case c_set:
    do_set(s);
    break;

  case dbg:
    debug = !debug;
    if (debug)
      mprintf("Debugging is on.\n");
    else
      mprintf("Debugging is off.\n");
    break;

  default:
    mprintf("?Parser error, bad return from lookup\n");
    break;
  }
  clear_command();
}



void init()
{
  char STR1[40];
  string STR2;
  char STR3[42];
  struct stat sb;

  rndcycle = 0;
  location = 1;   /* Great Hall */

  mywield = 0;   /* not initially wearing or weilding any weapon */
  mywear = 0;
  myhealth = 7;   /* how healthy they are to start */
  healthcycle = 0;   /* pretty much meaningless at the start */

  lowcase(userid, get_userid(STR2));
  if (!strcmp(userid, MM_userid)) {
    strcpy(myname, "Monster Manager");
    privd = true;
  } else if (!strcmp(userid, MVM_userid)) {
    privd = true;
    strcpy(myname, "Vice Manager");
  } else if (!strcmp(userid, FAUST_userid)) {
    privd = true;
    strcpy(myname, "Faust");
  } else {
    lowcase(myname, userid);
    myname[0] = _toupper(myname[0]);
    privd = false;
  }

  numcmds = 66;

  strcpy(show[s_exits-1], "exits");
  strcpy(show[s_object-1], "object");
  strcpy(show[s_quest-1], "?");
  strcpy(show[s_details-1], "details");
  numshow = 4;

  strcpy(setkey[y_quest-1], "?");
  strcpy(setkey[y_altmsg-1], "altmsg");
  strcpy(setkey[y_group1-1], "group1");
  strcpy(setkey[y_group2-1], "group2");
  numset = 4;

  numspells = 0;

  sprintf(STR1, "%s/rooms.mon", root);
  roomfile = open(STR1, O_RDWR|O_CREAT, MONSTER_FILE);
  if (roomfile < 0) {
	perror("can't open roomfile");
	exit(0);
  }

  sprintf(STR1, "%s/nams.mon", root);
  namfile = open(STR1, O_RDWR|O_CREAT, MONSTER_FILE);
  if (namfile < 0) {
	perror("can't open namfile");
	exit(0);
  }

  sprintf(STR3, "%s/events.mon", root);
  eventfile = open(STR3, O_RDWR|O_CREAT, MONSTER_FILE);
  if (eventfile < 0) {
	perror("can't open eventfile");
	exit(0);
  }

  sprintf(STR1, "%s/desc.mon", root);
  descfile = open(STR1, O_RDWR|O_CREAT, MONSTER_FILE);
  if (descfile < 0) {
	perror("can't open descfile");
	exit(0);
  }

  sprintf(STR1, "%s/index.mon", root);
  indexfile = open(STR1, O_RDWR|O_CREAT, MONSTER_FILE);
  if (indexfile < 0) {
	perror("can't open indexfile");
	exit(0);
  }

  sprintf(STR1, "%s/line.mon", root);
  linefile = open(STR1, O_RDWR|O_CREAT, MONSTER_FILE);
  if (linefile < 0) {
	perror("can't open linefile");
	exit(0);
  }

  sprintf(STR3, "%s/intfile.mon", root);
  intfile = open(STR3, O_RDWR|O_CREAT, MONSTER_FILE);
  if (intfile < 0) {
	perror("can't open intfile");
	exit(0);
  }

  sprintf(STR3, "%s/objects.mon", root);
  objfile = open(STR3, O_RDWR|O_CREAT, MONSTER_FILE);
  if (objfile < 0) {
	perror("can't open objfile");
	exit(0);
  }

  sprintf(STR3, "%s/spells.mon", root);
  spellfile = open(STR3, O_RDWR|O_CREAT, MONSTER_FILE);
  if (spellfile < 0) {
	perror("can't open spellfile");
	exit(0);
  }

	if (fstat(roomfile, &sb) == 0) {
		if (sb.st_size == 0) {
			rebuild_system();
		}
	}
}


void prestart()
{
  string s, STR1;
  char *TEMP;

  mprintf("Welcome to Monster!  Hit return to start: ");
  fgets(s, 81, stdin);
  TEMP = strchr(s, '\n');
  if (TEMP != NULL)
    *TEMP = 0;
  mprintf("\n\n");
  if (*s != '\0')
    special(lowcase(STR1, s));
}


void welcome_back(mylog)
int *mylog;
{
  string tmp;
  shortstring sdate, stime;

  getdate();
  freedate();

  mprintf("Welcome back, %s.", myname);
  if (strlen(myname) > 18)
    putchar('\n');

  mprintf("  Your last play was on");

  if (strlen(adate.idents[*mylog - 1]) < 11)
    mprintf(" ???\n");
  else {
    sprintf(sdate, "%.11s", adate.idents[*mylog - 1]);
	/* extract the date */
    if (strlen(adate.idents[*mylog - 1]) == 19)
      sprintf(stime, "%.7s", adate.idents[*mylog - 1] + 12);
    else
#if 0
      strcpy(stime, "???");
#else
      sprintf(stime, "%s", adate.idents[*mylog - 1] + 12);
#endif

    if (sdate[0] == ' ')
      strcpy(tmp, sdate);
    else
      sprintf(tmp, " %s", sdate);

    if (stime[0] == ' ')
      sprintf(tmp + strlen(tmp), " at%s", stime);
    else
      sprintf(tmp + strlen(tmp), " at %s", stime);
    mprintf("%s.\n", tmp);
  }
  putchar('\n');
}


boolean loc_ping()
{
  int i = 1;
  boolean found = false;

  inmem = false;
  gethere(0);


  /* first get the slot that the supposed "zombie" is in */
  while (!found && i <= maxpeople) {
    if (!strcmp(here.people[i-1].name, myname))
      found = true;
    else
      i++;
  }

  myslot = 0;   /* setup for ping_player */

  if (found) {
    setevent();
    return (ping_player(i, true));   /* TRUE = silent operation */
  } else {
    return true;
    /* well, if we can't find them, let's assume
       that they're not in any room records, so they're
       ok . . . Let's hope... */
  }
}



/* attempt to fix the player using loc_ping if the database incorrectly
   shows someone playing who isn' playing */

boolean fix_player()
{
  mprintf("There may have been some trouble the last time you played.\n");
  mprintf("Trying to fix it . . .\n");
  if (loc_ping()) {
    mprintf("All should be fixed now.\n\n");
    return true;
  } else {
    mprintf(
      "Either someone else is playing Monster on your account, or something is\n");
    mprintf("very wrong with the database.\n\n");
    return false;
  }
}


boolean revive_player(mylog)
int *mylog;
{
  boolean ok;
  int i;
  string STR2, STR3;

  if (exact_user(mylog, userid)) {  /* player has played before */
    getint(N_LOCATION);
    freeint();
    location = anint.int_[*mylog - 1];   /* Retrieve their old loc */

    getpers();
    freepers();
    strcpy(myname, pers.idents[*mylog - 1]);
	/* Retrieve old personal name */

    getint(N_EXPERIENCE);
    freeint();
    myexperience = anint.int_[*mylog - 1];

    getint(N_SELF);
    freeint();
    myself = anint.int_[*mylog - 1];

    getindex(I_ASLEEP);
    freeindex();

    if (indx.free[*mylog - 1]) {
      /* if player is asleep, all is well */
      ok = true;
    } else {
      /* otherwise, there is one of two possibilities:
              1) someone on the same account is
                 playing Monster
              2) his last play terminated abnormally
      */
      ok = fix_player();
    }

    if (ok)
      welcome_back(mylog);

  } else {
    if (alloc_log(mylog)) {
      mprintf("Welcome to Monster, %s!\n", myname);
      mprintf("You will start in the Great Hall.\n\n");

      /* Store their userid */
      getuser();
      lowcase(user.idents[*mylog - 1], userid);
      putuser();

      /* Set their initial location */
      getint(N_LOCATION);
      anint.int_[*mylog - 1] = 1;   /* Start out in Great Hall */
      putint();
      location = 1;

      getint(N_EXPERIENCE);
      anint.int_[*mylog - 1] = 0;
      putint();
      myexperience = 0;

      getint(N_SELF);
      anint.int_[*mylog - 1] = 0;
      putint();
      myself = 0;

      /* initialize the record containing the
         level of each spell they have to start;
         all start at zero; since the spellfile is
         directly parallel with mylog, we can hack
         init it here without dealing with SYSTEM */

      lseek(spellfile, (*mylog - 1L) * sizeof(spellrec), 0);
      bzero(&spellfile_hat, sizeof(spellfile_hat));
      for (i = 0; i < maxspells; i++)
	spellfile_hat.level[i] = 0;
      spellfile_hat.recnum = *mylog;
      write(spellfile, &spellfile_hat, sizeof(spellfile_hat));

      ok = true;
    }

    else
      ok = false;
  }
  /* must allocate a log block for the player */

  if (!ok) {  /* Successful, MYLOG is my log slot */
    mprintf("There is no place for you in Monster.  Contact the Monster Manager.\n");
    return false;
  }

  /* Wake up the player */
  getindex(I_ASLEEP);
  indx.free[*mylog - 1] = false;   /* I'm NOT asleep now */
  putindex();

  /* Set the "last date of play" */
  getdate();
  sprintf(adate.idents[*mylog - 1], "%s %s", sysdate(STR2), systime(STR3));
  putdate();
  return true;
}


boolean enter_universe()
{
  boolean Result = true;
  string orignam;
  int dummy;
  int i = 0;
  boolean ok;
  string STR1, STR2;



  /* take MYNAME given to us by init or revive_player and make
     sure it's unique.  If it isn't tack _1, _2, etc onto it
     until it is.  Code must come before alloc_log, or there
     will be an invalid pers record in there cause we aren't in yet
   */
  strcpy(orignam, myname);
  do {   /* tack _n onto pers name until a unique one is found */
    ok = true;

    /**** Should this use exact_pers instead?  Is this a copy of exact_pers code? */

    if (lookup_pers(&dummy, myname)) {
      if (!strcmp(lowcase(STR1, pers.idents[dummy-1]), lowcase(STR2, myname))) {
	ok = false;
	i++;
	sprintf(myname, "%s_%d", orignam, i);
      }
    }
  } while (!ok);



  if (!revive_player(&mylog)) {
    mprintf("revive_player failed.\n");
    return false;
  }
  if (!put_token(location, &myslot, 0)) {
    mprintf("put_token failed.\n");
    return false;
  }
  getpers();
  strcpy(pers.idents[mylog-1], myname);
  putpers();

  log_begin(location);
  setevent();
  do_look("");
  return true;
}


void leave_universe()
{
  boolean diddrop;

  diddrop = drop_everything(0);
  take_token(myslot, location);
  log_quit(location, diddrop);
  do_endplay(mylog, false);

  mprintf("You vanish in a brilliant burst of multicolored light.\n");
  if (diddrop)
    mprintf("All of your belongings drop to the ground.\n");
}


main(argc, argv)
int argc;
char *argv[];
{

  setbuf(stdout, 0);
  setbuf(stderr, 0);
  srandom(time(0));
  bzero(locked, sizeof(locked));
  *old_prompt = '\0';

  spellfile = -1;
  objfile = -1;
  intfile = -1;
  indexfile = -1;
  linefile = -1;
  descfile = -1;
  namfile = -1;
  eventfile = -1;
  roomfile = -1;
  done = false;
  init();
  prestart();
  setup_guts();
  if (!done) {
    if (enter_universe()) {
      do {
	parser();
	doawait(LONG_WAIT);
      } while (!done);
      leave_universe();
	doawait(LONG_WAIT);
    } else
      mprintf(
	"You attempt to enter the Monster universe, but a strange force repels you.\n");
  }
  finish_guts();
  if (roomfile >= 0)
    close(roomfile);
  if (eventfile >= 0)
    close(eventfile);
  if (namfile >= 0)
    close(namfile);
  if (descfile >= 0)
    close(descfile);
  if (linefile >= 0)
    close(linefile);
  if (indexfile >= 0)
    close(indexfile);
  if (intfile >= 0)
    close(intfile);
  if (objfile >= 0)
    close(objfile);
  if (spellfile >= 0)
    close(spellfile);
  exit(0);
}


/* Notes to other who may inherit this program:

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
form:getX(), freeX and putX.

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


*/



/* End. */