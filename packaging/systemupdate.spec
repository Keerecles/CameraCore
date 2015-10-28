Name:       systemupdate
Summary:    ivos Framework
Version: 0.1.0
Release:    1
Group:      API/C API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: 	systemupdate.manifest

BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
#BuildRequires:  pkgconfig(dlog-devel)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(dbus-1)
#BuildRequires:  pkgconfig(libgcrypt)
#BuildRequires:  pkgconfig(libgpg-error)
BuildRequires: 	dlt-devel
BuildRequires:  zlib-devel
BuildRequires: 	libgcrypt-devel
BuildRequires: 	libgpg-error-devel
BuildRequires: 	libwebsockets-devel


%description
ivos Framework.


%package devel
Summary:  ivos Framework (Development)
Group:    API/C API
Requires: %{name} = %{version}-%{release}


%description devel
ivos Framework.



%prep
%setup -q
cp %{SOURCE1001} .
#cp ivos-service-hf-api.pc.in .
%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake .  -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
%make_install
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/usr/include
#cp include/ivos_core_api.h %{buildroot}/usr/include/ivos_core_api.h
cp include/ivos-service-systemupdate.h %{buildroot}/usr/include/ivos-service-systemupdate.h
#cp include/ivos-service-data-analyze.h %{buildroot}/usr/include/ivos-service-data-analyze.h
#cp include/ivos-service-download.h %{buildroot}/usr/include/ivos-service-download.h
#cp include/ivos-service-install.h %{buildroot}/usr/include/ivos-service-install.h
cp LICENSE %{buildroot}/usr/share/license/%{name}

###########OTA###########
mkdir -p %{buildroot}/usr/lib/systemd/system/graphical.target.wants/
mkdir -p %{buildroot}/usr/lib/systemd/system/
mkdir -p %{buildroot}/etc/dbus-1/system.d/
mkdir -p %{buildroot}/update/

cp system/systemupdate.service %{buildroot}/usr/lib/systemd/system/graphical.target.wants/
chmod a+rx %{buildroot}/usr/lib/systemd/system/graphical.target.wants/systemupdate.service

cp system/systemupdate.service %{buildroot}/usr/lib/systemd/system/
chmod a+rx %{buildroot}/usr/lib/systemd/system/systemupdate.service

cp system/systemupdate.conf %{buildroot}/etc/dbus-1/system.d/
cp system/update.sh %{buildroot}/update/
chmod a+rx %{buildroot}/update/update.sh
###########OTA###########




%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
#%{_libdir}/systemupdate
%{_prefix}/bin/systemupdate
/usr/share/license/%{name}

###########OTA###########
%{_libdir}/systemd/system/graphical.target.wants/systemupdate.service
%{_libdir}/systemd/system/systemupdate.service
/etc/dbus-1/system.d/systemupdate.conf
/update/update.sh
###########OTA###########

%files devel
%manifest %{name}.manifest
#%{_includedir}/ivos_core_api.h
%{_includedir}/ivos-service-systemupdate.h
%{_libdir}/pkgconfig/*.pc
#%{_libdir}/libivos-service-systemupdate.so


