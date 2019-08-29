SUBDIRS = src

if USE_LIBCHECK
SUBDIRS += tests
endif

debuild: distdir
	tardir=$(distdir) && $(am__tar) | eval GZIP= gzip $(GZIP_ENV) -c > $(PACKAGE)_$(VERSION).orig.tar.gz
	cp -fpR packages/ubuntu/debian $(distdir)
	cd $(distdir); debuild -us -uc
	$(am__post_remove_distdir)

debuild-clean:
	$(am__remove_distdir)
	-rm -f $(PACKAGE)_$(VERSION)*
	-rm -f $(PACKAGE)-dbgsym_$(VERSION)*
	-rm -f $(PACKAGE)-cgi_$(VERSION)*
	-rm -f $(PACKAGE)-cgi-dbgsym_$(VERSION)*
	-rm -f $(PACKAGE)-dev_$(VERSION)*
