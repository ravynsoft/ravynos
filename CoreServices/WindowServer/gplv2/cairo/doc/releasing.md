# Releasing Cairo

Here are the steps to follow to create a new cairo release:

## 0. Decide type of release and checkout the appropriate branch

The Cairo project makes three types of releases: Development snapshot
releases, stable minor releases, and stable micro (aka "point") releases.
Micro releases should be only bugfixes and no API additions.  If there are
API additions consider making a Minor release.  Snapshot releases can be
done of the current development tree between Minor releases, as desired.

For stable releases (both minor and micro), the work should be done on the
given release branch.  E.g. for 1.14.12, check out the 1.14 branch via "git
checkout origin/1.14 -b 1.14".

## 1. Ensure that there are no local, uncommitted/unpushed mods

You're probably in a good state if both "git diff HEAD" and "git log
master..origin/master" give no output.

## 2. Verify that the code passes `meson dist`

First, make sure you have 'nm' and 'readelf' commands in `PATH`.  this
should be OK with any Linux distro.

Running "meson dist" should result in no warnings or errors and end with a
message of the form:

    Created source/cairo/_build/meson-dist/cairo-1.17.8.tar.xz

If the test suite does not pass, you can use:

    meson dist --no-tests

But this should only be allowed for development snapshots. Please, always
check the state of the CI pipeline on GitLab.

## 3. Decide what the new version number for the release will be.

Cairo uses even numbers for official releases, and odd numbers for
development snapshots.  Thus, for a Minor release it would be:

    LAST_RELEASE="X.Y.Z"     # e.g. 1.12.0
    THIS_RELEASE="X.Y+2.0"   # e.g. 1.14.0

While for a Micro release the version numbers should be:

    LAST_RELEASE="X.Y.Z"     # e.g. 1.14.0
    THIS_RELEASE="X.Y.Z+2"   # e.g. 1.14.2

Snapshots are similar but have odd minor versions.  Also, the first snapshot
release in a new series will be .2 rather than .0, e.g.:

    LAST_RELEASE="X.Y.Z"     # e.g. 1.14.0
    THIS_RELEASE="X.Y+1.0"   # e.g. 1.15.2

and subsequent snapshots in that series are just normal micro releases:

    LAST_RELEASE="X.Y.Z"     # e.g. 1.15.2
    THIS_RELEASE="X.Y.Z+2"   # e.g. 1.15.4

## 4. Fill out an entry in the NEWS file

Sift through the logs since the last release. This is most easily done with
a command such as:

    git log --stat ${LAST_RELEASE}..

Summarize major changes briefly in a style similar to other entries in NEWS.
Take special care to note any additions in the API. These should be easy to
find by noting modifications to .h files in the log command above. And more
specifically, the following command will show each patch that has changed a
public header file since the given version:

```
find src/ -name '*.h' ! -name '*-private.h' \
    ! -name 'cairoint.h' ! -name 'cairo-*features*.h' | \
    xargs git diff ${LAST_RELEASE}.. --
```


## 5. Increment `CAIRO_VERSION_{MINOR|MICRO}` in `src/cairo-version.h`:

If there are backward-incompatible changes in the API, stop now and don't
release. Go back and fix the API instead. Cairo is intended to remain
backwards-compatible as far as API.

So `CAIRO_VERSION_MAJOR` will not be incremented unless we come up with a
new versioning scheme to take advantage of it.

If there are API additions, then increment `CAIRO_VERSION_MINOR` and reset
`CAIRO_VERSION_MICRO` to 0. **NOTE**: The minor version is only incremented
for releases, not for snapshots.

Otherwise, (i.e. there are only bug fixes), increment `CAIRO_VERSION_MICRO`
to the next larger (even) number.

## 6. For Minor releases, add an index entry to `doc/public/cairo-docs.xml`

Towards the end of the file, add a new section for the stable release.
It'll look something like:

```xml
<index id="api-index-X-Y">
  <title>Index of new symbols in X.Y</title>
  <xi:include href="xml/api-index-X.Y.xml"/>
</index>
```

## 7. Commit the changes to `NEWS` and `src/cairo-version.h`

It's especially important to mention the new version number in your commit
log.

## 8. Generate the release archive

The `meson dist` command will:

 * Generate the final tar file
 * Generate an sha1sum file

Once you have the release archive you will need to:

 * Sign the sha1sum using your GPG setup (asks for your GPG password)
 * `scp` the three files to appear on https://cairographics.org/releases
 * Generate a versioned manual and upload it to appear as both:
   - `https://cairographics.org/manual-${THIS_RELEASE}`
   - `https://cairographics.org/manual`
 * Place local copies of the three files in the releases directory
 * Create a `LATEST-{package_version}` file (after deleting any old one)
 * Tag the entire source tree with a `${THIS_RELEASE}` tag, and sign the
   tag with your GPG key (asks for your GPG password, and you may need to
   set `GIT_COMMITTER_NAME` and `GIT_COMMITTER_EMAIL` to match your
   public-key's setting or this fails.)
 * Provide some text for the release announcement (see below).

## 9. Update master (or the stable branch) version number.

For Micro releases (X.Y.Z+2), increment `CAIRO_VERSION_MICRO` to the next
larger (odd) number in `src/cairo-version.h`.

    DEVEL_VERSION="X.Y.Z+1"  # e.g. 1.15.10 -> 1.15.11

For Minor releases (X.Y.0), increment `CAIRO_VERSION_MINOR` to the next
larger (odd) number, and set `CAIRO_VERSION_MICRO` to 1.

    DEVEL_VERSION="X.Y.Z+1"  # e.g. 1.16.0 -> 1.17.1

Then, commit:

    git commit src/cairo-version.h -m "Bump version for ${DEVEL_VERSION}"

## 9. Push the new tag out

Use:

    git push --atomic origin master ${THIS_RELEASE}

to ensure that both the tag and the latest changes in your tree are uploaded
at the same time.

## 10. Send out an announcement message.

Send a message to cairo-announce@cairographics.org and CC
cairo@cairographics.org, and ftp-release@lists.freedesktop.org (pr@lwn.net
as well for major releases) to announce the release, adding the excerpt from
NEWS and the shortlog of all changes since last release, generated by:

    git shortlog ${LAST_RELEASE}...

## 11. Add the announcement to the website

Add a new entry in the `news` directory.  It will automatically get indexed
onto the homepage when the site refreshes.
