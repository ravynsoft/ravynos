#import <Onyx2D/O2ClipState.h>
#import <Onyx2D/O2ClipPhase.h>
#import <Onyx2D/O2Path.h>

@implementation O2ClipState

-init {
   _type=O2ClipStateTypeNone;
   _phases=nil;
   return self;
}

-(void)dealloc {
   O2PathRelease(_path);
   [_phases release];
   [super dealloc];
}

O2ClipStateType O2ClipStateGetType(O2ClipState *self) {
   return self->_type;
}

O2ClipState *O2ClipStateCreateCopy(O2ClipState *self) {
   O2ClipState *result=NSCopyObject(self,0,NULL);
   
   result->_path=O2PathCreateCopy(self->_path);
   result->_phases=[[NSMutableArray alloc] initWithArray:self->_phases];
   
   return result;
}

void O2ClipStateReset(O2ClipState *self) {
   self->_type=O2ClipStateTypeNone;
   O2PathRelease(self->_path);
   self->_path=nil;
   [self->_phases release];
   self->_phases=nil;
}

O2Path *O2ClipStateOnePath(O2ClipState *self) {
   return self->_path;
}

-(NSArray *)clipPhases {
   return _phases;
}

-(void)addPath:(O2Path *)path evenOdd:(BOOL)evenOdd {
   O2Rect       rect;
   BOOL         pathIsRect=O2PathIsRect(path,&rect);

   path=O2PathCreateCopy(path);

// FIXME: when all the context types can digest coalesced clip states we dont need to add
// the phases to the array unless needed

// FIXME: EVEN ODD IS WRONG, merging for even odd clip is wrong, should prob. be separate merges

   switch(_type){
   
    case O2ClipStateTypeNone:
     if(pathIsRect){
      _type=O2ClipStateTypeOneRect;
      _rect=rect;
     }
     else {
      _type=O2ClipStateTypeOnePath;
      _evenOdd=evenOdd;
      _path=O2PathCreateCopy(path);
     }
     break;
     
    case O2ClipStateTypeOneRect:
     if(pathIsRect)
      _rect=O2RectIntersection(_rect,rect);
     else {
      _type=O2ClipStateTypeOneRectOnePath;
      _evenOdd=evenOdd;
      _path=O2PathCreateCopy(path);
     }
     break;
    
    case O2ClipStateTypeOnePath:
     if(pathIsRect){
      _type=O2ClipStateTypeOneRectOnePath;
      _rect=rect;
     }
     else {
      _type=O2ClipStateTypeManyPaths;
     }
     break;
     
    case O2ClipStateTypeOneRectOnePath:
     if(pathIsRect)
      _rect=O2RectIntersection(_rect,rect);
     else
      _type=O2ClipStateTypeOneRectManyPaths;
     break;
     
    case O2ClipStateTypeOneRectManyPaths:
    case O2ClipStateTypeManyPaths:
    case O2ClipStateTypeManyPathsAndMasks:
     break;
   }

   O2ClipPhase *phase;
   
   if(evenOdd)
    phase=[[O2ClipPhase allocWithZone:NULL] initWithEOPath:path];
   else
    phase=O2ClipPhaseInitWithNonZeroPath([O2ClipPhase allocWithZone:NULL],path);
 
   if(_phases==nil)
    _phases=[[NSMutableArray alloc] init];
    
   [_phases addObject:phase];
   
   [phase release];
   
   O2PathRelease(path);
 }

-(void)addNonZeroWindingPath:(O2Path *)path {
   [self addPath:path evenOdd:NO];
}

-(void)addEvenOddWindingPath:(O2Path *)path {
   [self addPath:path evenOdd:YES];
}

-(void)addMask:(O2Image *)image inRect:(O2Rect)rect transform:(O2AffineTransform)transform {
   O2ClipPhase *phase=[[O2ClipPhase alloc] initWithMask:image rect:rect transform:transform];
   
   if(_phases==nil)
    _phases=[[NSMutableArray alloc] init];

   [_phases addObject:phase];

   [phase release];

   _type=O2ClipStateTypeManyPathsAndMasks;
}

@end
