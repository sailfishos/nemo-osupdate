TEMPLATE = subdirs

SUBDIRS = \
        nemo-osupdate \
        plugin \
        packagemanagement

plugin.depends = nemo-osupdate
packagemanagement.depends = nemo-osupdate
