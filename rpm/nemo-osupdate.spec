Name:       nemo-osupdate
Summary:    OS Update for Nemo Mobile
Version:    0.0.1
Release:    1
Group:      System/Libraries
License:    BSD
URL:        https://git.merproject.org/mer-core/nemo-osupdate
Source0:    %{name}-%{version}.tar.bz2
BuildRequires: pkgconfig(Qt5Core)

%description
%{summary}.

%package devel
Summary:    Nemo OS Update development files
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
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

%post
/sbin/ldconfig || :

%postun
/sbin/ldconfig || :
