# Example .vbs file contents to invoke this from Windows 10:
# set p = wscript.createobject("wscript.shell")
# p.run "cmd.exe /c echo exec suid sshd; | C:\Windows\System32\bash.exe",0

sshd::::D:/:/bin/sh:-c:mkdir -pm700 /var/run/sshd && { flock -nx 1 && exec /usr/sbin/sshd -D </dev/null; } >> /var/run/sshd/lock 2>&1
