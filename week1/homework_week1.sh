#!/bin/sh

if [ $# -ne 1 ]; then
	echo "Usage: $0 [directory]"
	exit 1
fi

echo ">>> Directory : [$1]"

filelist=`find $1 -empty -type d`
filename=".gitkeep"

if [ "$filelist" ]; then
	echo ">>> Empty directory list"
	for file in $filelist
	do
		echo "    $file"
	done
fi

if [ "$filelist" ]; then
	echo ">>> Create .gitkeep in empty directories"
	for file in $filelist
	do
		touch "$file/$filename"
		echo "    Success to create .gitkeep in $file"
	done
fi
