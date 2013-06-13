Summary
=======

SmartKey is the webOS service for spell checking.  
Note: This is a limited functionality version, without the third party proprietary libraries used to support spell checking

SmartKey
========

This service supports the following methods, which are described in detail in the generated documentation:  

* com.palm.smartKey/addAutoReplace
* com.palm.smartKey/addPerson
* com.palm.smartKey/addUserWord
* com.palm.smartKey/exit
* com.palm.smartKey/forget
* com.palm.smartKey/getCompletion
* com.palm.smartKey/learn
* com.palm.smartKey/listAutoReplace
* com.palm.smartKey/listUserWords
* com.palm.smartKey/numAutoReplace
* com.palm.smartKey/numUserWords
* com.palm.smartKey/processTaps
* com.palm.smartKey/removeAutoReplace
* com.palm.smartKey/removePerson
* com.palm.smartKey/removeUserWord
* com.palm.smartKey/search
* com.palm.smartKey/updateWordUsage


How to Build on Linux
=====================

## Dependencies

Below are the tools (and their minimum versions) required to build SmartKey:

* cmake 2.8.7
* gcc 4.3
* make (any version)
* pkg-config 0.22
* glib 2.0

You should first install and build:
* openwebos/cjson 1.8.0
* openwebos/luna-service2 3.0.0
* pbnjson
* hunspell  1.3
* icu  3.6

## Generating documentation

The tools required to generate the documentation are:

* doxygen 1.6.3
* graphviz 2.20.2


# Copyright and License Information

All content, including all source code files and documentation files in this repository except otherwise noted are: 

 Copyright (c) 2010-2013 LG Electronics, Inc.

All content, including all source code files and documentation files in this repository except otherwise noted are:
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this content except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
