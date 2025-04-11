#
# SC3020
# Postgres with TPC-H Data
# InitDB Script
#

set -ueo pipefail

# link data files to /tmp/dss-data
# pg_tpch expects data at /tmp/dss-data
echo "pg_tpch: linking data"
mkdir -p /tmp/dss-data
for file in customer.csv  lineitem.csv  nation.csv  orders.csv  part.csv  partsupp.csv  region.csv  supplier.csv
do 
  ln -sf /data/$file /tmp/dss-data/$file 
done

# use pg_tpch sql scripts to load TPC-H data
echo "pg_tpch: loading data"
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" < /pg_tpch/tpch-load.sql

echo "pg_tpch: creating primary keys"
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" < /pg_tpch/tpch-pkeys.sql

echo "pg_tpch: creating foreign keys"
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" < /pg_tpch/tpch-alter.sql

echo "pg_tpch: creating indexes"
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" < /pg_tpch/tpch-index.sql
