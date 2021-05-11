#!/bin/sh

~/.local/bin/compiledb -o CM4/compile_commands.json -d "$(pwd)/CM4/Debug" -p ../.metadata/.plugins/org.eclipse.cdt.ui/audio_platform_CM4.build.log
~/.local/bin/compiledb -o CM7/compile_commands.json -d "$(pwd)/CM7/Debug" -p ../.metadata/.plugins/org.eclipse.cdt.ui/audio_platform_CM7.build.log

