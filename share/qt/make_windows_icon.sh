#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/i0coin256_X256.png
ICON_DST=../../src/qt/res/icons/i0coin.ico
convert ${ICON_SRC} -resize 16x16 i0coin-16.png
convert ${ICON_SRC} -resize 32x32 i0coin-32.png
convert ${ICON_SRC} -resize 48x48 i0coin-48.png
convert i0coin-16.png i0coin-32.png i0coin-48.png ${ICON_DST}

