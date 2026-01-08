#!/bin/bash
set -euo pipefail
source /opt/dp/dp/parts/gnome/sdk/run/environment.sh
set -x
make -j"24"
make -j"24" install DESTDIR="/opt/dp/dp/parts/gnome/sdk/install"
