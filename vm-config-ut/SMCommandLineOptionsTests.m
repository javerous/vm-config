/*
 *  SMCommandLineOptionsTests.m
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

#import <XCTest/XCTest.h>

#import "SMCommandLineOptions.h"

#import "SMTestsTools.h"


/*
** SMCommandLineOptionsTests
*/
#pragma mark - SMCommandLineOptionsTests

@interface SMCommandLineOptionsTests : XCTestCase

@end

@implementation SMCommandLineOptionsTests

#pragma mark - Tests

- (void)testUsage
{
	SMDeclareDefaultFiles;
	
	// Get standard options.
	SMCLOptions *options = [self standardOptions];
	
	_onExit {
		SMCLOptionsFree(options);
	};
	
	// Generate usage.
	SMCLOptionsPrintUsage(options, fout);
	fflush(fout);
		
	// Check if it seems valid.
	XCTAssertGreaterThan(sout, 50);
	XCTAssertContainString(*bout, sout, "version");
	
	XCTAssertContainString(*bout, sout, "show");
	XCTAssertContainString(*bout, sout, "change");
	
	XCTAssertContainString(*bout, sout, "vm");
	XCTAssertContainString(*bout, sout, "virtual machine bundle");

	XCTAssertContainString(*bout, sout, "--mandatory-nvalue");
	XCTAssertContainString(*bout, sout, "-a");
	
	XCTAssertContainString(*bout, sout, "--pre-option");

	XCTAssertContainString(*bout, sout, "vm-1");
	XCTAssertContainString(*bout, sout, "vm-2");
	XCTAssertContainString(*bout, sout, "vm-3");
}

#warning FIXME: Add more tests.

#pragma mark - Helpers

- (SMCLOptions *)standardOptions
{
	SMCLOptions *options = SMCLOptionsCreate();
	
	SMCLOptionsAddVerb(options, 0, "version", "Show version of this command line");
	SMCLOptionsVerb *show_verb = SMCLOptionsAddVerb(options, 0, "show", "Show configuration of a virtual machine bundle");
	SMCLOptionsVerb *change_verb = SMCLOptionsAddVerb(options, 1, "change", "Change configuration of a virtual machine bundle");

	
	SMCLOptionsVerbAddValue(show_verb, 0, "vm", "Path to the virtual machine bundle");
	SMCLOptionsVerbAddOption(show_verb, 1, false, "mandatory-nvalue", 0, "Mandatory without value");
	SMCLOptionsVerbAddOptionWithArgument(show_verb, 2, false, "mandatory-value", 'a', SMCLParameterArgumentTypeString, "hello", "Mandatory with value");
	SMCLOptionsVerbAddOption(show_verb, 3, true, "optional-nvalue", 0, "Optional without value");
	SMCLOptionsVerbAddOptionWithArgument(show_verb, 4, true, "optional-value", 0, SMCLParameterArgumentTypeString, NULL, "Optional with value");

	SMCLOptionsVerbAddOption(change_verb, 5, false, "pre-option", 0, "Option before value");
	SMCLOptionsVerbAddValue(change_verb, 6, "vm-1", "Path to first virtual machine bundle");
	SMCLOptionsVerbAddValue(change_verb, 7, "vm-2", "Path to second virtual machine bundle");
	SMCLOptionsVerbAddOption(change_verb, 8, false, "post-option-1", 0, "Option before value");
	SMCLOptionsVerbAddOption(change_verb, 9, true, "post-option-2", 0, "Option before value");
	SMCLOptionsVerbAddValue(change_verb, 7, "vm-3", "Path to second virtual machine bundle");
	
	return options;
}

@end
