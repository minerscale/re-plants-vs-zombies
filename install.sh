#!/usr/bin/env bash

cmake --build build -j6
if [ $? -eq 0 ]; then
	mkdir -p tools/run
	cp build/PlantsVsZombies tools/run/PlantsVsZombies
	cp build/libbass.so tools/run/libbass.so
fi
