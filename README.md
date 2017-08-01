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

- `sudo` becomes the parent of the called program.  This is for additinal safety, because in former times you were able to send signals to `suid` programs.  So the forked `suid` programs cannot access the PID of the caller (and `suid` does not forward this to the child either).

- I hate it to use `sudo` to allow users to do things on system level.  Also the `sudoers` file syntax is far from intuitive.  It is far more easy to just invoke a command, and enable the command to sort it out.

- This here is very easy to use, as it does not involve passwords (for now).


Is this secure?

- Hope so.  I did my best to avoid common pitfalls.  But no guarantees, though.

- If you find a bug, please open an Issue at GitHub.

- When sending pull requests, please stick to the license.  (This is, abandon all Copyright from what you wrote.)


License?

- See License below.

- Yeah, this is not really a license, but it defines the rules.


## License

This Works is placed under the terms of the Copyright Less License,
see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.

Read: This is free as in free beer, free speech and free baby.
Copyright on DNA is contradicting human rights.

