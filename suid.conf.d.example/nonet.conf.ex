# Needs:
#
# suid mk-nonet

nonet::::T-::suid:/sbin/ip:netns:exec:nonet:/bin/bash:-c:debian_chroot=nonet exec /usr/bin/setpriv --ruid "$SUIDUID" --rgid "$SUIDGID" --keep-groups -- /.fixenv "${@-/bin/bash}":setpriv
nonet-mk::::::/sbin/ip:netns:add:nonet
