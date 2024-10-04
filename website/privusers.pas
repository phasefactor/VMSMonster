{ These are PRIVILEDGED users.  The Monster Manager has the most power;
  this should be the game administrator.  The Monster Vice Manager can help
  the MM in his adminstrative duties.  Faust is another person who can do
  anything but is generally incognito. }

MM_userid	= 'dolpher';		{ Monster Manager	}
MVM_userid	= 'gary';		{ Monster Vice Manager	}
FAUST_userid	= 'skrenta';		{ Dr. Faustus		}

REBUILD_OK	= TRUE;		{ if this is TRUE, the MM can blow away
				  and reformat the entire universe.  It's
				  a good idea to set this to FALSE and
				  recompile after you've got your world
				  going }


root		= 'USERC:[ISP00475.CRA01453.DSYS]';
				{ this is where the Monster database goes
				  This directory and the datafiles Monster
				  creates in it must be world:rw for
				  people to be able to play.  This sucks,
				  but we don't have setgid to games on VMS }
