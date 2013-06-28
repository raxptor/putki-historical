#!/bin/bash


# ./build/ccg-ui-databuilder --csharp

rm -rf out/Unity
mkdir out/Unity

cp -r out/csharp32/Resources out/Unity
cp -r out/csharp32/packages out/Unity/Resources/

find out/Unity -name "*.*" -exec mv {} {}.bytes \;
