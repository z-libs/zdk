PROJECT = zdk
HEADERS = zalloc.h zerror.h zlist.h zmap.h zmath.h zops.h zrand.h zstr.h zvec.h zworld.h

# Install to /usr/include/zdk/ (Standard Linux path)
PREFIX ?= /usr/local
INCLUDEDIR ?= $(PREFIX)/include
INSTALL_DIR = $(INCLUDEDIR)/$(PROJECT)

.PHONY: all install uninstall

all:
	@echo "ZDK is a header-only library."
	@echo "Run 'sudo make install' to install."

install:
	@mkdir -p $(DESTDIR)$(INSTALL_DIR)
	@cp $(HEADERS) $(DESTDIR)$(INSTALL_DIR)

uninstall:
	@rm -rf $(DESTDIR)$(INSTALL_DIR)
