# SC3020 Project 1

B+Tree database storing NBA Games data:

- In Project 1, we are tasked with ingesting NBA games data from a Tab-Seperated Values (TSV)
  onto disk, and subquently performing a range query on stored values with a query criteria:

  > "FG_PCT_home" falls between 0.6 and 0.9 (inclusive).

- The range query is to performed over a B+Tree index built during storage. We are
  to demonstrate the performance benefits of index lookup vs a brute force
  full table scan by comparing runtimes.

## Setup

### Requirements

- Ensure you have `CMake` installed on your system.
- A compatible compiler (e.g., `gcc` or `clang`) is required.
- `make` should be available in your build environment.

### Building

To configure and build the project, run:

```sh
cmake -Bbuild -Ssrc -DCMAKE_BUILD_TYPE=Release && make -C build -j
```

### Testing

To run tests, execute:

```sh
cmake -Bbuild -Ssrc -DCMAKE_BUILD_TYPE=Debug && make -C build -j
./build/sc3020_p1_test
```

## Running

To execute the program:

```sh
./build/sc3020_p1 load /tmp/db games.txt
```

- Loads `games.txt` into the database file at `/tmp/db`.

### Querying

Run one of the following to perform a query:

```sh
./build/sc3020_p1 query /tmp/db scan
```

- Runs a **brute-force scan** query on the database.

```sh
./build/sc3020_p1 query /tmp/db index
```

- Runs an **index-based** query on the database.

```sh
rm /tmp/db
./build/sc3020_p1 load /tmp/db games.txt
./build/sc3020_p1 query /tmp/db scan
./build/sc3020_p1 query /tmp/db index
```

## Running with Docker

A pre-built Docker image is available:

### Loading Data

```sh
docker run --rm -v $(pwd)/data:/data ghcr.io/mrzzy/ntu-sc3020-project1:latest load /data/db games.txt
```

- Saves the database file (`db`) in `$(pwd)/data` on the host.
- Uses `games.txt` **from inside the container**.

### Querying the Database

```sh
docker run --rm -v $(pwd)/data:/data ghcr.io/mrzzy/ntu-sc3020-project1:latest query /data/db scan
```

- Runs a brute-force scan on the database.

```sh
docker run --rm -v $(pwd)/data:/data ghcr.io/mrzzy/ntu-sc3020-project1:latest query /data/db index
```

- Runs an index-based query on the database.
