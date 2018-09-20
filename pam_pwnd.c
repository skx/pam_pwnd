/*
 * pam_pwnd.c - Test password against the Have I Been Pwnd API.
 *
 * Steve
 *
 */

#define PAM_SM_AUTH

#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>

#include "sha1.h"

/*
 * The lookup of the password is handled via this external function.
 */
extern int was_leaked(char *hash);


/*
 * This function is called to handle the authentication.
 *
 * We first of all retrieve the username, then the password, and after that
 * we run the lookup test.
 */
PAM_EXTERN int pam_sm_authenticate(pam_handle_t * pamh, int flags, int argc, const
                                   char  ** argv)
{
    const char * userName = NULL;
    const char * userPasswd = NULL;

    /*
     * Get the username.
     */
    if (pam_get_user(pamh, &userName, NULL) != PAM_SUCCESS)
    {
        openlog("pam_pwnd", 0, 0);
        syslog(LOG_ERR, "pam_pwnd: cannot determine user name.");
        closelog();
        return PAM_USER_UNKNOWN;
    }

    /*
     * Get the password.
     */
    if (pam_get_authtok(pamh, PAM_AUTHTOK, (const char **)&userPasswd, NULL) !=
            PAM_SUCCESS)
    {
        openlog("pam_pwnd", 0, 0);
        syslog(LOG_ERR, "pam_pwnd: error getting user password.");
        closelog();
        return PAM_AUTH_ERR;
    }

    /*
     * Sanity-Check
     */
    if (userName == NULL || userPasswd == NULL)
    {
        openlog("pam_pwnd", 0, 0);
        syslog(LOG_ERR, "pam_pwnd: cowardly aborting due to null pointer(s).");
        closelog();
        return PAM_AUTHINFO_UNAVAIL;

    }

    /*
     * (SHA1) Hash the password.
     */
    unsigned char hash[20];
    SHA1_CTX ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, (unsigned char*)userPasswd, strlen(userPasswd));
    SHA1Final(hash, &ctx);

    /*
     * Convert to a hex-string.
     */
    char buf[41] = {'\0'};

    /*
     * NOTE: We upper-case the string here, but we also repeat that
     * later on.  Just because.
     */
    for (int i = 0; i < 20; i++)
        sprintf(buf, "%s%02X", buf, hash[i]);


    /*
     * Lookup the hash in the remote API.
     */
    int ret = was_leaked(buf);

    /*
     * Handle the result.
     */
    if (ret < 0)
    {

        /*
         * If the return value is <0 that means that something failed.
         *
         * We could deny the login here, but it is better to fail open.
         */
        openlog("pam_pwnd", 0, 0);
        syslog(LOG_ERR, "pam_pwnd: couldn't perform hash-test for %s properly.", userName);
        closelog();
        return PAM_SUCCESS;

    }
    else if (ret == 0)
    {

        /*
         * No result :)
         */
        openlog("pam_pwnd", 0, 0);
        syslog(LOG_ERR, "pam_pwnd: hash for %s was not listed in remote database (good).", userName);
        closelog();
        return PAM_SUCCESS;
    }
    else
    {

        /*
         * l33t hax0rs have already stolen your password.
         *
         */
        openlog("pam_pwnd", 0, 0);
        syslog(LOG_ERR, "pam_pwnd: hash for %s is included in remote database (bad)!", userName);
        closelog();
    }

    return PAM_AUTHINFO_UNAVAIL;

}


/*
 * Not sure why this is required, if it even is!
 */
PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char
                              *argv[])
{
    return (PAM_SUCCESS);
}
