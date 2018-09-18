/*
 * pam_test.c : Simple test-cases to exercise our code at least a little.
 */


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "sha1.h"

/*
 * The lookup of the password is handled via this external function.
 */
extern int was_leaked(char *hash);


/*
 * This structure describes a simple SHA1-test.
 *
 * We provide an input and ensure that the output hash matches
 * the given output.
 */
typedef struct sha1_test_case
{
    /*
     * Input to the test-case.
     */
    char *input;

    /*
     * Expected output.
     */
    char *output;

} sha1_test_case;


/*
 * This structure describes a simple API-lookup -test.
 *
 * We provide an input and ensure that the output result matches.
 */
typedef struct pwn_test_case
{
    /*
     * Input to the test-case.
     */
    char *input;

    /*
     * Expected result
     */
    int result;

} pwn_test_case;


/*
 * Test that our SHA1-implementation returns somewhat reasonable
 * results.
 */
void test_sha1()
{

    /*
     * Our test-cases - contain the expected input and output
     */
    sha1_test_case input[]  =
    {
        { "steve", "9ce5770b3bb4b2a1d59be2d97e34379cd192299f"},
        { "ssh.pass", "f9ecf6396e3b442df3dae72b81fec784d2b2900d" },
        { "ssh.pasS", "276ed889f7d9a00e24db1c07579f5b78f19ba204"},
        { "x", "11f6ad8ec52a2984abaafd7c3b516503785c2072"},
        { "xx", "dd7b7b74ea160e049dd128478e074ce47254bde8"},
        { "password", "5baa61e4c9b93f3f0682250b6cf8331b7ee68fd8"},
        { "secret", "e5e9fa1ba31ecd1ae84f75caaa474f3a663f05f4" },
        { "25121974", "137babe83f739e3b71211f422fe4c6850f322279"},
    };

    /*
     * For each case.
     */
    size_t cases = sizeof(input) / sizeof(input[0]);

    for (size_t i = 0; i < cases; i++)
    {

        /*
         * Calculate the SHA1-hash of the input, and ensure
         * it matches the expected value.
         */
        unsigned char hash[20];
        SHA1_CTX ctx;
        SHA1Init(&ctx);
        SHA1Update(&ctx, (unsigned char*)input[i].input, strlen(input[i].input));
        SHA1Final(hash, &ctx);

        /*
         * Convert the output to a readable-value.
         */
        char result[41] = {'\0'};

        for (int i = 0; i < 20; i++)
            sprintf(result, "%s%02x", result, hash[i]);


        if (strcmp(input[i].output, result) != 0)
        {
            printf("%zu - FAIL: Test input '%s' gave hash '%s' not '%s'\n",
                   i + 1, input[i].input, result, input[i].output);
            exit(1);
        }
        else
        {
            printf("%zu - OK: Test input '%s' gave expected hash '%s'\n",
                   i + 1, input[i].input, input[i].output);
        }
    }
}


/*
 * Test that the lookup against the remote API looks somewhat sane.
 *
 */
void test_pwn_lookup()
{
    /*
     * Plain-text passwords to lookup, and expected result.
     */
    pwn_test_case input[]  =
    {

        /* Listed in the DB*/
        { "steve", 1},
        { "secret", 1},
        { "secure", 1},
        { "CorrectHorseBatteryStaple", 1},

        /* Not listed in the DB */
        { "fodspfsdpfksdlfdfjlsdfjldfj", 0 },
        { "fdkslf930290kqldsdsfs", 0 },
        { "290809lkfddks,lfdfsdfdsfdsf-_FD-f0s-f09d-0f9sdf0-9q3q12", 0 },
    };

    /*
     * For each case.
     */
    size_t cases = sizeof(input) / sizeof(input[0]);

    for (size_t i = 0; i < cases; i++)
    {

        /*
         * Calculate the SHA1-hash of the input.
         */
        unsigned char hash[20];
        SHA1_CTX ctx;
        SHA1Init(&ctx);
        SHA1Update(&ctx, (unsigned char*)input[i].input, strlen(input[i].input));
        SHA1Final(hash, &ctx);

        /*
         * Convert the output to a readable-value.
         */
        char result[41] = {'\0'};

        for (int i = 0; i < 20; i++)
            sprintf(result, "%s%02x", result, hash[i]);


        /*
         * Now lookup that hash.
         */
        int found = was_leaked(result);

        if (found != input[i].result)
        {
            printf("%zu - FAIL: Test input '%s' gave '%d' not '%d'\n",
                   i + 1, input[i].input, found, input[i].result);
            exit(1);
        }
        else
        {
            printf("%zu - OK: Test input '%s' gave expected result.\n",
                   i + 1, input[i].input);
        }
    }
}



/*
 * Entry-Point.
 */
int main(int argc, char *argv[])
{

    /*
     * Test hash.
     */
    test_sha1();

    /*
     * Test Pwnage.
     */
    test_pwn_lookup();


    exit(0);
}
