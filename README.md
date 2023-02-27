# kMTFCompressor
Context aware Move To Front Transform based compressor. Bachelor thesis project.

C++ implementation of a compression algorithm based on the Move To Front (MTF) algorithm.

The thesis presents the theoretical basis and concepts, the implementation details and the experimental analysis comparing our algorithm with state-of-the-art compression software.
The result is a tunable and very fast stream algorithm with some theoretical guarantees. On the other hand the algorithm requires a good amount of memory to work well and cannot match the compression ratio of state-of-the-art compressors.

## High level description (translated excerpt of the thesis)

The generalization of the Move To Front transform uses a predetermined number of symbol that precede the current symbol, to make a better prediction.
The classic MTF transform uses only the current symbol, which uses less memory and time for the execution but with worse results unless the input presents local correlations.
It is usually used after other transforms, for example in Bzip2 it is used after the Burrows-Wheeler Transform which increments its performance.
The aim is avoiding this type of block sorting transforms which are costly and require block elaboration of the input.

The algorithm maintains a list of the contexts seen up to this moment in the data stream, implemented as an hash table. For each of the contexts a dedicated MTF list is maintained.
New contexts might overwrite others, depending on the hash function, load factor and the collision resolution technique applied.
The transform is executed for each symbol in its specific context list, returning its index and moving it to the correct position so in front of all the other symbols.
If the symbol isn't in the list, a specific code is returned and it is inserted. The code is the maximum size of the list summed to the symbol (interpreted as an integer number).
This is possible because the maximum length of the list is limited to reduce the memory usage. This works well because the longer the contexts, the less the amount of distinct symbols usually come after them (at least in structured data).
The decoder is able to distinguish between list index and new symbol with a simple check with the value and the maximum size of the list.

![alt text](images/MTF.png?raw=true)

The picture shows the distribution of the symbols in input vs the symbols in output on a sample text file.
Most of the output values are indices of the MTF lists, values between 0 and 7 as the lists have length 8 in this example.
The other values are the first sends for every symbol of every context.
The resulting data stream is easier to compress because few symbols appear most of the time.

## Additional compression steps
Other steps have been implemented after the Context aware Move To Front Transform:
  * Run-Length Encoding
  * Adaptive Huffman
  * Adaptive Arithmetic
