packer {
    required_plugins {
        googlecompute = {
            version = ">= 0.0.1"
            source = "github.com/hashicorp/googlecompute"
        }
    }
}

source "googlecompute" "airyxbuild_1" {
  disk_size               = "40"
  project_id              = "airyxos-images"
  source_image            = "packer-1642357192"
  #source_image_project_id = ["freebsd-org-cloud-dev"]
  ssh_username            = "packer"
  zone                    = "us-central1-a"
}

build {
  sources = ["source.googlecompute.airyxbuild_1"]
  provisioner "shell" {
      inline = [
          "echo Installing latest kernel and base artifacts",
          "sudo fetch -o /tmp/base.txz https://dl.cloudsmith.io/public/airyx/core/raw/names/base_airyx.txz/files/base.txz",
          "sudo fetch -o /tmp/kernel.txz https://dl.cloudsmith.io/public/airyx/core/raw/names/kernel_airyx.txz/files/kernel.txz",
          "sudo chflags -R 0 /",
          "sudo tar -C / -xf /tmp/kernel.txz",
          "sudo tar -C / -xf /tmp/base.txz --exclude=./etc/passwd --exclude=./etc/master.passwd --exclude=./etc/group",
          "sudo rm -f /tmp/kernel.txz /tmp/base.txz",
          "echo Finished provisioning"
      ]
  }
}