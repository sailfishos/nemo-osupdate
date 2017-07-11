TEMPLATE = subdirs

SUBDIRS = \
        nemo-osupdate \
        plugin

plugin.depends = nemo-osupdate
