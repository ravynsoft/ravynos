#import <AppKit/NSDockTile.h>

@implementation NSDockTile

-initWithOwner:owner {
   _owner=owner;
   _size=NSZeroSize;
   return self;
}

-(void)dealloc {
   [_badgeLabel release];
   [_contentView release];
   [super dealloc];
}

-(NSSize)size {
   return _size;
}

-owner {
   return _owner;
}

-(NSString *)badgeLabel {
   return _badgeLabel;
}

-(NSView *)contentView {
   return _contentView;
}

-(BOOL)showsApplicationBadge {
   return _showsApplicationBadge;
}

-(void)setBadgeLabel:(NSString *)value {
   value=[value copy];
   [_badgeLabel release];
   _badgeLabel=value;
}

-(void)setContentView:(NSView *)view {
   view=[view retain];
   [_contentView release];
   _contentView=view;
}

-(void)setShowsApplicationBadge:(BOOL)value {
   _showsApplicationBadge=value;
}

-(void)display {
}

@end
