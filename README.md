# SUID

Somewhat an inverse to `sudo`.


## Usage

    git clone https://github.com/hilbix/suid.git
    cd suid
    make
    sudo make install

Afterwards you can run something as

    suid command args..

`suid` is inverse to `sudo` in the sense, that `sudo` is `user->command`, where `suid` is `command->user`.


## Configuration and environment

See `suid.conf` sample file.


## FAQ

Why not `sudo`?

- `sudo` becomes the parent of the called program.  This is for additinal safety, because in former times you were able to send signals to `suid` programs.  So the forked `suid` programs cannot access the PID of the caller (and `sudo` does not expose the parent to the child either).

- I do not like `sudo` to allow users to do things on a system level.  Also the `sudoers` file syntax is far from intuitive and the calling convention of `sudo` is ugly.  Instead I think it is far more easy to wrap it like `suid` does and empower the called command to sort things out (safely).

- `suid` is very easy to use, as it does not involve passwords (for now).


Is `suid` secure?

- Hope so.  I did my best to avoid common pitfalls.  But no guarantees, though.

- If you find a bug, please [open an Issue at GitHub](https://github.com/hilbix/suid/issues).

- When sending pull requests, please stick to the "license".  (This is, abandon all Copyright from what you wrote.)

- `suid` does not automagically secure your wrappers in `/etc/suid.conf`, so do not use insecure directories like `/tmp/` (dirs with write access only from `root` should be ok).

Other conf?

- For security reasons `suid` only uses `/etc/suid.conf`.  (In future `/etc/suid.conf.d/` might show up, too.)

- It would be very difficult to allow several different `suid` wrappers with autoconfig.  So only one is supported.


`Missing privilege separation directory: /var/run/sshd`

- This can happen if you try to run `suid sshd` when `/etc/suid.conf` has a line like:

      sshd::::D:/:/usr/sbin/sshd:-D

- Solution: Wrap `sshd` a bit deeper:

      sshd::::D:/:/bin/sh:-c:mkdir -pm700 /var/run/sshd && { flock -nx 1 && exec /usr/sbin/sshd -D </dev/null; } >> /var/run/sshd/lock 2>&1

- This way you can start `sshd` on Windows 10 with a `.bat` like this:

      echo exec suid sshd; | C:\Windows\System32\bash.exe

  This then looks very nice and natural (here with `putty localhost`):
  
      $ pstree -p
      init(1)───sshd(2)───sshd(5)───sshd(38)─┬─bash(39)───vim(134)
                                             └─bash(92)───pstree(135)

Debianized version?

- I'm working on it.

- However I probably do not have time to become a Debian maintainer myself.


License?

- See License below.

- Yeah, this is not really a license, but it defines the rules.


## License

This Works is placed under the terms of the Copyright Less License,
see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.

Read: This is free as in free beer, free speech and free baby.
Copyright on DNA is contradicting human rights.

