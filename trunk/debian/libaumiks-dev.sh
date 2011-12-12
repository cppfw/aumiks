#!/bin/sh

packageName=libaumiks-dev

soName=0

libFileName=libaumiks.so

baseDir=debian/out/$packageName
mkdir -p $baseDir

#==========
#copy files

#copy header files
incDir=$baseDir/usr/include/aumiks
mkdir -p $incDir
cp src/aumiks/*.hpp $incDir

#create symbolic .so link to latest .so name
libDir=$baseDir/usr/lib
mkdir -p $libDir
ln -s /usr/lib/$libFileName.$soName $libDir/$libFileName

#copy pkg-config .pc file
pkgConfigDir=$libDir/pkgconfig/
mkdir -p $pkgConfigDir
cp pkg-config/*.pc $pkgConfigDir

#====================
#Generate deb package

#create dir where the output 'control' will be placed
mkdir -p $baseDir/DEBIAN

#remove substvars
rm -f debian/substvars

#generate final control file
dpkg-gencontrol -p$packageName -P$baseDir

dpkg -b $baseDir tmp-package.deb
dpkg-name -o -s .. tmp-package.deb #rename file to proper debian format (package_version_arch.deb)