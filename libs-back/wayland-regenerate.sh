#!/bin/bash

# This script is used to regenerate xdg-shell-protocol public code and headers
# from the XML specification.

wayland-scanner public-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml Source/wayland/xdg-shell-protocol.c
wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml Headers/wayland/xdg-shell-client-protocol.h
