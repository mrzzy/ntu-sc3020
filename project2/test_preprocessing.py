#
# SC3020
# Project 2
# Preprocessing Unit Tests
#

import json
import os
from pathlib import Path

from preprocessing import Postgres


def test_postgres_explain(db: Postgres):
    # run explain on each query in queries
    query_dir = Path(__file__).parent / "queries"
    for query_file in query_dir.glob("*.sql"):
        db.explain(query_file.read_text())
