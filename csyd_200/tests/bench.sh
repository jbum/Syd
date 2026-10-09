#!/bin/sh
# Best-of-3 render times of the heaviest corpus patches with 1 thread and with all cores,
# for ./csyd and for 1.5.2. Steadier than compare.py's single runs.
cd "$(dirname "$0")/../.." || exit 1
for p in csyd_102/testWhitneyChromatic.syd:5 csyd_102/patches/Whitney2StageChromaticPan48.syd:10 \
         csyd_102/patches/whitneyStereo.syd:20 jsyd_102/JSydBeta/patches/AdditiveDrone2.syd:20; do
  path=${p%:*}; secs=${p#*:}
  line="$(basename "$path" .syd) (${secs}s):"
  for bin in "csyd_152/csyd" "csyd_200/csyd -j 1" "csyd_200/csyd"; do
    best=999
    for i in 1 2 3; do
      t=$( { /usr/bin/time -p sh -c "cd $(dirname "$path") && SYD_SEED=12345 $PWD/$bin -q -d $secs -o /tmp/bench.wav $(basename "$path") >/dev/null 2>&1"; } 2>&1 | awk '/real/{print $2}')
      best=$(echo "$t $best" | awk '{print ($1<$2)?$1:$2}')
    done
    line="$line  ${bin#csyd_}=${best}s"
  done
  echo "$line"
done
