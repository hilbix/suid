# SUID

Inverse to `sudo`.


## Usage

    git clone https://github.com/hilbix/suid.git
    cd suid
    make
    make install

Afterwards you can run something as

    suid command args..

`suid` is inverse to `sudo` in the sense, that `sudo` is `user->command`, where `suid` is `command->user`.


## Configuration

You need a file named `/etc/suid.conf` which is in `/etc/passwd` like format:

    command:pw:user:group:min-maxargs:/path/to/binary args..

- `command` must not contain a `:` and is the first argument
- `pw` is a dummy and must be empty today.
- `user` is the user to change to.  If empty, the user is not changed.
- `group` is the group to change to.  If empty, the group is not changed.
- `min-max` are the minimum and maximum arguments allowed.  The default is `0-0`.  It may change in a compatible way in future if more flags need to be added.


## Environment

Following variables are populated in the called environment:

- `SUID_ORIG_USER` the original `USER` variable
- `SUID_ORIG_UID` the original `UID`
- `SUID_ORIG_PID` the original `PID`


## FAQ

Why not `sudo`?

- `sudo` becomes the parent of the called program.  This is for additinal safety, because in former times you were able to send signals to `suid` programs.  So the forked `suid` programs cannot access the PID of the caller (and `suid` does not forward this to the child either).

- I hate it to use `sudo` to allow users to do things on system level.  Also the `sudoers` file syntax is far from intuitive.  It is far more easy to just invoke a command, and enable the command to sort it out.

- This here is very easy to use.


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

