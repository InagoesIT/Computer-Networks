-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA1

Format: 3.0 (quilt)
Source: totem
Binary: libtotem0, totem, totem-common, totem-plugins, gir1.2-totem-1.0, libtotem-dev
Architecture: any all
Version: 3.34.1-2ubuntu2
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Uploaders: Jeremy Bicha <jbicha@debian.org>, Laurent Bigonville <bigon@debian.org>
Homepage: https://wiki.gnome.org/Apps/Videos
Standards-Version: 4.3.0
Vcs-Browser: https://git.launchpad.net/~ubuntu-desktop/ubuntu/+source/totem
Vcs-Git: https://git.launchpad.net/~ubuntu-desktop/ubuntu/+source/totem
Build-Depends: appstream-util, debhelper-compat (= 12), dh-sequence-gir, dh-sequence-gnome, dh-sequence-python3, dpkg-dev (>= 1.16.1), gsettings-desktop-schemas-dev, gstreamer1.0-plugins-base (>= 1.6.0), gstreamer1.0-plugins-good, gstreamer1.0-tools, gtk-doc-tools (>= 1.14), itstool, libatk1.0-dev (>= 1.29.4), libbluetooth-dev [linux-any], libcairo2-dev (>= 1.14.0), libclutter-gst-3.0-dev, libclutter-gtk-1.0-dev (>= 1.8.1), libclutter-1.0-dev (>= 1.17.3), libgdata-dev (>= 0.8.0), libgdk-pixbuf2.0-dev (>= 2.23.0), libgirepository1.0-dev (>= 0.9.12), libglib2.0-dev (>= 2.35.0), libgnome-desktop-3-dev, libgrilo-0.3-dev (>= 0.3.0), libgstreamer-plugins-base1.0-dev, libgstreamer1.0-dev (>= 1.6.0), libgtk-3-dev (>= 3.19.4), libpeas-dev (>= 1.1.0), librsvg2-dev (>= 2.16.0-2), libsoup2.4-dev, libtotem-plparser-dev (>= 3.10.1), libxml2-dev (>= 2.6.0), meson (>= 0.49.0), python3 (>= 3.2), python-gi-dev (>= 2.90.3), shared-mime-info (>= 0.22), xauth <!nocheck>, xvfb <!nocheck>
Package-List:
 gir1.2-totem-1.0 deb introspection optional arch=any
 libtotem-dev deb libdevel optional arch=any
 libtotem0 deb libs optional arch=any
 totem deb video optional arch=any
 totem-common deb video optional arch=all
 totem-plugins deb video optional arch=any
Checksums-Sha1:
 c8ec27f81c6163b86a84e466275342fb52a2f9e3 2157160 totem_3.34.1.orig.tar.xz
 ba06659f60e015bc32b3f1312593067c208d06fb 28380 totem_3.34.1-2ubuntu2.debian.tar.xz
Checksums-Sha256:
 6bf89ab096dfc564f260976e484165f09bd886434d184410d2f944bfba611a09 2157160 totem_3.34.1.orig.tar.xz
 786fc67b4b48bcc2ce905993b9d8c42c5f8c5365c125b908a8a4b8dfb501c6ac 28380 totem_3.34.1-2ubuntu2.debian.tar.xz
Files:
 7f8e99c8f59cf4e53f185cc6b0e7f017 2157160 totem_3.34.1.orig.tar.xz
 9cc92c95e42959494c1f3785dcde8e83 28380 totem_3.34.1-2ubuntu2.debian.tar.xz
Debian-Vcs-Browser: https://salsa.debian.org/gnome-team/totem
Debian-Vcs-Git: https://salsa.debian.org/gnome-team/totem.git
Original-Maintainer: Debian GNOME Maintainers <pkg-gnome-maintainers@lists.alioth.debian.org>

-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1

iEYEARECAAYFAl5U8hMACgkQQxo87aLX0pIjPACgwax5yMn5dihlO/X7ZeGRVVyl
J30An2X5n5/7t3sDCwn8uHg9etk7c4IZ
=S0ij
-----END PGP SIGNATURE-----
