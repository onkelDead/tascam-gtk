includedir = $(prefix)/include
datadir = $(prefix)/share/tascamgtk-4.0/data
desktopdir       = $(prefix)/share/applications
pixmapsdir       = $(prefix)/share/pixmaps
tascamgtkdir = src
bin_PROGRAMS = tascamgtk
tascamgtk_SOURCES = src/main.cpp  src/OAlsa.cpp  src/OComp.cpp  src/ctrl/ODial.cpp  src/OEq.cpp src/OStrip.cpp \
    src/OMainWnd.cpp  src/OMaster.cpp  src/ORouting.cpp  src/ctrl/OMeter.cpp  src/OMeterWorker.cpp src/ctrl/ORoute.cpp \
    src/OStripLayout.cpp src/ODspLayout.cpp src/OConfig.cpp src/OOscDialog.cpp src/ctrl/OSwitch.cpp src/ctrl/OFader.cpp src/ctrl/OOscControl.cpp
noinst_HEADERS = src/OMainWnd.h src/OTypes.h src/OComp.h src/OMaster.h src/OEq.h src/OStrip.h src/OAlsa.h \
    src/OMeterWorker.h src/OStripLayout.h src/ctrl/ODial.h src/ctrl/OMeter.h src/ODspLayout.h src/ORouting.h src/OConfig.h \
    src/OOscDialog.h src/ctrl/OSwitch.h src/ctrl/OFader.h src/ctrl/ORoute.h

data_DATA = data/tascam-gtk.css data/icon.png
dist_data_DATA = data/tascam-gtk.css data/icon.png

desktop_DATA = data/tascamgtk.desktop
pixmaps_DATA     = data/tascamgtk.png

tascamgtk_CPPFLAGS = ${CXXFLAGS} -std=c++17 -Isrc/ctrl -Isrc ${GTK3_CFLAGS} ${GTKMM3_CFLAGS} ${ALSA_CFLAGS} ${XML3_CFLAGS} ${XML26_CFLAGS} ${LO_CFLAGS} -DPKGDATADIR='"$(datadir)"'
tascamgtk_LDADD = ${GTK2_LIBS} ${GTKMM3_LIBS} ${ALSA_LIBS} ${XML3_LIBS} ${XML26_LIBS} ${CONFIG_LIBS} -lpthread

EXTRA_DIST = $(gsettings_SCHEMAS) \
	     $(pixmaps_DATA) \
	     $(desktop_DATA)

if HAVE_OSC
   tascamgtk_CPPFLAGS += ${LO_CFLAGS}
   tascamgtk_LDADD +=  ${LO_LIBS}
endif
