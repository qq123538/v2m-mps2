#!/bin/bash

if ! command -v vcpkg-shell &>/dev/null; then
	echo "vcpkg-shell is not available"

	if [ -f ~/.vcpkg/vcpkg-init ]; then
		echo "Initializing and activating vcpkg-shell from ~/.vcpkg/vcpkg-init"
		. /home/wl_ubuntu/.vcpkg/vcpkg-init
		vcpkg-shell activate
	else
		echo "vcpkg-init not found in ~/.vcpkg/. Please install vcpkg-shell."
	fi
else
	echo "vcpkg-shell is available"
fi
