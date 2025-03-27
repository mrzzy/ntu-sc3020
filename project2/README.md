# SC3020 Project 2

## Project Structure
This project consists of four main Python files:

- **interface.py**: Contains the code for the GUI.
- **pipesyntax.py**: Responsible for generating the pipe-syntax version of a query.
- **preprocessing.py**: Handles input reading and any preprocessing necessary for the algorithm.
- **project.py**: The main entry point that orchestrates the necessary functions from the other three files.

## Installation

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
