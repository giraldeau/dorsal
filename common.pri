CONFIG += c++11

unix:!macx: LIBS += -L$${top_builddir}/lib/ -ldorsal

INCLUDEPATH += $${top_srcdir}/lib/
DEPENDPATH += $${top_srcdir}/lib/

