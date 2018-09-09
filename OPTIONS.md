# `suid.conf` options

Now that `suid.conf` (or `.conf` files below `suid.conf.d/`) got a bit more complex,
you probably want to see some easy to understand examples.

## Rationale

`suid` is build around following philosopies:

- Do only one thing, and do it right.
- Don't Repeat Yourself.
- Perfection is reached, if there is nothing left, which can be left away.

Out of neccessity, some things must be grouped together:

- To use namespaces, you need superuser rights.  Hence you already need `suid`.
- Forked programs cannot inject namespaces in the parent, so it must be done on parent level.
- You probably want to be able to change the namespace from some unprivileged user.
- You definitively do not want to need to invent the wheel again for each new script.
- You definitively do not want to need to think about details each time you use things.
- 2nd FA or password checking etc. must be done before a privileged program runs.
- `chroot` must be done before dropping privileges, so it naturally goes into `suid`.

So the natural thing is to combine all these into one single and flexible tool,
but leave all the complicated bits out, which can be solved by other programs.

While the `/etc/passwd` syntax is very simple, it has proven very stable for more than 2 decades.
And it happens that this very well known format can bue used with only very little tweaking,
to express everything we want to express.

The only real decision was, to keep options and suid-programs into the excactly same file format.
It could have been splitted into separated files, too, like `/etc/passwd` and `/etc/groups`.

But as the usage is very similar and usually the config files are very small,
it seemed to be better to just combine things.  At least, it is kept very readable anyway.

One last point is to consider if to really follow the `/etc/passwd` format,
or change the columns to something better.
(In particular, I am not happy with the `pw` columns being the second column.)

However why change such a detail, which might create confusion.
Keeping things similar usually is a good thing.
As the `/etc/passwd` format was appropriate and fits, it was usable and hence should be used..


## Options and options chainging

One very important bit is, that options can have options.  But this is error prone.

> Humans tend to err.  Hence there must be something, which makes it hard for humans to err.
>
> This must be reflected in the design of something new.  If this new thing is not designed this way,
> it is a bug.  A grave bug.
> A bug which should be seen so extraordniary important, that the new thing cannot be used in production
> until until this bug is ultimatively solved and gone.
>
> So much to `systemd` and why it is one of the most harmful threats to modern computers these days.
> Because this ultimative and most important design principle is violated.
> Systemd makes it extremely easy to err if you overlook just a single bit.
> Which is usual if something new comes around.
> And the simple bit which may be overlooked is very simple indeed:
> "Oh, I forgot, we use systemd now."
> This must not create a problem at all.
> So switching from sysvinit to systemd must not create a problem!
> But it does.  So the fault is at the side of systemd, because sysvinit is the old thing,
> the one which shall be replaced.  You cannot blame the past.
> If you do, you are, to use the words of Linus Torvalds, a `f*cking idiot`.  Period.
> So if you use systemd, then be aware, that it just creates new problems.
>
> Most time by most people, there were no real problems to solve with sysvinit.
> So systemd ieplaces something which works with something completely different.
> This not neccessarily needs to be a problem.
> Having different init systems is basically a good thing.
> And apps shall support as many init systems as possible.
> However the one point of critique is not solved with systemd:
> Systemd is a new thing which radically breaks with what's already there.
> And hence it is creating errors in case people do not listen.
> And this is a flaw of systemd, not of the poeple!
>
> And there is a fundamental principle in IT:  Never change a running system.
>
> Replacing a thing which is working with something is flawed is just one thing:  Very, very dumb.

To not fall into the same trap, `suid` makes it hard to just do the wrong thing.
It might look a bit clumsy, but it is quite effective:

- Options are a comma separated list of options
- The name of an option is prefixed by `!` to distinguish it from commands.
- Also the flags field must begin with `!` to distinguish it from commands.
- If an option has options, these options must have one `!` more in front of their name.

Consider following:

	uname:local:::D-:/:/bin/uname
	ifconfig:local:=:=:D-:/:/sbin/ifconfig
	bash:totp,local:=:=:D-::/bin/bash
	!totp::::!D:/:/checktotp.sh
	!local::net:local:!D:/:/setup-net.sh:local

This is error-prone:

- You need to specify the exact list of options, over and over again.
- You need to remember to use `:=:=:` to not let the command run privileged
- and so on.

It is much better, to group common used things and reuse them:

	uname:local:::D-:/:/bin/uname
	ifconfig:local:::D-:/:/sbin/ifconfig
	bash:totp,local:::D-::/bin/bash
	!totp::::!D:/:/checktotp.sh
	!local:user,group,local:::!::
	!!user::user:*:!::
	!!group::group:*:!::
	!!local::net:local:!D:/:/setup-net.sh:local

Please note:

- You still can use `!!group` as option from commands, just use `cmd:!group:..`

- Have a look at `!local` and `!!local`.  This probably is bad practice here,
  as typing `!local` instead of `local` (which might happen once you have adopted to `suid`),
  might happen far too easliy.  So better rename one of those two into something completely different.

  However this is not bad design of `suid`, as `suid` only is the framework,
  which is designed to be helpful, forgiving, and allows you to reduce accidental errors
  if you use it properly.  However it leaves you all freedom, even to do stupid things.
  (There cannot be freedom at all if you are, all way, protected from doing stupid things!)

- Those `!` perhaps looks awful.  I decided against using `.`, `-`, `_` or `*` because of following:
  - `*` already is used elsewhere in `suid.conf` and has a meaning in `/etc/passwd`, so it's confusing
  - `.` looks ugly.  And you perhaps want to have commands like `suid .cmd`
  - `-` is looking like an option.  There is nothing wrong to have `suid -local args..`
  - `_` is a valid start for any command, and I do not want to mix options with commands.
    - it just looks like an option to `suid`, but why not?
  - `!` already has a good tradition in `/etc/passwd` meaning `disabled`.  This meaning is reused.
  - `!id` is a shell event, so it is unlikely you ever want something type something like `suid !cmd`
- The only vaild other variant is somthing like `[option]::::!::`:
  - The nice things here is, that the option really looks like something with a special meaning.
  - However, the 5th field `!` no more corresponds to the option itself, which is bad.
    - The alternative `[option]::::[]::` somehow looks bad.  It leaves you in the dark:
      - `[option]::::[D]::/bin/echo` or
      - `[option]::::[]D::/bin/echo`, which one?
  - It is slightly more to type, but this is not a valid objection.

- Options cannot be confused with commands, because options have `!` in 5th column.
  So you still have `!cmd`, if you want.
  The only restriction is, that there cannot be anoption with the same name.
  (In that case, I would suggest, rename the option from `!cmd` to `!!cmd`.  Or `!!!cmd` etc.)

- If you forget the `!` in the 5th column, the option changes to a command.
  But this ususal is no catastrophy, as options are not meant to do evil things if invoked.
  And if so, well, you broke it, so the pieces are all yours.


## password / 2nd FA and similar

Please remember, that all option scripts are run from `suid` with elevated privileges.

Hence if used as authenticator, be sure to never call user defined scripts this way.

`suid` checks the ownership of the file before running it.
Hence the owner must match to the elevated privileges.
Note that the owner `root` always has the correct privileges.

Note that `suid` can be chained.  As `suid` does no path search by itself,
the specified path to the binary needs to be a full path.
Hence we can have some special commands there:

- `` (empty command) makes `suid` fails with the message (lines) given as arguments.
  - This mostly is for debugging in case you enter too many `:`
- `.` re-invokes `suid` itself with the given commandline
  - 
- `..` is like `.`, but switches into the home directory of the calling user first
  - This way you can use relative commands as well.

As `.` and `..` usually are directories and never can be invoked,
this cannot be mistaken for anything.

if command is empty (but there are arguments), it recursively calls itself,
such that you can invoke other `suid` command which then see the same effective user.

Example:

	!auth::::!::..:userauth
	!userauth::*:*:D::bin/userauth.sh

Yes, sorry, this looks a bit cryptic.
