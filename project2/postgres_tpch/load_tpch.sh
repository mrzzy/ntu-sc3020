#
# SC3020
# Postgres with TPC-H Data
# InitDB Script
#

set -ueo pipefail

# download TPC-H data from huggingface
# pg_tpch expects data at /tmp/dss-data
echo "pg_tpch: downloading data"
mkdir -p /tmp/dss-data
for file in customer.csv  lineitem.csv  nation.csv  orders.csv  part.csv  partsupp.csv  region.csv  supplier.csv
do 
  curl -LO --output-dir /tmp/dss-data  https://huggingface.co/datasets/mrzzy/tpc-h/resolve/main/$file
done

# use pg_tpch sql scripts to load TPC-H data
echo "pg_tpch: loading data"
ls -l /pg_tpch/tpch-load.sql
cat /pg_tpch/tpch-load.sql
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" < /pg_tpch/tpch-load.sql

echo "pg_tpch: creating primary keys"
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" < /pg_tpch/tpch-pkeys.sql

echo "pg_tpch: creating foreign keys"
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" < /pg_tpch/tpch-alter.sql

echo "pg_tpch: creating indexes"
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" < /pg_tpch/tpch-index.sql

# cleanup data files no longer needed
rm -rf /tmp/dss-data

