#!/bin/bash

while [ true ]; 
    do
        sleep 1;

        SERVER=`cat $DCLONER_STATUS/1`;

		# Reload web server if there any changes
        if [ -f '$DCLONER_STATUS/1' ]; then
            service $SERVER reload;

            rm $DCLONER_STATUS/1;
        fi;
    done;
