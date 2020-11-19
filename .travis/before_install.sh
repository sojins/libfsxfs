#!/bin/sh
# Script to run before_install step on Travis-CI
#
# Version: 20201118

# Exit on error.
set -e;

if test ${TRAVIS_OS_NAME} = "linux";
then
	sudo apt-get update;
	sudo apt-mark hold openssh-server postgresql-12;
	sudo apt-get --fix-missing -o Dpkg::Options::="--force-confold" upgrade -y --allow-unauthenticated;

	sudo apt-get install -y autoconf automake autopoint build-essential git libssl-dev libtool pkg-config python-dev-is-python3 python2-dev;

elif test ${TRAVIS_OS_NAME} = "osx";
then
	# Prevent from the 30 days autoclean being triggered on install.
	export HOMEBREW_NO_INSTALL_CLEANUP=1;

	brew update;

	brew install gettext gnu-sed;

	if test ${TARGET} = "macos-gcc-python-setup-py38";
	then
		python3 -m pip install -U pip twine;
	fi
fi

