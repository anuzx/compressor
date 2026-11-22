## Compressor


#### huffman coding :
It is a lossless data compression algorithm, it works by assigning variable-length codes to input characters based on their frequency, ensuring that more frequent characters receive shorter codes and less frequent ones receive longer codes

<br>
the algorithm  constructs prefix tree (huffman tree) using a greedy approach:
<br>
- it creates a leaf node for each character with its frequency
<br>
- it repeatedly combines the two nodes with the lowest frequencies into a new internal node, whose frequency is the sum of its children 
<br>
- this process continues until a single root node remains, creating a tree where no code is a prefix of another, allowing for unambiguous decoding

<br>
<br>
this method minimizes the total number of buts required to represent the data and is widely used in compression formats like ZIP and GZIP
