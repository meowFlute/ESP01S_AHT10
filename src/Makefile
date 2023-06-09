#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

# This will determine the name of the bin/elf/map files generated
PROJECT_NAME=esp01s_aht10

# COMPONENT_DIRS
# You can use the COMPONENT_DIRS or EXTRA_COMPONENT_DIRS variable to add components to the make path
# Be aware, that $(IDF_PATH)/components is one of the default ones, so it is probably smart to use the EXTRA* variable as needed instead
# 
# Additionally, $(PROJECT_PATH)/main is included by default (i.e., the 'main' folder) so you are not required to add additional components unless it makes sense
# The hello_world/main/component.mk is a good example of the default behaviour (i.e. is a pseudo makefile with no content so the source files will be built)
# 
# EXTRA_COMPONENT_DIRS = <application components I write aside from the main/ folder go here

# PROJECT_PATH
# The included makefile does PROJECT_PATH by
# PROJECT_PATH := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
# so you could do the same if you wanted to write something like
#
# EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/<component_name>
# 
# and then it would be similar to how their main makefile works
PROJECT_PATH := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/tinyprintf
 

# EXCLUDE_COMPONENTS
# You can explicitly exclude components by listing them in this variable

# TEST_ALL=1
# I actually don't know what this does yet, because I don't know what 'test' means in this context, but it is in there

# 'make print_flash_cmd' will tell you the locations and .bin files being flashed with 'make flash' 
# 	- Additionally it will list a csv of the partition table

# 'make list-components' will list the components being built in (and excluded)

# 'make size', 'make size-files', 'make size-components', 'make size-symbols' all yield interesting datausing $(IDF_PATH)/tools/idf_size.py 

include $(IDF_PATH)/make/project.mk

