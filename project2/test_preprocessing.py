#
# SC3020
# Project 2
# Preprocessing Unit Tests
#

import os
from pathlib import Path

import pytest

from preprocessing import (
    CTENameTransformer,
    IndexKeyTransformer,
    Postgres,
    apply,
    preprocess,
    transform,
)


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
    plan = transform(plan, [CTENameTransformer()])

    nodes = []

    def collect_cte(qep_node: dict, depth: int):
        if qep_node["Node Type"] == "CTE Scan" and "Relation Name" in qep_node:
            nodes.append(qep_node)
        return qep_node

    apply(plan, collect_cte)

    assert len(nodes) == 2
    assert all(n["Relation Name"] == n["CTE Name"] for n in nodes)


def test_preprocess(db: Postgres, query_sqls: list[str]):
    for sql in query_sqls:
        preprocess(sql, db)
