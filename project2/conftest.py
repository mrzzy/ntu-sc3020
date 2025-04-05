#
# SC3020
# Project 2
# Pytest fixtures
#

import os
import pytest

from preprocessing import Postgres


@pytest.fixture
def db() -> Postgres:
    return Postgres(
        host="localhost", user="postgres", password=os.environ["POSTGRES_PASSWORD"]
    )
