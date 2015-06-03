#!/bin/sh
UC=uncrustify
find $1 -name "*.cpp" -exec $UC -c uncrustify.cfg --replace {} \;
find $1 -name "*.h" -exec $UC -c uncrustify.cfg --replace {} \;
find $1 -name "*.cs" -exec $UC -c uncrustify.cfg --replace {} \;
find $1 -name "*.java" -exec $UC -c uncrustify.cfg --replace {} \;


