# make && sudo make install
# sudo ln -s linuxid.conf.ex /etc/suid.conf.d/linuxid.conf
# make linuxid && sudo cp linuxid /tmp/linuxid && sudo cp linuxid /tmp/linuxid0 && sudo cp linuxid /tmp/linuxid1
# sudo chown 0:0 /tmp/linuxid /tmp/linuxid0
# sudo chown 1:1 /tmp/linuxid1
# sudo chmod 755 /tmp/linuxid*
# sudo chmod +s /tmp/linuxid[01]
# /tmp/linuxid0
# suid linuxid0a
# suid linuxid0b
# /tmp/linuxid1
# suid linuxid1a
# suid linuxid1b
linuxid0a::0:0::/:suid:/tmp/linuxid
linuxid0b::0:0::/:     /tmp/linuxid
linuxid0c::0:0::/:root:/tmp/linuxid
linuxid1a::1:1::/:suid:/tmp/linuxid
linuxid1b::1:1::/:     /tmp/linuxid
linuxid1c::1:1::/:root:/tmp/linuxid
