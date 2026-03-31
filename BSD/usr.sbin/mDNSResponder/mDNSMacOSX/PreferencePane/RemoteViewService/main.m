/*
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import <Foundation/Foundation.h>
#import <PreferencePanes/PreferencePaneXPCMain.h>

int
main(int inArgc, const char *inArgv[])
{
	int	result = 0;

	@autoreleasepool
	{
		@try
		{
			result = PreferencePaneMain(inArgc, inArgv);
		}
		@catch(NSException * e)
		{
			NSLog(@"%s caught %@: '%@' with user dictionary %@ and call stack %@", __func__, [e name], [e reason], [e userInfo], [e callStackSymbols]);
			result = 1;
		}
	}

	return result;
}
