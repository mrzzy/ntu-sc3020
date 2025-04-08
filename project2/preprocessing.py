#
# SC3020
# Project 2
# Preprocessing
#

from abc import ABC, abstractmethod
from typing import Callable, Iterable

import psycopg
from sqlglot import exp, parse_one


class Postgres:
    """Postgres DB facade."""

    def __init__(self, **conn_args) -> None:
        self.connection = psycopg.connect(**conn_args)

    def explain(self, sql: str) -> dict:
        """Fetch the query plan of the given sql statement."""
        # fetch query plan by executing 'EXPLAIN'
        with self.connection.transaction():
            cursor = self.connection.execute("""EXPLAIN (VERBOSE, FORMAT JSON)""" + sql)  # type: ignore
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


class Enricher(ABC):
    """QEP node enricher that takes as input a QEP node, enriches it and returns it"""

    @abstractmethod
    def enrich(self, qep_node: dict, depth: int) -> dict:
        """Enrich & return the given QEP node that given depth (root = 0 depth)."""


class ProjectionEnricher(Enricher):
    """QEP node enricher that adds 'Projections' for SELECT nodes from SQL query."""

    def __init__(self, sql: str):
        ast = parse_one(sql, dialect="postgres")
        self.selects = ast.find_all(exp.Select)

    def enrich(self, qep_node: dict, depth: int) -> dict:
        # match plan nodes that are select statements using heurstics:
        # - top-level nodes are top-level select statements
        # - init plan & subplans are nested select statements
        if depth == 0 or qep_node.get("Parent Relationship", "") in [
            "InitPlan",
            "SubPlan",
        ]:
            try:
                select = next(self.selects)
            except StopIteration:
                raise RuntimeError("QEP Plan node & Select AST count mismatch")
            qep_node["Projections"] = [str(c) for c in select.expressions]
        return qep_node


class PrimaryKeyEnricher(Enricher):
    """QEP node enricher that adds 'Primary Key' for Table / Relation nodes"""

    def __init__(self, db: Postgres):
        self.db = db

    def enrich(self, qep_node: dict, depth: int) -> dict:
        relation = "Relation Name"
        if relation in qep_node:
            qep_node["Primary Key"] = self.db.get_primary_key(qep_node[relation])
        return qep_node


def transform(plan: dict, fn: Callable[[dict, int], dict]) -> dict:
    """Transform the given QEP using the given transform fn.
    Traverses the given QEP nodes post-order and calling the given transform fn on each node.
    Transform fn is given QEP node & depth (root=0)."""

    def dfs(
        qep_node: dict,
        depth: int,
    ):
        """Perform post-order DFS to enrich the QEP plan nodes"""
        # transform using given fn
        qep_node = fn(qep_node, depth)
        # recursively traverse children if any
        if "Plans" in qep_node:
            for child in qep_node["Plans"]:
                dfs(child, depth=depth + 1)
        return plan

    plan = dfs(plan, depth=0)
    return plan


def enrich(plan: dict, enrichers: Iterable[Enricher]) -> dict:
    """Enrich the query execution plan using the given entrichers."""

    def enrich_fn(qep_node: dict, depth: int):
        for enricher in enrichers:
            qep_node = enricher.enrich(qep_node, depth)
        return qep_node

    return transform(plan, enrich_fn)


def preprocess(sql: str, db: Postgres) -> dict:
    """Parses, preprocess given SQL into enriched QEP plan using the given Postgres DB."""
    plan = db.explain(sql)
    plan = enrich(plan, [ProjectionEnricher(sql), PrimaryKeyEnricher(db)])
    return plan
