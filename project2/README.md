# SC3020 Project 2

## Project Structure

This project consists of four main Python files:

- **interface.py**: Contains the code for the GUI.
- **pipesyntax.py**: Responsible for generating the pipe-syntax version of a query.
- **preprocessing.py**: Handles input reading and any preprocessing necessary for the algorithm.
- **project.py**: The main entry point that orchestrates the necessary functions from the other three files.

## Setup

### Postgres with TPC-H

Start PostgreSQL database configured with TPC-H data using Docker Compose for development:

1. Ensure **Docker** and **Docker Compose** are installed.
2. Define `POSTGRES_PASSWORD` in a `.env` file or export it as an environment variable.
3. Start the database with:

   ```bash
   docker-compose up
   ```
   > **Note:** On first startup, the database container will take additional time to download and load the TPC-H dataset.
   > Look for `database system is ready to accept connections` line in the database's log to signal that load of TPC-H is complete.

4. Connect using `psql` or any PostgreSQL client with:
   - **Host:** `localhost`
   - **Port:** `5432`
   - **User:** `postgres`
   - **Password:** `${POSTGRES_PASSWORD}`

### Project Setup

Ensure you have Python installed. Clone the repository and install the dependencies:

```sh
pip install -r requirements.txt
```

## Usage

To run the project, execute:

```sh
python project.py
```

## Contributing

Before committing, run the following checks:

### Code Formatting

To format the code:

```sh
black .
isort .
```

### Code Linting

```sh
black --check .
isort --check .
ruff check
```

### Testing

Run tests using:

```sh
pytest
```
