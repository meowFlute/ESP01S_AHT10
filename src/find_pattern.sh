#!/bin/bash

# TODO
# Use getopt to accept filenames for file lists to search

# Defines $BASEDIR as the dir in which the script is located
# also includes echo_colorized() from $HOME/runcom/echo_colors/echo_colors
boiler_plate_path="$HOME/runcom/boiler_plate/boiler_plate"
[ -z "$_boiler_plate" ] && . $boiler_plate_path

echo_colorized -fb -bg "Scott's project search tool:"

for arg in "$@"
do
    echo_colorized -fg "Searching for $arg in the files listed in tags_files_used"
    # We'll just grep through each file
    for file in $(cat tags_files_used)
    do
	grep -nHs $file -e "$arg"
    done
done
