/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

#ifndef STUB_MOCK_IMPLEMENTATION_H
#define STUB_MOCK_IMPLEMENTATION_H


typedef enum {
	SMKY_STATUS_NONE = 0,
	SMKY_STATUS_ERROR = 1,
	SMKY_STATUS_NO_MEMORY = 3,
	SMKY_STATUS_BAD_PARAM = 6,
	SMKY_STATUS_WORD_EXISTS = 7,
	SMKY_STATUS_NO_MATCHING_WORDS = 8,
	SMKY_STATUS_INVALID_MEMORY = 20,
	SMKY_STATUS_READ_DB_FAIL,
	SMKY_STATUS_WRITE_DB_FAIL
} SMKY_STATUS;

typedef enum {
	SMKY_REQ_MODE_GETEXACTWORDS,
	SMKY_REQ_MODE_GETALLWORDS
} SMKY_REQ_MODE;

typedef struct {
	void                   *pPublicExtension;       /**< pointer for OEM extension. */
} SMKY_LINFO;

#endif  // STUB_MOCK_IMPLEMENTATION
