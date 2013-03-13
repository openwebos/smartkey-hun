# @@@LICENSE
#
#      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
#      Copyright (c) 2013 LG Electronics
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@

TEMPLATE = app

CONFIG += qt

ENV_BUILD_TYPE = $$(BUILD_TYPE)
!isEmpty(ENV_BUILD_TYPE) {
	CONFIG -= release debug
	CONFIG += $$ENV_BUILD_TYPE
} else {
    config += release
    BUILD_TYPE = release
}

# Prevent conflict with usage of "signal" in other libraries
CONFIG += no_keywords

CONFIG += link_pkgconfig
PKGCONFIG = glib-2.0 gthread-2.0 pbnjson_cpp

QT = core gui

VPATH = ./Src

INCLUDEPATH = $$VPATH

DEFINES += QT_WEBOS \

# For shipping version of the code, as opposed to a development build. Set this to 1 late in the process...
DEFINES += SHIPPING_VERSION=0

# Uncomment to compile in trace statements in the code for debugging
# DEFINES += ENABLE_TRACING

# DEFINES += HAVE_CALLGRIND=1

# This allows the use of the % for faster QString concatentation
# See the QString documentation for more information
# DEFINES += QT_USE_FAST_CONCATENATION

# Uncomment this for all QString concatenations using +
# to go through the faster % instead.  Not sure what impact
# this has performance wise or behaviour wise.
# See the QString documentation for more information
# DEFINES += QT_USE_FAST_OPERATOR_PLUS

SOURCES = PerfTimer.cpp \
        Settings.cpp \
        SmartKeyService.cpp \
        SmkyAutoSubDatabase.cpp \
        SmkyFileKeywords.cpp \
        SmkyFilePairs.cpp \
        SmkyHunspellDatabase.cpp \
        SmkyManufacturerDatabase.cpp \
        SmkySpellCheckEngine.cpp \
        SmkyUserDatabase.cpp \
        SpellCheckClient.cpp \
        StringUtils.cpp \

HEADERS = Database.h \
        PerfTimer.h \
        Settings.h \
        SmartKeyService.h \
        SmkyAutoSubDatabase.h \
        SmkyFileKeywords.h \
        SmkyFilePairs.h \
        SmkyHunspellDatabase.h \
        SmkyKeywordsBundle.h \
        SmkyManufacturerDatabase.h \
        SmkyPairsBundle.h \
        SmkySpellCheckEngine.h \
        SmkyUserDatabase.h \
        SpellCheckClient.h \
        StringUtils.h \

QMAKE_CXXFLAGS += -fno-rtti -fno-exceptions -Wall -Werror
QMAKE_CXXFLAGS += -DFIX_FOR_QT

# Override the default (-Wall -W) from g++.conf mkspec (see linux-g++.conf)
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unused-variable -Wno-reorder -Wno-missing-field-initializers -Wno-extra -Wno-deprecated

##LIBS += -lluna-service2 -lpbnjson_cpp -lhunspell-1.3
LIBS += -lluna-service2 -lpbnjson_cpp -lcjson -licui18n -licuuc

INCLUDEPATH += $$(LUNA_STAGING)/include

linux-g++ || linux-g++-64 {
    MACHINE_NAME = x86
    QMAKE_MAKEFILE = Makefile.Ubuntu
    DEFINES += TARGET_DESKTOP
} else {
    MACHINE_NAME = $$(MACHINE)
}
linux-qemux86-g++ {
	QMAKE_CXXFLAGS += -fno-strict-aliasing
}

linux-armv7-g++ {
}

linux-armv6-g++ {
}

DESTDIR = ./$${BUILD_TYPE}-$${MACHINE_NAME}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc

TARGET = com.palm.smartkey

# Comment these out to get verbose output
#QMAKE_CXX = @echo Compiling $(@)...; $$QMAKE_CXX
#QMAKE_LINK = @echo Linking $(@)...; $$QMAKE_LINK
#QMAKE_MOC = @echo Mocing $(@)...; $$QMAKE_MOC

