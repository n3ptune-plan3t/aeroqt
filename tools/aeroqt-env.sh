#!/bin/sh
# Make KDE apps follow AeroQt (Qt Widgets + Kirigami/QML). Source this file, or add
# its contents to ~/.profile (and restart Plasma / re-login):
#   . /usr/share/aeroqt/aeroqt-env.sh
# GTK apps are NOT affected (they use GTK themes, not Qt) - excluded on purpose.
export QT_STYLE_OVERRIDE=aeroqt
export QT_QUICK_CONTROLS_STYLE=AeroQt
# Optional built-in theme: frutiger-aero (default) | flat-dark | win7-classic
# export AEROQT_THEME_NAME=flat-dark
