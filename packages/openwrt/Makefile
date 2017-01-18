#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wslog
PKG_VERSION:=0.6
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=file:///home/gbuchetet/perso/workspace/ws2300/.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=d11cb58275c9a193f350d0b5c00872510bd7c9a6
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
#PKG_MD5SUM:=df3d201aecb3155a80633990cc467831

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/wslog
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Weather station utility
  DEPENDS:=+libcurl +libsqlite3 +librt +libpthread
endef

define Package/wslog/description
  This is an utility suite used to communicate with and collect data from
  weather stations.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
endef

define Build/Install
	$(MAKE) -C $(PKG_BUILD_DIR) ROOT=$(PKG_INSTALL_DIR)/usr install
endef

define Package/wslog/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/lib/libws/libws.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/cmd/ws23xx/ws23xx $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/cmd/wslogd/wslogd $(1)/usr/bin/
endef

$(eval $(call BuildPackage,wslog))
