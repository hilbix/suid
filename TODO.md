# To Do

- Enforce commands having at least one `/` in it. (Leaves room for future extensions.)

- Close all file descriptors except 0/1/2
  - Flag (Other) to pass other filedescriptors unchanged
  - Is there a way to find the "highest open file descriptor" under Linux?
  - https://stackoverflow.com/questions/899038/getting-the-highest-allocated-file-descriptor

- Flag (Parent) to run as the parent of the given suid (as `sudo` does).
  - Subflag (lowercase?) to create a temporay CWD to run in.  The directory is removed afterwards.
  - Subflag to allocate PTYs for TTY file descriptors (0/1/2), and forward them to the original PTY
  - Subflag to use Pipes instead of PTY

- Refactor how Flags are done
  - Currently, flags are hardcoded in the call setup, this is error prone to edit.
  - Make them an easy to extend and understand structure which keeps the flags.
  - Use callbacks/behavior to set the flags.

# Flags

- `A`:	- (proposed #17 pass env again)
- `B`:	- (proposed #17 pass env without prefix)
- `C`:	Cmd (arg0 given to cmd is first arg given to `suid`)
- `D`:	Debug (print debugging output to stderr)
- `E`:	-
- `F`:	Filename (arg0 file portion)
- `G`:	allow unknown GID
- `H`:	-
- `I`:	Insecure (file can be owned by anybody)
- `J`:	-
- `K`:	Keep groups (behavior until V3.3)
- `L`:	-
- `M`:	-
- `N`:	Next (arg0 is sandwiched before arg1)
- `O`:	- (Pass Other file descriptors without closing them)
- `P`:	- (Parent: `suid` does not `fork()+exec()` to run the child)
- `Q`:	-
- `R`:	Realpath (arg0 is the full path to the bin)
- `S`:	allow ShellShock-attack
- `T`:	allow TIOCSTI-attack (no setsid())
- `U`:	allow unknown UID
- `V`:	- (proposed #17 pass verification information via environment)
- `W`:	Wrap (pass script via FD to shell)
- `X`:	-
- `Y`:	-
- `Z`:	-

