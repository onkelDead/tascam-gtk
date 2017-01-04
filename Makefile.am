
bin_PROGRAMS = tascamgtk
tascamgtk_SOURCES = src/main.cpp  src/OAlsa.cpp  src/OComp.cpp  src/ODial.cpp  src/OEq.cpp  src/OMainWnd.cpp  src/OMaster.cpp  src/OMeter.cpp  src/OMeterWorker.cpp  src/OStripLayout.cpp

tascamgtk_CPPFLAGS = -O2 -std=c++11 ${GTK2_CFLAGS} ${GTKMM2_CFLAGS} ${ALSA_CFLAGS}
tascamgtk_LDADD = ${GTK2_LIBS} ${GTKMM2_LIBS} ${ALSA_LIBS} -lpthread

