# Mcc

Mcc is a self-hosting compiler that compiles a subset of the C language into the x86-64 assembly.

# Build and Test

|make rule|use|
|:---|:---|
|`make mcc`|Build stage 1 mcc using an existing compiler like gcc or clang.|
|`make test`|Test stage 1 mcc.|
|`make mcc2`|Build stage 2 mcc using stage 1 mcc.|
|`make test2`|Test stage 2 mcc.|

# References

- https://www.sigbus.info/compilerbook (Japanese)
- https://github.com/rui314/chibicc
- https://godbolt.org/
- https://port70.net/~nsz/c/c11/n1570.pdf
