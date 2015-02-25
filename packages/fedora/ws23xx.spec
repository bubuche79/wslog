Name:		ws23xx
Version:	0.1
Release:	1%{?dist}
Summary:	WS23XX weather station utility

License:	GPLv3+
#URL:		
Source0:	http://ftp.gnu.org/gnu/%{name}/%{name}-%{version}.tar.gz

#BuildRequires:	
#Requires:	

%description
A WS23XX weather station utility.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
%make_install

%files
%{_bindir}/ws2300

%changelog
* Wed Feb 25 2015 Tue Sep 06 2011 <bubuche.pub@free.fr>
- Initial version of the package
