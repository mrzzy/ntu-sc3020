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
        with self.connection:
            cursor = self.connection.execute("EXPLAIN (FORMAT JSON) " + sql)  # type: ignore
            result = cursor.fetchone()
        if result is None:
            raise RuntimeError("fetch of query plan of SQL returned no results.")
        return result[0][0]["Plan"]


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
