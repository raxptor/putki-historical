#!/bin/bash

# ./build/ui-example-databuilder --csharp
DYLD_LIBRARY_PATH=build ./build/ui-example-databuilder --csharp

rm -rf out/Unity
mkdir out/Unity

cp -r out/csharp32/Resources out/Unity
cp -r out/csharp32/packages out/Unity/Resources/

find out/Unity -name "*.*" -exec mv {} {}.bytes \;

cp -r out/Unity/Resources/* ~/CCGUnityUI/Assets/Resources/


