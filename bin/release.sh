#!/bin/sh -eu

getpermission() {
  local response

  echo "$@ [y/n]"
  read response
  [ "$response" = "y" ]
}

new_ver="$(cat .version)"

getpermission "bump version to '${new_ver}'" && {
  sed -i 's/\(ALTPRINTF_VERSION\) ".*"/\1 "'"${new_ver}"'"/g' \
    subprojects/libaltprintf/include/altprintf/altprintf.h

  sed -i 's/\(VERSION ||=\) ".*"/\1 "'"${new_ver}"'"/g' \
    gem/lib/altprintf/version.rb

  git -P status -s
  git -P diff --shortstat

  getpermission "commit?" && {
    git commit -am "release version ${new_ver}"

    getpermission "tag?" && {
      git tag v"${new_ver}"

      getpermission "push?" && {
        git push
        git push --tags

        getpermission "publish gem?" && {
          cd gem/
          bundle exec rake gem:publish
        }
      }
    }
  }
}
