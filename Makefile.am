
ACLOCAL_AMFLAGS = -I config

SUBDIRS = include examples docs tests msc config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(PACKAGE).pc
# Include the optional build integrations in Autotools release archives.
EXTRA_DIST = $(PACKAGE).pc.in VERSION \
    CMakeLists.txt examples/CMakeLists.txt \
    meson.build meson_options.txt examples/meson.build tests/meson.build \
    autotools.sh scripts/release.sh

DISTCLEANFILES = $(PACKAGE).pc
