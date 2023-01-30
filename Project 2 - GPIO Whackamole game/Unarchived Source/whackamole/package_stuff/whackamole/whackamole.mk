##################################################################################
#																				                     #
#     									   WHACKAMOLE  									      #
#																                     				#
##################################################################################

WHACKAMOLE_VERSION = 0.98
#topdir is the directory of buildroot base folder.
WHACKAMOLE_SITE = $(TOPDIR)/../whackamole
WHACKAMOLE_SITE_METHOD = local
WHACKAMOLE_DEPENDENCIES = libgpiod

define WHACKAMOLE_BUILD_CMDS
   $(MAKE) $(TARGET_CONFIGURE_OPTS) whackamole -C $(@D)
endef
define WHACKAMOLE_INSTALL_TARGET_CMDS 
   $(INSTALL) -D -m 0755 $(@D)/whackamole $(TARGET_DIR)/usr/bin 
endef
WHACKAMOLE_LICENSE = Proprietary

$(eval $(generic-package))
