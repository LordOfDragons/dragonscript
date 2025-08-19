#/bin/bash

buildPackage=false
uploadPackage=false
for arg in "$*"; do
  case "$arg" in
  --build-package)
    buildPackage=true ;;
  --upload-package)
    uploadPackage=true ;;
  --help|-h)
    echo './debianBuild [--build-package] [--upload-package] [--help|-h]'
    exit 0
    ;;
  esac
done

umask 0
cd /sources/dragonscript

apt update -y -q \
  && apt-get -y -q install software-properties-common \
  && add-apt-repository -y -u ppa:rpluess/dragondreams \
  && apt-get -y -q upgrade || exit 1

export SCONSFLAGS="-j 8"

# required since noble or git fails
git config --global --add safe.directory /sources/dragonscript

git clean -dfx || exit 1

cleanScons() {
  find -type d -name "__pycache__" | xargs -- rm -rf
  rm -f config.log
  rm -f build.log
  rm -rf .sconf_temp
  rm -f .sconsign.dblite
}

cleanScons

rm -rf /sources/dragonscript_*.orig.tar.gz
rm -rf /sources/dragonscript_*.orig.tar
rm -rf /sources/dragonscript_*-ppa*
rm -rf /sources/libdscript*.deb
rm -rf /sources/libdscript*.ddeb

if [ $buildPackage = true ]; then
  gbp buildpackage --git-debian-branch=debian --git-upstream-tree=debian --git-ignore-new --git-force-create || exit 1
  ./debian/rules override_dh_auto_build || exit 1
  ./debian/rules override_dh_auto_install || exit 1
  ./debian/rules override_dh_shlibdeps || exit 1
else
  gbp export-orig --upstream-tree=debian --force-create || exit 1
fi

git clean -dfx || exit 1

debuild -S -sa || exit 1

if [ $uploadPackage = true ]; then
  dput ppa:rpluess/dragondreams /sources/dragonscript_*_source.changes || exit 1
fi
