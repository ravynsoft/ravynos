packer {
    required_plugins {
        googlecompute = {
            version = ">= 0.0.1"
            source = "github.com/hashicorp/googlecompute"
        }
        sshkey = {
            version = ">= 0.0.1"
            source = "github.com/ivoronin/sshkey"
        }
    }
}

source "googlecompute" "ravynbuild" {
  disk_size               = "80"
  project_id              = "ravyn-images"
  source_image            = "freebsd-14-0-current-amd64-v20220505"
  source_image_project_id       = ["freebsd-org-cloud-dev"]
  #source_image            = "ravynbuild-0-4-b2"
  ssh_username            = "packer"
  temporary_key_pair_type = "ed25519"
  zone                    = "us-central1-a"
  image_name = "ravynbuild-0-4-b10"
}

build {
  sources = ["source.googlecompute.ravynbuild"]
  provisioner "shell" {
      inline = [
          "echo Installing latest kernel and base artifacts",
          "sudo fetch -o /tmp/kernel.txz https://dl.cloudsmith.io/public/ravynsoft/ravynOS/raw/names/kernel_main.txz/files/kernel.txz",
          "sudo fetch -o /tmp/base.txz https://dl.cloudsmith.io/public/ravynsoft/ravynOS/raw/names/base_main.txz/files/base.txz",
          "sudo chflags -R 0 /",
          "sudo tar -C / -xf /tmp/kernel.txz",
          "echo 'mach_load=\"YES\"'|sudo tee /boot/loader.conf",
          "echo 'cryptodev_load=\"YES\"'|sudo tee /boot/loader.conf",
          "echo 'zfs_load=\"YES\"'|sudo tee /boot/loader.conf",
	  "echo 'init_path=\"/sbin/init\"'|sudo tee /boot/loader.conf",
          #"sudo tar -C / -xf /tmp/base.txz ./etc/pkg/ravynOS.conf",
          "sudo rm -f /etc/pkg/FreeBSD.conf",
          #"sudo -E IGNORE_OSVERSION=yes pkg update && sudo -E IGNORE_OSVERSION=yes pkg install -yf openssl git-tiny cmake bash dbus dbus-glib expat fontconfig freetype2 gdk-pixbuf2 gettext-runtime gettext-tools glib jpeg-turbo mesa-libs mesa-dri pkgconf py38-pip python3 py38-setuptools qt5-buildtools qt5-qmake libqtxdg sqlite3 tiff png",
          #"sudo rm -f /tmp/kernel.txz /var/cache/pkg/*",
          "sudo tar -C / -xf /tmp/base.txz --exclude=./etc/*passwd --exclude=./etc/*pwd.db --exclude=./boot/efi*",
          "echo Finished provisioning"
      ]
  }
}
