#!/usr/bin/env bash

git ls-tree -r $(git rev-parse --abbrev-ref HEAD) --name-only |
	grep -E "(\.cpp|\.c|\.hpp|\.h)$" |
	while read line; do
		clang-format -style=file -i "$line";
	done
