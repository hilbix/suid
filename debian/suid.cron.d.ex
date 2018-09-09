#
# Regular cron jobs for the suid package
#
0 4	* * *	root	[ -x /usr/bin/suid_maintenance ] && /usr/bin/suid_maintenance
