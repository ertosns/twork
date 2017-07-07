#!/bin/bash

#where do you need your profile?
PROF=${HOME}/twork
#where you alarm media resides?
ALERT=${HOME}/twork/media
#where you database backup resides?
BACKUP=${HOME}/twork/backup
DEVELOP=${HOME}/prj

mkdir -p $PROF $BACKUP $ALERT
cp twork.out twork $PROF
chmod 770 $PROF/twork.out
chmod 770 $PROF/twork
cp twork $PROF
ln -f -s ${PROF}/twork /usr/local/bin/twork
