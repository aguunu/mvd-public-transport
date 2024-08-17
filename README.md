# Montevideo Public Transportation System Data Processing
This repository contains the data processing implementation used in our research about Montevideo's public transportation system. You can read our research [here](https://www.researchgate.net/publication/383200218).

## About The Project
Public transport system has an important role in the daily routines of Montevideo's citizens. Transport speed is a fundamental indicator not only for evaluating service performance, but also for controlling speed limits and for several arrival prediction models. This project aims to develop a model capable of estimating public transport speed across Montevideo city, using high-performance computing techniques on several geospatial datasets related to the city's public transport system.

## Getting Started

### Clone The Repository
```bash
git clone https://github.com/aguunu/mvd-public-transport.git
```

### Building The Project
```bash
make all
```

### Configure `config.toml`
```toml
[files]
[files.input]
# paths to files containing the input data to be processed
data = [
        "input/chunk-0.csv",
        "input/chunk-1.csv",
        "input/chunk-2.csv",
    ]

# path to file containing sections information
sections = "input/sections.txt"

# path to file containing critical points information
points = "input/critical-points.csv"

# path to file variants information.
variants = "input/variant-sections.txt"

[files.output]
# path where the results related to sections will be saved
sections = "input/results-sections.csv"

# path where the results related to points will be saved
points = "input/results-points.csv"

[analysis]
# start date and time for the analysis period
from = 2024-05-01T00:00:00

# end date and time for the analysis period
to = 2024-05-31T23:00:00

# time interval (in seconds) for analysis
interval = 3600

# if save processed results or not
save_results = true
```

## Example Usage
Here’s an example of how to use the binary with multiple options:

> [!WARNING]  
> Make sure all files in `config.toml` exist and contain valid data.

```bash
./bin/binary --threads 16 config.toml
```

Here you can find all available command-line options for the binary and how to use them.
```
Options:

        -v | --version
                Prints mvd-public-transport version.

        -h | --help
                Prints a help message.

        --no-colors
                Does not use colors for printing.

        --threads
                Number of threads to use by the program.

```

## BibTeX

```markdown
@unknown{unknown,
    author = {Núñez, Agustín and Abbona, Paula},
    year = {2024},
    month = {08},
    pages = {},
    title = {Estimación de la Velocidad del Transporte Público en la Ciudad de Montevideo}
}
```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Authors
- Agustín Núñez - <agustin.nunez@fing.edu.uy>
- Paula Abbona - <paula.abbona@fing.edu.uy>

