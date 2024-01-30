#!/usr/bin/env bash

cd tools/run/

perf record -e cycles -e cache-misses -e branch-misses -g --call-graph=dwarf -F 762 ./PlantsVsZombies
#perf script -F +pid > test-conv.perf
