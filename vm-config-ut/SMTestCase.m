/*
 *  SMTestCase.m
 *
 *  Copyright 2022 Avérous Julien-Pierre
 *
 *  This file is part of vm-config.
 *
 *  vm-config is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  vm-config is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vm-config.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#import "SMTestCase.h"


NS_ASSUME_NONNULL_BEGIN


@implementation SMTestCase

- (void)setUp
{
	[super setUp];
	
#if defined(CHECK_LEAKS) && CHECK_LEAKS
	self.checkMemoryLeaks = YES;
#endif
}

- (void)tearDown
{
	[super tearDown];
	
	if (self.checkMemoryLeaks)
	{
		NSTask *task = [[NSTask alloc] init];
		
		task.executableURL = [NSURL fileURLWithPath:@"/usr/bin/leaks"];
		task.arguments = @[ [@(getpid()) stringValue] ];
		
		[task launch];
		[task waitUntilExit];
		
		if (task.terminationReason == NSTaskTerminationReasonExit && task.terminationStatus != 0)
			XCTFail(@"Leaks detected");
	}
}

@end


NS_ASSUME_NONNULL_END
