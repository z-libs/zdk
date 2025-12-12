PROJECT = zdk
HEADERS = zalloc.h zerror.h zlist.h zmap.h zmath.h zops.h zrand.h zstr.h zvec.h zworld.h

# Install to /usr/local/include/zdk/
PREFIX ?= /usr/local
INCLUDEDIR ?= $(PREFIX)/include
INSTALL_DIR = $(INCLUDEDIR)/$(PROJECT)

.PHONY: all install uninstall

all:
	@echo "ZDK is a header-only library."
	@echo "Run 'sudo make install' to install to $(INSTALL_DIR)"

install:
	@echo "Installing headers to $(INSTALL_DIR)..."
	@mkdir -p $(INSTALL_DIR)
	@cp $(HEADERS) $(INSTALL_DIR)
	@echo "Done! Usage: #include <zdk/zworld.h>"

uninstall:
	@echo "Removing $(INSTALL_DIR)..."
	@rm -rf $(INSTALL_DIR)
	@echo "Done."