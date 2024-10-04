[inherit ('sys$library:starlet')]
 
module guts(input,output);
 
const
        SHORT_WAIT = 0.1;
        LONG_WAIT = 0.2;
        maxcycle = 15;          { attempting to fine tune nextkey }
 
type
        $uword = [word] 0 .. 65535;
        string = varying[80] of char;
        ident = packed array[1..12] of char;
 
        iosb_type = record
                cond: $uword;
                trans: $uword;
                junk: unsigned; {longword}
        end;
 
        il3 = record
             buflen : $uword;
             itm    : $uword;
             baddr  : unsigned;
             laddr  : unsigned;
        end;
 
 
var
        pbrd_id:        unsigned;       { pasteboard id    }
        kbrd_id:        unsigned;
        pbrd_volatile:  [volatile] unsigned;
        mbx:            [volatile] packed array[1..132] of char;
        mbx_in,                                 { channels for input and   }
        mbx_out:        [volatile] $uword;      { output to the subprocess }
        pid:            integer;
        iosb:           [volatile] iosb_type;   { i/o status block         }
        status:         [volatile] unsigned;
        save_dcl_ctrl:  unsigned;
        trap_flag:      [global,volatile] boolean;
        trap_msg:       [global,volatile] string;
        out_chan:       $uword;
        vaxid:          [global] packed array[1..12] of char;
        advise:         [external] string;
        line:           [global] string;
        old_prompt: [global] string;
 
        seed: integer;
 
        user,uname:varying[31] of char;
        sts:integer;
        il:array[1..2] of il3;
        key:$uword;
 
        userident: [global] ident;
 
 
[asynchronous, external (lib$signal)]
function lib$signal (
   %ref status : [unsafe] unsigned) : unsigned; external;
 
[asynchronous, external (str$trim)]
function str$trim (
   destination_string : [class_s] packed array [$l1..$u1:integer] of char;
   source_string : [class_s] packed array [$l2..$u2:integer] of char;
   %ref resultant_length : $uword) : unsigned; external;
 
[asynchronous, external (smg$read_keystroke)]
function smg$read_keystroke (
    %ref keyboard_id : unsigned;
    %ref word_integer_terminator_code : $uword;
    prompt_string : [class_s] packed array [$l3..$u3:integer] of char := %immed
0;
    %ref timeout : unsigned := %immed 0;
    %ref display_id : unsigned := %immed 0;
    %ref rendition_set : unsigned := %immed 0;
    %ref rendition_complement : unsigned := %immed 0) : unsigned; external;
 
[asynchronous, external (smg$create_virtual_keyboard)]
function smg$create_virtual_keyboard (
    %ref keyboard_id : unsigned;
    filespec : [class_s] packed array [$l2..$u2:integer] of char := %immed 0;
    default_filespec : [class_s] packed array [$l3..$u3:integer] of char := %immed 0;
    resultant_filespec : [class_s]
    packed array [$l4..$u4:integer] of char := %immed 0) : unsigned; external;
 
[asynchronous, external (lib$disable_ctrl)]
function lib$disable_ctrl (
    %ref disable_mask : unsigned;
    %ref old_mask : unsigned := %immed 0) : unsigned; external;
 
[asynchronous, external (lib$enable_ctrl)]
function lib$enable_ctrl (
    %ref enable_mask : unsigned;
    %ref old_mask : unsigned := %immed 0) : unsigned; external;
 
 
procedure syscall( s: [unsafe] unsigned );
 
begin
   if not odd( s ) then begin
      lib$signal( s );
   end;
end;
 
 
[external,asynchronous]
function mth$random(var seed: integer): real;
external;
 
[global]
function random: real;
 
begin
        random := mth$random(seed);
end;
 
[global]
function rnd100: integer;       { random int between 0 & 100, maybe }
 
begin
        rnd100 := round(mth$random(seed)*100);
end;
 
 
[external] function lib$wait(seconds:[reference] real):integer;
external;
 
[global]
procedure wait(seconds: real);
 
begin
        syscall( lib$wait(seconds) );
end;
 
[external] procedure checkevents(silent: boolean := false);
extern;
 
 
[global]
procedure doawait(time: real);
 
begin
        syscall( lib$wait(time) );
end;
 
 
[global]
function trim(s: string): string;
var
        tmp: [static] string := '';
 
begin
        syscall( str$trim(tmp.body,s,tmp.length) );
        trim := tmp;
end;
 
 
[global]
function get_userid: string;
 
begin
  il:=zero;
  il[1].itm    := jpi$_username;
  il[1].buflen := size(user.body);
  il[1].baddr  := iaddress(user.body);
  il[1].laddr  := iaddress(user.length);
  syscall($getjpiw(,,,il));
  syscall( str$trim(uname.body,user,uname.length) );
  userident := user;
  get_userid := uname;
end;
 
 
[global]
procedure putchars(s: string);
var
        msg: packed array[1..128] of char;
        len: integer;
 
begin
        msg := s;
        len := length(s);
        syscall($qiow(,out_chan,io$_writevblk,,,,msg,len,,,,));
end;
 
[global]
function keyget: char;
var
        term: [static] $uword := 0;
        i: integer;
 
begin
        if (smg$read_keystroke(kbrd_id,term,,0,,,) mod 2) = 0 then
                keyget := chr(0)
        else begin
                i := term;
                keyget := chr(i);
        end;
end;
 
[global] function firstkey:char;
var
        ch: char;
 
begin
        ch := keyget;
        if ch = chr(0) then begin
                repeat
                        checkevents;
                        doawait(LONG_WAIT);
                        ch := keyget;
                until ch <> chr(0);
        end;
        firstkey := ch;
end;
 
[global] function nextkey:char;
var
        ch: char;
        cycle: integer;
 
begin
        cycle := 0;
        ch := keyget;
        if ch = chr(0) then begin
                repeat
                        if cycle = maxcycle then begin
                                checkevents;
                                doawait(SHORT_WAIT);
                                cycle := 0;
                        end;
                        cycle := cycle + 1;
 
        { should doawait(SHORT_WAIT) be in the cycle check, or should
          it be only in the repeat loop (done every repeat) ?
          When the system gets slow, character input gets VERY slow.
          Is this because of the checkevents, the wait, or is it
          just a symptom of a slow VAX? }
 
                        ch := keyget;
                until ch <> chr(0);
        end;
        nextkey := ch;
end;
 
 
[global]
procedure grab_line(prompt:string; var s:string;echo:boolean := true);
var
        ch: char;
        pos: integer;
        i: integer;
 
begin
        old_prompt := prompt;
        putchars(chr(10)+prompt);
        line := '';
        ch := firstkey;
        while (ch <> chr(13)) do begin
{ del char }    if (ch = chr(8)) or (ch = chr(127)) then begin
                        case length(line) of
                                0: ch := firstkey;
                                1:begin
                                        line := '';
                                        if echo then
                                                putchars(chr(8)+' '+chr(8));
                                        ch := nextkey;
                                  end;
                                2:begin
                                        line := line[1];
                                        if echo then
                                                putchars(chr(8)+' '+chr(8));
                                        ch := nextkey;
                                  end;
                                otherwise begin
                                        line := substr(line,1,length(line)-1);
                                        if echo then
                                                putchars(chr(8)+' '+chr(8));
                                        ch := nextkey;
                                end;
                        end;    { CASE }
{ cancel line } end { if } else if ch = chr(21) then begin
                        putchars(chr(13)+chr(27)+'[K');
{                       pos := length(prompt) + length(line);
                        if pos > 0 then
                                for i := 1 to pos do
                                        putchars(chr(8)+' '+chr(8));    }
                        putchars(prompt);
                        line := '';
                        ch := firstkey;
{line too long} end else if length(line) > 76 then begin
                        putchars(chr(7));
                        ch := nextkey;
{ no ctrls }    end else if ord(ch) > 31 then begin
                        line := line + ch;
                        if echo then
                                putchars(ch);
                        ch := nextkey;
                end else
                        ch := nextkey;
{ ***           ch := nextkey;           *** }
        end;    { WHILE }
        pos := length(prompt) + length(line);
 
{       if pos > 0 then
                for i := 1 to pos do
                        putchars(chr(8));       }
 
        putchars(chr(13));
 
        s := line;
end;
 
 
[global]
procedure setup_guts;
var
        border: unsigned;
        rows,cols: integer;
        mask: unsigned;
 
begin
        seed := clock;
        old_prompt := '';
 
{   border := 1;
   smg$create_pasteboard(pbrd_id,,rows,cols,border);    }
 
   smg$create_virtual_keyboard(kbrd_id);
   pbrd_volatile := pbrd_id;
 
   syscall($assign('SYS$OUTPUT',out_chan));
 
   mask := %X'02000000';        { CTRL/Y  Just for DCL }
{ mask := ...21... for ctrl-t too }
   syscall( lib$disable_ctrl( mask, save_dcl_ctrl ));
 
   mask := %X'02000008';        { nuke CTRL/Y & CTRL/C completely }
{   syscall( smg$set_out_of_band_asts(pbrd_id,mask,trapper)); }
end;
 
[global]
procedure finish_guts;
 
begin
        syscall( lib$enable_ctrl(save_dcl_ctrl));       { re-enable dcl ctrls }
end;
 
end.