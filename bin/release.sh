#!/bin/sh -eu

new_ver="$(cat .version)"
echo "updating version strings to ${new_ver}"

sed -i 's/\(ALTPRINTF_VERSION\) ".*"/\1 "'"${new_ver}"'"/g' \
  subprojects/libaltprintf/include/altprintf/altprintf.h

sed -i 's/\(VERSION ||=\) ".*"/\1 "'"${new_ver}"'"/g' \
  gem/lib/altprintf/version.rb

echo "commit? [y/n]"
read response
if [ "$response" = "y" ]; then
  git commit -am "release version ${new_ver}"
fi

echo "tag? [y/n]"
read response
if [ "$response" = "y" ]; then
  git tag v"${new_ver}"
fi
