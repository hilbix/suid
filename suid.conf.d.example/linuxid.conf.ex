# (no more really needed as everything is output with D flag now, use to verify)
# make && sudo make install
# sudo cp -i suid.conf.d.example/linuxid.conf.ex /etc/suid.conf.d/linuxid.conf
# make linuxid
# for a in '' 0 1; do sudo cp linuxid /linuxid$a; sudo chown 0:0 /linuxid$a; sudo chmod 755 /linuxid$a; done
# sudo chown 1:1 /linuxid1
# sudo chmod +s  /linuxid[01]
# for a in 0 1; do echo; echo = $a =; /linuxid$a; echo $?; for b in a b c d e; do echo; echo === $a $b ===; suid linuxid$a$b; echo $?; done; done
linuxid0a::0:0:D:/:suid:/linuxid
linuxid0b::0:0:D:/:     /linuxid
linuxid0c::1:1:D:/:root:/linuxid
linuxid0d::1:1:D:/:root:/linuxid0
linuxid0e::1:1:D:/:root:/linuxid1
linuxid1a::1:1:D:/:suid:/linuxid
linuxid1b::1:1:D:/:     /linuxid
linuxid1c::1:1:D:/:root:/linuxid
linuxid1d::1:1:D:/:root:/linuxid0
linuxid1e::1:1:D:/:root:/linuxid1
