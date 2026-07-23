#ifndef _IOKIT_IOREPORTER_H
#define _IOKIT_IOREPORTER_H

#include <IOKit/IOService.h>
#include <IOKit/IOReportTypes.h>

typedef struct {
    uint64_t base_bucket_width;
    uint32_t scale_flag;
    uint32_t segment_idx;
    uint32_t segment_bucket_count;
} IOHistogramSegmentConfig;

class IOReporter : public OSObject {};

class IOHistogramReporter : public IOReporter {
public:
    static IOHistogramReporter *with(IOService *, IOReportCategories, uint64_t,
                                     const char *, IOReportUnit, uint32_t,
                                     IOHistogramSegmentConfig *)
    {
        return NULL;
    }

    IOReturn configureReport(IOReportChannelList *, IOReportConfigureAction,
                             void *, void *)
    {
        return kIOReturnSuccess;
    }

    IOReturn updateReport(IOReportChannelList *, IOReportUpdateAction,
                          void *, void *)
    {
        return kIOReturnSuccess;
    }

    int tallyValue(int64_t)
    {
        return 0;
    }
};

class IOReportLegend {
public:
    static IOReturn addReporterLegend(IOService *, IOReporter *, const char *,
                                      const char *)
    {
        return kIOReturnSuccess;
    }
};

#endif /* _IOKIT_IOREPORTER_H */
