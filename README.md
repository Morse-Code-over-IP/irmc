How to build on Debian
apt-get install -y alsa-oss oss-compat build-essential autoconf libao-dev libtool
wget https://github.com/8cH9azbsFifZ/morse/archive/v0.1.tar.gz
tar xzf v0.1.tar.gz
cd morse-0.1
libtoolize
./autogen.sh
./configure  --with-portaudio
make
sudo make install


How to build on OSX:
TBD: Driver: serial - 2usb!

TBD: OSX - howto install dependencies
https://github.com/8cH9azbsFifZ/morse/archive/v0.1.tar.gz


How to use:

 1 usage: irmc [hostname] [port] [channel] [id] [serialport]
./irmc mtc-kob.dyndns.org 7890 103 123 /dev/tty.usbserial 
faeroes.sdf.org.7890


=== Original post ===
I have been using cwcom to practice sending morsecode, unfortunately my main computer at home is running openbsd and it has gotten harder to gain access to a reliable MS Windows machine. So I wrote my own client that works on openbsd. 
This is written in C and although not tested it should compile under other OS. If any one wants to try it and send me feedback. you can download version 0.01 here http://fernan.bitbucket.org/irmc.tgz
Les Kerr of https://home.comcast.net/~morsekob/ has been very patient in answering my questions regarding the cwcom protocol and setting up a test server during debugging.

There are few difference between this and the official cwcom client
1. It is a command line tool 
2. It does not send the characters of the message to the receivers screen.
3. It does not translate CW for you. You can try fldigi if you just want see the transmission.
4. Tone pitch is currently hard coded to 650Hz
5. It does not have a way to show you who is listening.  

Bugs
There are a few that I am working on right now, but feel free to send me feedback if you get a chance to try it out.

File(s)
http://fernan.bitbucket.org/irmc.tgz


Resources
The following people have helped me a lot to learn the protocols and setting up test servers.
Les Kerr -  https://home.comcast.net/~morsekob/
Bob Denny - http://morse-rss-news.sourceforge.net/
John Samin - http://www.mrx.com.au/

=== Source: http://fernski.blogspot.de/2013/03/internet-relay-morsecode.html ===
=== End Original Post ===

