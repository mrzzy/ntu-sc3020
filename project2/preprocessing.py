#
# SC3020
# Project 2
# Preprocessing
#

import os

import psycopg


class Postgres:
    def __init__(self, **conn_args) -> None:
        self.connection = psycopg.connect(**conn_args)

    def explain(self, sql: str) -> dict:
        """Fetch the query plan of the given sql statement."""
        # fetch query plan by executing 'EXPLAIN'
        with self.connection.transaction():
            cursor = self.connection.execute("EXPLAIN (FORMAT JSON)\n" + sql)  # type: ignore
            result = cursor.fetchone()
        if result is None:
            raise RuntimeError("fetch of query plan of SQL returned no results.")
        return result[0][0]["Plan"]

    def get_primary_key(self, table: str, schema: str = "public") -> str:
        """Get the primary key of the given table / relation in the given schema."""
        # query primary key from information schema
        with self.connection.transaction():
            cursor = self.connection.execute(
                """
SELECT
    kc.table_schema,
    kc.table_name,
    kc.column_name
FROM
    information_schema.table_constraints tc
JOIN
    information_schema.key_column_usage kc
    ON tc.constraint_name = kc.constraint_name
    AND tc.table_schema = kc.table_schema
WHERE
    tc.constraint_type = 'PRIMARY KEY'
    AND kc.table_name = %s
    AND kc.table_schema = %s
    """,
                (table, schema),
            )
            result = cursor.fetchone()
        if result is None:
            raise RuntimeError("fetch of query plan of SQL returned no results.")

        # result: (schema, table, primary key)
        return result[2]


def main(qep):
    return None


if __name__ == "__main__":
    print(
        Postgres(
            host="localhost", user="postgres", password=os.environ["POSTGRES_PASSWORD"]
        ).explain(
            """
                       select
	sum(l_extendedprice * l_discount) as revenue
from
	lineitem
where
	l_shipdate >= date '1997-01-01'
	and l_shipdate < date '1997-01-01' + interval '1' year
	and l_discount between 0.02 - 0.01 and 0.02 + 0.01
	and l_quantity < 25
LIMIT 1;
                       """
        )
    )
