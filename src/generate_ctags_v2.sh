#!/bin/bash

boiler_path="$HOME/runcom/boiler_plate/boiler_plate"
[ -z "$_boiler_plate" ] && . $boiler_path

echo_colorized -fb -bg "Scott's ctag generator V2"

echo_colorized -fg "clearing out tags_files_used"
> tags_files_used

echo_colorized -fg "reading all .c and .h files in the entire repo"

files=`find "$IDF_PATH" -type f -name "*.[ch]"`
i=0
for file in $files
do
    i=$((i+1))
    echo $file >> tags_files_used
done
echo_colorized -fg "$i files were found"
