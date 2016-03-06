#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ws23xx
PKG_VERSION:=0.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=file:///home/gbuchetet/perso/workspace/ws2300
#PKG_MD5SUM:=df3d201aecb3155a80633990cc467831

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ws23xx
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=WS23XX weather station utility
  DEPENDS:=+libcurl
endef

define Package/ws23xx/description
  This is an utility suite used to communicate with and collect data from a
  WS23XX wheather station.
endef

define Package/ws23xx/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ws2300 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ws23xx))
