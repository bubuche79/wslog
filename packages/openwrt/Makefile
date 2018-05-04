#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wslog
PKG_VERSION:=0.8.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=file:///home/gbuchetet/perso/workspace/wslog

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Configure
  $(call Build/Configure/Default)
endef

define Package/wslog
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Weather station logger
  DEPENDS:=+libcurl +libsqlite3 +librt +libpthread
endef

define Package/wslog/description
  This is an utility suite used to communicate with and collect data from
  weather stations.
endef

define Package/wslog/conffiles
/etc/wslogd.conf
endef

define Package/wslog/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/wslogd.conf $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/wslogd $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libws.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vantage $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ws23xx $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wslogc $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wslogd $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/wslog
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/share/wslog/sqlite.sql $(1)/usr/share/wslog
endef

define Package/wslog/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
  /etc/init.d/wslogd enable
fi
exit 0
endef

define Package/wslog/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
  /etc/init.d/wslogd stop >/dev/null 2>&1
  /etc/init.d/wslogd disable
fi
exit 0
endef

define Package/wslog-cgi
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Weather station CGI viewer
  DEPENDS:=+wslog +uhttpd +liblua +luci-base +luci-theme-bootstrap
endef

define Package/wslog-cgi/description
  This is the cgi-bin script for the weather station logger.
endef

define Package/wslog-cgi/install
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wsview/cgi-bin/wsview $(1)/www/cgi-bin
	$(INSTALL_DIR) $(1)/www/wsview-static
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/wsview/wsview-static/* $(1)/www/wsview-static
	$(INSTALL_DIR) $(1)/usr/lib/lua/wsview
	$(INSTALL_DIR) $(1)/usr/lib/lua/wsview/sgi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wsview/wsview/*.lua $(1)/usr/lib/lua/wsview
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wsview/wsview/sgi/*.lua $(1)/usr/lib/lua/wsview/sgi
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libwsview.so $(1)/usr/lib/lua/wsview.so
endef

$(eval $(call BuildPackage,wslog))
$(eval $(call BuildPackage,wslog-cgi))
