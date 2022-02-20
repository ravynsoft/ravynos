// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>

#include <memory>
#include <sys/mman.h>

namespace {

class WaylandClient {
 public:
  static WaylandClient* Create() {
    std::unique_ptr<WaylandClient> client(new WaylandClient);
    if (client->Initialize())
      return client.release();
    return nullptr;
  }

  ~WaylandClient() {
    wl_pointer_destroy(pointer_);
    wl_seat_destroy(seat_);
    wl_shell_destroy(shell_);
    wl_shm_destroy(shm_);
    wl_compositor_destroy(compositor_);
    wl_display_disconnect(display_);
  }

  bool running() const { return running_; }
  struct wl_compositor* compositor() const {
    return compositor_;
  }
  struct wl_display* display() const {
    return display_;
  }
  struct wl_shell* shell() const {
    return shell_;
  }
  struct wl_shm* shm() const {
    return shm_;
  }

 private:
  WaylandClient() {}
  WaylandClient(const WaylandClient&) = delete;
  void operator=(const WaylandClient&) = delete;

  bool Initialize() {
    display_ = wl_display_connect(nullptr);
    if (!display_) {
      ::perror("Cannot open display");
      return false;
    }

    struct wl_registry* registry = wl_display_get_registry(display_);
    const struct wl_registry_listener registry_listener = {
        WaylandClient::OnRegistry, WaylandClient::OnRemoveRegistry};
    wl_registry_add_listener(registry, &registry_listener, this);
    wl_display_roundtrip(display_);
    wl_registry_destroy(registry);
    running_ = true;

    return true;
  }

  static void OnRemoveRegistry(void* a, struct wl_registry* b, uint32_t c) {}

  static void OnRegistry(void* data,
                         struct wl_registry* registry,
                         uint32_t name,
                         const char* interface,
                         uint32_t version) {
    WaylandClient* me = static_cast<WaylandClient*>(data);
    if (!strcmp(interface, wl_compositor_interface.name)) {
      me->compositor_ = static_cast<wl_compositor*>(
          wl_registry_bind(registry, name, &wl_compositor_interface, version));
    } else if (!strcmp(interface, wl_shm_interface.name)) {
      me->shm_ = static_cast<wl_shm*>(
          wl_registry_bind(registry, name, &wl_shm_interface, version));
    } else if (!strcmp(interface, wl_shell_interface.name)) {
      me->shell_ = static_cast<wl_shell*>(
          wl_registry_bind(registry, name, &wl_shell_interface, version));
    } else if (!strcmp(interface, wl_seat_interface.name)) {
      me->seat_ = static_cast<wl_seat*>(
          wl_registry_bind(registry, name, &wl_seat_interface, version));
      me->pointer_ = wl_seat_get_pointer(me->seat_);
      const struct wl_pointer_listener pointer_listener = {
          OnPointerEnter, OnPointerLeave, OnPointerMotion, OnPointerButton,
          OnPointerAxis};
      wl_pointer_add_listener(me->pointer_, &pointer_listener, data);
    }
  }

  static void OnPointerEnter(void* data,
                             struct wl_pointer* wl_pointer,
                             uint32_t serial,
                             struct wl_surface* surface,
                             wl_fixed_t surface_x,
                             wl_fixed_t surface_y) {}
  static void OnPointerLeave(void* data,
                             struct wl_pointer* wl_pointer,
                             uint32_t serial,
                             struct wl_surface* wl_surface) {}

  static void OnPointerMotion(void* data,
                              struct wl_pointer* wl_pointer,
                              uint32_t time,
                              wl_fixed_t surface_x,
                              wl_fixed_t surface_y) {}

  // Program exits if clicking any mouse buttons.
  static void OnPointerButton(void* data,
                              struct wl_pointer* wl_pointer,
                              uint32_t serial,
                              uint32_t time,
                              uint32_t button,
                              uint32_t state) {
    WaylandClient* me = static_cast<WaylandClient*>(data);
    me->running_ = false;
  }

  static void OnPointerAxis(void* data,
                            struct wl_pointer* wl_pointer,
                            uint32_t time,
                            uint32_t axis,
                            wl_fixed_t value) {}

  bool running_ = false;
  struct wl_compositor* compositor_ = nullptr;
  struct wl_display* display_ = nullptr;
  struct wl_pointer* pointer_ = nullptr;
  struct wl_seat* seat_ = nullptr;
  struct wl_shell* shell_ = nullptr;
  struct wl_shm* shm_ = nullptr;
};

class DemoWindow {
 public:
  DemoWindow() {}
  ~DemoWindow() {
    wl_buffer_destroy(buffer_);
    wl_shell_surface_destroy(shell_surface_);
    wl_surface_destroy(surface_);
  }

  int Run() {
    client_.reset(WaylandClient::Create());
    if (!client_) {
      ::perror("Cannot create WaylandClient");
      return EXIT_FAILURE;
    }

    if (!CreateBuffer()) {
      ::perror("Creating a buffer failed");
      return EXIT_FAILURE;
    }

    if (!CreateSurface()) {
      ::perror("Creating a surface failed");
      return EXIT_FAILURE;
    }

    Redraw(this, nullptr, 0);
    int ret = 0;
    while (client_->running() && ret != -1)
      ret = wl_display_dispatch(client_->display());

    return EXIT_SUCCESS;
  }

 private:
  DemoWindow(const DemoWindow&) = delete;
  void operator=(const DemoWindow&) = delete;

  bool CreateBuffer() {
    int stride = WIDTH * 4;
    int size = stride * HEIGHT;
    ScopedFD fd(CreateAnonymousFile(size));
    if (fd.Get() < 0) {
      ::perror("Creating a buffer file failed");
      return false;
    }
    data_ =
        mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd.Get(), 0);
    if (data_ == MAP_FAILED) {
      ::perror("mmap failed");
      return false;
    }

    struct wl_shm_pool* pool =
        wl_shm_create_pool(client_->shm(), fd.Get(), size);
    buffer_ = wl_shm_pool_create_buffer(pool, 0, WIDTH, HEIGHT, stride,
                                        WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    return true;
  }

  class ScopedFD {
   public:
    explicit ScopedFD(int fd) : fd_(fd) {}
    ~ScopedFD() {
      if (auto_close_)
        close(fd_);
    }
    int Get() { return fd_; }
    int Release() {
      auto_close_ = false;
      return fd_;
    }

   private:
    ScopedFD(const ScopedFD&) = delete;
    void operator=(const ScopedFD&) = delete;

    const int fd_;
    bool auto_close_ = true;
  };

  int CreateAnonymousFile(off_t size) {
    static const std::string file_name( "/weston-shared-XXXXXX");
    const std::string path(getenv("XDG_RUNTIME_DIR"));
    if (path.empty()) {
      errno = ENOENT;
      return -1;
    }

    std::string name (path + file_name);
    char* c_name = const_cast<char*>(name.data());
    ScopedFD fd(mkstemp(c_name));
    if (fd.Get() < 0)
      return -1;

    long flags = fcntl(fd.Get(), F_GETFD);
    if (flags == -1)
      return -1;
    if (fcntl(fd.Get(), F_SETFD, flags | FD_CLOEXEC) == -1)
      return -1;
    if (ftruncate(fd.Get(), size) < 0)
      return -1;

    return fd.Release();
  }

  bool CreateSurface() {
    surface_ = wl_compositor_create_surface(client_->compositor());
    if (!surface_)
      return false;

    shell_surface_ = wl_shell_get_shell_surface(client_->shell(), surface_);
    if (!shell_surface_) {
      wl_surface_destroy(surface_);
      return false;
    }

    wl_shell_surface_set_toplevel(shell_surface_);
    wl_shell_surface_set_user_data(shell_surface_, surface_);
    wl_surface_set_user_data(surface_, nullptr);
    return true;
  }

  static void Redraw(void* data, struct wl_callback* callback, uint32_t time) {
    DemoWindow* me = static_cast<DemoWindow*>(data);

    PaintPixels(me->data_, 20, WIDTH, HEIGHT, time);

    const int border_width = 20;
    wl_surface_attach(me->surface_, me->buffer_, 0, 0);
    wl_surface_damage(me->surface_, border_width, border_width,
                      WIDTH - 2 * border_width, HEIGHT - 2 * border_width);
    if (callback)
      wl_callback_destroy(callback);
    me->callback_ = wl_surface_frame(me->surface_);
    const struct wl_callback_listener frame_listener = {Redraw};
    wl_callback_add_listener(me->callback_, &frame_listener, me);
    wl_surface_commit(me->surface_);
  }

  // Copied from
  // https://cgit.freedesktop.org/wayland/weston/tree/clients/simple-shm.c
  // which is in MIT license.
  static void PaintPixels(void* image,
                          int padding,
                          int width,
                          int height,
                          uint32_t time) {
    uint32_t* pixel = static_cast<uint32_t*>(image);
    const int halfh = padding + (height - padding * 2) / 2;
    const int halfw = padding + (width - padding * 2) / 2;

    /* squared radii thresholds */
    int outr = (halfw < halfh ? halfw : halfh) - 8;
    int inr = outr - 32;
    outr *= outr;
    inr *= inr;

    pixel += padding * width;
    for (int y = padding; y < height - padding; y++) {
      int y2 = (y - halfh) * (y - halfh);
      pixel += padding;
      for (int x = padding; x < width - padding; x++) {
        uint32_t v;

        /* squared distance from center */
        int r2 = (x - halfw) * (x - halfw) + y2;

        if (r2 < inr)
          v = (r2 / 32 + time / 64) * 0x0080401;
        else if (r2 < outr)
          v = (y + time / 32) * 0x0080401;
        else
          v = (x + time / 16) * 0x0080401;
        v &= 0x00ffffff;

        /* cross if compositor uses X from XRGB as alpha */
        if (abs(x - y) > 6 && abs(x + y - height) > 6)
          v |= 0xff000000;

        *pixel++ = v;
      }

      pixel += padding;
    }
  }

  static const unsigned WIDTH = 300;
  static const unsigned HEIGHT = 300;

  std::unique_ptr<WaylandClient> client_;
  struct wl_shell_surface* shell_surface_ = nullptr;
  struct wl_surface* surface_ = nullptr;
  struct wl_buffer* buffer_ = nullptr;
  void* data_ = nullptr;
  struct wl_callback* callback_ = nullptr;
};

}  // namespace

int main(int argc, char** argv) {
  std::unique_ptr<DemoWindow> window(new DemoWindow);
  return window->Run();
}

