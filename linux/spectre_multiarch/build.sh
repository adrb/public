#!/bin/bash

gcc -o m -I . measure-threshold.c perf.c
gcc -o s -I . spectre.c perf.c

