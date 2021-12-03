# Security Policy

## Supported Versions

Only the most recent version (the highest version on `master` branch) of `suid` is supported for now.

`suid` comes with a strict security first policy.

So if you find any security related issue which, by default, might affect just a minority in very improbable cases,
this is a vulnerability, so please report it!

> Example: `suid` protects `bash`-scripts against ShellShock by default.  Because this makes things more secure.

Using no flags in `/etc/suid.conf` means to have the most secure setting.  But flags shall work as advertised also.
If not, this either is a implementation bug or some documentation flaw.  Both must be fixed.

## Reporting a Vulnerability

If you find any security related issue, please open an Issue on GH.

If you send PRs, please remove all Copyrights first
(see License( else I cannot merge.
