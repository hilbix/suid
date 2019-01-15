> **Warning!  Versions before 1.0.0 had a major security flaw!** (major version incremented due to the flaw)

[![suid dev Build Status](https://api.cirrus-ci.com/github/hilbix/suid.svg?branch=dev)](https://cirrus-ci.com/github/hilbix/suid/dev)


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

Return values:

- 125 for suid failure (like usage)
- 126 for option failure (compare: bash -c /dev/null)
- 127 for command not found (compare: bash -c /notfound)


## Configuration and environment

See `suid.conf` sample file.


## FAQ

Why not `sudo`?

- `sudo` becomes the parent of the called program.  This is for additinal safety, because in former times you were able to send signals to `suid` programs.  So the forked `suid` programs cannot access the PID of the caller (and `sudo` does not expose the parent to the child either).

- I do not like `sudo` to allow users to do things on a system level.  Also the `sudoers` file syntax is far from intuitive and the calling convention of `sudo` is ugly.  Instead I think it is far more easy to wrap it like `suid` does and empower the called command to sort things out (safely).

- `suid` is very easy to use, as it does not involve passwords (for now).

- `suid` easily supports Namespaces, 2nd Factor etc.

- `suid` allows to call (and control) SUID-aware programs without need to set SUID flags in filesystem


Why is `:` escaped to `\\:\:` and arguments should be followed by `\\:`?

- Escaping is not particular human friendly, but it is easy to script and parse this way.
  - Simple commandlines which do not include a `:` can be written as-is with spaces changed to `:`
  - If an argument contains no `\`, it is sufficient to add a `\` in front of the `:` each
  - If an argument ends on `\`, you need to append `\\:` on the argument to allow the next separator.
  - If there are `\:` sequences in the argument, fully escape `:` with `\\:\:` to make it unambiguous.
- If you want a single rule which always works:
  - Escape each occurance of `:` to `\\:\:`
  - Join command/argument together with `\\::`
  - Python: `def es(*cmd): return "\\\\::".join([str(a).replace(":", "\\\\:\\:") for a in cmd])`
- Parsing this has following properties in a high level language:
  - Split the string on perl-regex `/(?<!\\):/`
  - In the splitted arguments, replace all occurances of `\:` with `:`
  - In the result of the previous, replace all occurances of `\:` with the empty string.
  - Python: `def de(s): return [a.replace("\\:",":").replace("\\:","") for a in re.split(r'(?<!\\):', s)]`
- The de-escaper of `suid` is a bit faster and a bit more clever than this
  as no regex are needed, also only a single character at-a-time needs to be looked at:
- Low-level parsing forwards means to look left up to 2 characters to remove them:
  - If `:` is encountered check the previous character for `\`.  If not, it is a separator.
  - We saw `\:`, so remove the `\`.  Look at the characer before.
  - If it is `\`, then remove the caracter, too, we are ready (the `:` is swallowed).
  - Else we saw `\:`, so just output `:` (note that we already removed the `\`).
- Low-level parsing backwards allows a simple state machine:
  - state0: state=state1, goto state1
  - state1: If c is `:`, enter state2 and return, else output c and return
  - state2: If c is `\`, enter state3 and return, else output separator, goto state0
  - state2: If c is `\`, state=state0 and return, else output `:`, goto state0


Is `suid` secure?

- Hope so.  I did my best to avoid common pitfalls.  But no guarantees, though.

- If you find a bug, please [open an Issue at GitHub](https://github.com/hilbix/suid/issues).

- When sending pull requests, please stick to the "license".  (This is, abandon all Copyright from what you wrote.)

- `suid` does not automagically secure your wrappers in `/etc/suid.conf`, so do not use insecure directories like `/tmp/` (dirs with write access only from `root` should be ok).


Other conf?

- For security reasons `suid` configuration is kept in `/etc/suid.conf` and files `/etc/suid.conf.d/*.conf`

- It would be very difficult to allow several different `suid` wrappers with autoconfig.  So there is only one supported.


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

