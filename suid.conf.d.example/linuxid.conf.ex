# (no more really needed as everything is output with D flag now, use to verify)
# make && sudo make install
# sudo cp -i suid.conf.d.example/linuxid.conf.ex /etc/suid.conf.d/linuxid.conf
# make linuxid
# for a in '' 0 1; do sudo cp linuxid /linuxid$a; sudo chown 0:0 /linuxid$a; chmod 755 /linuxid$a; done
# sudo chown 1:1 /linuxid1
# sudo chmod +s  /linuxid[01]
# /linuxid0
# suid linuxid0a
# suid linuxid0b
# suid linuxid0c
# /linuxid1
# suid linuxid1a
# suid linuxid1b
# suid linuxid1c
linuxid0a::0:0:D:/:suid:/linuxid
linuxid0b::0:0:D:/:     /linuxid
linuxid0c::0:0:D:/:root:/linuxid
linuxid1a::1:1:D:/:suid:/linuxid
linuxid1b::1:1:D:/:     /linuxid
linuxid1c::1:1:D:/:root:/linuxid
