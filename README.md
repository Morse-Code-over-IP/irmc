irmc - Internet Relay Morse Code
================================

IRMC stands for Internet Relay Morse Code and is an implementation of [MOIP](https://github.com/8cH9azbsFifZ/moip).
It implements the [CWCom protocol](http://kob.sdf.org/morsekob/docs/cwcom.pdf) 
as adopted by [MorseKOB](http://kob.sdf.org/morsekob/docs/history.pdf). 
You can try out the software in a [browser](http://kob.sdf.org/morsekob/morsekob30/index.htm) using Java.

# How to build?
## Install dependency: morse keyer library
```
wget https://github.com/8cH9azbsFifZ/morse/archive/v0.1.tar.gz
tar xzf v0.1.tar.gz
cd morse-0.1
libtoolize
./autogen.sh
./configure  --with-portaudio
make
sudo make install
```

## Debian (Wheezy)
Some dependencies have to be installed:
```
apt-get install -y alsa-oss oss-compat build-essential autoconf libao-dev libtool
```
Afterwards compilation with `make` should work. If something went wrong, you may have
to adjust your `LD_LIBRARY_PATH`. Alternatively try:
```
LD_LIBRARY_PATH=/usr/local/lib ./irmc mtc-kob.dyndns.org 7890 33 123 
```

## OSX (Yosemite)
Compilation with make :)

For the USB serial devices you need a PL2303 driver 
(i.e. [PL2303_Serial-USB_on_OSX_Lion.pkg](http://changux.co/osx-installer-to-pl2303-serial-usb-on-osx-lio/)).

## Testing
./irmc morsecode.dyndns.org 7890 2348 test

Or you may want to use tcpdump, i.e.:
```
sudo tcpdump -i all -vvvv "host faeroes.sdf.org"
```


# How to use:

The usage is: `irmc [hostname] [port] [channel] [id] [serialport`

For example:
`./irmc mtc-kob.dyndns.org 7890 103 MyID /dev/tty.usbserial´

                                                      
## Hardware interface options
A good description on how to build differnt interfaces (telegraph key, sounder or both) 
is given on the [MorseKOB Website](http://kob.sdf.org/morsekob/interface.htm). 
Landline telegraphs use "closed circuits" for communications; if you have built one at home, 
you may also use the [loop interface](http://kob.sdf.org/morsekob/docs/loopinterface.pdf).

Connection of a morse key:
Serial PIN: 4 & 6
[layout of pins](http://techpubs.sgi.com/library/dynaweb_docs/0650/SGI_Admin/books/MUX_IG/sgi_html/figures/4-2.serial.port.con.gif)
Connecting the palm radio: keep an eye on the grounding :)

# Changelog
* v0.2 [zip](https://github.com/8cH9azbsFifZ/irmc/archive/v0.2.zip) - ported to debian wheezy and osx yosemite, DG6FL
* v0.1 [zip](https://github.com/8cH9azbsFifZ/irmc/archive/v0.1.zip) - original version, VE7FEB

# References
Original [post](http://fernski.blogspot.de/2013/03/internet-relay-morsecode.html) by Fernan Bolando (VE4FEB)

> There are few difference between this and the official cwcom client
> 1. It is a command line tool 
> 2. It does not send the characters of the message to the receivers screen.
> 3. It does not translate CW for you. You can try fldigi if you just want see the transmission.
> 4. Tone pitch is currently hard coded to 650Hz
> 5. It does not have a way to show you who is listening.  


Code Quality
============
This is experimental code.


