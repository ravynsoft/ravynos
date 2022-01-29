packer {
    required_plugins {
        googlecompute = {
            version = ">= 0.0.1"
            source = "github.com/hashicorp/googlecompute"
        }
    }
}

source "googlecompute" "airyxbuild" {
  disk_size               = "40"
  project_id              = "airyxos-images"
  source_image_family            = "freebsd-13-0"
  source_image_project_id       = ["freebsd-org-cloud-dev"]
  #source_image            = "airyxbuild-0-4-b1"
  ssh_username            = "packer"
  zone                    = "us-central1-a"
  image_name = "airyxbuild-0-4-b5"
}

build {
  sources = ["source.googlecompute.airyxbuild"]
  provisioner "shell" {
      inline = [
          "echo Installing latest kernel and base artifacts",
          "sudo fetch -o /tmp/kernel.txz https://dl.cloudsmith.io/public/airyx/13_0/raw/names/kernel_main.txz/files/kernel.txz",
          "sudo fetch -o /tmp/base.txz https://dl.cloudsmith.io/public/airyx/13_0/raw/names/base_main.txz/files/base.txz",
          "sudo chflags -R 0 /",
          "sudo tar -C / -xf /tmp/kernel.txz",
          "echo 'mach_load=\"YES\"'|sudo tee /boot/loader.conf",
          "echo 'cryptodev_load=\"YES\"'|sudo tee /boot/loader.conf",
          "sudo tar -C / -xf /tmp/base.txz ./etc/pkg/Airyx.conf",
          "sudo rm -f /etc/pkg/FreeBSD.conf",
          "sudo pkg update && sudo pkg install -yf git-tiny cmake bash dbus dbus-glib expat fontconfig freetype2 gdk-pixbuf2 gettext-runtime gettext-tools glib glib-networking jpeg-turbo kf5-kcoreaddons mesa-libs mesa-dri pkgconf py38-pip python3 py38-setuptools qt5-buildtools qt5-qmake libqtxdg qt5-x11extras sqlite3 tiff png kf5-plasma-framework",
          "sudo tar -C / -xf /tmp/base.txz --exclude=./etc/*passwd --exclude=./etc/*pwd.db --exclude=./boot/efi*",
          "echo Finished provisioning"
      ]
  }
}
