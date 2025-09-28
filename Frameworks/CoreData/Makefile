FRAMEWORK=CoreData
.include "../framework.common.mk"

SRCS= \
CoreDataErrors.m \
NSAtomicStore.m \
NSAtomicStoreCacheNode.m \
NSAttributeDescription.m \
NSEntityDescription.m \
NSFetchRequest.m \
NSFetchedPropertyDescription.m \
NSInMemoryPersistentStore.m \
NSManagedObject.m \
NSManagedObjectContext.m \
NSManagedObjectID.m \
NSManagedObjectModel.m \
NSManagedObjectMutableSet.m \
NSManagedObjectSet.m \
NSManagedObjectSetEnumerator.m \
NSPersistentStore.m \
NSPersistentStoreCoordinator.m \
NSPropertyDescription.m \
NSRelationshipDescription.m \
NSXMLPersistentStore.m 

INCS= \
CoreData.h \
CoreDataErrors.h \
CoreDataExports.h \
NSAtomicStore.h \
NSAtomicStoreCacheNode.h \
NSAttributeDescription.h \
NSEntityDescription-Private.h \
NSEntityDescription.h \
NSFetchRequest.h \
NSFetchedPropertyDescription.h \
NSInMemoryPersistentStore.h \
NSManagedObject-Private.h \
NSManagedObject.h \
NSManagedObjectContext-Private.h \
NSManagedObjectContext.h \
NSManagedObjectID-Private.h \
NSManagedObjectID.h \
NSManagedObjectModel.h \
NSManagedObjectMutableSet.h \
NSManagedObjectSet.h \
NSManagedObjectSetEnumerator.h \
NSPersistentStore.h \
NSPersistentStoreCoordinator-Private.h \
NSPersistentStoreCoordinator.h \
NSPropertyDescription.h \
NSRelationshipDescription.h \
NSXMLPersistentStore.h

RESOURCES=Info.plist English.lproj
CFLAGS+= ${FMWK_CFLAGS} -I${.CURDIR}/.. -I.. -I../libobjc2 -I../Foundation/Headers

.include <rvn.framework.mk>

installresources: std_install_hook
