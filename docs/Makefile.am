man1_MAN = \
	wslogd.1

man5_MAN = \
	wslogd.conf.5

dist_man_MANS = $(man1_MAN) $(man5_MAN)

CLEANFILES = $(dist_man_MANS)

.txt.1:
	a2x -f manpage $^

.txt.5:
	a2x -f manpage $^
