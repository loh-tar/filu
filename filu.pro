TEMPLATE = subdirs

# compile app
SUBDIRS += libs
SUBDIRS += progs
#SUBDIRS += userfiles

userfiles.path = /usr/local/lib/Filu/userfiles
userfiles.files = ./userfiles/*
INSTALLS += userfiles
