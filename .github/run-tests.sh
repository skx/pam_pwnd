#!/bin/sh


# update system
apt-get update -qq

# install build-dependencies
apt-get install --yes \
        build-essential devscripts make libcurl4-gnutls-dev libpam0g-dev

# compile
make

# run the tests
make test
