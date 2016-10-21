Name:		greenfx-tickq-to-kairosdb
Version:	1.0
Release:	0.1
Summary:	Collect and publish ticks from Oanda

Group:	        Applications
License:	GPL
URL:		http://github.com/atgreen/greenfx-tickq-to-kairosdb
Source0:	greenfx-tickq-to-kairosdb-1.0.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  libcurl-devel activemq-cpp-devel json-c-devel boost-devel

%description
Pull ticks from message bus and archive in kairosdb.

%prep 
%setup -q

%build
autoreconf
%configure
make %{?_smp_mflags}

%pre
getent group greenfx >/dev/null || groupadd -r greenfx
getent passwd greenfx >/dev/null || \
    useradd -r -m -g greenfx -d /var/lib/greenfx -s /sbin/nologin \
    -c "GreenFX Service Account" greenfx
chmod 755 /var/lib/greenfx
exit 0

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/var/lib/greenfx/tickq-to-kairosdb

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/*
%attr(0777,greenfx,greenfx) %dir /var/lib/greenfx/tickq-to-kairosdb

%changelog
* Thu Sep 29 2016 Anthony Green <anthony@atgreen.org> 1.0-1
- Created.
