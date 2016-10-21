FROM docker.io/centos:7

MAINTAINER anthony@atgreen.org

RUN rpm -ivh http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-8.noarch.rpm
ADD greenfx-tickq-to-kairosdb-1.0-0.1.x86_64.rpm /tmp
RUN yum install -y /tmp/greenfx-tickq-to-kairosdb-1.0-0.1.x86_64.rpm && \
    yum -y update && yum clean all && \
    rm /tmp/*.rpm

USER 1001
CMD tickq-to-kairosdb

