#import <QuartzCore/CALayer.h>
#import <QuartzCore/CAAnimation.h>
#import <QuartzCore/CALayerContext.h>
#import <QuartzCore/CATransaction.h>
#import <Foundation/NSDictionary.h>

NSString * const kCAFilterLinear=@"linear";
NSString * const kCAFilterNearest=@"nearest";
NSString * const kCAFilterTrilinear=@"trilinear";

@implementation CALayer

+layer {
   return [[[self alloc] init] autorelease];
}


-(CALayerContext *)_context {
   return _context;
}

-(void)_setContext:(CALayerContext *)context {
   if(_context!=context){
    [_context deleteTextureId:_textureId];
    [_textureId release];
    _textureId=nil;
   }
   
   _context=context;
   [_sublayers makeObjectsPerformSelector:@selector(_setContext:) withObject:context];
}

-(CALayer *)superlayer {
   return _superlayer;
}

-(NSArray *)sublayers {
   return _sublayers;
}

-(void)setSublayers:(NSArray *)sublayers {
   sublayers=[sublayers copy];
   [_sublayers release];
   _sublayers=sublayers;
   [_sublayers makeObjectsPerformSelector:@selector(_setSuperLayer:) withObject:self];
   [_sublayers makeObjectsPerformSelector:@selector(_setContext:) withObject:_context];
}

-(id)delegate {
   return _delegate;
}

-(void)setDelegate:value {
   _delegate=value;
}

-(CGPoint)anchorPoint {
   return _anchorPoint;
}

-(void)setAnchorPoint:(CGPoint)value {
   _anchorPoint=value;
}

-(CGPoint)position {
   return _position;
}

-(void)setPosition:(CGPoint)value {
   CAAnimation *animation=[self animationForKey:@"position"];
      
   if(animation==nil && ![CATransaction disableActions]){
    id action=[self actionForKey:@"position"];
    
    if(action!=nil)
     [self addAnimation:action forKey:@"position"];
   }
   
   _position=value;
}

-(CGRect)bounds {
   return _bounds;
}

-(void)setBounds:(CGRect)value {
   CAAnimation *animation=[self animationForKey:@"bounds"];
   
   if(animation==nil && ![CATransaction disableActions]){
    id action=[self actionForKey:@"bounds"];

    if(action!=nil)
     [self addAnimation:action forKey:@"bounds"];
   }
   
   _bounds=value;
}

-(CGRect)frame {
   CGRect result;
   
   result.size=_bounds.size;
   result.origin.x=_position.x-result.size.width*_anchorPoint.x;
   result.origin.y=_position.y-result.size.height*_anchorPoint.y;
   
   return result;
}

-(void)setFrame:(CGRect)value {

   CGPoint position;
   
   position.x=value.origin.x+value.size.width*_anchorPoint.x;
   position.y=value.origin.y+value.size.height*_anchorPoint.y;
   
   [self setPosition:position];
   
   CGRect bounds=_bounds;

   bounds.size=value.size;
      
   [self setBounds:bounds];
}

-(float)opacity {
   return _opacity;
}

-(void)setOpacity:(float)value {
   CAAnimation *animation=[self animationForKey:@"opacity"];
   
   if(animation==nil && ![CATransaction disableActions]){
    id action=[self actionForKey:@"opacity"];

    if(action!=nil)
     [self addAnimation:action forKey:@"opacity"];
   }

   _opacity=value;
}

-(BOOL)opaque {
   return _opaque;
}

-(void)setOpaque:(BOOL)value {
   _opaque=value;
}

-(id)contents {
   return _contents;
}

-(void)setContents:(id)value {
   value=[value retain];
   [_contents release];
   _contents=value;
}

-(CATransform3D)transform {
   return _transform;
}

-(void)setTransform:(CATransform3D)value {
   _transform=value;
}

-(CATransform3D)sublayerTransform {
   return _sublayerTransform;
}

-(void)setSublayerTransform:(CATransform3D)value {
   _sublayerTransform=value;
}

-(NSString *)minificationFilter {
   return _minificationFilter;
}

-(void)setMinificationFilter:(NSString *)value {
   value=[value copy];
   [_minificationFilter release];
   _minificationFilter=value;
}

-(NSString *)magnificationFilter {
   return _magnificationFilter;
}

-(void)setMagnificationFilter:(NSString *)value {
   value=[value copy];
   [_magnificationFilter release];
   _magnificationFilter=value;
}

-init {
   _superlayer=nil;
   _sublayers=[NSArray new];
   _delegate=nil;
   _anchorPoint=CGPointMake(0.5,0.5);
   _position=CGPointZero;
   _bounds=CGRectZero;
   _opacity=1.0;
   _opaque=YES;
   _contents=nil;
   _transform=CATransform3DIdentity;
   _sublayerTransform=CATransform3DIdentity;
   _minificationFilter=kCAFilterLinear;
   _magnificationFilter=kCAFilterLinear;
   _animations=[[NSMutableDictionary alloc] init];
   return self;
}

-(void)dealloc {
   [_sublayers release];
   [_animations release];
   [_minificationFilter release];
   [_magnificationFilter release];
   [super dealloc];
}

-(void)_setSuperLayer:(CALayer *)parent {
   _superlayer=parent;
}

-(void)_removeSublayer:(CALayer *)child {
   NSMutableArray *layers=[_sublayers mutableCopy];
   [layers removeObjectIdenticalTo:child];
   [self setSublayers:layers];
   [layers release];
}

-(void)addSublayer:(CALayer *)layer {
   [self setSublayers:[_sublayers arrayByAddingObject:layer]];
}

-(void)replaceSublayer:(CALayer *)layer with:(CALayer *)other {
   NSMutableArray *layers=[_sublayers mutableCopy];
   NSUInteger      index=[_sublayers indexOfObjectIdenticalTo:layer];
   
   [layers replaceObjectAtIndex:index withObject:other];
   
   [self setSublayers:layers];
   [layers release];
   
   layer->_superlayer=nil;
}

-(void)display {
   if([_delegate respondsToSelector:@selector(displayLayer:)])
    [_delegate displayLayer:self];
   else {
#if 0


#warning create bitmap context

    [self drawInContext:context];
    _contents=image;
    [self setContents:image];
#endif
}
}

-(void)displayIfNeeded {
}

-(void)drawInContext:(CGContextRef)context {
   if([_delegate respondsToSelector:@selector(drawLayer:inContext:)])
    [_delegate drawLayer:self inContext:context];
}

-(BOOL)needsDisplay {
   return _needsDisplay;
}

-(void)removeFromSuperlayer {
   [_superlayer _removeSublayer:self];
   _superlayer=nil;
   [self _setContext:nil];
}

-(void)setNeedsDisplay {
   _needsDisplay=YES;
}

-(void)setNeedsDisplayInRect:(CGRect)rect {
   _needsDisplay=YES;
}

-(void)addAnimation:(CAAnimation *)animation forKey:(NSString *)key {
   if(_context==nil)
    return;
    
   [_animations setObject:animation forKey:key];
   [_context startTimerIfNeeded];
}

-(CAAnimation *)animationForKey:(NSString *)key {
   return [_animations objectForKey:key];
}

-(void)removeAllAnimations {
   [_animations removeAllObjects];
}

-(void)removeAnimationForKey:(NSString *)key {
   [_animations removeObjectForKey:key];
}

-(NSArray *)animationKeys {
   return [_animations allKeys];
}

-valueForKey:(NSString *)key {
// FIXME: KVC appears broken for structs

   if([key isEqualToString:@"bounds"])
    return [NSValue valueWithRect:_bounds];
   if([key isEqualToString:@"frame"])
    return [NSValue valueWithRect:[self frame]];
    
   return [super valueForKey:key];
}

-(id <CAAction>)actionForKey:(NSString *)key {
   CABasicAnimation *basic=[CABasicAnimation animationWithKeyPath:key];
   
   [basic setFromValue:[self valueForKey:key]];
   
   return basic;
}

-(NSNumber *)_textureId {
   return _textureId;
}

-(void)_setTextureId:(NSNumber *)value {
   value=[value copy];
   [_textureId release];
   _textureId=value;
}

@end
