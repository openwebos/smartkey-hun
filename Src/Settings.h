/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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

#ifndef SMKY_SETTINGS_H
#define SMKY_SETTINGS_H

#include <glib.h>

#include <string>

namespace SmartKey
{

struct Settings
{
	Settings();
	~Settings() {};

	bool	load(const std::string& settingsFile);

	std::string backupDirectory;

	std::string	locale;

	std::string readOnlyDataDir;
	std::string readWriteDataDir;
	std::string smkyDataDirectory;
};

}  // namespace SmartKey

using SmartKey::Settings;

#endif  // SMKY_SETTINGS_H
