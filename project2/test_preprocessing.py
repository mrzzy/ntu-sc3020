#
# SC3020
# Project 2
# Preprocessing Unit Tests
#

import os
from copy import deepcopy
from pathlib import Path

import pytest

from preprocessing import (
    EXPR_LIST_KEYS,
    EXPR_SINGLE_KEYS,
    CTETransformer,
    DialectTransformer,
    IndexKeyTransformer,
    Postgres,
    apply,
    correct_sql_arrays,
    preprocess,
    transform,
)


def test_correct_arrays():
    test_cases = [
        (
            "CAST('{16,18,47,11,1,42,10,27}' AS ARRAY<INT64>)",
            "[CAST(16 AS INT64), CAST(18 AS INT64), CAST(47 AS INT64), CAST(11 AS INT64), CAST(1 AS INT64), CAST(42 AS INT64), CAST(10 AS INT64), CAST(27 AS INT64)]",
        ),
        (
            "CAST('{a,b,c}' AS ARRAY<VARCHAR>)",
            "[CAST(a AS VARCHAR), CAST(b AS VARCHAR), CAST(c AS VARCHAR)]",
        ),
    ]

    for expr, expected in test_cases:
        assert correct_sql_arrays(expr) == expected


@pytest.fixture
def query_sqls() -> list[str]:
    """Fixture of the TPC-H queries in order of ascending query id"""
    query_dir = Path(__file__).parent / "queries"

    def get_query_id(query_path: Path):
        return int(os.path.basename(query_path).split(".")[0])

    query_files = sorted(query_dir.glob("*"), key=get_query_id)
    return [query_file.read_text() for query_file in query_files]


def test_postgres_explain(db: Postgres, query_sqls: list[str]):
    # run explain on each query in queries
    for sql in query_sqls:
        db.explain(sql)


def test_postgres_get_index_key(db: Postgres):
    assert db.get_index_key("idx_lineitem_part_supp") == ["l_partkey", "l_suppkey"]


def test_index_key_transform(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 8th query 8.sql
    plan = db.explain(query_sqls[8 - 1])
    plan = transform(plan, [IndexKeyTransformer(db)])

    nodes = []

    def collect_key(qep_node: dict, depth: int):
        if "Index Key" in qep_node:
            nodes.append(qep_node)
        return qep_node

    apply(plan, collect_key)

    assert len(nodes) == 4
    keys = [n["Index Key"] for n in nodes]
    assert keys == [
        [
            "l_partkey",
            "l_suppkey",
        ],
        [
            "o_orderkey",
        ],
        [
            "c_nationkey",
        ],
        [
            "n_nationkey",
        ],
    ]


def test_cte_name_transform(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 15th query 15.sql
    plan = db.explain(query_sqls[15 - 1])
    plan = transform(plan, [CTETransformer()])

    nodes = []

    def collect_cte(qep_node: dict, depth: int):
        if qep_node["Node Type"] == "CTE Scan" and "Relation Name" in qep_node:
            nodes.append(qep_node)
        return qep_node

    apply(plan, collect_cte)

    assert len(nodes) == 2
    assert all(n["Relation Name"] == n["CTE Name"] for n in nodes)


def test_dialect_transform(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 16th query 16.sql
    plan = db.explain(query_sqls[16 - 1])
    new_plan = transform(deepcopy(plan), [DialectTransformer(db)])

    def collect_to(exprs: list[str]):
        def collect_expr(qep_node: dict, depth: int):
            for key in qep_node.keys():
                if key in EXPR_LIST_KEYS:
                    exprs.extend(qep_node[key])
                if key in EXPR_SINGLE_KEYS:
                    exprs.append(qep_node[key])
            return qep_node

        return collect_expr

    old_exprs, exprs = [], []
    apply(plan, collect_to(old_exprs))
    apply(new_plan, collect_to(exprs))

    assert len(exprs) == 46
    assert (
        old_exprs[34] == "(NOT (ANY (partsupp.ps_suppkey = (hashed SubPlan 1).col1)))"
    )
    assert exprs[34] == "(NOT (ANY(partsupp.ps_suppkey = `(SubPlan 1)`)))"
    assert (
        old_exprs[45]
        == "((part.p_brand <> 'Brand#53'::bpchar) AND ((part.p_type)::text !~~ 'SMALL POLISHED%'::text) AND (part.p_size = ANY ('{16,18,47,11,1,42,10,27}'::integer[])))"
    )
    assert (
        exprs[45]
        == "((part.p_brand <> CAST('Brand#53' AS STRING)) AND (NOT CAST((part.p_type) AS STRING) LIKE CAST('SMALL POLISHED%' AS STRING)) AND (part.p_size = ANY([CAST(16 AS INT64), CAST(18 AS INT64), CAST(47 AS INT64), CAST(11 AS INT64), CAST(1 AS INT64), CAST(42 AS INT64), CAST(10 AS INT64), CAST(27 AS INT64)])))"
    )


def test_preprocess(db: Postgres, query_sqls: list[str]):
    for sql in query_sqls:
        preprocess(sql, db)
