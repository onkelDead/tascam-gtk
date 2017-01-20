tascamgtkdir = src
bin_PROGRAMS = tascamgtk
tascamgtk_SOURCES = src/main.cpp  src/OAlsa.cpp  src/OComp.cpp  src/ODial.cpp  src/OEq.cpp src/OFader.cpp src/OMainWnd.cpp  src/OMaster.cpp  src/ORoute.cpp  src/OMeter.cpp  src/OMeterWorker.cpp  src/OStripLayout.cpp src/ODspLayout.cpp
tascamgtk_HEADERS = src/OMainWnd.h src/OTypes.h src/OComp.h src/OMaster.h src/OEq.h src/OFader.h src/OAlsa.h src/OMeterWorker.h src/OStripLayout.h src/ODial.h src/OMeter.h src/ODspLayout.h src/ORoute.h
dist_tascamgtk_DATA = src/tascam-gtk.css

tascamgtk_CPPFLAGS = -O2 -std=c++11 ${GTK3_CFLAGS} ${GTKMM3_CFLAGS} ${ALSA_CFLAGS} ${XML2_CFLAGS} ${LO_CFLAGS}
tascamgtk_LDADD = ${GTK2_LIBS} ${GTKMM3_LIBS} ${ALSA_LIBS} ${XML2_LIBS} -lpthread

if HAVE_OSC
   tascamgtk_CPPFLAGS += ${LO_CFLAGS}
   tascamgtk_LDADD +=  ${LO_LIBS}
endif