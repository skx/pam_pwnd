# pam_pwnd

This repository contains a simple PAM module for testing whether a
password being used for authentication has been listed in the
[have I been pwned](https://haveibeenpwned.com/) database.


## Sponsorship

The development of this module was sponsored by three individuals who made charitable donations.  (Anonymous primarily because I didn't ask for permission to name them publicly.)

If you wish to "sponsor this" please [email me](https://steve.kemp.fi/) a reciept of your donation and I'll add your name here.  I support the [RNLI](https://en.wikipedia.org/wiki/Royal_National_Lifeboat_Institution), but feel free to pick whatever you like.



## Compilation

These are the the dependencies I expect:

* For SSL (SHA1 hash calculation)
  * `apt-get install libssl1.0-dev`
* For fetching a remote URI:
  * `apt-get install libcurl4-gnutls-dev`
* For pam:
  * `apt-get install libpam0g-dev`

Assuming you have everything appropriate installed then you should be able to compile the code via a simple `make`:

    $ make
    gcc -fPIC -c pam_pwnd.c -lpam -lpam_misc -lpamc
    gcc -fPIC -c pwn_chk.c  -lcurl -lssl -lcrypto
    ld -x --shared -o pam_pwnd.so pam_pwnd.o pwn_chk.o -lcurl -lssl -lcrypto -lpam -lpam_misc -lpamc



## Installation & Configuration


Once you have compiled the code you should copy the resulting file `pam_pwnd.so` to the appropriate PAM-module directory upon your system.  In my case that means running this command:

    sudo install pam_pwnd.so  /lib/x86_64-linux-gnu/security/


The final step is to enable the module, by editing the PAM configuration file.

In my case I'm using SSH keys for authentication, so I'm only concerned with ensuring that no known-bad passwords are used with sudo.  I append the following line to `/etc/pamd.d/sudo`:

    auth   required   pam_pwnd.so  try_first_pass

The complete file then reads:

      #%PAM-1.0
      session    required   pam_env.so readenv=1 user_readenv=0
      session    required   pam_env.so readenv=1 envfile=/etc/default/locale user_readenv=0
      @include common-auth
      @include common-account
      @include common-session-noninteractive
      auth   required   pam_pwnd.so  try_first_pass

At this point I can test the module hasn't broken my system by reseting the `sudo` cache, and re-authenticating:

     frodo ~ $ sudo -k
     frodo ~ $ sudo su -
     [sudo] password for skx:
     root@frodo:~#


### Installation Paranoia

If things are horribly broken it might be that you'll be unable to
run `sudo` to fix them.

For the duration of any installation you should ensure you have an
open terminal/connection with `root` privileges.

That's just common-sense, I hope!


## Logging

Results will be sent to syslog.  Serch for `pam_pwnd`.


## Security Notes

The code makes a single outgoing HTTP-request for each authentication
request.  If this request fails then we default to failing-open, allowing
the authentication to proceed.

(We assume other modules will actually validate the password, if we
allowed a failure to invoke the API we'd deny all sudo-operations in
the event your DNS, networking, or similar things were broken.)

There are zero memory allocations in this module, which should ensure
that we don't leak anything.  Instead we generate a single temporary
file to hold the results of our HTTP-response, and that temporary file
is cleaned up after use.


## Feedback

Bug reports welcome.


Steve
--
