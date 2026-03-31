#import <Foundation/Foundation.h>
#import <OpenDirectory/NSOpenDirectory.h>
#import <OpenDirectory/OpenDirectory.h>
#import <DirectoryService/DirectoryService.h>
#import <unistd.h>

uint32_t    gTestCase       = 0;
BOOL        gLogErrors      = NO;
BOOL        gVerbose        = NO;
char        *gLogPath       = NULL;
char        *gAdminAccount  = NULL;
char        *gAdminPassword = NULL;
char        *gProxyHost     = NULL;
char        *gProxyPassword = NULL;
char        *gProxyUser     = NULL;
uint32_t    gTestTimes      = 1;
uint32_t    gCopies         = 1;

static void usage( char *argv[] )
{
    fprintf( stderr, "Usage:  TestAppCocoa -a adminPass -p adminPass [-v] [-e] [-l path] [-N runNumber]\n", argv[0] );
    fprintf( stderr, "                 [-c copies] [-t times] [-u proxyUser -P proxyPass -h proxyHost]\n" );
    fprintf( stderr, "             -v  verbose output\n" );
    fprintf( stderr, "             -e  output error log\n" );
    fprintf( stderr, "             -l  log path (default \"./Logs/<runNumber>/\"\n" );
    fprintf( stderr, "             -N  test run number\n" );
    fprintf( stderr, "             -c  number of copies to fork for stress testing\n" );
    fprintf( stderr, "             -t  number of times to run the test\n" );
    fprintf( stderr, "             -u  Proxy username\n" );
    fprintf( stderr, "             -P  Proxy username password\n" );
    fprintf( stderr, "             -h  Proxy host\n" );
    fprintf( stderr, "             -a  local admin account\n" );
    fprintf( stderr, "             -p  local admin password\n" );
}

static void parseOptions( int argc, char *argv[] )
{
    int                 ch;
    
    gTestCase = (uint32_t) CFAbsoluteTimeGetCurrent();
    
    while ((ch = getopt(argc, argv, "vel:N:c:t:u:P:h:a:p:")) != -1)
    {
        switch (ch)
        {
            case 'v':
                gVerbose = YES;
                break;
            case 'e':
                gLogErrors = YES;
                break;
            case 'l':
                gLogPath = optarg;
                break;
            case 'N':
                gTestCase = strtol( optarg, NULL, 10 );
                break;
            case 'c':
                if( NULL != optarg )
                {
                    gCopies = strtol( optarg, NULL, 10 );
                    if( gCopies > 1024 )
                        gCopies = 1024;
                }
                else
                    usage( argv );
                break;
            case 't':
                if( NULL != optarg )
                {
                    gTestTimes = strtol( optarg, NULL, 10 );
                    if( 0 == gTestTimes )
                        gTestTimes = 1;
                }
                else
                    usage( argv );
                break;
            case 'u':   // proxy user
                gProxyUser = optarg;
                break;
            case 'P':   // proxy password
                gProxyPassword = optarg;
                break;
            case 'h':   // host
                gProxyHost = optarg;
                break;
            case 'a':   // admin
                gAdminAccount = optarg;
                break;
            case 'p':   // local admin password
                gAdminPassword = optarg;
                break;
            case '?':
            default:
                usage( argv );
                exit(1);
        }
    }
    
    if( NULL == gAdminAccount || NULL == gAdminPassword )
    {
        usage( argv );
        exit( 1 );
    }
    else if( NULL != gProxyHost || NULL != gProxyUser || NULL != gProxyHost  )
    {
        if( NULL == gProxyHost || NULL == gProxyUser || NULL == gProxyHost  )
        {
            usage( argv );
            exit( 1 );
        }
    }
}

int main( int argc, char *argv[] )
{
    NSAutoreleasePool   *pool               = [NSAutoreleasePool new];
    ODSession           *sessionRef         = nil;
    ODNode              *nodeRef            = nil;
    NSError             *error              = nil;
    uint32_t            ii;
    NSString            *proxyHost          = nil;
    NSString            *proxyUser          = nil;
    NSString            *proxyPassword      = nil;
    
    parseOptions( argc, argv );

    if( NULL != gProxyHost )
    {
        proxyHost       = [NSString stringWithUTF8String: gProxyHost];
        proxyUser       = [NSString stringWithUTF8String: gProxyUser];
        proxyPassword   = [NSString stringWithUTF8String: gProxyPassword];
    }
    
    NSString    *adminAccount   = [NSString stringWithUTF8String: gAdminAccount];
    NSString    *adminPassword  = [NSString stringWithUTF8String: gAdminPassword];
    NSString    *dstestAccount  = [NSString stringWithUTF8String: "dstest"];
    
    argc -= optind;
    argv += optind;    
	
    for( ii = 0; ii < gTestTimes; ii++ )
    {
        if( NULL != gProxyHost )
        {
            NSDictionary    *options;
            
            options = [NSDictionary dictionaryWithObjectsAndKeys:   proxyHost, ODSessionProxyAddress,
                                                                    proxyUser, ODSessionProxyUsername,
                                                                    proxyPassword, ODSessionProxyPassword,
                                                                    NULL ];
            
            sessionRef = [[ODSession alloc] initWithOptions: options error: &error];
            if( nil != sessionRef )
            {
                printf("-[ODSession initWithOptions:] over Proxy - PASS\n");
                
                ODNode  *odNode = [ODNode nodeWithSession: sessionRef type: kODNodeTypeAuthentication error: &error];
                if( nil != odNode )
                {
                    NSArray *searchPolicy = [odNode subnodeNamesAndReturnError: &error];
                    
                    if( nil != searchPolicy )
                    {
                        printf( "-[ODNode subnodeNames] (kODTypeAuthenticationSearchNode) over Proxy - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODNode subnodeNames] (kODTypeAuthenticationSearchNode) over Proxy - FAIL (%s)\n", [[error description] UTF8String] );
                    }
                }
                else
                {
                    printf( "[ODNode nodeWithSession: type:] (kODTypeAuthenticationSearchNode) over Proxy - FAIL (%s)\n", [[error description] UTF8String] );
                }
                
                [sessionRef release];
                sessionRef = nil;
            }
            else
            {
                printf("-[ODSession initWithOptions:] over Proxy - FAIL (%s)\n", [[error description] UTF8String] );
            }
        }
        else
        {
            printf( "-[ODSession initWithOptions:] over Proxy - SKIP\n" );
        }
        
        sessionRef = [[ODSession alloc] initWithOptions: nil error: &error];
        if( nil != sessionRef )
        {
            printf( "-[ODSession initWithOptions: nil] - PASS\n" );
            
            NSArray *nodeNames = [sessionRef nodeNamesAndReturnError: nil];
            if( [nodeNames count] > 0 )
            {
                printf( "-[ODSession nodeNames:] returned results - PASS\n" );
            }
            else
            {
                printf( "-[ODSession nodeNames:] returned results - FAIL\n" );
            }
            
            ODNode  *odNode = [ODNode nodeWithSession: sessionRef type: kODNodeTypeAuthentication error: &error];
            if( nil != odNode )
            {
                ODQuery *pSearch = [ODQuery queryWithNode: odNode
                                           forRecordTypes: @kDSStdRecordTypeUsers
                                                attribute: @kDSNAttrRecordName
                                                matchType: kODMatchEqualTo
                                              queryValues: dstestAccount
                                         returnAttributes: nil
                                           maximumResults: 0
                                                    error: &error];
                
                if( nil != pSearch )
                {
                    printf( "-[ODQuery searchWithNode:forRecordTypes::::::] returns Object - PASS\n" );
                    
                    NSArray *results = [pSearch resultsAllowingPartial: NO error: &error];
                    
                    if( NULL != results )
                    {
                        printf( "-[ODQuery resultsForSearch:partialResults:] returned non-NULL - PASS\n" );
                        
                        if( [results count] != 0 ) 
                        {
                            printf( "-[ODQuery resultsForSearch:partialResults:] returned results - PASS\n" );
                        }
                        else
                        {
                            printf( "-[ODQuery resultsForSearch:partialResults:] returned results - FAIL (%s)\n", [[error description] UTF8String] );
                        }
                    }
                    else
                    {
                        printf( "-[ODQuery resultsForSearch:partialResults:outError:] returned non-NULL - FAIL (%s)\n", [[error description] UTF8String] );
                    }
                }

                NSArray *searchPolicy = [odNode subnodeNamesAndReturnError: &error];
                
                if( nil != searchPolicy )
                {
                    printf( "-[ODNode subnodeNames] (kODTypeAuthenticationSearchNode) - PASS\n" );
                }
                else
                {
                    printf( "-[ODNode subnodeNames] (kODTypeAuthenticationSearchNode) - FAIL (%s)\n", [[error description] UTF8String] );;
                }
                
                NSArray *unreachableSubnodes = [odNode unreachableSubnodeNamesAndReturnError: &error];
                
                if( nil != unreachableSubnodes)
                {
                    printf( "-[ODNode unreachableSubnodeNames] (kODTypeAuthenticationSearchNode) - FAIL - returned %d unreachable nodes\n", [unreachableSubnodes count] );
                }
                else
                {
                    printf( "-[ODNode unreachableSubNodeNames] (kODTypeAuthenticationSearchNode) - PASS\n" );
                }
            }
            else
            {
                printf( "[ODNode nodeWithSession: type:] (kODTypeAuthenticationSearchNode) - FAIL (%s)\n", [[error description] UTF8String] );
            }
            
            [sessionRef release];
            sessionRef = nil;
        }
        else
        {
            printf( "-[ODSession initWithOptions: nil] - FAIL (%s)\n", [[error description] UTF8String] );
        }
        
        nodeRef = [[ODNode alloc] initWithSession: [ODSession defaultSession] name: @"/LDAPv3/od.apple.com" error: &error];
        if( nil != nodeRef )
        {
            printf( "-[ODNode initWithSession:name:] with /LDAPv3/od.apple.com - PASS\n" );
            
            [nodeRef release];
            nodeRef = nil;
        }
        else
        {
            printf( "-[ODNode initWithSession:name:] with /LDAPv3/od.apple.com - FAIL (%s)\n", [[error description] UTF8String] );
        }
        
        nodeRef = [ODNode nodeWithSession: [ODSession defaultSession] name: @"/LDAPv3/od.apple.com" error: &error];
        if( nil != nodeRef )
        {
            printf( "-[ODNode nodeWithSession:name:] with /LDAPv3/od.apple.com - PASS\n" );

            ODNode *nodeRef2 = [nodeRef copy];
            if( nil != nodeRef2 )
            {
                printf( "-[ODNode copy] - PASS\n" );
                
                [nodeRef2 release];
                nodeRef2 = nil;
            }
            else
            {
                printf( "-[ODNode copy] - FAIL (%s)\n", [[error description] UTF8String] );
            }

            ODQuery *pSearch = [ODQuery queryWithNode: nodeRef
                                       forRecordTypes: @kDSStdRecordTypeUsers
                                            attribute: @kDSNAttrRecordName
                                            matchType: kODMatchEqualTo
                                          queryValues: dstestAccount
                                     returnAttributes: nil
                                       maximumResults: 0
												error: &error];
            
            if( nil != pSearch )
            {
                printf( "-[ODQuery searchWithNode:forRecordTypes::::::] returns Object - PASS\n" );
                
                NSArray *results = [pSearch resultsAllowingPartial: NO error: &error];
                
                if( NULL != results )
                {
                    printf( "-[ODQuery resultsForSearch:partialResults:] returned non-NULL - PASS\n" );
                    
                    if( [results count] != 0 ) 
                    {
                        printf( "-[ODQuery resultsForSearch:partialResults:] returned results - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODQuery resultsForSearch:partialResults:] returned results - FAIL (%s)\n", [[error description] UTF8String] );
                    }
                }
                else
                {
                    printf( "-[ODQuery resultsForSearch:partialResults:outError:] returned non-NULL - FAIL (%s)\n", [[error description] UTF8String] );
                }
            }
        }
        else
        {
            printf( "-[ODNode nodeWithSession:name:] with /LDAPv3/od.apple.com - FAIL (%s)\n", [[error description] UTF8String] );
        }
        
        nodeRef = (ODNode *)ODNodeCreateWithNodeType( kCFAllocatorDefault, kODSessionDefault, kODNodeTypeLocalNodes, (CFErrorRef *) &error );
        if( nil != nodeRef )
        {
            printf( "ODNodeCreateWithType with kODTypeLocalNode - PASS\n" );
            
            ODQuery *pSearch = [ODQuery queryWithNode: nodeRef
                                       forRecordTypes: @kDSStdRecordTypeUsers
                                            attribute: @kDSNAttrRecordName
                                            matchType: kODMatchEqualTo
                                          queryValues: adminAccount
                                     returnAttributes: nil
                                       maximumResults: 0
												error: &error];
            
            if( nil != pSearch )
            {
                printf( "-[ODQuery searchWithNode:forRecordTypes::::::] returns Object - PASS\n" );
                
                NSArray *results = [pSearch resultsAllowingPartial: NO error: &error];
                
                if( NULL != results )
                {
                    printf( "-[ODQuery resultsForSearch:partialResults:] returned non-NULL - PASS\n" );
                    
                    if( [results count] != 0 ) 
                    {
                        printf( "-[ODQuery resultsForSearch:partialResults:] returned results - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODQuery resultsForSearch:partialResults:] returned results - FAIL (%s)\n", [[error description] UTF8String] );
                    }
                }
                else
                {
                    printf( "-[ODQuery resultsForSearch:partialResults:outError:] returned non-NULL - FAIL (%s)\n", [[error description] UTF8String] );
                }
            }
            
            ODRecord *userRecord = [nodeRef recordWithRecordType: @kDSStdRecordTypeUsers name: adminAccount attributes: [NSArray arrayWithObjects: @kDSNAttrRecordName, @kDSNativeAttrTypePrefix "name", nil] error: &error];
            if( nil != userRecord )
            {
                printf( "-[ODNode recordWithRecordType: name: attributes:] - PASS\n" );
                
				NSDictionary *attributes = [userRecord recordDetailsForAttributes: [NSArray arrayWithObject: @kDSAttributesNativeAll] error: nil];
                if( [attributes count] > 0 )
                {
                    printf( "-[ODRecord recordDetailsForAttributes: @kDSAttributesNativeAll] - PASS (%d)\n", [attributes count] );
                }
                else
                {
                    printf( "-[ODRecord recordDetailsForAttributes: @kDSAttributesNativeAll] - FAIL\n" );
                }
                
				attributes = [userRecord recordDetailsForAttributes: [NSArray arrayWithObject: @kDSAttributesStandardAll] error: nil];
                if( [attributes count] > 0 )
                {
                    printf( "-[ODRecord recordDetailsForAttributes: @kDSAttributesStandardAll] - PASS (%d)\n", [attributes count] );
                }
                else
                {
                    printf( "-[ODRecord recordDetailsForAttributes: @kDSAttributesStandardAll] - FAIL\n" );
                }

				attributes = [userRecord recordDetailsForAttributes: [NSArray arrayWithObject: @kDSAttributesAll] error: nil];
                if( [attributes count] > 0 )
                {
                    printf( "-[ODRecord recordDetailsForAttributes: @kDSAttributesAll] - PASS (%d)\n", [attributes count] );
                }
                else
                {
                    printf( "-[ODRecord recordDetailsForAttributes: @kDSAttributesAll] - FAIL\n" );
                }
				
                NSDictionary *policy = [userRecord passwordPolicyAndReturnError: &error];
                if( nil != policy )
                {
                    printf( "-[ODRecord passwordPolicy] - PASS\n" );
                }
                else
                {
                    printf( "-[ODRecord passwordPolicy] - FAIL (%s)\n", [[error description] UTF8String] );
                }
                
                if( [userRecord verifyPassword: adminPassword error: &error] )
                {
                    printf( "-[ODRecord verifyPassword:] - PASS\n" );
                }
                else
                {
                    printf( "-[ODRecord verifyPassword:] - FAIL (%s)\n", [[error description] UTF8String] );
                }
                
                NSDictionary *values = [userRecord recordDetailsForAttributes: nil error: &error];
                if( nil != values )
                {
                    printf( "-[ODRecord recordDetailsForAttributes:] - PASS\n" );
                }
                else
                {
                    printf( "-[ODRecord recordDetailsForAttributes:] - FAIL (%s)\n", [[error description] UTF8String] );
                }
                
                NSArray *attribValues = [userRecord valuesForAttribute: @kDS1AttrNFSHomeDirectory error: &error];
                if( nil != attribValues )
                {
                    printf( "-[ODRecord valuesForAttribute:] - PASS\n" );
                }
                else
                {
                    printf( "-[ODRecord valuesForAttribute:] - FAIL (%s)\n", [[error description] UTF8String] );
                }
                
                ODRecord *group = [nodeRef recordWithRecordType: @kDSStdRecordTypeGroups name: @"admin" attributes: nil error: &error];
                if( nil != group )
                {
                    printf( "-[ODNode recordWithRecordType:name:attributes:] (admin) - PASS\n" );
                    
                    if( [group isMemberRecord: userRecord error: &error] == YES )
                    {
                        printf( "-[ODRecord isMemberRecord:] - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODRecord isMemberRecord:] - FAIL (%s)\n", [[error description] UTF8String] );
                    }
                }
                else
                {
                    printf( "-[ODNode openRecordType:recordName:] (admin) - FAIL (%s)\n", [[error description] UTF8String] );
                }
                
#warning needs ODNodeAuthenticateExtended
//                dsStatus = ODNodeAuthenticateExtended( dsNodeRef, CFSTR(kDSStdRecordTypeUsers), CFSTR(), inAuthItems, 
//                                                                    &outAuthItems, &dsContext );
//                if( eDSNoErr == dsStatus )
//                {
//                    printf( "ODNodeAuthenticateExtended - PASS\n" );
//                }
//                else
//                {
//                    printf( "ODNodeAuthenticateExtended - FAIL (%s)\n", [[error description] UTF8String] );
//                }
                
                if( [userRecord setNodeCredentials: adminAccount 
                                          password: adminPassword error: &error] )
                {
                    printf( "-[ODRecord setNodeCredentials:password:] - PASS\n" );
                }
                else
                {
                    printf( "-[ODRecord setNodeCredentials:password:] - FAIL (%s)\n", [[error description] UTF8String] );
                }
            }
            
            NSString *nodeName = [nodeRef nodeName];
            if( nil != nodeName )
            {
                printf( "-[ODNode nodeName] - PASS\n" );
            }
            else
            {
                printf( "-[ODNode nodeName] - FAIL (%s)\n", [[error description] UTF8String] );
            }
            
            NSDictionary *nodeInfo = [nodeRef nodeDetailsForKeys: nil error: &error];
            if( nil != nodeInfo )
            {
                printf( "-[ODNode nodeDetailsForKeys:] - PASS\n" );
            }
            else
            {
                printf( "-[ODNode nodeDetailsForKeys:] - FAIL (%s)\n", [[error description] UTF8String] );
            }
            
            NSArray *recTypes = [nodeRef supportedRecordTypesAndReturnError: &error];
            if( nil != recTypes )
            {
                printf( "-[ODNode supportedRecordTypes] - PASS\n" );
            }
            else
            {
                printf( "-[ODNode supportedRecordTypes] - FAIL (%s)\n", [[error description] UTF8String] );
            }
            
            NSArray *cfAttribTypes = [nodeRef supportedAttributesForRecordType: nil error: &error];
            if( nil != cfAttribTypes )
            {
                printf( "-[ODNode supportedAttributesForRecordType:] - PASS\n" );
            }
            else
            {
                printf( "-[ODNode supportedAttributesForRecordType:] - FAIL (%s)\n", [[error description] UTF8String] );
            }
                        
            if( [nodeRef setCredentialsWithRecordType: @kDSStdRecordTypeUsers 
                                           recordName: adminAccount
                                             password: adminPassword error: &error] )
            {
                printf( "-[ODNode setCredentialsWithRecordType:recordName:password:] - PASS\n" );
                
                ODRecord *record = [nodeRef createRecordWithRecordType: @kDSStdRecordTypeUsers name: @"TestCFFramework" attributes: nil error: &error];
                if( nil != record )
                {
                    printf( "-[ODNode createRecordWithRecordType:name:attributes:] - PASS\n" );
                        
                    if( [record deleteRecordAndReturnError: &error] )
                    {
                        printf( "-[ODRecord deleteRecord] - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODRecord deleteRecord] - FAIL (%s)\n", [[error description] UTF8String] );;
                    }
                }
                else
                {
                    printf( "[nodeRef createRecordWithRecordType:name:attributes:] - FAIL (%s)\n", [[error description] UTF8String] );
                }
                
                NSMutableDictionary     *attributes = [NSMutableDictionary dictionary];
                
                [attributes setObject:[NSArray arrayWithObject:@"<home_url>afp://test/home</home_url>"] forKey: @kDSNAttrHomeDirectory];
                [attributes setObject:[NSArray arrayWithObject:@"/Users/blah"] forKey: @kDS1AttrNFSHomeDirectory];
                [attributes setObject:[NSArray arrayWithObjects: @"TestAddRecord", @"TestAddRecordAlias", nil] forKey:@kDSNAttrRecordName];
                
                record = [nodeRef createRecordWithRecordType: @kDSStdRecordTypeUsers name: @"TestAddRecord" attributes: attributes error: &error];
                if( nil != record )
                {
                    printf( "-[ODNode addRecordWithRecordType:attributes:resultRecord:] (record returned) - PASS\n" );
                    
                    NSArray *value = [record valuesForAttribute: @kDS1AttrNFSHomeDirectory error: &error];
                    if( nil != value && [value count] == 1 )
                    {
                        printf( "-[ODRecord valuesForAttribute:kDS1AttrNFSHomeDirectory] (value exist) - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODRecord valuesForAttribute:kDS1AttrNFSHomeDirectory] (value exist) - FAIL (%s)\n", [[error description] UTF8String] );
                    }
                    
                    value = [record valuesForAttribute: @kDSNAttrHomeDirectory error: &error];
                    if( nil != value && [value count] == 1 )
                    {
                        printf( "-[ODRecord valuesForAttribute:kDSNAttrHomeDirectory] (value exist) - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODRecord valuesForAttribute:kDSNAttrHomeDirectory] (value exist) - FAIL (%s)\n", [[error description] UTF8String] );
                    }
                    
                    if( [record changePassword: nil toPassword: @"test" error: &error] )
                    {
                        printf( "-[ODNode changePassword:toPassword:] - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODNode changePassword:toPassword:] - FAIL (%s)\n", [[error description] UTF8String] );;
                    }
                    
                    if( [record changePassword: @"test" toPassword: @"test2" error: &error] )
                    {
                        printf( "-[ODNode changePassword:toPassword:] - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODNode changePassword:toPassword:] - FAIL (%s)\n", [[error description] UTF8String] );;
                    }
                    
                    if( [record setValue: @"Test street" forAttribute: @kDSNAttrState error: &error] )
                    {
                        printf( "-[ODRecord setValues:forAttribute:] - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODRecord setValues:forAttribute:] - FAIL (%s)\n", [[error description] UTF8String] );;
                    }
                    
                    if( [record addValue: @"Test street 2" toAttribute: @kDSNAttrState error: &error] )
                    {
                        printf( "-[ODRecord addValue:toAttribute:] - PASS\n" );
                    }
                    else
                    {
                        printf( "[ODRecord addValue:toAttribute:] - FAIL (%s)\n", [[error description] UTF8String] );;
                    }
                    
                    if( [record removeValue: @"Test street 2" fromAttribute: @kDSNAttrState error: &error] )
                    {
                        printf( "-[ODRecord removeValue:fromAttribute:] - PASS\n" );
                    }
                    else
                    {
                        printf( "-[ODRecord removeValue:fromAttribute:] - FAIL (%s)\n", [[error description] UTF8String] );;
                    }
                    
                    if( [record removeValuesForAttribute: @kDSNAttrState error: &error] )
                    {
                        printf( "-[ODRecord removeValuesForAttribute:] (deleting attrib) - PASS\n" );
                        
                        NSArray *values = [record valuesForAttribute: @kDSNAttrState error: &error];
                        if( nil == values || [values count] == 0 )
                        {
                            printf( "-[ODRecord removeValuesForAttribute:] (attrib gone) - PASS\n" );
                        }
                        else
                        {
                            printf( "-[ODRecord removeValuesForAttribute:] (attrib gone) - FAIL (%s)\n", [[error description] UTF8String] );
                        }
                    }
                    else
                    {
                        printf( "-[ODRecord removeValuesForAttribute:] (deleting attrib) - FAIL (%s)\n", [[error description] UTF8String] );
                    }
                    
                    // Try adding this record to group admin
                    ODRecord *group = [nodeRef createRecordWithRecordType: @kDSStdRecordTypeGroups name: @"TestGroup" attributes: nil error: &error];
                    if( nil != group )
                    {
                        if( [group addMemberRecord: record error: &error] )
                        {
                            printf( "-[ODRecord addMemberRecord:] - PASS\n" );
                            
                            if( [group removeMemberRecord: record error: &error] )
                            {
                                printf( "-[ODRecord removeRecordFromGroup:] - PASS\n" );
                            }
                            else
                            {
                                printf( "-[ODRecord removeRecordFromGroup:] - FAIL (%s)\n", [[error description] UTF8String] );;
                            }
                        }
                        else
                        {
                            printf( "[ODRecord addMemberRecord:] - FAIL (%s)\n", [[error description] UTF8String] );;
                        }
                        
                        [group deleteRecordAndReturnError: &error];
                    }
                    else
                    {
                        printf( "-[ODNode createRecordWithRecordType:name:attributes:] (Group) - FAIL (%s)\n", [[error description] UTF8String] );;
                    }
                    
                    [record deleteRecordAndReturnError: &error];
                }
                else
                {
                    printf( "-[ODNode addRecordWithRecordType:attributes:resultRecord] - FAIL (%s)\n", [[error description] UTF8String] );;
                    printf( "--- Other tests fail due to this failure\n" );
                }
            }
            else
            {
                printf( "-[ODNode setCredentialsWithRecordType:recordName:password:] - FAIL (%s)\n", [[error description] UTF8String] );
            }
            
            [nodeRef release];
            nodeRef = nil;
        }
        else
        {
            printf( "ODNodeCreateWithType with kODTypeLocalNode - FAIL (%s)\n", [[error description] UTF8String] );
        }
    }
    
    [pool release];
    
    return 0;
}
