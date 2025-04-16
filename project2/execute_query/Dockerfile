#
# SC3020
# Docker
# ZettaSQL Execute Query
#

FROM ubuntu:24.04

# install time zone information need by execute_query
RUN apt-get update && apt-get install --no-install-recommends -y tzdata && \
    rm -rf /var/lib/apt/lists/*
# install zetasql execute_query
ADD https://github.com/google/zetasql/releases/download/2025.03.1/execute_query_linux /usr/local/bin/execute_query
RUN chmod u+x /usr/local/bin/execute_query

ENTRYPOINT [ "/usr/local/bin/execute_query" ]
