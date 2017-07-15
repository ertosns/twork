#!/bin/bash

#where do you need your profile?
PROF=${HOME}/twork

mkdir -p $PROF $BACKUP $ALERT
cp twork.out twork $PROF
chmod 770 $PROF/twork.out
chmod 770 $PROF/twork
cp twork $PROF
ln -f -s ${PROF}/twork /usr/local/bin/twork
