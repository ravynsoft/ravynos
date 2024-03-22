#include "pan_pps_perf.h"

#include <lib/pan_device.h>
#include <perf/pan_perf.h>
#include <pps/pps.h>
#include <util/ralloc.h>

namespace pps {
PanfrostDevice::PanfrostDevice(int fd)
    : ctx{ralloc_context(nullptr)},
      dev{reinterpret_cast<struct panfrost_device *>(
         new struct panfrost_device())}
{
   assert(fd >= 0);
   panfrost_open_device(ctx, fd, dev);
}

PanfrostDevice::~PanfrostDevice()
{
   if (ctx) {
      panfrost_close_device(dev);
   }
   if (dev) {
      delete dev;
   }
}

PanfrostDevice::PanfrostDevice(PanfrostDevice &&o): ctx{o.ctx}, dev{o.dev}
{
   o.ctx = nullptr;
   o.dev = nullptr;
}

PanfrostDevice &
PanfrostDevice::operator=(PanfrostDevice &&o)
{
   std::swap(ctx, o.ctx);
   std::swap(dev, o.dev);
   return *this;
}

PanfrostPerf::PanfrostPerf(const PanfrostDevice &dev)
    : perf{reinterpret_cast<struct panfrost_perf *>(
         rzalloc(nullptr, struct panfrost_perf))}
{
   assert(perf);
   assert(dev.dev);
   panfrost_perf_init(perf, dev.dev);
}

PanfrostPerf::~PanfrostPerf()
{
   if (perf) {
      panfrost_perf_disable(perf);
      ralloc_free(perf);
   }
}

PanfrostPerf::PanfrostPerf(PanfrostPerf &&o): perf{o.perf}
{
   o.perf = nullptr;
}

PanfrostPerf &
PanfrostPerf::operator=(PanfrostPerf &&o)
{
   std::swap(perf, o.perf);
   return *this;
}

int
PanfrostPerf::enable() const
{
   assert(perf);
   return panfrost_perf_enable(perf);
}

void
PanfrostPerf::disable() const
{
   assert(perf);
   panfrost_perf_disable(perf);
}

int
PanfrostPerf::dump() const
{
   assert(perf);
   return panfrost_perf_dump(perf);
}

} // namespace pps
