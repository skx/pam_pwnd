/*
 * pwn_chk.c - Test an SHA1 hash against the HaveIBeenPwnd API.
 *
 * Steve
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <syslog.h>
#include <ctype.h>

/*
 * This callback-function is invoked when fresh data has been downloaded
 * via libcurl.
 */
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

/*
 * Test to see if the given SHA1-hash is known to the HaveIBeenPwnd site.
 *
 * Return value:
 *
 * <0:  Error performing the test.
 *  0:  No leak.
 * >0:  Leaked.
 *
 */
int was_leaked(char *hash)
{

    /*
     * We're going to make a request via Curl, so we need a curl
     * object/structure for interfacing with that library.
     */
    CURL *curl;
    CURLcode res;

    /*
     * For the HaveIBeenPwnd library we need to make a request with the
     * first five characters of the SHA1-hash, and then look for the
     * rest of the hash in the response.
     */
    char start[6] = {'\0'};
    char rest[48] = {'\0'};
    strncpy(start, hash, 5);
    strcpy(rest, hash + 5);

    /*
     * Our hash-segments should be upper-cased.
     */
    for (unsigned int i = 0; i < strlen(start); i++)
    {
        start[i] = toupper(start[i]);
    }

    for (unsigned int i = 0; i < strlen(rest); i++)
    {
        rest[i] = toupper(rest[i]);
    }

    /*
     * The URL we're fetching.
     */
    char url[100] = {'\0'};
    snprintf(url, sizeof(url) - 1,
             "https://api.pwnedpasswords.com/range/%s", start);

    /*
     * Create a temporary file to hold the response.
     */
    FILE * fd = tmpfile();

    /*
     * Initialize curl.
     */
    curl = curl_easy_init();
    if (!curl)
    {
        openlog("pwn_chk.c", 0, 0);
        syslog(LOG_ERR, "Failed to setup curl.");
        closelog();
        fclose(fd);
        return -1;
    }

    /*
     * Setup the request.
     */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);

    /*
     * Perform the request - this is blocking.
     */
    long res_code = 0;
    res = curl_easy_perform(curl);

    /*
     * Did we receive an error from the remote call?
     */
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
    if (!(res_code == 200) && res != CURLE_ABORTED_BY_CALLBACK)
    {

        /*
         * Log the return-code.
         */
        openlog("pwn_chk.c", 0, 0);
        syslog(LOG_ERR, "HTTP Response code was not 200: %ld", res_code);
        closelog();

        curl_easy_cleanup(curl);
        fclose(fd);
        return -1;
    }

    /*
     * Cleanup the curl request.
     */
    curl_easy_cleanup(curl);

    /*
     * At this point we should have a temporary-file containing the result
     * of the request which will be a series of lines of the form:
     *
     *  HASH:COUNT
     *
     * "HASH" is the SHA1-hash (minus the first five characters).
     *
     * "COUNT" is the number of times the hash was leaked (we're going to
     * ignore that, we just care if the password-hash is present at all).
     *
     */

    /*
     * Rewind the file-handle and scan the contents, line-by-line.
     */
    rewind(fd);
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int found = 0;

    while ((read = getline(&line, &len, fd)) != -1)
    {
        if (strncmp(rest, line, strlen(rest)) == 0)
        {
            openlog("pwn_chk.c", 0, 0);
            syslog(LOG_ERR, "Password hash is known-leaked: %s", hash);
            closelog();

            found = 1;
        }
    }

    /*
     * Close the file-handle, which should trigger an unlink too.
     */
    fclose(fd);
    return found;

}
