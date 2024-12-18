# Tascam US-16x08 DSP mixer

This application is based on GTK+ and relies on [US16x08 support](https://github.com/torvalds/linux/blob/master/sound/usb/mixer_us16x08.c) in the Linux snd-usb-audio driver, available in Linux 4.11 and newer. 

![screenshot.png](/screenshot.png?raw=true)

As I bought my Tascam audio interface I was surprised, that the manufacturer do not support linux. 
After some investigation of USB traffic with wireshark, I could determine the needs of an alsa based mixer quirk. The mentioned kernel driver is the result.

Because this device contains about 280 control elements, working with traditional mixer applications like alsammixer or gnome-alsamixer was no option.

So I started to develop my own mixer application to get comfortable access to the DSP effects the device has build in.

Another option is to use the LV2 plugin (https://github.com/onkelDead/tascam.lv2) I have developed, that give me control to the EQ and compressor from any LV2 capable DAW software (in my case Ardour). Please keep in mind, this LV2 plugin do not operate on the stream ports, it's intention is to control hardware effects of the Tascam interface via USB.

## Building

In order to build tascamgtk, you need to have essential GNU build tools such as:

- autoconf
- automake
- autopoint
- make
- g++

And the libraries and header files which tascamgtk depends on:

- libgtkmm-3.0
- libxml++ (2.6 to 5.0 are possible)
- optionnaly, liblo

Basically, on a Debian-based Linux system, requirements should be satified with:
```
apt install build-essential autoconf automake autopoint libgtkmm-3.0-dev libxml++2.6-dev liblo-dev
```

On a Arch-based Linux system, the packages to install should be `base-devel autoconf automake gtkmm3 libxml++-5.0 liblo`

Once those prerequisites are installed, run:
```
autoreconf -fiv
./configure
make
```

Note that you can also choose a different install prefix other than `/usr/local`, and you also can disable OSC support. See `./configure --help` to list all options.

To install tascamgtk, run `make install` as root user.
