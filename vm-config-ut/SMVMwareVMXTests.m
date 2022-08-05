/*
 *  SMVMwareVMXTests.m
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

#import "SMVMwareVMXHelper.h"


/*
** SMVMwareVMXTests
*/
#pragma mark - SMVMwareVMXTests

@interface SMVMwareVMXTests : XCTestCase

@end

@implementation SMVMwareVMXTests

#pragma mark - Tests

- (void)testDetailedDataParsing
{
	// Valid 1.
	const char		*data1 = "architecture='X86' bitness='64' buildNumber='21G72' distroName='Mac OS X' distroVersion='10.16' familyName='Darwin' kernelVersion='21.6.0'";
	NSDictionary	*refdict1 = @{ @"architecture" : @"X86", @"bitness": @"64", @"buildNumber": @"21G72", @"distroName": @"Mac OS X", @"distroVersion": @"10.16", @"familyName": @"Darwin", @"kernelVersion": @"21.6.0" };
	NSDictionary	*parsedict1 = [self dictionaryFromFields:SMDetailedFieldsFromString(data1) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict1, refdict1);
	
	// Valid 2.
	const char		*data2 = "architecture='X86' bitness='64'";
	NSDictionary	*refdict2 = @{ @"architecture" : @"X86", @"bitness": @"64" };
	NSDictionary	*parsedict2 = [self dictionaryFromFields:SMDetailedFieldsFromString(data2) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict2, refdict2);
	
	// Valid 3.
	const char		*data3 = "architecture='X86' bitness=''";
	NSDictionary	*refdict3 = @{ @"architecture" : @"X86", @"bitness": @"" };
	NSDictionary	*parsedict3 = [self dictionaryFromFields:SMDetailedFieldsFromString(data3) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict3, refdict3);
	
	// Valid 4.
	const char		*data4 = "architecture='X86'";
	NSDictionary	*refdict4 = @{ @"architecture" : @"X86" };
	NSDictionary	*parsedict4 = [self dictionaryFromFields:SMDetailedFieldsFromString(data4) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict4, refdict4);
		
	// Valid 5.
	const char		*data5 = "architecture=''";
	NSDictionary	*refdict5 = @{ @"architecture" : @"" };
	NSDictionary	*parsedict5 = [self dictionaryFromFields:SMDetailedFieldsFromString(data5) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict5, refdict5);
	
	// Valid 6.
	const char		*data6 = "architecture='X86' bitness='64' buildNumber='21G72' distroName='Mac\\' OS X' distroVersion='10.16' familyName='D\\\\arwin' kernelVersion='21.6.0'";
	NSDictionary	*refdict6 = @{ @"architecture" : @"X86", @"bitness": @"64", @"buildNumber": @"21G72", @"distroName": @"Mac' OS X", @"distroVersion": @"10.16", @"familyName": @"D\\arwin", @"kernelVersion": @"21.6.0" };
	NSDictionary	*parsedict6 = [self dictionaryFromFields:SMDetailedFieldsFromString(data6) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict6, refdict6);
	
	// Valid 7.
	const char		*data7 = "architecture='X86' ";
	NSDictionary	*refdict7 = @{ @"architecture" : @"X86" };
	NSDictionary	*parsedict7 = [self dictionaryFromFields:SMDetailedFieldsFromString(data7) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict7, refdict7);
	
	// Valid 8.
	const char		*data8 = "a='b' ";
	NSDictionary	*refdict8 = @{ @"a" : @"b" };
	NSDictionary	*parsedict8 = [self dictionaryFromFields:SMDetailedFieldsFromString(data8) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict8, refdict8);
	
	// Invalid 1.
	const char		*idata1 = "architecture='X86' bitness='64' buildNumber='21G72' distroName='Mac OS X' distroVersion='10.16' familyName='Darwin' kernelVersion='21.6.0";
	NSDictionary	*irefdict1 = @{ @"architecture" : @"X86", @"bitness": @"64", @"buildNumber": @"21G72", @"distroName": @"Mac OS X", @"distroVersion": @"10.16", @"familyName": @"Darwin" };
	NSDictionary	*iparsedict1 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata1) freeFields:YES];
	
	XCTAssertEqualObjects(iparsedict1, irefdict1);
	
	// Invalid 2.
	const char		*idata2 = "architecture='X86' bitness='";
	NSDictionary	*irefdict2 = @{ @"architecture" : @"X86" };
	NSDictionary	*iparsedict2 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata2) freeFields:YES];
	
	XCTAssertEqualObjects(iparsedict2, irefdict2);
	
	// Invalid 3.
	const char		*idata3 = "architecture='X86' bitness=";
	NSDictionary	*irefdict3 = @{ @"architecture" : @"X86" };
	NSDictionary	*iparsedict3 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata3) freeFields:YES];
	
	XCTAssertEqualObjects(iparsedict3, irefdict3);
	
	// Invalid 3.
	const char		*idata4 = "architecture='X86' bitness";
	NSDictionary	*irefdict4 = @{ @"architecture" : @"X86" };
	NSDictionary	*iparsedict4 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata4) freeFields:YES];
	
	XCTAssertEqualObjects(iparsedict4, irefdict4);

	// Invalid 5.
	const char		*idata5 = "";
	NSDictionary	*irefdict5 = @{ };
	NSDictionary	*iparsedict5 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata5) freeFields:YES];

	XCTAssertEqualObjects(iparsedict5, irefdict5);
}

#pragma mark - Helpers

- (NSDictionary *)dictionaryFromFields:(SMDetailedField *)fields freeFields:(BOOL)freeFields
{
	if (!fields)
		return @{ };
	
	NSMutableDictionary *result = [NSMutableDictionary dictionary];
	
	for (size_t i = 0; ; i++)
	{
		if (!fields[i].key || !fields[i].value)
			break;
		
		result[@(fields[i].key)] = @(fields[i].value);
	}
	
	if (freeFields)
		SMDetailedFieldsFree(fields);
	
	return result;
}


@end
