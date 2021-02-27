# Previously you were able to run "dmesg" as unprivileged user to diagnose
# Today this is no more possible.  "suid dmesg" and "suid dmesgrw" revives
# this without exposing --clear etc.

dmesg:::::/:/bin/dmesg
dmesgrw:::::/:/bin/dmesg:-rw

