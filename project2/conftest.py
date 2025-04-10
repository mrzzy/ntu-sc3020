#
# SC3020
# Project 2
# Pytest fixtures
#

import os
from pathlib import Path

import pytest

from preprocessing import Postgres


@pytest.fixture
def db() -> Postgres:
    return Postgres(
        host="localhost", user="postgres", password=os.environ["POSTGRES_PASSWORD"]
    )


@pytest.fixture
def query_sqls() -> list[str]:
    """Fixture of the TPC-H queries in order of ascending query id"""
    query_dir = Path(__file__).parent / "queries"

    def get_query_id(query_path: Path):
        return int(os.path.basename(query_path).split(".")[0])

    query_files = sorted(query_dir.glob("*"), key=get_query_id)
    return [query_file.read_text() for query_file in query_files]
