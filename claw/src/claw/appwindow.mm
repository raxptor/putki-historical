#include "appwindow.h"

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>

struct update_info
{
	claw::appwindow::updatefunc f;
	claw::appwindow::data *d;
};

@interface TestView : NSOpenGLView
{
   CVDisplayLinkRef displayLink; //display link for managing rendering thread
   @public update_info uinfo;
}

-(void)drawRect:(NSRect)rect;
- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime;

@end

@implementation TestView

-(void)drawRect:(NSRect)rect {
    [[NSColor blueColor] set];
    NSRectFill( [self bounds] );
}

-(void)windowWillClose:(NSNotification *)note {
    [[NSApplication sharedApplication] terminate:self];
}

- (void)prepareOpenGL
{
    // Synchronize buffer swaps with vertical refresh rate
    
    // (i.e. all openGL on this thread calls will go to this context)
    [[self openGLContext] makeCurrentContext];
    
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
  
    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    
    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    
    NSOpenGLPixelFormatAttribute attrs[] = {
     NSOpenGLPFADoubleBuffer,
     NSOpenGLPFADepthSize, 32,
     0
   };

	CGLPixelFormatObj cglPixelFormat  = (CGLPixelFormatObj) [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
 

	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	CVDisplayLinkSetOutputCallback(displayLink, MyDisplayLinkCallback, self);
	CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
          
    // Activate the display link
    CVDisplayLinkStart(displayLink);
}

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [(TestView*)displayLinkContext getFrameForTime:outputTime];
    printf("Frame!\n");
    return result;
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime
{
    // Add your drawing codes here
    uinfo.f();
    return kCVReturnSuccess;
}

- (void)dealloc
{
    // Release the display link
    CVDisplayLinkRelease(displayLink);
 
    [super dealloc];
}
@end


namespace claw
{
	namespace appwindow
	{
		struct data
		{
			NSAutoreleasePool *pool;
			NSApplication *app;
			NSWindow *window;
			TestView *view;
		};
		
		data * create(const char *title, int width, int height)
		{
			data *d = new data();
			d->pool = [NSAutoreleasePool new];
			d->app = [NSApplication sharedApplication];
			
			NSRect frame = NSMakeRect( 100., 100., 300., 300. );
    
			d->window = [[NSWindow alloc]
				initWithContentRect:frame
				styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask
				backing:NSBackingStoreBuffered
				defer:false];
    
			[d->window setTitle:@"Testing"];
    
			d->view = [[[TestView alloc] initWithFrame:frame] autorelease];
			d->view->uinfo.d = d;

			[d->window setContentView:d->view];
			[d->window makeKeyAndOrderFront:nil];
			[d->window setLevel:kCGMaximumWindowLevel];
    
			[d->app activateIgnoringOtherApps:YES];
			[d->app run];
    
			[d->pool release];
	
			return d;
		}

		void destroy(data *d)
		{
		
		}
		
		void set_title(data *d, const char *title)
		{
		
		}
		
		void run_loop(data *d, updatefunc f)
		{
			d->view->uinfo.f = f;
			[d->app run];
		}

	}
}

