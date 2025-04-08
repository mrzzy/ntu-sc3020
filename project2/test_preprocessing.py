#
# SC3020
# Project 2
# Preprocessing Unit Tests
#

import os
from pathlib import Path

import pytest

from preprocessing import (
    Postgres,
    PrimaryKeyEnricher,
    ProjectionEnricher,
    enrich,
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


def test_postgres_get_primary_key(db: Postgres):
    assert db.get_primary_key("customer") == "c_custkey"


def test_postgres_enrich_projections(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 2th query 2.sql
    sql = query_sqls[1]
    plan = db.explain(sql)
    plan = enrich(plan, [ProjectionEnricher(sql)])

    proj_nodes = []

    def collect_proj(qep_node: dict, depth: int):
        if "Projections" in qep_node:
            proj_nodes.append(qep_node)
        return qep_node

    transform(plan, collect_proj)

    assert len(proj_nodes) == 2
    assert proj_nodes[0]["Projections"] == [
        "s_acctbal",
        "s_name",
        "n_name",
        "p_partkey",
        "p_mfgr",
        "s_address",
        "s_phone",
        "s_comment",
    ]
    assert proj_nodes[1]["Projections"] == ["MIN(ps_supplycost)"]


def test_postgres_enrich_primary_key(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 13th query 13.sql
    plan = db.explain(query_sqls[12])
    plan = enrich(plan, [PrimaryKeyEnricher(db)])

    pk_nodes = []

    def collect_pk(qep_node: dict, depth: int):
        if "Primary Key" in qep_node:
            pk_nodes.append(qep_node)
        return qep_node

    transform(plan, collect_pk)

    assert len(pk_nodes) == 2

    assert len(pk_nodes) == 2
    assert pk_nodes[0]["Primary Key"] == "o_orderkey"
    assert pk_nodes[1]["Primary Key"] == "c_custkey"


def test_preprocess(db: Postgres, query_sqls: list[str]):
    for sql in query_sqls:
        preprocess(sql, db)
