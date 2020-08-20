%global release_name server-industry-20200613

Name:           dde-control-center
Version:        5.1.0.19
Release:        2
Summary:        New control center for Linux Deepin
License:        GPLv3
URL:            https://uos-packages.deepin.com/uos/pool/main/d/dde-control-center/
Source0:        %{name}_%{version}-%{release_name}.orig.tar.xz
Source1:        locale.gen
Source2:        locale-gen

#BuildRequires:  dtkcore-devel >= 5.1.1
BuildRequires:  gcc-c++
BuildRequires:  desktop-file-utils
BuildRequires:  dde-dock-devel
BuildRequires:  pkgconfig(dde-network-utils)
#BuildRequires:  pkgconfig(dtkwidget) >= 5.1
BuildRequires:  dtkwidget-devel
BuildRequires:  dtkgui-devel dtkcore-devel
#BuildRequires:  pkgconfig(dtkwidget2)
#BuildRequires:  pkgconfig(dframeworkdbus) >= 2.0
BuildRequires:  dde-qt-dbus-factory-devel
BuildRequires:  pkgconfig(gsettings-qt)
BuildRequires:  pkgconfig(geoip)
BuildRequires:  pkgconfig(libnm)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  pkgconfig(xcb-ewmh)
BuildRequires:  kf5-networkmanager-qt-devel
BuildRequires:  udisks2-qt5-devel
BuildRequires:  qt5-linguist
BuildRequires:  cmake
Requires:       dde-account-faces
Requires:       dde-api
Requires:       dde-daemon
Requires:       dde-qt5integration
Requires:       dde-network-utils
Requires:       startdde
Requires:       dde-server-industry-config

%description
New control center for Linux Deepin.

%package devel
Summary:        %{summary}
BuildArch:      noarch

%description devel
%{summary}.

%prep
%setup -q -n %{name}-%{version}-%{release_name}
sed -i 's|lrelease|lrelease-qt5|' translate_generation.sh
sed -i '/%{name}/s|\.\./lib|%{_libdir}|' src/frame/pluginscontroller.cpp
sed -i -E '/add_compile_definitions/d' CMakeLists.txt

%build
%cmake . -DDCC_DISABLE_GRUB=YES \
         -DDISABLE_SYS_UPDATE=YES
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}
# place holder plugins dir
mkdir -p %{buildroot}%{_libdir}/%{name}/plugins
# https://github.com/linuxdeepin/dde-control-center/issues/115
# And we disabled SYS_UPDATE, so reboot-reminder-dialog is useless.
#rm %{buildroot}%{_bindir}/reboot-reminder-dialog
mkdir -p %{buildroot}/usr/lib64/cmake/DdeControlCenter
mv %{buildroot}/cmake/DdeControlCenter/DdeControlCenterConfig.cmake %{buildroot}/usr/lib64/cmake/DdeControlCenter
mv %{buildroot}/usr/lib/libdccwidgets.so %{buildroot}%{_libdir}/
install -Dm644 com.deepin.controlcenter.addomain.policy %{buildroot}%{_datadir}/polkit-1/actions/
install -Dm644 %{_sourcedir}/locale.gen %{buildroot}%{_sysconfdir}/locale.gen
install -Dm755 %{_sourcedir}/locale-gen %{buildroot}/sbin/locale-gen

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop ||:

%ldconfig_scriptlets

%post
if [ $1 -ge 1 ]; then
   mkdir -p  /etc/profile.d
   echo '[ -f $HOME/.config/locale.conf ] && source $HOME/.config/locale.conf  > /dev/null 2>&1 || return 0' > /etc/profile.d/dde.sh
   chmod 644 /etc/profile.d/dde.sh
fi

%postun
if [ $1 -eq 0 ]; then
   rm -f /etc/profile.d/dde.sh
fi

%files
%doc README.md
%license LICENSE
%{_sysconfdir}/
%{_bindir}/abrecovery
%{_bindir}/dde-control-center
%{_datadir}/
/bin/restore-tool
/sbin/locale-gen
%{_libdir}/libdccwidgets.so
/etc/xdg/autostart/deepin-ab-recovery.desktop

%files devel
%{_includedir}/dde-control-center
%{_libdir}/cmake/DdeControlCenter/

%changelog
* Fri Jul  3 2020 uniontech <uoser@uniontech.com> - 5.1.0.19-2
- Add dde.sh to profile.d

* Mon Jun 15 2020 uniontech <uoser@uniontech.com> - 5.1.0.19
- Remove the universal menu.

* Fri May 29 2020 uniontech <uoser@uniontech.com> - 5.0.30
- Project init.

