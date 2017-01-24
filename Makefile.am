includedir = $(prefix)/include
datadir = $(prefix)/share/tascamgtk-4.0/data
tascamgtkdir = src
bin_PROGRAMS = tascamgtk
tascamgtk_SOURCES = src/main.cpp  src/OAlsa.cpp  src/OComp.cpp  src/ODial.cpp  src/OEq.cpp src/OFader.cpp src/OMainWnd.cpp  src/OMaster.cpp  src/ORoute.cpp  src/OMeter.cpp  src/OMeterWorker.cpp  src/OStripLayout.cpp src/ODspLayout.cpp
noinst_HEADERS = src/OMainWnd.h src/OTypes.h src/OComp.h src/OMaster.h src/OEq.h src/OFader.h src/OAlsa.h src/OMeterWorker.h src/OStripLayout.h src/ODial.h src/OMeter.h src/ODspLayout.h src/ORoute.h

data_DATA = data/tascam-gtk.css data/icon.png
dist_data_DATA = data/tascam-gtk.css data/icon.png

tascamgtk_CPPFLAGS = -O2 -std=c++11 ${GTK3_CFLAGS} ${GTKMM3_CFLAGS} ${ALSA_CFLAGS} ${XML2_CFLAGS} ${LO_CFLAGS} -DPKGDATADIR='"$(datadir)"'
tascamgtk_LDADD = ${GTK2_LIBS} ${GTKMM3_LIBS} ${ALSA_LIBS} ${XML2_LIBS} -lpthread

gsettings_SCHEMAS = de.paraair.tascamgtk.gschema.xml
EXTRA_DIST = $(gsettings_SCHEMAS)
@GSETTINGS_RULES@

if HAVE_OSC
   tascamgtk_CPPFLAGS += ${LO_CFLAGS}
   tascamgtk_LDADD +=  ${LO_LIBS}
endif