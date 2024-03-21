#!/bin/sh -eu

build_dir=build-release

if ! type glab >/dev/null; then
	echo "glab is needed to create a release"
	exit 1
fi

case "$(git rev-parse --abbrev-ref HEAD)" in
main | [0-9]*.[0-9]*)
	;;
*)
	echo "Not on the main or a stable branch"
	exit 1
esac

if [ -n "$(git log @{upstream}..)" ]; then
	echo "The main branch has unpushed commits"
	exit 1
fi

meson_options=""
if [ -e "$build_dir" ]; then
	meson_options="$meson_options --wipe"
fi
meson setup "$build_dir" $meson_options

prev_version="$(git describe --tags --abbrev=0)"
version="$(meson introspect "$build_dir" --projectinfo | jq -r .version)"
if [ "$version" = "$prev_version" ]; then
	echo "Version not bumped"
	exit 1
fi

name="$(meson introspect "$build_dir" --projectinfo | jq -r .descriptive_name)"
if [ "$name" = "" ]; then
	echo "Cannot determine project name"
	exit 1
fi

ninja -C "$build_dir" dist

archive_name="$name-$version.tar.xz"
archive_path="$build_dir/meson-dist/$archive_name"
gpg --detach-sig "$archive_path"

sha256="$(cd $build_dir/meson-dist && sha256sum $archive_name)"
sha512="$(cd $build_dir/meson-dist && sha512sum $archive_name)"
archive_url="https://gitlab.freedesktop.org/wayland/$name/-/releases/$version/downloads/$archive_name"
announce_path="$build_dir/meson-dist/$name-$version-announce.eml"
current_branch=$(git branch --show-current)
remote_name=$(git config --get branch.${current_branch}.remote)

cat >"$announce_path" <<EOF
To: <wayland-devel@lists.freedesktop.org>
Subject: [ANNOUNCE] $name $version

`git shortlog --no-merges "$prev_version.."`

git tag: $version

$archive_url
SHA256: $sha256
SHA512: $sha512
PGP:    $archive_url.sig
EOF

echo "Release announcement written to $announce_path"

echo -n "Release $name $version? [y/N] "
read answer
if [ "$answer" != "y" ]; then
	exit 1
fi

git tag -s -m "$version" "$version"
git push "$remote_name" "$version"
glab release create "$version" "$archive_path"* --notes ""
