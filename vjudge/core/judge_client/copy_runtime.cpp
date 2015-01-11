#include <stdio.h>

#include "judge_client.h"

extern int DEBUG;
extern int db_port;
extern int shm_run;
extern int max_running;
extern int db_timeout;
extern int sleep_time;
extern int java_time_bonus;
extern int java_memory_bonus;
extern int sim_enable;
extern int oi_mode;
extern int use_max_time;
extern char record_call;
extern char db_host[BUFSIZE];
extern char db_user[BUFSIZE];
extern char work_dir[BUFSIZE];
extern char db_passwd[BUFSIZE];
extern char work_dir[BUFSIZE];
extern char db_name[BUFSIZE];
extern char oj_home[BUFSIZE];
extern char java_xms[BUFSIZE];
extern char java_xmx[BUFSIZE];
extern char LANG_NAME[BUFSIZE];
extern char lang_ext[15][8];
extern MYSQL *conn;
extern struct solution_t *solution;
extern int call_counter[BUFSIZE];
extern int call_array_size;

void copy_shell_runtime(void)
{

	execute_cmd("/bin/mkdir %s/lib", work_dir);
	execute_cmd("/bin/mkdir %s/lib64", work_dir);
	execute_cmd("/bin/mkdir %s/bin", work_dir);
	execute_cmd("/bin/cp /lib/* %s/lib/", work_dir);
	execute_cmd("/bin/cp -a /lib/i386-linux-gnu %s/lib/", work_dir);
	execute_cmd("/bin/cp -a /lib/x86_64-linux-gnu %s/lib/", work_dir);
	execute_cmd("/bin/cp /lib64/* %s/lib64/", work_dir);
	execute_cmd("/bin/cp -a /lib32 %s/", work_dir);
	execute_cmd("/bin/cp /bin/busybox %s/bin/", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/sh", work_dir);
	execute_cmd("/bin/cp /bin/bash %s/bin/bash", work_dir);

}

void copy_objc_runtime(void)
{
	copy_shell_runtime();
	execute_cmd("/bin/mkdir -p %s/proc", work_dir);
	execute_cmd("/bin/mount -o bind /proc %s/proc", work_dir);
	execute_cmd("/bin/mkdir -p %s/lib/", work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/libdbus-1.so.3                          %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/libgcc_s.so.1                           %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/libgcrypt.so.11                         %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/libgpg-error.so.0                       %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/libz.so.1                               %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/tls/i686/cmov/libc.so.6                 %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/tls/i686/cmov/libdl.so.2                %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/tls/i686/cmov/libm.so.6                 %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/tls/i686/cmov/libnsl.so.1               %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/tls/i686/cmov/libpthread.so.0           %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /lib/tls/i686/cmov/librt.so.1                %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libavahi-client.so.3                %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libavahi-common.so.3                %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libdns_sd.so.1                      %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libffi.so.5                         %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libgnustep-base.so.1.19             %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libgnutls.so.26                     %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libobjc.so.2                        %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libtasn1.so.3                       %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libxml2.so.2                        %s/lib/ ",
	     work_dir);
	execute_cmd
	    ("/bin/cp -aL /usr/lib/libxslt.so.1                        %s/lib/ ",
	     work_dir);

}

void copy_bash_runtime(void)
{
	//char cmd[BUFSIZE];
	//const char * ruby_run="/usr/bin/ruby";
	copy_shell_runtime();
	execute_cmd("/bin/cp `which bc`  %s/bin/", work_dir);
	execute_cmd("busybox dos2unix Main.sh", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/grep", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/awk", work_dir);
	execute_cmd("/bin/cp /bin/sed %s/bin/sed", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/cut", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/sort", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/join", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/wc", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/tr", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/dc", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/dd", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/cat", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/tail", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/head", work_dir);
	execute_cmd("/bin/ln -s /bin/busybox %s/bin/xargs", work_dir);
	execute_cmd("chmod +rx %s/Main.sh", work_dir);

}

void copy_ruby_runtime(void)
{

	copy_shell_runtime();
	execute_cmd("mkdir -p %s/usr", work_dir);
	execute_cmd("mkdir -p %s/usr/lib", work_dir);
	execute_cmd("mkdir -p %s/usr/lib64", work_dir);
	execute_cmd("cp -a /usr/lib/libruby* %s/usr/lib/", work_dir);
	execute_cmd("cp -a /usr/lib/ruby* %s/usr/lib/", work_dir);
	execute_cmd("cp -a /usr/lib64/ruby* %s/usr/lib64/", work_dir);
	execute_cmd("cp -a /usr/lib64/libruby* %s/usr/lib64/", work_dir);
	execute_cmd("cp -a /usr/bin/ruby* %s/", work_dir);

}

void copy_guile_runtime(void)
{

	copy_shell_runtime();
	execute_cmd("/bin/mkdir -p %s/proc", work_dir);
	execute_cmd("/bin/mount -o bind /proc %s/proc", work_dir);
	execute_cmd("/bin/mkdir -p %s/usr/lib", work_dir);
	execute_cmd("/bin/mkdir -p %s/usr/share", work_dir);
	execute_cmd("/bin/cp -a /usr/share/guile %s/usr/share/", work_dir);
	execute_cmd("/bin/cp /usr/lib/libguile* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/libgc* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/i386-linux-gnu/libffi* %s/usr/lib/",
		    work_dir);
	execute_cmd("/bin/cp /usr/lib/i386-linux-gnu/libunistring* %s/usr/lib/",
		    work_dir);
	execute_cmd("/bin/cp /usr/lib/*/libgmp* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/libgmp* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/*/libltdl* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/libltdl* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/bin/guile* %s/", work_dir);

}

void copy_python_runtime(void)
{

	copy_shell_runtime();
	execute_cmd("mkdir -p %s/usr/include", work_dir);
	execute_cmd("mkdir -p %s/usr/lib", work_dir);
	execute_cmd("mkdir -p %s/usr/lib64", work_dir);
	execute_cmd("mkdir -p %s/usr/local/lib", work_dir);
	execute_cmd("cp /usr/bin/python* %s/", work_dir);
	execute_cmd("cp -a /usr/lib/python* %s/usr/lib/", work_dir);
	execute_cmd("cp -a /usr/lib64/python* %s/usr/lib64/", work_dir);
	execute_cmd("cp -a /usr/local/lib/python* %s/usr/local/lib/", work_dir);
	execute_cmd("cp -a /usr/include/python* %s/usr/include/", work_dir);
	execute_cmd("cp -a /usr/lib/libpython* %s/usr/lib/", work_dir);

}

void copy_php_runtime(void)
{

	copy_shell_runtime();
	execute_cmd("/bin/mkdir %s/usr", work_dir);
	execute_cmd("/bin/mkdir %s/usr/lib", work_dir);
	execute_cmd("/bin/cp /usr/lib/libedit* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/libdb* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/libgssapi_krb5* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/libkrb5* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/libk5crypto* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/*/libedit* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/*/libdb* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/*/libgssapi_krb5* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/*/libkrb5* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/*/libk5crypto* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/libxml2* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/lib/x86_64-linux-gnu/libxml2.so* %s/usr/lib/",
		    work_dir);
	execute_cmd("/bin/cp /usr/bin/php* %s/", work_dir);
	execute_cmd("chmod +rx %s/Main.php", work_dir);

}

void copy_perl_runtime(void)
{

	copy_shell_runtime();
	execute_cmd("/bin/mkdir %s/usr", work_dir);
	execute_cmd("/bin/mkdir %s/usr/lib", work_dir);
	execute_cmd("/bin/cp /usr/lib/libperl* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /usr/bin/perl* %s/", work_dir);

}

void copy_freebasic_runtime(void)
{

	copy_shell_runtime();
	execute_cmd("/bin/mkdir -p %s/usr/local/lib", work_dir);
	execute_cmd("/bin/mkdir -p %s/usr/local/bin", work_dir);
	execute_cmd("/bin/cp /usr/local/lib/freebasic %s/usr/local/lib/",
		    work_dir);
	execute_cmd("/bin/cp /usr/local/bin/fbc %s/", work_dir);
	execute_cmd("/bin/cp -a /lib32/* %s/lib/", work_dir);

}

void copy_mono_runtime(void)
{
	copy_shell_runtime();
	execute_cmd("/bin/mkdir %s/usr", work_dir);
	execute_cmd("/bin/mkdir %s/proc", work_dir);
	execute_cmd("/bin/mkdir -p %s/usr/lib/mono/2.0", work_dir);
	execute_cmd("/bin/cp -a /usr/lib/mono %s/usr/lib/", work_dir);
	execute_cmd("/bin/mkdir -p %s/usr/lib64/mono/2.0", work_dir);
	execute_cmd("/bin/cp -a /usr/lib64/mono %s/usr/lib64/", work_dir);

	execute_cmd("/bin/cp /usr/lib/libgthread* %s/usr/lib/", work_dir);

	execute_cmd("/bin/mount -o bind /proc %s/proc", work_dir);
	execute_cmd("/bin/cp /usr/bin/mono* %s/", work_dir);

	execute_cmd("/bin/cp /usr/lib/libgthread* %s/usr/lib/", work_dir);
	execute_cmd("/bin/cp /lib/libglib* %s/lib/", work_dir);
	execute_cmd("/bin/cp /lib/tls/i686/cmov/lib* %s/lib/tls/i686/cmov/",
		    work_dir);
	execute_cmd("/bin/cp /lib/libpcre* %s/lib/", work_dir);
	execute_cmd("/bin/cp /lib/ld-linux* %s/lib/", work_dir);
	execute_cmd("/bin/cp /lib64/ld-linux* %s/lib64/", work_dir);
	execute_cmd("/bin/mkdir -p %s/home/judge", work_dir);
	execute_cmd("/bin/chown judge %s/home/judge", work_dir);
	execute_cmd("/bin/mkdir -p %s/etc", work_dir);
	execute_cmd("/bin/grep judge /etc/passwd>%s/etc/passwd", work_dir);

}

void copy_runtime(void)
{
	//create chroot for ruby bash python
	int lang = solution->language;
	write_log("copy runtime lib.\n");
	switch (lang) {
		case 4: copy_ruby_runtime(); break;
		case 5: copy_bash_runtime(); break;
		case 6: copy_python_runtime(); break;
		case 7: copy_php_runtime(); break;
		case 8: copy_perl_runtime(); break;
		case 9: copy_mono_runtime(); break;
		case 10: copy_objc_runtime(); break;
		case 11: copy_freebasic_runtime(); break;
		case 12: copy_guile_runtime(); break;
	}
}
