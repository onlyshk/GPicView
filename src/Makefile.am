NULL=

AM_CPPFLAGS = \
	-I$(srcdir) \
	-DPACKAGE_DATA_DIR=\""$(datadir)/gpicview"\" \
	-DPACKAGE_UI_DIR=\""$(datadir)/gpicview/ui"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	$(NULL)

bin_PROGRAMS = gpicview
gpicview_SOURCES = \
	gpicview.c \
	$(NULL)   \
	mainwin.h   \
	mainwin.c   \
	image-list.c   \
	image-list.h   \
	utils.h   \
	file-dlgs.c   \
	file-dlgs.h   \
	pref.c   \
	pref.h   \
	jhead.c   \
	jhead.h   \
	exif.c   \
	jpegfile.c   \
	jpeg-tran.c   \
	jpeg-tran.h   \
	tran-supp.h   \
	tran-supp.c   \
	exif.h   \
	crop.c   \
	crop.h   \
	wallpaper.c   \
	wallpaper.h   \
	loader.c   \
	loader.h   \
	gpsinfo.c   \
	iptc.c   \
	makernote.c   \
	paths.c   \
	exifdialog.c   \
	exifdialog.h   \
	printing.c   \
	printing.h   \
	utils.c   \
	screenshot.h   \
	imlib.c   \
	screenshot.c   \
	screenshot-dlg.c   \
	screenshot-dlg.h   \
	about.c   \
	about.h

gpicview_CFLAGS = \
	$(GTK_CFLAGS) \
	-Werror-implicit-function-declaration \
	$(NULL)

gpicview_LDADD = \
	$(GTK_LIBS) \
	$(INTLLIBS) \
	$(NULL)


# Little program to optimize size of xml files
noinst_PROGRAMS=xml-purge
xml_purge_SOURCES=$(top_srcdir)/src/xml-purge.c
xml_purge_CFLAGS=$(GTK_CFLAGS)
xml_purge_LDADD=$(GTK_LIBS)
