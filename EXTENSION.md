# `suid` extension

There are several common things you want to do when it comes to `suid` programs:

Following can be expressed quite easily with `suid` now:

- You want to run things with elevated privileges
- You want to run things with dropped privileges (or namespaces)

However there is one thing, which still is a bit complicated:

Often you want to run things in a `chroot` way, restricting access to the filesystem.

While this can be archived with a careful setup of namespaces today
(however this needs a lot of care to be taken)
it still is not quite satisfying:

Quite often you want to run things like `sftp` in a `chroot`, such that you cannot see other things at all.

This includes, that you cannot see the `sftp` binary as well in the `chroot`!
Which means, that you cannot run `sftp` once you are chrooted, as there is no `sftp` present in the `chroot`.  Checkmate.

Traditionally to solve this, you need to drop the privileges within `sftp`.
Which again means, you must trust `sftp` fully, that it does it right and so on.

- Most times programs are not `chroot` aware.
  So you cannot start them and have them drop the privileges.
- Doing it with hacks like `LD_PRELOAD` is not better,
  as this heavily depends on the innner design of the command,
  which might change after some security update of the tool.
  Hence the security update might impose a security threat.
  We certainly never want such hacks.

I do not have a real solution yet, but I am confident that `suid` might change that in future.
Using `suid extensions`.

## Extend suid

In the command part, there must be the full path to the binary.

We can abuse this fact to provide extensions.
If the path is just a word, we can search for some `word.so` in some `suid`-directory,
load that shared object and then execute some standard function in it.

However we then need to add some public API, such that there can be some reasonable things be done.

I cannot think about any today.  But perhaps some need will show up.

> Read:
>
> To stay compatible to the future, commands must contain at least one `/`.

We probably should enforce this on `suid` today.


## Run non-chroot aware dynamically linked programs

This means, the program to execute is first loaded into the address space of `suid`.
By mechanisms like shared libraries are loaded.

The program then is linked and ready to run, but must not run at this stage.

- This imposes a problem, because `ld.so` usually is run by the program, and hence must run from within `suid`.
- So either we cannot use `ld.so` (which is bad) or we must find a way to delay this run, until the program is in the target environment.

Then `suid` can do the `chroot`, drop privileges and start the program in the target environment.

Perhaps we must implement this as follows:

- First, we open a bi-directional communication channel (two pipes).
- Now fork.
- The parent now drops all privileges and does the `chroot`
- The child now still has all privileges as `suid`
- Then the parent "executes" the program, thereby asking the child to provide all data
- If this setup has finished, parent and child close the pipes and then the program starts.

The parent might get uncooperative in this process.  Or it might do bad things.

Hence the child needs to check permissions and must be a bit selective on which data to provide.
Perhaps we disallow access to data outside of certain directories which cannot be read by all.

Another variant would be to create something like a memory based filesystem:

- `fork()`
- try to find everything, which might be needed by the program to run
- map this all into memory a truely readonly way (if not possible, do a copy)
. create an index of all this
- Now drop privilegs and do `chroot`
- Then run the program, with some injection, which allows to access the in-memory objects instead of files.  Only `ld.so` must be supported

That's it.

But I leave this to the future.

If ever implemented, this just would go into a flag or an option.  Depending how complex it is to add support.

