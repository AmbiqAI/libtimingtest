# mve-kernel-exp

A playground for experimenting with custom MVE kernel operations.

## Target Operations

### Leaky ReLU

The Leaky ReLU is a variant of the ReLU activation function that allows a small, positive gradient when the input is negative. The Leaky ReLU is defined as:

```bash
f(x) = x if x > 0
       alpha * x if x <= 0
```

where `alpha` is a small constant.

## Notes

To view disassembly of the compiled kernel, use the following command:

```bash
arm-none-eabi-objdump -d ./build/apollo5b_evb/arm-none-eabi/src/nlrls.o
```
