#
# SC3020
# Project 2
# Preprocessing Unit Tests
#

from pathlib import Path

import pytest

from preprocessing import ColumnEnricher, Postgres, enrich, transform


@pytest.fixture
def query_sqls() -> list[str]:
    query_dir = Path(__file__).parent / "queries"
    return [query_file.read_text() for query_file in query_dir.glob("*.sql")]


def test_postgres_explain(db: Postgres, query_sqls: list[str]):
    # run explain on each query in queries
    for sql in query_sqls:
        db.explain(sql)


def test_postgres_enrich_columns(db: Postgres, query_sqls: list[str]):
    # run explain on each query in queries
    # test: TPC-H 4th query 4.sql
    sql = query_sqls[3]
    plan = db.explain(sql)
    plan = enrich(plan, [ColumnEnricher(sql)])

    col_nodes = []

    def collect_cols(qep_node: dict, depth: int):
        if "Columns" in qep_node:
            col_nodes.append(qep_node)
        return qep_node

    transform(plan, collect_cols)

    assert len(col_nodes) == 2
    assert col_nodes[0]["Columns"] == [
        "s_acctbal",
        "s_name",
        "n_name",
        "p_partkey",
        "p_mfgr",
        "s_address",
        "s_phone",
        "s_comment",
    ]
    assert col_nodes[1]["Columns"] == ["MIN(ps_supplycost)"]


def test_postgres_get_primary_key(db: Postgres):
    assert db.get_primary_key("customer") == "c_custkey"
