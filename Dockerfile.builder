FROM docker.io/centos:7

MAINTAINER anthony@atgreen.org

RUN rpm -ivh http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-8.noarch.rpm && \
    yum -y install \
      rpm-build gcc-c++ gcc make patch \
      libcurl-devel activemq-cpp-devel json-c-devel boost-devel \
      autoconf automake && \
    mkdir -p /root/rpmbuild/SOURCES

CMD cd /opt && autoreconf --install && \
    ./configure && \
    make dist && \
    chown -R root.root * && \
    mv greenfx-tickq-to-kairosdb-1.0.tar.gz /root/rpmbuild/SOURCES && \
    rpmbuild -ba greenfx-tickq-to-kairosdb.spec && \
    mv /root/rpmbuild/RPMS/x86_64/*.rpm /opt
