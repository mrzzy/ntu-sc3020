#
# SC3020
# Project 2
# QEP Preprocessing
#

import re
from abc import ABC, abstractmethod
from typing import Callable, Iterable, Optional

import psycopg
import sqlglot


## Postgres integration
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


## QEP preprocessing
# regular expressions for matching
SUBPLAN_REGEX = re.compile(r"\((SubPlan \d+)\)")
CORRECT_ARRAY_REGEX = re.compile(
    r"CAST\('\{(?P<elements>.*)\}' AS ARRAY<(?P<dtype>.*)>\)"
)

# qep node keys with lists of expressions
EXPR_LIST_KEYS = {"Group Key", "Sort Key", "Output"}
# qep node keys with single atomic expressions
EXPR_SINGLE_KEYS = {
    "Filter",
    "Join Filter",
    "Hash Cond",
    "Index Cond",
    "Merge Cond",
    "Recheck Cond",
}
# qep node keys with filter 'WHERE' conditions
FILTER_KEYS = [
    "Index Cond",
    "Recheck Cond",
    "Filter",
    "Join Filter",
]
# qep node keys with join 'ON' conditions
JOIN_ON_KEYS = [
    "Hash Cond",
    "Merge Cond",
]


def apply(plan: dict, transform: Callable[[dict, int, str], dict]) -> dict:
    """Apply the given transform fn on the given QEP.
    Traverses the given QEP nodes post-order and calling the given transform fn on each node.
    Transform fn is given QEP node & depth (root=0)."""

    def dfs(qep_node: dict, depth: int, subplan: str):
        """Perform post-order DFS to transform the QEP plan nodes"""
        # update subplan name if recursed into a different subplan
        key = "Subplan Name"
        if key in qep_node:
            subplan = qep_node[key]

        # transform using given fn
        qep_node = transform(qep_node, depth, subplan)
        # recursively traverse children if any
        if "Plans" in qep_node:
            for child in qep_node["Plans"]:
                dfs(child, depth + 1, subplan)
        return plan

    plan = dfs(plan, depth=0, subplan="MainPlan")
    return plan


def correct_sql_arrays(expr: str) -> str:
    """Correct SQLGlot transpiled arrays in Pipeline SQL syntax

    SQLGlot transpiles Postgres arrays to Pipeline SQL syntax as:
    eg. CAST('{16,18,47,11,1,42,10,27}' AS ARRAY<INT64>))))

    Correct to:
    eg. [CAST('16' AS INT64), CAST('18' AS INT64), CAST('47' AS INT64), CAST('11' AS INT64), CAST('1' AS INT64), CAST('42' AS INT64), CAST('10' AS INT64), CAST('27' AS INT64))]

    Args:
        expr: SQL expression to correct
    Returns:
        str: Corrected SQL expression
    """

    def rewrite(m: re.Match) -> str:
        """Rewrite the matched array to the correct Pipeline SQL syntax"""
        elements = m.group("elements").split(",")
        dtype = m.group("dtype")
        # create casted array
        return f"[{', '.join([f'CAST({e} AS {dtype})' for e in elements])}]"

    return re.sub(CORRECT_ARRAY_REGEX, rewrite, expr)


class Transformer(ABC):
    """QEP node transformer that takes as input a QEP node, transforms it and returns it"""

    @abstractmethod
    def transform(self, qep_node: dict, depth: int, subplan: str) -> dict:
        """Transform & return the given QEP node

        Args:
            qep_node: QEP node to transform.
            depth: Depth of the QEP node in the tree.
            subplan: Subplan name.

        Returns:
            dict: Transformed QEP node
        """
        pass


class IndexKeyTransformer(Transformer):
    """QEP node transformer that adds index key columns for index nodes"""

    def __init__(self, db: Postgres):
        self.db = db

    def transform(self, qep_node: dict, depth: int, subplan: str) -> dict:
        relation = "Index Name"
        if relation in qep_node:
            qep_node["Index Key"] = self.db.get_index_key(qep_node[relation])
        return qep_node


class SubplanNameTransformer(Transformer):
    """QEP node transformer that conforms Subplan naming."""

    def transform(self, qep_node: dict, depth: int, subplan: str) -> dict:
        key = "Subplan Name"
        if key in qep_node:
            # remove nonstandard 'CTE ' prefix from subplan names
            qep_node[key] = qep_node[key].replace("CTE ", "")
        return qep_node


class ExprTransformer(Transformer):
    """QEP node transform that rewrites expressions in in the QEP node."""

    def transform(self, qep_node: dict, depth: int, subplan: str) -> dict:
        for key in qep_node.keys():
            if key in EXPR_LIST_KEYS:
                qep_node[key] = [self.rewrite(p, depth, subplan) for p in qep_node[key]]
            if key in EXPR_SINGLE_KEYS:
                qep_node[key] = self.rewrite(qep_node[key], depth, subplan)

        return qep_node

    @abstractmethod
    def rewrite(self, expr: str, depth: int, subplan: str) -> str:
        """Rewrite & return the given expression"""


class DialectTransformer(ExprTransformer):
    """QEP node transform that transforms Postgres QEP dialect into Pipeline SQL dialect."""

    COL_REGEX = re.compile(r"\.col[0-9]+")
    INITPLAN_REGEX = re.compile(r"\((InitPlan \d+)\)")
    ORDER_REGEX = re.compile(r"AS `(ASC|DESC)`")

    def rewrite(self, expr: str, depth: int, subplan: str) -> str:
        """Rewrite the given Postgres QEP expression into Pipeline SQL dialect."""
        # clean up non sql "hashed" and ".colX" in expr
        expr = expr.replace("hashed ", "")
        expr = re.sub(self.COL_REGEX, "", expr)

        # quote & remove parathensis around initplan references
        expr = re.sub(self.INITPLAN_REGEX, lambda m: f'"{m[1]}"', expr)

        # quote subplan references
        expr = re.sub(SUBPLAN_REGEX, lambda m: f'"{m[0]}"', expr)

        # transpile to pipeline sql dialect (bigquery)
        results = sqlglot.transpile(expr, read="postgres", write="bigquery")
        if len(results) != 1:
            raise ValueError(
                "Expected SQLGlot to transpile expression to 1 statement, got: ",
                len(results),
            )
        expr = results[0]

        # correct types
        expr = expr.replace("BPCHAR", "STRING")

        # correct order direction specifiers ("ASC", "DESC") wrongly transpiled as aliases
        expr = re.sub(self.ORDER_REGEX, r"\1", expr)

        # correct array syntax
        expr = correct_sql_arrays(expr)

        return expr


class CTETransformer(Transformer):
    """QEP node transformer conforms 'CTE Scan' to other scan nodetypes."""

    def transform(self, qep_node: dict, depth: int, subplan: str) -> dict:
        if qep_node["Node Type"] == "CTE Scan":
            qep_node["Relation Name"] = qep_node["CTE Name"]
        return qep_node


class JoinKeyTransformer(Transformer):
    """QEP node transformer standardises Join condition "On" keys"""

    def transform(self, qep_node: dict, depth: int, subplan: str) -> dict:
        if "Join Type" in qep_node:
            for key in JOIN_ON_KEYS:
                if key in qep_node:
                    if "Join On" in qep_node:
                        raise ValueError("QEP Node already has 'Join On' condition")
                    # add join condition to QEP node
                    qep_node["Join On"] = qep_node[key]
                    break
        return qep_node


class FilterTransformer(Transformer):
    """QEP node transformer standardises filter condition keys."""

    def transform(self, qep_node: dict, depth: int, subplan: str) -> dict:
        for key in FILTER_KEYS:
            if key in qep_node:
                filters = qep_node.get("Filters", [])
                # add filter condition to QEP node
                filters.append(qep_node[key])
                qep_node["Filters"] = filters
        return qep_node


def transform(plan: dict, transformers: Iterable[Transformer]) -> dict:
    """Transform the query execution plan using the given transformers."""

    def apply_all(qep_node: dict, depth: int, subplan: str):
        for transformer in transformers:
            qep_node = transformer.transform(qep_node, depth, subplan)
        return qep_node

    return apply(plan, apply_all)


def pushup_aliases(plan: dict) -> dict:
    """Push up aliases in the QEP plan from child to parent.
    Only push up aliases if the parent only has one child plan.
    """

    def pushup(plan: dict) -> Optional[str]:
        if "Plans" in plan:
            # pushup child aliases
            results = [pushup(child) for child in plan["Plans"]]
            aliases = [alias for alias in results if alias is not None]
            # only pushup alias from child if parent does not have one
            # and the parent has only 1 child with alias
            if "Alias" not in plan and len(aliases) == 1:
                plan["Alias"] = aliases[0]

        if "Alias" in plan:
            return plan["Alias"]
        return None

    pushup(plan)
    return plan


def preprocess(sql: str, db: Postgres) -> dict:
    """Parses, preprocess given SQL into transformed QEP plan using the given Postgres DB."""
    plan = db.explain(sql)
    # 1st pass
    plan = transform(
        plan,
        [
            IndexKeyTransformer(db),
            SubplanNameTransformer(),
            DialectTransformer(),
            CTETransformer(),
            JoinKeyTransformer(),
            FilterTransformer(),
        ],
    )
    plan = pushup_aliases(plan)
    return plan
