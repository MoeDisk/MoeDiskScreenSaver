#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <ScreenSaver/ScreenSaver.h>

@interface MoeDiskScreenSaverView : ScreenSaverView
@end

@interface MoeDiskScreenSaverView ()
@property(nonatomic, strong) NSImage *sourceLogo;
@property(nonatomic, strong) NSImage *tintedLogo;
@property(nonatomic, strong) NSArray<NSColor *> *colors;
@property(nonatomic) NSInteger colorIndex;
@property(nonatomic) NSPoint position;
@property(nonatomic) NSPoint velocity;
@property(nonatomic) NSSize logoSize;
@property(nonatomic) NSSize lastBoundsSize;
@property(nonatomic, strong) CALayer *logoLayer;
@end

@implementation MoeDiskScreenSaverView

- (instancetype)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview
{
    self = [super initWithFrame:frame isPreview:isPreview];
    if (!self) {
        return nil;
    }
    self.animationTimeInterval = 1.0 / 60.0;
    self.colors = @[
        [NSColor colorWithRed:190.0 / 255.0 green:0 blue:1 alpha:1],
        [NSColor colorWithRed:1 green:0 blue:139.0 / 255.0 alpha:1],
        [NSColor colorWithRed:1 green:131.0 / 255.0 blue:0 alpha:1],
        [NSColor colorWithRed:0 green:38.0 / 255.0 blue:1 alpha:1],
        [NSColor colorWithRed:1 green:250.0 / 255.0 blue:0 alpha:1],
    ];
    self.colorIndex = -1;
    self.velocity = NSMakePoint(isPreview ? 0.8 : 2.2, isPreview ? 0.65 : 1.7);

    self.wantsLayer = YES;
    self.layer.backgroundColor = NSColor.blackColor.CGColor;
    self.logoLayer = [CALayer layer];
    self.logoLayer.contentsGravity = kCAGravityResizeAspect;
    self.logoLayer.contentsScale = NSScreen.mainScreen.backingScaleFactor ?: 2.0;
    [self.layer addSublayer:self.logoLayer];

    NSBundle *bundle = [NSBundle bundleForClass:self.class];
    NSString *logoPath = [bundle pathForResource:@"DVDVideo360" ofType:@"png"];
    if (logoPath) {
        self.sourceLogo = [[NSImage alloc] initWithContentsOfFile:logoPath];
    }

    [self advanceColor];
    [self updateLayoutIfNeeded:YES];
    return self;
}

- (BOOL)isOpaque
{
    return YES;
}

- (void)startAnimation
{
    [super startAnimation];
    [self updateLayoutIfNeeded:YES];
}

- (void)animateOneFrame
{
    [self updateLayoutIfNeeded:NO];
    NSRect bounds = self.bounds;
    CGFloat maxX = MAX(0, NSWidth(bounds) - self.logoSize.width);
    CGFloat maxY = MAX(0, NSHeight(bounds) - self.logoSize.height);
    NSPoint next = NSMakePoint(self.position.x + self.velocity.x,
                               self.position.y + self.velocity.y);
    BOOL bounced = NO;

    if (next.x <= 0 || next.x >= maxX) {
        self.velocity = NSMakePoint(-self.velocity.x, self.velocity.y);
        next.x = MIN(MAX(next.x, 0), maxX);
        bounced = YES;
    }
    if (next.y <= 0 || next.y >= maxY) {
        self.velocity = NSMakePoint(self.velocity.x, -self.velocity.y);
        next.y = MIN(MAX(next.y, 0), maxY);
        bounced = YES;
    }
    if (bounced) {
        [self advanceColor];
    }

    self.position = next;
    [self updateLayers];
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect
{
    [[NSColor blackColor] setFill];
    NSRectFill(rect);

    if (!self.tintedLogo) {
        return;
    }

    NSGraphicsContext.currentContext.imageInterpolation = NSImageInterpolationHigh;
    NSRect target = NSMakeRect(self.position.x, self.position.y,
                               self.logoSize.width, self.logoSize.height);
    [self.tintedLogo drawInRect:target
                       fromRect:NSZeroRect
                      operation:NSCompositingOperationSourceOver
                       fraction:1.0
                 respectFlipped:YES
                          hints:nil];
}

- (void)updateLayoutIfNeeded:(BOOL)randomize
{
    NSSize boundsSize = self.bounds.size;
    if (NSEqualSizes(boundsSize, self.lastBoundsSize) && !randomize) {
        return;
    }
    self.lastBoundsSize = boundsSize;

    CGFloat ratio = 822.0 / 360.0;
    CGFloat diagonal = hypot(boundsSize.width, boundsSize.height);
    CGFloat width = MAX(60.0, diagonal / 8.0);
    width = MIN(width, MAX(60.0, boundsSize.width * 0.45));
    self.logoSize = NSMakeSize(width, width / ratio);

    CGFloat maxX = MAX(0, boundsSize.width - self.logoSize.width);
    CGFloat maxY = MAX(0, boundsSize.height - self.logoSize.height);
    if (randomize) {
        self.position = NSMakePoint(SSRandomFloatBetween(0, maxX),
                                    SSRandomFloatBetween(0, maxY));
    } else {
        self.position = NSMakePoint(MIN(MAX(self.position.x, 0), maxX),
                                    MIN(MAX(self.position.y, 0), maxY));
    }
    [self updateLayers];
}

- (void)advanceColor
{
    if (!self.sourceLogo || self.colors.count == 0) {
        return;
    }

    self.colorIndex = (self.colorIndex + 1) % self.colors.count;
    NSImage *image = [[NSImage alloc] initWithSize:self.sourceLogo.size];
    [image lockFocus];
    NSColor *color = self.colors[(NSUInteger)self.colorIndex];
    [color setFill];
    NSRectFill(NSMakeRect(0, 0, image.size.width, image.size.height));
    [self.sourceLogo drawAtPoint:NSZeroPoint
                        fromRect:NSZeroRect
                       operation:NSCompositingOperationDestinationIn
                        fraction:1.0];
    [image unlockFocus];
    self.tintedLogo = image;
    [self updateLayers];
}

- (void)updateLayers
{
    if (!self.logoLayer || !self.tintedLogo) {
        return;
    }

    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    self.logoLayer.frame = CGRectMake(self.position.x, self.position.y,
                                      self.logoSize.width, self.logoSize.height);
    NSRect imageRect = NSMakeRect(0, 0, self.tintedLogo.size.width, self.tintedLogo.size.height);
    CGImageRef image = [self.tintedLogo CGImageForProposedRect:&imageRect context:nil hints:nil];
    self.logoLayer.contents = (__bridge id)image;
    [CATransaction commit];
}

@end
