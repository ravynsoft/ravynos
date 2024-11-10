#import <AppKit/AppKit.h>

int main(int argc, const char **argv) {
    __NSInitializeProcess(argc, argv);

    mach_port_t tmp;
    int rc = bootstrap_look_up(bootstrap_port, "com.ravynos.WindowServer", &tmp);
    if(rc != KERN_SUCCESS)
        NSLog(@"Failed to locate WindowServer port: rc=%d", rc);

    return rc;
}
