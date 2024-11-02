includedir = $(prefix)/include
datadir = $(prefix)/share/tascamgtk-4.0/data
desktopdir       = $(prefix)/share/applications
pixmapsdir       = $(prefix)/share/pixmaps
tascamgtkdir = src
bin_PROGRAMS = tascamgtk
tascamgtk_SOURCES = src/main.cpp  src/OAlsa.cpp  src/OComp.cpp  src/ODial.cpp  src/OEq.cpp src/OFader.cpp src/OMainWnd.cpp  src/OMaster.cpp  src/ORoute.cpp  src/OMeter.cpp  src/OMeterWorker.cpp  src/OStripLayout.cpp src/ODspLayout.cpp
noinst_HEADERS = src/OMainWnd.h src/OTypes.h src/OComp.h src/OMaster.h src/OEq.h src/OFader.h src/OAlsa.h src/OMeterWorker.h src/OStripLayout.h src/ODial.h src/OMeter.h src/ODspLayout.h src/ORoute.h

data_DATA = data/tascam-gtk.css data/icon.png
dist_data_DATA = data/tascam-gtk.css data/icon.png

desktop_DATA = data/tascamgtk.desktop
pixmaps_DATA     = data/tascamgtk.png

tascamgtk_CPPFLAGS = -g -O0 -std=c++11 ${GTK3_CFLAGS} ${GTKMM3_CFLAGS} ${ALSA_CFLAGS} ${XML3_CFLAGS} ${XML26_CFLAGS} ${LO_CFLAGS} -DPKGDATADIR='"$(datadir)"'
tascamgtk_LDADD = ${GTK2_LIBS} ${GTKMM3_LIBS} ${ALSA_LIBS} ${XML3_LIBS} ${XML26_LIBS} -lpthread

gsettings_SCHEMAS = de.paraair.tascamgtk.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = $(gsettings_SCHEMAS) \
	     $(pixmaps_DATA) \
	     $(desktop_DATA)

if HAVE_OSC
   tascamgtk_CPPFLAGS += ${LO_CFLAGS}
   tascamgtk_LDADD +=  ${LO_LIBS}
endif
