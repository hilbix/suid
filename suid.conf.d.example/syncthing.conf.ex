# Run syncthing in a contained network namespace, such that it is not reachable from and not talkactive to the outside world without permission.
# Use HaProxy or similar to access it in a controlled fashion and give it a proper and controlled proxy/relay/anything this way.
# This is far more manageable than to implement some complex and error prone firewall rules!
#
# Just put "suid" in front of your normal syncthing call.  To enter the syncthing ns, just enter "suid syncthing sh [command]"
syncthing::::KT-::/bin/bash:-c:set -- "/usr/bin/$0" "$@"; [ sh = "$2" ] && set -- "${SUID_SHELL\:-/bin/bash}" "${@\:3}"; [ -e "/run/netns/$0" ] || ip netns add "$0" && debian_chroot="$0" HOME="$SUID_HOME" exec /usr/bin/setpriv --reuid "$SUIDUID" --regid "$SUIDGID" --keep-groups --inh-caps=-all -- "$@":syncthing
