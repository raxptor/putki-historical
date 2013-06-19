#!/bin/bash


./build/ccg-ui-databuilder --csharp

rm -rf out/Resources
mkdir out/Resources

find out/csharp32 -name "*.pkg" -exec cp {} out/Resources \;
find out/Resources -name "*.pkg" -exec mv {} {}.bytes \;
