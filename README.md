# LZ77 Compression Algorithm

The most widely used general purpose compressors (gzip, zstd) are based on LZ77. It encodes data by referencing earlier occurrences of the same data within a sliding window.

## Key Features

- **Universal Compression**: Theoretically, LZ77 achieves the entropy rate of any stationary process, making it a versatile tool for diverse data types.
- **Efficient Match Finding**: Utilizes a hash-based indexing approach for quick identification of repeated sequences.
- **Adaptability**: Effective for text, binary files, and images.

## Usage

- **Building the Project**:

  ```bash
  make lz77
  ```

- **Compression**:

  ```bash
  ./lz77 -encode [input file] [output file]
  ```

- **Decompression**:
  ```bash
  ./lz77 -decode [input file] [output file]
  ```

## Implementation Details

### Data Structures

- `LZ77Context`: Manages the state, including buffers, hash table, and counters.

### Core Functions

- `UpdateHash`: Updates index references in the hash table.
- `FindMatch`: Identifies the longest match in the input data.
- `WriteLiteral`: Handles literal (uncompressed) data blocks.
- `WriteCompressedBlock`: Manages the encoding of compressed data.
- `LZ77Encode`: Core compression function.
- `LZ77Decode`: Core decompression function.

## Current Lmitations & Future Improvements

1. **Memory Efficiency**: The entire output buffer is held in memory during decompression, which could be optimized for large files.
2. **Compression Optimality**: The current greedy parsing strategy may not always yield the best compression ratio. Future updates could explore more sophisticated strategies.
3. **Special Case Handling**: Enhancements are needed for better handling of long repeating sequences.

## Benchmarks

Below is a comparison of LZ77 compression against gzip across various file types:

| file                                             | raw size  | lz77 ratio | gzip ratio |
| ------------------------------------------------ | --------- | ---------- | ---------- |
| eff.html                                         | 41684     | 2.37       | 4.32       |
| the_brothers_karamazov.txt                       | 2004392   | 1.24       | 2.78       |
| sev_financial_statement_data_sets_2023Q3_num.txt | 281614209 | 5.63       | 10.87      |
| bootstrap-3.3.6.min.css                          | 121260    | 3.10       | 6.19       |
| jquery-2.1.4.min.js                              | 84344     | 1.56       | 2.85       |
| leveldb.tar                                      | 3041280   | 1.30       | 1.43       |
| random.bin (random bytes)                        | 1000000   | 0.995      | 0.999      |
| repeat_As.txt                                    | 1000000   | 84.98      | 982.32     |
| cat.png                                          | 1056320   | 1.00       | 1.00       |

Data for some files were sourced from the [Squash Compression Corpus](https://github.com/nemequ/squash-corpus).

All sizes are in bytes.

## Resources

For further reading and understanding of the underlying principles of data compression, refer to [Stanford EE274: Data Compression Course Notes](https://stanforddatacompressionclass.github.io/notes/contents.html).
