#include <IOKit/IOInterrupts.h>
#include <IOKit/IOService.h>
#include <IOKit/IOPlatformExpert.h>

class IOFramebuffer : public IOService
{
    OSDeclareDefaultStructors(IOFramebuffer);

private:
    void   *fbBase;
    UInt32  width;
    UInt32  height;
    UInt32  pitch;
    UInt32  bpp;

public:
    IOService * probe(IOService * provider, SInt32 * score) override;
    virtual bool start(IOService * provider) override;
    virtual void stop(IOService * provider) override;
    virtual void * getBaseAddress() ;
    virtual uint32_t getWidth() ;
    virtual uint32_t getHeight() ;
    virtual uint32_t getPitch() ;
    virtual uint32_t getDepth() ;

#if 0
    virtual IOReturn enableController() override;

    virtual const char * getPixelFormats() override;
    virtual IOReturn getCurrentDisplayMode(IODisplayModeID * displayMode,
                                           IOIndex * depth) override;

    virtual IOReturn setDisplayMode(IODisplayModeID displayMode,
                                    IOIndex depth) override;

    virtual IODeviceMemory * getApertureRange(IOPixelAperture aperture) override;

    virtual IOReturn getInformationForDisplayMode(
        IODisplayModeID displayMode,
        IODisplayModeInformation * info) override;

    virtual UInt64 getPixelFormatsForDisplayMode(
        IODisplayModeID displayMode,
        IOIndex depth) override;

    virtual IOReturn getPixelInformation(
        IODisplayModeID displayMode, IOIndex depth,
        IOPixelAperture aperture, IOPixelInformation * info ) override;

    virtual IOReturn getDisplayModes(IODisplayModeID * allDisplayModes) override;

    virtual IOItemCount getDisplayModeCount( void ) override;
#endif
};
