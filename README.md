# What is IRMC?

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

### Serial Drivers
TBD: Driver: serial - 2usb!


# How to use:

The usage is: `irmc [hostname] [port] [channel] [id] [serialport`

For example:
`./irmc mtc-kob.dyndns.org 7890 103 MyID /dev/tty.usbserial´

## Morse KOB Servers
* faeroes.sdf.org 7890
* mtc-kob.dyndns.org 7890


# References

## Original post
Copied on 20150103 from http://fernski.blogspot.de/2013/03/internet-relay-morsecode.html

> I have been using cwcom to practice sending morsecode, unfortunately my main computer at home is running openbsd and it has gotten harder to gain access to a reliable MS Windows machine. So I wrote my own client that works on openbsd. 
> This is written in C and although not tested it should compile under other OS. If any one wants to try it and send me feedback. you can download version 0.01 here http://fernan.bitbucket.org/irmc.tgz
> Les Kerr of https://home.comcast.net/~morsekob/ has been very patient in answering my questions regarding the cwcom protocol and setting up a test server during debugging.
>
> There are few difference between this and the official cwcom client
> 1. It is a command line tool 
> 2. It does not send the characters of the message to the receivers screen.
> 3. It does not translate CW for you. You can try fldigi if you just want see the transmission.
> 4. Tone pitch is currently hard coded to 650Hz
> 5. It does not have a way to show you who is listening.  
> 
> Bugs:
> There are a few that I am working on right now, but feel free to send me feedback if you get a chance to try it out.
> 
> File(s)
> http://fernan.bitbucket.org/irmc.tgz
> The following people have helped me a lot to learn the protocols and setting up test servers.
> Les Kerr -  https://home.comcast.net/~morsekob/
> Bob Denny - http://morse-rss-news.sourceforge.net/
> John Samin - http://www.mrx.com.au/



## Resources


