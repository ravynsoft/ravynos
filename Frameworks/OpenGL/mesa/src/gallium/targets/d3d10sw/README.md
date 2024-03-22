The resulting d3d10sw.dll implements D3D10's software rendering interface, like
WARP.


It can be used directly from WLK 1.6 and WHCK 2.0 D3D10+ tests, via the -Src
and -SWDLL options. For example:

    wgf11blend.exe -Debug -DoNotCatchExceptions -DXGI:1.1 -FeatureLevel:10.0 -Src:SW -SWDLL:d3d10sw.dll -LogClean -LogVerbose

However, as of WHCK version 2.1 this mechanism no longer works reliably.
Either you use WHCK 2.0 binaries, or you must use the alternative method
cribed below (of copying d3d10sw.dll into the executable directory and rename
it such that it matches the D3D10 UMD of the test machine).


Examples can be easily modified to load it too:

    D3D10CreateDeviceAndSwapChain(NULL,
                                  D3D10_DRIVER_TYPE_SOFTWARE,
                                  LoadLibraryA("d3d10sw"), /* Software */
                                  Flags,
                                  D3D10_SDK_VERSION,
                                  &SwapChainDesc,
                                  &g_pSwapChain,
                                  &g_pDevice);

    D3D11CreateDeviceAndSwapChain(NULL, /* pAdapter */
                                  D3D_DRIVER_TYPE_SOFTWARE,
                                  LoadLibraryA("d3d10sw"), /* Software */
                                  Flags,
                                  FeatureLevels,
                                  sizeof FeatureLevels / sizeof FeatureLevels[0],
                                  D3D11_SDK_VERSION,
                                  &SwapChainDesc,
                                  &g_pSwapChain,
                                  &g_pDevice,
                                  NULL, /* pFeatureLevel */
                                  &g_pDeviceContext); /* ppImmediateContext */


Alternatively, it can be renamed into the system's D3D10 UMD driver (e.g.,
vm3dum_10.dll for VMware vGPU, nvwgf2um.dll for NVIDIA GPU), and placed into
the application directory, or system's directory, and used instead.

For the DLL to be picked from the application directory you'll need to do the
following once:

    reg add "HKLM\System\CurrentControlSet\Control\Session Manager" /v "SafeDllSearchMode" /t REG_DWORD /d 0 /f

See also https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order
