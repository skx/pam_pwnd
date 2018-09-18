#
# Simple makefile for our project.
#


#
# Make our PAM-module by default.
#
all: pam_pwnd.so


#
# Format our code consistently.
#
format:
	astyle --style=allman -A1 --indent=spaces=4   --break-blocks --pad-oper --pad-header --unpad-paren --max-code-length=200 *.c *.h


#
# Make the module
#
pam_pwnd.so: Makefile pam_pwnd.c pwn_chk.c sha1.c sha1.h
	gcc -fPIC -c pam_pwnd.c -lpam -lpam_misc -lpamc
	gcc -fPIC -c pwn_chk.c  -lcurl
	gcc -fPIC -c sha1.c
	ld -x --shared -o pam_pwnd.so pam_pwnd.o pwn_chk.o sha1.o -lcurl -lpam -lpam_misc -lpamc


#
# Install the module
#
install: pam_pwnd.so
	install pam_pwnd.so /lib/x86_64-linux-gnu/security


#
# Cleanup
#
clean:
	rm ./main *.o *.so || true


#
# Build a simple test-script
#
pam_test: pam_test.c sha1.c sha1.h pwn_chk.c
	gcc -o pam_test pam_test.c pwn_chk.c sha1.c  -lcurl

#
# Run the test-cases.
#
test: pam_test
	./pam_test
