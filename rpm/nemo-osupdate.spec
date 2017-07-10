Name:       nemo-osupdate
Summary:    OS Update for Nemo Mobile
Version:    0.0.1
Release:    1
Group:      System/Libraries
License:    BSD
URL:        https://git.merproject.org/mer-core/nemo-osupdate
Source0:    %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(Qt5Core)
BuildRequires:  PackageKit-Qt5-devel
BuildRequires:  pkgconfig(rpm)
BuildRequires:  ssu-devel

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:   PackageKit-Qt5
Requires:   PackageKit-zypp

%description
%{summary}.

%package devel
Summary:    Nemo OS Update development files
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
%{summary}.

%package packagemanagement
Summary:    Nemo OS Update PackageKit Extensions
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description packagemanagement
%{summary}.

%package packagemanagement-devel
Summary:    Nemo OS Update PackageKit Extensions development files
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description packagemanagement-devel
%{summary}.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 "VERSION=%{version}"
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
%dir %{_libdir}/qt5/qml/Nemo/OsUpdate
%{_libdir}/qt5/qml/Nemo/OsUpdate/libnemoosupdate.so
%{_libdir}/qt5/qml/Nemo/OsUpdate/qmldir
%{_libdir}/libnemoosupdate.so.*

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/nemo-osupdate
%{_includedir}/nemo-osupdate/*.h
%{_libdir}/libnemoosupdate.so
%{_libdir}/pkgconfig/nemoosupdate.pc

%files packagemanagement
%defattr(-,root,root,-)
%{_libdir}/libnemoosupdate-packagemanagement.so.*

%files packagemanagement-devel
%defattr(-,root,root,-)
%dir %{_includedir}/nemo-osupdate/packagemanagement
%{_includedir}/nemo-osupdate/packagemanagement/*.h
%{_libdir}/libnemoosupdate-packagemanagement.so
%{_libdir}/pkgconfig/nemoosupdate-packagemanagement.pc

%post
/sbin/ldconfig || :

%postun
/sbin/ldconfig || :

