[![Travis CI](https://img.shields.io/travis/skx/pam_pwnd/master.svg?style=flat-square)](https://travis-ci.org/skx/pam_pwnd)
[![license](https://img.shields.io/github/license/skx/pam_pwnd.svg)](https://github.com/skx/pam_pwnd/blob/master/LICENSE)
[![Release](https://img.shields.io/github/release/skx/pam_pwnd.svg)](https://github.com/skx/pam_pwnd/releases/latest)

# pam_pwnd

This repository contains a simple PAM module for testing whether a
password being used for authentication has been listed in the
[have I been pwned](https://haveibeenpwned.com/) database.


## Sponsorship

The development of this module was sponsored by three individuals who made charitable donations.  (Anonymous primarily because I didn't ask for permission to name them publicly.)

If you wish to "sponsor this" software, and be listed here, just [email me](https://steve.kemp.fi/) a receipt of your donation.  I support the [RNLI](https://en.wikipedia.org/wiki/Royal_National_Lifeboat_Institution), but feel free to pick whatever charity you wish.

The code is released under the [BSD-license](LICENSE) so you can fork it, improve it, use it, and enjoy it!  Feel free to report bugs, or feature-suggestions on the [issue-page](https://github.com/skx/pam_pwnd/issues).



## Compilation

These are the the dependencies I expect you would need for compiling the project:

* For fetching a remote URI we use `libcurl`:
  * `apt-get install libcurl4-gnutls-dev`
* For compiling PAM modules you'll need the appropriate development package:
  * `apt-get install libpam0g-dev`

Assuming you have the dependencies installed then compilation should only require a simple `make`:

    $ make
    gcc -fPIC -c pam_pwnd.c -lpam -lpam_misc -lpamc
    gcc -fPIC -c pwn_chk.c  -lcurl
    gcc -fPIC -c sha1.c
    ld -x --shared -o pam_pwnd.so pam_pwnd.o pwn_chk.o sha1.o -lcurl -lpam -lpam_misc -lpamc



## Installation & Configuration


Once you have compiled the code you should copy the resulting file `pam_pwnd.so` to the appropriate PAM-module directory upon your system.  In my case that means running this command:

    sudo install pam_pwnd.so  /lib/x86_64-linux-gnu/security/


The final step is to enable the module, by editing the PAM configuration file.

In my case I'm using SSH keys for authentication, so I'm only concerned with ensuring that no known-bad passwords are used with sudo.  I append the following line to `/etc/pamd.d/sudo`:

    auth   required   pam_pwnd.so  try_first_pass

The complete file, on an Ubuntu system, might then look like this:

      #%PAM-1.0
      session    required   pam_env.so readenv=1 user_readenv=0
      session    required   pam_env.so readenv=1 envfile=/etc/default/locale user_readenv=0
      @include common-auth
      @include common-account
      @include common-session-noninteractive
      auth   required   pam_pwnd.so  try_first_pass

Upon the "stretch" release of Debian GNU/Linux the complete file might instead look like this:

      #%PAM-1.0

      @include common-auth
      @include common-account
      @include common-session-noninteractive
      auth   required   pam_pwnd.so  try_first_pass

Regardless of what your file looks like, once you've added the reference to `pam_pwnd.so`, you should then be ready to test the module hasn't broken your system by reseting the `sudo` cache, and re-authenticating:

     frodo ~ $ sudo -k
     frodo ~ $ sudo su -
     [sudo] password for skx:
     root@frodo:~#

Assuming nothing is broken you should:

* Be prompted for your password, only once, as expected.
* Receive your root-prompt.

If things are horribly broken, such that you get segfaults or failures from _this_ module then you will probably be unable to run `sudo` to fix them, so for the duration of any installation you should ensure you have an open terminal/connection with `root` privileges.

The module will log results to syslog, search for `pam_pwnd` to see them.



## Security Notes

The code makes a single outgoing HTTP-request for each authentication
request:

* The outgoing request contains the first five characters of your __hashed__ password.
   * i.e. If you password is "secret" it is hashed to `e5e9fa1ba31ecd1ae84f75caaa474f3a663f05f4`, then an outgoing request is made with the characters `e5e9f`.

If the API-lookup request fails then we default to failing-open, allowing the authentication to proceed.   (We assume other modules will actually validate the password, if we allowed a failure to invoke the API we'd deny all PAM-based operations in the event your DNS, networking, or similar things were broken.)

There are zero memory allocations in this module, which should ensure that we don't leak anything.  Instead we generate a single temporary file to hold the results of our HTTP-response, and that temporary file is cleaned up after use.


## Testing Notes

There is a simple test-driver included in this project which exercises some of
the code, it is not designed to be a complete test-case, nor to perform exhaustive testing.

If you're planning to submit pull-requests that change the code you should ensure the tests pass even with your additions:

    $ make test


## Feedback

Bug reports welcome.


Steve
--
