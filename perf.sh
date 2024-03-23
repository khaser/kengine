#/usr/bin/env bash
perf record -F 10000 -g ./build/kengine $@
perf report -g fractal
rm perf.data
