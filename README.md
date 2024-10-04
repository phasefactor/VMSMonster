# VMS Monster[^1]
Monster was written in Pascal on a VMS 4.6 machine at the Northwestern University by Richard Skrenta.  Monster's source and documentation were released on USENET and influenced the creation of TinyMUD (the progenitor of many MU* codebases).

The Monster source code, documentation, and some related files were hosted on Richard Skrenta's website at `http://www.skrenta.com/monster/`, but now the domain redirects to his LinkedIn.

This repo is an archive of what was originally posted on his website.


## Richard Skrenta's Monster Website

> Trudging home from the Libes late one night, you notice a figure darting furtively through the shadows, obviously trying not to be seen, yet your eyes seemed to have been alerted to his presence, as if by instinct. There's something disturbingly peculiar about him, so you decide to follow him to see what he's doing on campus. He flashes across Sheridan road onto Emerson, then takes a quick cut to the north. Again your gaze seems to be riveted to his form, so much so that you are hardly aware of threading your way in and out of the Plex's courtyards, then back across Emerson again. Why does this guy keep doubling back on himself? Does he know you are following?? Faster and faster the two of you race; now you're trying not to be seen as well, either by him or anyone else who would be wondering what sort of activities you had been engaged in while out on the lakefill that would cause you to be running about like a decapitated chicken.
> 
> You've just about caught up with him, when suddenly he rounds a corner and stops in front of a large doorway. Is it Willard? You stay behind some bushes so that he won't notice you, and you observe him to be fumbling in his backpack. A couple of things fall out as he yanks his hand out, clutching a small, tattered slip of paper. The objects look like a large gold key and -- is that a crystal ball?! Hastily, he scoops them back into his pack, glances at the slip of paper, mumbles something -- and vanishes in a burst of brilliant, multicolored light!
> 
> Now you wish you'd have gone straight home: obviously you need some rest after concentrating so hard under those harsh fluorescent lights in Reserve. But wait, you don't remember seeing this part of the sorority quads before. That doorway is NOT Willard's. Then you notice that he dropped that slip of paper. You decide to pick it up, since you don't know if you can find your way back here in the daylight. For that matter, you don't know if you can find your way HOME. You look at the paper, trying to decipher the scrawled words...

```
      RUN USERD:[CON00907.DOLPHER]MONSTER
```

> What the --? Guess there's only one way to find out...

- A [paper](./website/final.html) about Monster
- Original source package:
  - [readme.txt](./website/readme.txt)
  - [install.txt](./website/install.txt)
  - [monster.txt](./website/monster.txt)
  - [mon.pas](./website/mon.pas)
  - [guts.pas](./website/guts.pas)
  - [privusers.pas](./website/privusers.pas)
- [A port of VMS Monster to C](./website/monster.c)
- [Description of Monster Helsinki](./website/monster_helsinki.txt), an expanded descendant
  - [monster_helsinki.tgz](./website/monster_helsinki.tgz)
- A [game log](./website/sunyab-monster.txt) from University of Buffalo's monster descendant, by Brent LaVelle, Rob Rothkopf and Mark Cromwell

## Note on the License
The license selected for this repository is based on the the following, from `readme.txt`:

```
You may freely copy, distribute and change Monster as you wish.
```

As far as I can see, no further requirements are stated anywhere in the codebase.  Although the author says that he would love to hear from anyone who gets the code running.

## Bonus Files
- An [alternate source](./bonus/monster.tar.gz) for the original Monster code.
  
  Found in the [MudBytes.net](https://www.mudbytes.net/files/766) file repository; contains the same license in the README.
- A copy of the [University of Buffalo's Monster](./bonus/ub_vms_monster.zip) descendant mentioned by Skrenta.
  
  Found on [Matt McGlincy's GitHub](https://github.com/mcglincy/ub_vms_monster); contains no license information.
  
  



[^1]: [Fandom - MUD Wiki --- Monster (game)](https://mud.fandom.com/wiki/Monster_(game))
