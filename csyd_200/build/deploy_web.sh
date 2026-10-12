#!/bin/sh
# Publish build/web (made by build_web.py) to syd.jbum.com with rsync --delete. The destination
# (user@host:dir/) comes from $DEPLOY_DEST or build/deploy.dest, which is not committed.
set -e
cd "$(dirname "$0")"
[ -f web/editor.html ] || { echo "build/web is missing: run build/build_web.py first"; exit 1; }
DEST="${DEPLOY_DEST:-$(cat deploy.dest 2>/dev/null || true)}"
[ -n "$DEST" ] || { echo "no destination: set DEPLOY_DEST=user@host:dir/ or write it in build/deploy.dest"; exit 1; }
rsync -az --delete web/ "$DEST"
echo "published to https://syd.jbum.com/"
