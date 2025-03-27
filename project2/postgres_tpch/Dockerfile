#
# SC3020
# Docker
# Postgres with TPC-H
#

FROM postgres:17.4

# install curl using apt-get
RUN apt-get update && \
    apt-get install -y --no-install-recommends curl ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# install pg_tpch sql scripts for loading TPC-H
RUN mkdir -p /pg_tpch
ADD https://raw.githubusercontent.com/tvondra/pg_tpch/refs/heads/master/dss/tpch-load.sql /pg_tpch
ADD https://raw.githubusercontent.com/tvondra/pg_tpch/refs/heads/master/dss/tpch-pkeys.sql /pg_tpch
ADD https://raw.githubusercontent.com/tvondra/pg_tpch/refs/heads/master/dss/tpch-alter.sql /pg_tpch
ADD https://raw.githubusercontent.com/tvondra/pg_tpch/refs/heads/master/dss/tpch-index.sql /pg_tpch
RUN chown -R postgres /pg_tpch

# load_tpch.sh loads TPC-H into postgres on startup
COPY load_tpch.sh /docker-entrypoint-initdb.d/
