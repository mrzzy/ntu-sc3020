#
# SC3020
# Project 2
# Preprocessing Unit Tests
#

import pytest
import json
import os
from pathlib import Path

from preprocessing import Postgres, enrich


def test_postgres_explain(db: Postgres):
    # run explain on each query in queries
    query_dir = Path(__file__).parent / "queries"
    for query_file in query_dir.glob("*.sql"):
        db.explain(query_file.read_text())


def test_postgres_enrich(db: Postgres):
    # run explain on each query in queries
    query_dir = Path(__file__).parent / "queries"
    for query_file in query_dir.glob("*.sql"):
        sql = query_file.read_text()
        plan = db.explain(sql)
        __import__('pprint').pprint(enrich(plan, sql))


def test_postgres_get_primary_key(db: Postgres):
    assert db.get_primary_key("customer") == "c_custkey"
