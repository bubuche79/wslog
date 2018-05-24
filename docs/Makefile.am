man1_MAN = \
	vantage.1 \
	wslogd.1

man5_MAN = \
	wslogd.conf.5

man_MANS = $(man1_MAN) $(man5_MAN)

EXTRA_DIST = \
	vantage.txt \
	wslogd.txt \
	wslogd.conf.txt

CLEANFILES = $(man_MANS)

.txt.1:
	a2x -f manpage $^

.txt.5:
	a2x -f manpage $^
