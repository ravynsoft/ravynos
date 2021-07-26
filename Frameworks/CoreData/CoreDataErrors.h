#import <Foundation/NSString.h>
#import <CoreData/CoreDataExports.h>

COREDATA_EXPORT NSString *const NSAffectedStoresErrorKey;
COREDATA_EXPORT NSString *const NSDetailedErrorsKey;

enum {
    NSPersistentStoreInvalidTypeError = 134000,
    NSPersistentStoreTypeMismatchError = 134010,
    NSPersistentStoreIncompatibleSchemaError = 134020,
    NSPersistentStoreSaveError = 134030,
    NSPersistentStoreIncompleteSaveError = 134040,
    NSPersistentStoreOperationError = 134070,
    NSPersistentStoreOpenError = 134080,
    NSPersistentStoreTimeoutError = 134090,
    NSPersistentStoreIncompatibleVersionHashError = 134100,
};
