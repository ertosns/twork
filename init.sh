#!/bin/bash

#where do you need your profile?
PROF=${HOME}/twork

mkdir -p $PROF
cp twork.out twork $PROF
chmod 770 $PROF/twork.out
chmod 770 $PROF/twork
ln -f -s ${PROF}/twork /usr/local/bin/twork
