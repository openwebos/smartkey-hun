OBJDIR := $(BUILD_TYPE)-$(PLATFORM)
SRCDIR := Src

# @@@LICENSE
#
#      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
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

TOP_DIR = $(shell pwd)

# ------------------------------------------------------------------

SMART_KEY_SOURCES := \
	SmkySpellCheckEngine.cpp \
	SmkyDatabase.cpp \
	SmkyUserDatabase.cpp \
	SmkyAutoSubDatabase.cpp \
	SmkyManufacturerDatabase.cpp \
	Settings.cpp \
	StringUtils.cpp

SMART_KEY_SERVICE_SOURCES := $(SMART_KEY_SOURCES) \
    SmartKeyService.cpp

CLIENT_SOURCES := \
	SpellCheckClient.cpp

SMART_KEY_OBJS := $(SMART_KEY_SOURCES:%.cpp=$(OBJDIR)/%.o)
CLIENT_OBJS := $(CLIENT_SOURCES:%.cpp=$(OBJDIR)/%.o)
SMART_KEY_SERVICE_OBJS := $(SMART_KEY_SERVICE_SOURCES:%.cpp=$(OBJDIR)/%.o)
SMART_KEY_SERVICE_OBJS += $(C_SOURCES:%.c=$(OBJDIR)/%.o)

SMART_KEY_SERVICE := $(OBJDIR)/com.palm.smartkey

INCLUDES := $(INCLUDES) \
	`pkg-config --cflags glib-2.0`

# ------------------------------------------------------------------

FLAGS_OPT += -fno-rtti -fPIC -fno-exceptions -Wno-error

ifeq ("$(BUILD_TYPE)", "debug")
FLAGS_OPT += -O0 -g
else
FLAGS_OPT += -O2 -g -DG_DISABLE_ASSERT
endif

LIBS := $(LIBS) \
	-lglib-2.0 -llunaservice -lcjson -licui18n -lhunspell-1.3 \

LOCAL_CFLAGS := $(CFLAGS) -Wno-deprecated -Wall $(FLAGS_OPT) $(ARCH_CFLAGS)

LOCAL_LFLAGS := $(LDFLAGS) $(FLAGS_OPT) $(LIBS) 

# ------------------------------------------------------------------

all: $(OBJDIR) $(SMART_KEY_SERVICE_OBJS) $(SMART_KEY_SERVICE) $(OBJDIR)/libSpellCheck.a

$(OBJDIR):
	@mkdir -p $(OBJDIR)

vpath %.cpp $(SRCDIR)

$(SMART_KEY_SERVICE): $(SMART_KEY_SERVICE_OBJS)
	$(CXX) -o $(SMART_KEY_SERVICE) $(SMART_KEY_SERVICE_OBJS) $(LOCAL_LFLAGS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) -MMD $(INCLUDES) $(LOCAL_CFLAGS)  -fno-rtti -c $< -o $@

-include $(SMART_KEY_SERVICE_SOURCES:%.cpp=$(OBJDIR)/%.d)

.PHONY: clean
clean:
	rm -rf $(OBJDIR)
	rm -f *.d
	rm -f $(INSTALL_DIR)/usr/bin/com.palm.smartkey 
	rm -f $(INSTALL_DIR)/etc/palm/smartkey.conf
	rm -f $(INSTALL_DIR)/etc/event.d/smartkey
	rm -rf $(INSTALL_DIR)/etc/palm/smartkey

.PHONY: stage
stage: $(SMART_KEY_SERVICE) $(OBJDIR)/libSpellCheck.a
	install -d $(INCLUDE_DIR)
	install -m 444 Src/SpellCheckClient.h $(INCLUDE_DIR)/SpellCheckClient.h
	install -d $(LIB_DIR)
	install -m 444 $(OBJDIR)/libSpellCheck.a $(LIB_DIR)/libSpellCheck.a

.PHONY: install-data
install-data: $(SMART_KEY_SERVICE)
	install -d $(INSTALL_DIR)/usr/palm/smartkey/Smky/DefaultData/whitelist
	cd DefaultData && tar -cf - `find . -iname "text-edit-autoreplace" -print` | ( cd $(INSTALL_DIR)/usr/palm/smartkey/Smky/DefaultData && tar xBf - )
	cd DefaultData && tar -cf - `find . -iname "man-db-entries" -print` | ( cd $(INSTALL_DIR)/usr/palm/smartkey/Smky/DefaultData && tar xBf - )
	cd DefaultData && tar -cf - `find . -iname "*whitelist-entries" -print` | ( cd $(INSTALL_DIR)/usr/palm/smartkey/Smky/DefaultData && tar xBf - )
	cd DefaultData && tar -cf - `find . -iname "locale-words" -print` | ( cd $(INSTALL_DIR)/usr/palm/smartkey/Smky/DefaultData && tar xBf - )
	find $(INSTALL_DIR)/usr/palm/smartkey/Smky/DefaultData/whitelist -name whitelist-entries -exec $(TOP_DIR)/DefaultData/whitelist/whitescramble.pl {} \;
	install -d $(INSTALL_DIR)/usr/palm/smartkey/hunspell
	cd hunspell && tar -cf - `find . -iname "basicGrammar.aff" -print` | ( cd $(INSTALL_DIR)/usr/palm/smartkey/hunspell && tar xBf - )

.PHONY: install-service-files
install-service-files: $(SMART_KEY_SERVICE)
	install -d $(INSTALL_DIR)/usr/bin
	install -m 750 -p $(SMART_KEY_SERVICE) $(INSTALL_DIR)/usr/bin/com.palm.smartkey
	install -d $(INSTALL_DIR)/etc/palm
	install -m 755 -p conf/smartkey.conf $(INSTALL_DIR)/etc/palm/smartkey.conf
	install -d $(INSTALL_DIR)/etc/event.d
	install -m 555 -p smartkey.start $(INSTALL_DIR)/etc/event.d/smartkey

.PHONY: install
ifeq ("$(PLATFORM)","arm")
install: install-service-files install-data
else
ifeq ("$(MACHINE)","qemux86")
install: install-service-files install-data
else
install: install-service-files
endif
endif

#
# The spelling 
#
$(OBJDIR)/libSpellCheck.a: $(CLIENT_OBJS)
	$(AR) rcs $(OBJDIR)/libSpellCheck.a $(CLIENT_OBJS)



#
# -----------------------------------------------------------------
# build tests
#	

INSERTION_TEST_NAME := InsertionTest

INSERTION_TEST := $(OBJDIR)/InsertionTest

INSERTION_TEST_SOURCES := \
	InsertionTest.cpp

INSERTION_TEST_INCLUDES := \
	-I$(SRCDIR)
    
INSERTION_TEST_OBJS := $(INSERTION_TEST_SOURCES:%.cpp=$(OBJDIR)/%.o) 

vpath %.cpp Tests

INSERTION_TEST_LIBS := $(LIBS) \
	-ltcmalloc -lprofiler

$(INSERTION_TEST): $(INSERTION_TEST_OBJS) $(SMART_KEY_OBJS)
	$(CXX) -o $(INSERTION_TEST) $(INCLUDES) $(INSERTION_TEST_OBJS) $(SMART_KEY_OBJS) $(INSERTION_TEST_LIBS) $(LOCAL_LFLAGS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) -MMD $(INCLUDES) $(INSERTION_TEST_LIBS) $(INSERTION_TEST_INCLUDES) $(LOCAL_CFLAGS)  -fno-rtti -c $< -o $@

-include $(INSERTION_TEST_SOURCES:%.cpp=$(OBJDIR)/%.d)

tests: setup $(INSERTION_TEST)

#
# -------------------------------------------------------------------------
# compiler
#

COMPILER_NAME := DictionaryCompiler

COMPILER := $(OBJDIR)/DictionaryCompiler

COMPILER_SOURCES := \
	DictionaryCompiler.cpp

COMPILER_INCLUDES := \
	-I$(SRCDIR)
    
COMPILER_OBJS := $(COMPILER_SOURCES:%.cpp=$(OBJDIR)/%.o) 

vpath %.cpp Tools

COMPILER_LIBS := $(LIBS) \
	-ltcmalloc -lprofiler

$(COMPILER): $(COMPILER_OBJS) $(SMART_KEY_OBJS)
	$(CXX) -o $(COMPILER) $(INCLUDES) $(COMPILER_OBJS) $(SMART_KEY_OBJS) $(COMPILER_LIBS) $(LOCAL_LFLAGS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) -MMD $(INCLUDES) $(COMPILER_LIBS) $(COMPILER_INCLUDES) $(LOCAL_CFLAGS)  -fno-rtti -c $< -o $@

-include $(COMPILER_SOURCES:%.cpp=$(OBJDIR)/%.d)

compiler: setup $(COMPILER)


