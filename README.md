irmc - Internet Relay Morse Code
================================
IRMC stands for Internet Relay Morse Code and is an implementation of [MOIP](http://8ch9azbsfifz.github.io/moip/).

# Hardware interface options
A good description on how to build different interfaces (telegraph key, sounder or both) 
is given on the [MorseKOB Website](http://kob.sdf.org/morsekob/interface.htm). 
Landline telegraphs use "closed circuits" for communications; if you have built one at home, 
you may also use the [loop interface](http://kob.sdf.org/morsekob/docs/loopinterface.pdf).

Connection of a morse key:
[layout of pins](http://techpubs.sgi.com/library/dynaweb_docs/0650/SGI_Admin/books/MUX_IG/sgi_html/figures/4-2.serial.port.con.gif)

|  RS232    | DB9    | Function |   
| :-------- |:-------| :------  | 
|  DTR      | 4      | Manual Key / paddle common|
|  DSR      | 6      | Manual key / dot paddle|
|  CTS      | 8      | Dash paddle|
|  RTS      | 7      | Sounder output|
|  SG       | 5      | Sounder ground|


# Changelog
* v0.3 [zip](https://github.com/8cH9azbsFifZ/irmc/archive/v0.3.zip) - commandline option cleanup
* v0.2 [zip](https://github.com/8cH9azbsFifZ/irmc/archive/v0.2.zip) - ported to debian wheezy and osx yosemite, DG6FL
* v0.1 [zip](https://github.com/8cH9azbsFifZ/irmc/archive/v0.1.zip) - original version, VE7FEB

Code Quality
============
This is experimental code.
