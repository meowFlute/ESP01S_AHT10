#!/bin/bash

# Let's locate the absolute path of this script first
# I got this from https://stackoverflow.com/questions/59895/how-do-i-get-the-directory-where-a-bash-script-is-located-from-within-the-script/246128#246128
SOURCE=${BASH_SOURCE[0]}
pushd . > /dev/null # stores off where we were before all that cd'ing, might not be necessary
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
BASEDIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
popd > /dev/null # This just gets us back to where we were before all that cd'ing
# so now $BASEDIR should be the directory this script is being run from no matter how it was invoked

# This allows us to use the echo_colorized function (see echo_colors)
lib_path="$HOME/runcom/echo_colors/echo_colors"
[ -z "$_echo_colors" ] && . $lib_path

# First we'll make sure that we're in the base directory of the app
echo_colorized -fp "Changing directory to $BASEDIR"
cd $BASEDIR

# Then we'll run 'compiledb make' so we get an up to date .json database
echo_colorized -fp "Running 'compiledb make'"
# compiledb make
echo_colorized -fy "JK I commented that out"

# Now that we have that file we can extract all of the cfiles out from the build
echo_colorized -fp "Generating file $BASEDIR/tags_files_used"
grep -i '"file":' compile_commands.json | awk -F '"' '{print $4}' > tags_files_used
# $IDF_PATH should be available since that is how their make system is working
# tmp_path="$IDF_PATH/components/esp8266/include"
# tmp_path_folders=( "" "driver" "esp8266" "esp_private" "rom" "soc" "xtensa" )
# for tmp_path_folder in "${tmp_path_folders[@]}"
# do
#     tmp_full_path="${tmp_path}/${tmp_path_folder}"
#     for filename in `ls $tmp_full_path | grep '.h'`
#     do
# 	echo "${tmp_full_path}/${filename}" >> tags_files_used
#     done
#done

# Now that we have that file we can call ctags on every file and have it append
# echo_colorized -fp "Running ctags on every file used in the build"
# ctags # start fresh each time...? idk

echo_colorized -fy -br "EXPERIMENTAL ZONE"
header_files=`find "$IDF_PATH" -type f -name "*.h"`
i=0
for header_file in $header_files
do
    i=$((i+1))
    echo $header_file >> tags_files_used
done
echo_colorized -fy "$i header files were found"
