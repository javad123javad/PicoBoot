#!/bin/bash
qemu-system-arm   -machine lm3s6965evb   -kernel image.bin   -nographic -S -s
