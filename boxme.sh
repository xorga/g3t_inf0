#!/bin/bash

mkdir -p sandbox
cp -r inf0 sandbox
cp -r .extract0r sandbox
cd sandbox || exit
rm -rf 0x_includes 0x_srcs art/ post_host Makefile
mv inf0 unzip