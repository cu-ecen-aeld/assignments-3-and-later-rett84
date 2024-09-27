#!/bin/sh



filedir=$1
searchstr=$2


if [ -z "$filedir" ] ||  [ -z "$searchstr" ]; then


     echo -e "Error one of the parameters is empty" 
     exit 1

 else

    if [ -d "$filedir" ]; then
    

       # echo - "lucas"
#        echo -e "The number of files are" $(find "$filedir" -type f | wc -l) "and the number of matching lines are" $(grep -o $searchstr)

        echo -e "The number of files are" $(find "$filedir" -type f | wc -l)  "and the number of matching lines are" $(grep -R "$searchstr" $filedir | wc -l)

    else


        echo -e "The directory" $filedir "does not exist"
        exit 1

    fi

fi

