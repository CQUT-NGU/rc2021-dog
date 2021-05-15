#!/usr/bin/env bash

value_dir='User Src Inc'

find $value_dir -regex '.*\.\(cpp\|hpp\|cu\|c\|h\)' -exec clang-format --verbose -style=file -i {} \;
