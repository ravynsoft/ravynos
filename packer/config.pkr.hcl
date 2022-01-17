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
  source_image            = "packer-1642387643"
  ssh_username            = "packer"
  zone                    = "us-central1-a"
  image_name = "airyxbuild-0-4pre1"
}

build {
  sources = ["source.googlecompute.airyxbuild"]
  provisioner "shell" {
      inline = [
          "echo Installing latest kernel and base artifacts",
          "sudo fetch -o /tmp/kernel.txz https://dl.cloudsmith.io/public/airyx/core/raw/names/kernel_airyx.txz/files/kernel.txz",
          "sudo fetch -o /tmp/base.txz https://dl.cloudsmith.io/public/airyx/core/raw/names/base_airyx.txz/files/base.txz",
          "sudo chflags -R 0 /",
          "sudo tar -C / -xf /tmp/kernel.txz",
          "echo 'mach_load=\"YES\"'|sudo tee /boot/loader.conf",
          "sudo tar -C / -xf /tmp/base.txz --exclude=./etc/*passwd",
          "echo Finished provisioning"
      ]
  }
}