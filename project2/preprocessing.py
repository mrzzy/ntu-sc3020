#
# SC3020
# Project 2
# Preprocessing
#

import os
from typing import Iterator

import psycopg
from sqlglot import exp
from sqlglot import parse_one
from sqlglot.optimizer.scope import build_scope


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


def enrich(plan: dict, sql: str) -> dict:
    """Enrich the given query execution plan using elements parsed from the given SQL."""
    ast = parse_one(sql, dialect="postgres")

    def annotate_cols(
        plan: dict,
        selects: Iterator[exp.Select],
        top_level: bool = True,
    ):
        """Perform post-order DFS to annotate the given QEP plan with columns from the given SELECT AST nodes.
        Matching QEP Plan nodes will have a 'Columns' key set a list of matched columns.
        """
        # match plan nodes that are select statements using heurstics:
        # - top-level nodes are top-level select statements
        # - init plan & subplans are nested select statements
        if top_level or plan.get("Parent Relationship", "") in ["InitPlan", "SubPlan"]:
            try:
                select = next(selects)
            except StopIteration:
                raise RuntimeError("QEP Plan node & Select AST count mismatch")
            plan["Columns"] = [str(c) for c in select.expressions]

        # recursively traverse children if any
        if "Plans" in plan:
            for child in plan["Plans"]:
                annotate_cols(child, selects, top_level=False)
        return plan

    plan = annotate_cols(plan, selects=ast.find_all(exp.Select))
    return plan


def main(qep):
    return None
