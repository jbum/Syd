#!/bin/sh
# Publish build/web (made by build_web.py) to syd.jbum.com: a static site on mibbles,
# served from /home/jbum/syd.jbum.com (/var/www/syd.jbum.com links to it).
set -e
cd "$(dirname "$0")"
[ -f web/editor.html ] || { echo "build/web is missing: run build/build_web.py first"; exit 1; }
rsync -az --delete web/ jbum@mibbles:syd.jbum.com/
echo "published to https://syd.jbum.com/"
