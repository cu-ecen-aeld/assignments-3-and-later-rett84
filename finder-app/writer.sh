#!/bin/bash



writefile=$1
writestr=$2
count_file=0
count_str=0




if [ -z "$writefile" ] ||  [ -z "$writestr" ]; then


     echo -e "Error one of the parameters is empty" 
     exit 1

 else


    mkdir -p "$(dirname $writefile)" && touch "$writefile"
   
    echo $2 > $1


fi


 if [ ! -f $writefile ]; then

        echo -e " Error creating path and file"
        exit 1
fi