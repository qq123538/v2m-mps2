#!/bin/bash

SCRIPT_DIR=$(cd -- "$(dirname -- "$0")" &>/dev/null && pwd)
echo "SCRIPT_DIR: $SCRIPT_DIR"

if ! command -v vcpkg-shell &>/dev/null; then
	echo "vcpkg-shell is not available"

	if [ -f ~/.vcpkg/vcpkg-init ]; then
		echo "Initializing and activating vcpkg-shell from ~/.vcpkg/vcpkg-init"
		. ~/.vcpkg/vcpkg-init
		cd $SCRIPT_DIR && vcpkg-shell activate
		cd - &> /dev/null
	else
		echo "vcpkg-init not found in ~/.vcpkg/. Please install vcpkg-shell."
	fi
else
	echo "vcpkg-shell is available"
fi
