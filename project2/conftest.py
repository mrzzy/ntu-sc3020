#
# SC3020
# Project 2
# Pytest fixtures
#

import os
from pathlib import Path
from typing import Generator, Iterable

import docker
import psycopg
import pytest
from testcontainers.compose import DockerCompose
from testcontainers.core.container import wait_for_logs

from preprocessing import Postgres


@pytest.fixture(scope="module")
def db() -> Iterable[Postgres]:
    with DockerCompose(".", keep_volumes=True, wait=True) as compose:
        yield Postgres(
            host="localhost",
            port=5432,
            user="postgres",
            password=os.environ["POSTGRES_PASSWORD"],
        )


@pytest.fixture
def query_sqls() -> list[str]:
    """Fixture of the TPC-H queries in order of ascending query id"""
    query_dir = Path(__file__).parent / "queries"

    def get_query_id(query_path: Path):
        return int(os.path.basename(query_path).split(".")[0])

    query_files = sorted(query_dir.glob("*"), key=get_query_id)
    return [query_file.read_text() for query_file in query_files]
