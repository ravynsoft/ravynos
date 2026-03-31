
#ifndef PGO_H
#define PGO_H

void pgo_start_thread(OSKextRef kext);

bool pgo_scan_kexts(CFArrayRef kexts);

#endif /* PGO_H */
