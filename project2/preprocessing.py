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
            # disable parallel query planning to simplify plan retrieved
            self.connection.execute("SET max_parallel_workers_per_gather = 0;")
            # fetch verbose plan to include projected columns
            cursor = self.connection.execute("""EXPLAIN (VERBOSE, FORMAT JSON)""" + sql)  # type: ignore
            result = cursor.fetchone()
        if result is None:
            raise RuntimeError("fetch of query plan of SQL returned no results.")
        return result[0][0]["Plan"]

    def get_index_key(self, index: str, schema: str = "public") -> list[str]:
        """Get the key columns of the given index / relation in the given schema.

        Args:
            index (str): Index name
            schema (str): Schema name

        Returns:
            list[str]: List of index key column names
        """
        # query primary key from information schema
        with self.connection.transaction():
            cursor = self.connection.execute(
                """
SELECT
    a.attname
FROM
    pg_index ix
JOIN
    pg_class i ON i.oid = ix.indexrelid
JOIN
    pg_attribute a ON a.attrelid = ix.indrelid
                 AND a.attnum = ANY(ix.indkey)
JOIN
    pg_namespace n ON n.oid = i.relnamespace
WHERE
    i.relname = %s
    AND n.nspname = %s;
        """,
                (index, schema),
            )
            result = cursor.fetchall()
        if result is None:
            raise RuntimeError("Fetch of index key columns returned no results.")

        return [r[0] for r in result]


class Transformer(ABC):
    """QEP node transformer that takes as input a QEP node, transforms it and returns it"""

    @abstractmethod
    def transform(self, qep_node: dict, depth: int) -> dict:
        """Transform & return the given QEP node that given depth (root = 0 depth)."""


class IndexKeyTransformer(Transformer):
    """QEP node transformer that adds index key columns for index nodes"""

    def __init__(self, db: Postgres):
        self.db = db

    def transform(self, qep_node: dict, depth: int) -> dict:
        relation = "Index Name"
        if relation in qep_node:
            qep_node["Index Key"] = self.db.get_index_key(qep_node[relation])
        return qep_node


class CTENameTransformer(Transformer):
    """QEP node transformer that rename 'CTE' key to 'Relation Name' in 'CTE Scan' nodetypes."""

    def transform(self, qep_node: dict, depth: int) -> dict:
        if qep_node["Node Type"] == "CTE Scan":
            qep_node["Relation Name"] = qep_node["CTE Name"]
        return qep_node


def apply(plan: dict, transform: Callable[[dict, int], dict]) -> dict:
    """Apply the given transform fn on the given QEP.
    Traverses the given QEP nodes post-order and calling the given transform fn on each node.
    Transform fn is given QEP node & depth (root=0)."""

    def dfs(
        qep_node: dict,
        depth: int,
    ):
        """Perform post-order DFS to transform the QEP plan nodes"""
        # transform using given fn
        qep_node = transform(qep_node, depth)
        # recursively traverse children if any
        if "Plans" in qep_node:
            for child in qep_node["Plans"]:
                dfs(child, depth=depth + 1)
        return plan

    plan = dfs(plan, depth=0)
    return plan


def transform(plan: dict, transformers: Iterable[Transformer]) -> dict:
    """Transform the query execution plan using the given transformers."""

    def apply_all(qep_node: dict, depth: int):
        for transformer in transformers:
            qep_node = transformer.transform(qep_node, depth)
        return qep_node

    return apply(plan, apply_all)


def preprocess(sql: str, db: Postgres) -> dict:
    """Parses, preprocess given SQL into transformed QEP plan using the given Postgres DB."""
    plan = db.explain(sql)
    plan = transform(plan, [IndexKeyTransformer(db), CTENameTransformer()])
    return plan
